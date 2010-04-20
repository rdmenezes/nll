#ifndef MVV_PARSER_COMPILER_H_
# define MVV_PARSER_COMPILER_H_

# include "mvvScript.h"
# include "symbol-table.h"
# include "parser-context.h"
# include "visitor-print.h"
# include "visitor-default.h"
# include "visitor-bind.h"
# include "visitor-register-declarations.h"
# include "visitor-type.h"
# include "visitor-evaluate.h"
# include "function-runnable.h"
# include "import.h"

namespace mvv
{
namespace parser
{
   /**
    @brief Front end for the compiler & interpreter
    */
   class MVVSCRIPT_API CompilerFrontEnd
   {
   public:
      typedef std::set< platform::RefcountedTyped<Ast> >                         Trees;
      typedef std::map<const AstDeclFun*, platform::RefcountedTyped<FunctionRunnable> >   ImportedFunctions;
      typedef std::set<mvv::Symbol> Files;
      typedef std::vector<mvv::Symbol> FilesOrder;

   public:
      /**
       @brief construct the frontend
       @param parseTrace if true, the parsing trace will be displayed on std::cout
       @param scanTrace if true, the tokens will be displayed on std::cout
       */
      CompilerFrontEnd( bool parseTrace = false, bool scanTrace = false ) : _context( parseTrace, scanTrace )
      {
      }

      ~CompilerFrontEnd()
      {
         clear();
      }

      /**
       @brief Clear the front end as if it just has been created
       */
      void clear()
      {
         _context.clear();
         _vars.clear();
         _funcs.clear();
         _classes.clear();
         _executionTrees.clear();

         for ( ui32 n = 0; n < _handleLibs.size(); ++n )
         {
            FreeLibraryWrapper( _handleLibs[ n ] );
         }
         _handleLibs.clear();
         _imported.clear();
         _lastErrors.clear();
         _parsedFiles.clear();
      }

      /**
       @brief Returns the directories that will be searched to find an include/import code file

       This 
       */
      FilesOrder& getImportDirectories()
      {
         return _importDirectories;
      }



      /**
       @brief run the string.
       @throw RuntimeException when the compiler fails execute incorrectly (i.e. out of bounds, dangling reference...)
       @note variables, functions & classes declared in this string are saved in the current context and available
             for later usage.
       */
      Error::ErrorType run( const std::string& s )
      {
         _context.clear(); // clear the previous errors

         // local copy, so that if there is an error, we don't mess up the correct AST...
         SymbolTableVars     vars = _vars;
         SymbolTableFuncs    funcs = _funcs;
         SymbolTableClasses  classes = _classes;

         std::list<Ast*> exps;
         Ast* exp = _context.parseString( s );

         Files importedLib;      // lib to be imported (just before runtime, we need to link function body...)

         if ( exp )
         {
            // explore all files: current + includes + import
            exps.push_back( exp );
            _explore( _context, vars, funcs, classes, exps, exp, importedLib );
            if ( !_context.getError().getStatus() )
            {
               // run the binding visitor on all the trees
               for ( std::list<Ast*>::iterator it = exps.begin(); it != exps.end(); ++it )
               {
                  VisitorBind visitorBind( _context, vars, funcs, classes );
                  visitorBind( **it );
               }

               if ( !_context.getError().getStatus() )
               {
                  // run the typing visitor on all the trees
                  for ( std::list<Ast*>::iterator it = exps.begin(); it != exps.end(); ++it )
                  {
                     VisitorType visitorType( _context, vars, funcs, classes );
                     visitorType( **it );
                  }

                  // update variables and stuff..
                  if ( _context.getError().getStatus() == Error::SUCCESS )
                  {
                     // propagate if no error...
                     _vars = vars;
                     _funcs = funcs;
                     _classes = classes;

                     // save the tree for further execution
                     for ( std::list<Ast*>::iterator it = exps.begin(); it != exps.end(); ++it )
                     {
                        _executionTrees.insert( platform::RefcountedTyped<Ast>( *it ) );
                     }

                     // before runtime, load the new DLL
                     for ( Files::iterator it = importedLib.begin(); it != importedLib.end(); ++it )
                     {
                        importDll( it->getName() );
                     }

                     // evaluate only the initial file
                     VisitorEvaluate visitorEvaluate( _context, vars, funcs, classes );
                     visitorEvaluate( *exp );
                  } else {

                     // free the memory, we don;t want to save the results
                     for ( std::list<Ast*>::iterator it = exps.begin(); it != exps.end(); ++it )
                     {
                        delete *it;
                     }
                  }
               }
            }
         }

         _lastErrors = _context.getError().getMessage().str();
         return _context.getError().getStatus();
      }

