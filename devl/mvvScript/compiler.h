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
# include <mvvPlatform/context.h>

namespace mvv
{
namespace parser
{
   /**
    @brief Front end for the compiler & interpreter
    @note the front end can throw exception if it is not correctly used: if we store
          a reference on a runtime value, and if this trees are destroyed->we can't
          deallocate correctly the constructor as the tree and runtime environment has
          been destroyed
    */
   class MVVSCRIPT_API CompilerFrontEnd
   {
      /**
       @brief Stores info on the tree built
       */
      struct TreeInfo
      {
         TreeInfo( platform::RefcountedTyped<Ast> t, bool isucode ) : tree( t ), isUserCode( isucode )
         {
            static ui32 val = 0;
            ++val;

            _id = val;
         }

         bool operator<( const TreeInfo& t ) const
         {
            return _id < t._id;
         }

         platform::RefcountedTyped<Ast>  tree;  // store the AST
         bool  isUserCode;                      // true if the code is the one defined in interactive mode (the file or command to parse, but no includes or imports)

      private:
         ui32     _id;  // we must keep the user code ordered from first to last in case we want to export it
      };

   public:
      typedef std::set< TreeInfo > Trees;
      typedef std::map<const AstDeclFun*, platform::RefcountedTyped<FunctionRunnable> >   ImportedFunctions;
      typedef std::set<mvv::Symbol> Files;
      typedef std::vector<mvv::Symbol> FilesOrder;

   public:
      /**
       @brief construct the frontend
       @param parseTrace if true, the parsing trace will be displayed on std::cout
       @param scanTrace if true, the tokens will be displayed on std::cout
       @param stackSize the number of elements the stack can contain at maximum
       */
      CompilerFrontEnd( bool parseTrace = false, bool scanTrace = false, ui32 stackSize = 10000 ) : _context( parseTrace, scanTrace )
      {
         // if we are bigger than that, expect very wrong result due to vector resizing which invalidate references...
         _env.stack.reserve( stackSize );

         _eval = platform::RefcountedTyped<VisitorEvaluate>( new VisitorEvaluate( _context, _vars, _funcs, _classes, _env ) );

         // empty context
         _contextExt = platform::RefcountedTyped<platform::Context>( new platform::Context() );
      }

      ~CompilerFrontEnd()
      {
         clear();
      }

      /**
       @brief Context mecanism needed to safely transmit context info for import libraries
              i.e. in the case of mvv, we can send info on the volume context & segments...
       */
      void setContextExtension( platform::RefcountedTyped<platform::Context> c )
      {
         _contextExt = c;
      }

