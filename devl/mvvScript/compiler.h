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
      typedef std::vector< platform::RefcountedTyped<Ast> >                         Trees;
      typedef std::map<const AstDeclFun*, platform::RefcountedTyped<FunctionRunnable> >   ImportedFunctions;

   public:
      /**
       @brief construct the frontend
       @param parseTrace if true, the parsing trace will be displayed on std::cout
       @param scanTrace if true, the tokens will be displayed on std::cout
       */
      CompilerFrontEnd( bool parseTrace = false, bool scanTrace = false ) : _context( parseTrace, scanTrace )
      {
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
      }

      /**
       @brief run the string.
       @throw RuntimeException when the compiler fails execute incorrectly (i.e. out of bounds, dangling reference...)
       @note variables, functions & classes declared in this string are saved in the current context and available
             for later usage.
       */
      Error::ErrorType run( const std::string& s )
      {
         Ast* exp = _context.parseString( s );
         _executionTrees.push_back( platform::RefcountedTyped<Ast>( exp ) );

         if ( exp )
         {
            VisitorRegisterDeclarations visitor( _context, _vars, _funcs, _classes );
            visitor( *exp );
            if ( !_context.getError().getStatus() )
            {
               VisitorBind visitorBind( _context, _vars, _funcs, _classes );
               visitorBind( *exp );
               if ( !_context.getError().getStatus() )
               {
                  VisitorType visitorType( _context, _vars, _funcs, _classes );
                  visitorType( *exp );
                  if ( !_context.getError().getStatus() )
                  {
                     VisitorEvaluate visitorEvaluate( _context, _vars, _funcs, _classes );
                     visitorEvaluate( *exp );
                  }
               }
            }
         }

         return _context.getError().getStatus();
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
      ImportedFunctions   _imported;         // the function htat have been imported
   };
}
}

#endif