      /**
       @brief Returns the latest error message if any, associated to the latest run
       */
      const std::string& getLastErrorMesage() const
      {
         return _lastErrors;
      }

      /**
       @brief find a global variable in the execution context
       @throw std::exception if the variable can't be found
       @return it's runtime value & type
       */
      const RuntimeValue& getVariable( const mvv::Symbol& name ) const
      {
         const AstDeclVar* val = _vars.find( name );
         if ( !val )
         {
            throw std::exception("can't find this variable in the current execution context" );
         }
         return val->getRuntimeValue();
      }

      /**
       @brief find a class definition using it's full path (i.e. Class1::Class2::ClassWeWant => create the vector (Class1, Class2, ClassWeWant))
       @brief return 0 if class not found
       */
      const Type* getClass( const std::vector<mvv::Symbol>& path ) const
      {
         const AstDeclClass* c = _classes.find( path );
         if ( !c )
         {
            return 0;
         }

         return c->getNodeType();
      }

      /**
       @brief find a function/member function given a path and a prototype
       @param path path to a class + function for a member function or function name for global function
       @param prototype the prototype to mach (prototype = all argument types)
       @return 0 if function not found
       */
      const AstDeclFun* getFunction( const std::vector<mvv::Symbol>& path, const std::vector< const Type* >& prototype )
      {
         std::vector<AstDeclFun*> possible;

         if ( path.size() == 0 )
            return 0;
         if ( path.size() == 1 )
         {
            possible = VisitorType::getFunctionsFromGlobal( _funcs, path[ 0 ] );
         } else {
            std::vector<mvv::Symbol> pathToClass( path );
            pathToClass.pop_back();

            AstDeclClass* c = _classes.find( pathToClass );
            if ( !c )
            {
               return 0;
            }

            possible = VisitorType::getFunctionsFromClass( *c, path[ path.size() - 1 ] );
         }

         return getMatchingFunction( possible, prototype );
      }

      /**
       @brief Registers a function to be called at runtime when the runtime detect function.getFunctionPointer()
              is about to be run

       @note it is expected form an import DLL to use this function to populate the imported
             function. As the dynamic memory is created by the import DLL and released from this DLL,
             these 2 DLLs must share exactly the same CRT version!
       */
      void registerFunctionImport( platform::RefcountedTyped<FunctionRunnable> function )
      {
         _imported[ (*function).getFunctionPointer() ] = function;

         // we const_cast as the user should not be able to modify the function pointer himself...
         // however we need to update the function declaration to run the imported function...
         AstDeclFun* f = const_cast<AstDeclFun*>( (*function).getFunctionPointer() );
         f->setImportedFunction( function );
      }

      /**
       @brief This function is parsing the declaration file "name.ludo", then the "name.dll" will be dynamically
              loaded and invoke the function <code>void importFunctions( CompilerFrontEnd )</code>
       */
      void importDll( const std::string& name );