      /**
       @brief Clear the front end as if it just has been created
       */
      void clear()
      {
         _env.clear();
         _context.clear();
         _vars.clear();
         _funcs.clear();
         _classes.clear();
         _executionTrees.clear();
         _typedefs.clear();
         
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
       */
      const FilesOrder& getImportDirectories() const
      {
         return _importDirectories;
      }

      VisitorEvaluate& getEvaluator()
      {
         return *_eval;
      }

      /**
       @brief Returns the path, from first directory to last, where the DLL will be searched for
       */
      const FilesOrder& getRuntimePath() const
      {
         return _runtimePath;
      }

      /**
       @brief Export of the user code in text format
       @note it is assumed that the first file to parse or all the interactive commands are user code
             (i.e. includes, imports are not and won't be exported)
       */
      void exportCode( std::ostream& o ) const
      {
         for ( Trees::const_iterator it = _executionTrees.begin(); it != _executionTrees.end(); ++it )
         {
            if ( it->isUserCode )
            {
               VisitorPrint visitor( o );
               visitor( *it->tree );
            }
            o << iendl;
         }
      }

      /**
       @brief Export the user code to the specified path
       */
      void exportCode( const std::string& name )
      {
         std::ofstream file( name.c_str() );
         if ( !file.good() )
            return;
         exportCode( file );
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
         _env.framePointer = static_cast<ui32>( _env.stack.size() ); // we need to set the FP to the end, so the need tree will be correct!

         // local copy, so that if there is an error, we don't mess up the correct AST...
         SymbolTableVars     vars = _vars;
         SymbolTableFuncs    funcs = _funcs;
         SymbolTableClasses  classes = _classes;
         SymbolTableTypedef  typedefs = _typedefs;

         std::list<Ast*> exps;
         Ast* exp = _context.parseString( s );

         Files importedLib;      // lib to be imported (just before runtime, we need to link function body...)

         if ( exp )
         {
            // explore all files: current + includes + import
            exps.push_back( exp );
            _explore( _context, vars, funcs, classes, typedefs, exps, exp, importedLib );
            if ( !_context.getError().getStatus() )
            {
               // run the binding visitor on all the trees
               for ( std::list<Ast*>::iterator it = exps.begin(); it != exps.end(); ++it )
               {
                  VisitorBind visitorBind( _context, vars, funcs, classes, typedefs );
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
                     _typedefs = typedefs;

                     // save the tree for further execution
                     for ( std::list<Ast*>::iterator it = exps.begin(); it != exps.end(); ++it )
                     {
                        _executionTrees.insert( TreeInfo( platform::RefcountedTyped<Ast>( *it ), false ) );
                     }
                     _executionTrees.rbegin()->isUserCode = true; // the last exp parse is always user code
                     

                     // before runtime, load the new DLL
                     for ( Files::iterator it = importedLib.begin(); it != importedLib.end(); ++it )
                     {
                        importDll( it->getName() );
                     }

                     // evaluate ALL the files
                     VisitorEvaluate visitorEvaluate( _context, vars, funcs, classes, _env, _eval.getDataPtr() ); // only one visitor so the FP is correct
                     if ( exps.size() > 1 )
                     {
                        // we must evaluate in the same order than explore, so that the static link match the reality!

                        for ( std::list<Ast*>::reverse_iterator it = ++exps.rbegin(); it != exps.rend(); ++it )
                        {
                           visitorEvaluate( **it );
                        }
                     }

                     // finally update our source file
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

         _env.resultRegister = RuntimeValue();

          //_env.resultRegister.vals.unref();   // unref the result in case it is holding a ref on a var that need to call a destructor...
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
       @note if a RuntimeValue& is used still, while the front end is destroyed, this will crash (destructor)
       */
      const RuntimeValue& getVariable( const mvv::Symbol& name )
      {
         const AstDeclVar* val = _vars.find( name );
         if ( !val )
         {
            throw std::exception("can't find this variable in the current execution context" );
         }
         if ( val->getRuntimeIndex() >= _env.stack.size() )
         {
            throw std::exception("incorrect frame pointer" );
         }

         RuntimeValue& res = _env.stack[ val->getRuntimeIndex() ];
         while ( res.type == RuntimeValue::REF )
         {
            res = *res.ref;
         }
         return res;
      }

      /**
       @brief find a class definition using it's full path (i.e. Class1::Class2::ClassWeWant => create the vector (Class1, Class2, ClassWeWant))
       @brief return 0 if class not found
       */
      const Type* getType( const std::vector<mvv::Symbol>& path ) const
      {
         const AstDeclClass* c = _classes.find( path, _typedefs );
         if ( !c )
         {
            return 0;
         }

         return c->getNodeType();
      }

      /**
       @brief find a class definition using it's full path (i.e. Class1::Class2::ClassWeWant => create the vector (Class1, Class2, ClassWeWant))
       @brief return 0 if class not found
       */
      const AstDeclClass* getClass( const std::vector<mvv::Symbol>& path ) const
      {
         const AstDeclClass* c = _classes.find( path, _typedefs );
         if ( !c )
         {
            return 0;
         }

         return c;
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

            AstDeclClass* c = _classes.find( pathToClass, _typedefs );
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
                     SymbolTableTypedef& typedefs,
                     std::list<Ast*>& store,
                     Ast* toExplore,
                     Files& importedLib )
      {
         VisitorRegisterDeclarations visitor( context, vars, funcs, classes, typedefs, _env.framePointer );
         visitor( *toExplore );

         if ( !_context.getError().getStatus() )
         {
            const VisitorRegisterDeclarations::Symbols& includes = visitor.getFilesToInclude();
            const VisitorRegisterDeclarations::Symbols& imports =  visitor.getFilesToImport();

            // in case import & include, => we import it!
            for ( VisitorRegisterDeclarations::Symbols::const_iterator it = imports.begin(); it != imports.end(); ++it )
            {
               // check it has never been pared before
               if ( _parsedFiles.find( *it ) == _parsedFiles.end() )
               {
                  Ast* exp = _context.parseFile( it->getName() + std::string( ".ludo" ) );
                  _parsedFiles.insert( *it );
                  if ( exp )
                  {
                     // recursively check the dependencies
                     store.push_front( exp );   // we push front as we need the include to be parsed before... (it would work else, but les efficient)
                     _explore( context, vars, funcs, classes, typedefs, store, exp, importedLib );
                     importedLib.insert( *it );   // after type visitor, we must link the imported functions
                  }
               }
            }

            // parse the import/include
            for ( VisitorRegisterDeclarations::Symbols::const_iterator it = includes.begin(); it != includes.end(); ++it )
            {
               // check it has never been pared before
               if ( _parsedFiles.find( *it ) == _parsedFiles.end() )
               {
                  Ast* exp = _context.parseFile( it->getName() + std::string( ".ludo" ) );
                  _parsedFiles.insert( *it );
                  if ( exp )
                  {
                     // recursively check the dependencies
                     store.push_front( exp );   // we push front as we need the include to be parsed before... (it would work else, but les efficient)
                     _explore( context, vars, funcs, classes, typedefs, store, exp, importedLib );
                  }
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
      SymbolTableTypedef  _typedefs;// store the typedefs list

      Trees               _executionTrees;   // the trees that have been parsed
      ImportedFunctions   _imported;         // the functions that have been imported
      std::string         _lastErrors;       // save the last errors

      Files               _parsedFiles;      // the set of files that have been already parsed (so an import/include with this file won't do anything...)
      FilesOrder          _importDirectories;// directories where the import/include files will be searched, from first to last order
      FilesOrder          _runtimePath;      // directories where the DLL will be looked at while a import is done
      std::vector<void*>  _handleLibs;       // save the handles on the DLL manually loaded
      RuntimeEnvironment  _env;              // the current environment
      platform::RefcountedTyped<VisitorEvaluate>   _eval;   // the evaluate visitor. We must keep him alive as we are expecting delayed evaluation due to the destructor of some objects
      platform::RefcountedTyped<platform::Context> _contextExt;// context necessary for external lib...
   };
}
}

#endif