   private:
      /**
       @brief explore (parse, visit declaration) the the import & include files recursively & original file
       */
      void _explore( ParserContext& context,
                     SymbolTableVars& vars,
                     SymbolTableFuncs& funcs,
                     SymbolTableClasses& classes,
                     std::list<Ast*>& store,
                     Ast* toExplore,
                     Files& importedLib )
      {
         VisitorRegisterDeclarations visitor( context, vars, funcs, classes );
         visitor( *toExplore );

         if ( !_context.getError().getStatus() )
         {
            const VisitorRegisterDeclarations::Symbols& includes = visitor.getFilesToInclude();
            const VisitorRegisterDeclarations::Symbols& imports =  visitor.getFilesToImport();

            // parse the import/include
            for ( VisitorRegisterDeclarations::Symbols::const_iterator it = includes.begin(); it != includes.end(); ++it )
            {
               // check it has never been pared before
               if ( _parsedFiles.find( *it ) == _parsedFiles.end() )
               {
                  Ast* exp = _context.parseFile( it->getName() + std::string( ".ludo" ) );
                  if ( exp )
                  {
                     // recursively check the dependencies
                     store.push_front( exp );   // we push front as we need the include to be parsed before... (it would work else, but les efficient)
                     _explore( context, vars, funcs, classes, store, exp, importedLib );
                  }
                  _parsedFiles.insert( *it );
               }
            }

            for ( VisitorRegisterDeclarations::Symbols::const_iterator it = imports.begin(); it != imports.end(); ++it )
            {
               // check it has never been pared before
               if ( _parsedFiles.find( *it ) == _parsedFiles.end() )
               {
                  Ast* exp = _context.parseFile( it->getName() + std::string( ".ludo" ) );
                  if ( exp )
                  {
                     // recursively check the dependencies
                     store.push_front( exp );   // we push front as we need the include to be parsed before... (it would work else, but les efficient)
                     _explore( context, vars, funcs, classes, store, exp, importedLib );
                     importedLib.insert( *it );   // after type visitor, we must link the imported functions
                  }
                  _parsedFiles.insert( *it );
               }
            }
         }
      }

      /**
       @brief Find the possible match from a set of functions and argument types
       */
      AstDeclFun* getMatchingFunction( const std::vector<AstDeclFun*>& fn, const std::vector< const Type* >& prototype )
      {
         std::vector<AstDeclFun*> possibleFunctions;
         for ( ui32 n = 0; n < fn.size(); ++n )
         {
            if ( fn[ n ]->getVars().getVars().size() != prototype.size() )
            {
               // different number of arguments
               continue;
            }

            ui32 nn = 0;
            bool match = true;
            for ( AstDeclVars::Decls::const_iterator it = fn[ n ]->getVars().getVars().begin(); it != fn[ n ]->getVars().getVars().end(); ++it, ++nn )
            {
               if ( !(*it)->getNodeType()->isEqual( *prototype[ nn ] ) )
               {
                  match = false;
                  break;
               }
            }

            if ( !match )
               continue;

            possibleFunctions.push_back( fn[ n ] );
         }
         if ( possibleFunctions.size() )
         {
            ensure( possibleFunctions.size() <= 1, "compiler error: a prototype must be unique" );
            return possibleFunctions[ 0 ];
         } else {
            return 0;
         }
      }


   private:
      ParserContext       _context; // parsing context
      SymbolTableVars     _vars;    // current list of variable definition
      SymbolTableFuncs    _funcs;   // current list of functions definition
      SymbolTableClasses  _classes; // current list of class definition

      Trees               _executionTrees;   // the trees that have been parsed
      ImportedFunctions   _imported;         // the functions that have been imported
      std::string         _lastErrors;       // save the last errors

      Files               _parsedFiles;      // the set of files that have been already parsed (so an import/include with this file won't do anything...)
      FilesOrder          _importDirectories;// directories where the import/include files will be searched, form first to last order
      FilesOrder          _runtimePath;      // directories where the DLL will be looked at while a import is done
      std::vector<void*>  _handleLibs;       // save the handles on the DLL manually loaded
   };
}
}

#endif