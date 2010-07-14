#ifndef MVV_PARSE_BIND_VISITOR_H_
# define MVV_PARSE_BIND_VISITOR_H_

# include "visitor-default.h"
# include "parser-context.h"
# include "symbol-table.h"
# include "utils.h"

namespace mvv
{
namespace parser
{
   namespace impl
   {
      inline void reportUndeclaredType( const YYLTYPE& current, mvv::parser::ParserContext& context, const std::string& msg )
      {
         std::stringstream ss;
         ss << current << msg << std::endl;
         context.getError() << ss.str() << mvv::parser::Error::BIND;
      }

      /**
       Order the definition first by variable, then anything else...
       */
      struct ClassDefinitionSorter
      {
         bool operator()( const AstDecl* a, const AstDecl* b ) const
         {
            if ( dynamic_cast<const AstDeclVar*>( a ) && !dynamic_cast<const AstDeclVar*>( b ) )
            {
               return false;
            }
            return true;
         }
      };
   }

   /**
    @brief It will link all variable/function/class declaration to where they are used.
           It will also report all binding errors.

           It is using the previously computed class/function/var in global scope

           However, The bind visitor will need to register the simple variable & function decl in scopes level > 1 (not global),
           as well as arguments in function declarations
           Additionally, untyped variable declaration ( i.e. n = 3.0; ), the bind visitor must register them if they are not declared, but issue a
           warning if it has been declared in another file

           Note: unnamed varaible can't be created in expresions, if, ... Just by var = exp

           TODO: check for function/class if they are in LUT, else they have been created in a scope wich is wrong...
    */
   class VisitorBind : public VisitorDefault
   {
   public:
      typedef std::set<mvv::Symbol> Symbols;
      using VisitorDefault::operator();

   public:
      VisitorBind( ParserContext& context,
                   SymbolTableVars& vars,
                   SymbolTableFuncs& funcs,
                   SymbolTableClasses& classes,
                   SymbolTableTypedef& typedefs ) : _context( context ), _vars( vars ), _funcs( funcs ), _classes( classes ), _typedefs( typedefs )
      {
         _scopeDepth = 0;
         _functionCallsNeeded = 0;
         _isInFunction = 0;
         _isInFunctionDeclaration = false;

         _typedefs.resetScope();
      }

      // TODO: handle typedef within field
      // path: the current location at the time we request this specific class
      // field: the field we fully need to match
      // name: the name of the class
      AstDecl* findClassDecl( const std::vector<mvv::Symbol>& path, const std::vector<mvv::Symbol>& field, const mvv::Symbol& name )
      {
         // A::D  B C
         // first: - check the local path A::D, look for C from here
         //        - check if declared within source class: check from D if B::C, else check from A if B::C
         //        - else check in global scope B::C
         //

         if ( field.size() )
         {
            std::vector<mvv::Symbol> up = field;
            up.push_back( name );
            AstDecl* current = _classes.find_within_scope( path, up, _typedefs );
            return current;
            /*
            // local
            // concatenate the field to the path
            std::vector<mvv::Symbol> up( path );
            for ( size_t n = 0; n < field.size(); ++n )
            {
               up.push_back( field[ n ] );
            }
            AstDecl* current = _classes.find_in_class( up, name, _typedefs );
            if ( current )
               return current;

            // check within class
            std::vector<mvv::Symbol> up2( field );
            up2.push_back( name );
            AstDecl* within = _classes.find_within_scope( path, up2, _typedefs );
            if ( within )
               return within;

            // finally check global scope
            AstDecl* global = _classes.find( up2, _typedefs );
            if ( global )
               return global;
            return 0;
            */
         } else {
            // current class
            AstDecl* current = _classes.find_in_class( _defaultClassPath, name, _typedefs );
            if ( current )
               return current;
            AstDecl* within = _classes.find_within_scope( _defaultClassPath, nll::core::make_vector<mvv::Symbol>( name ), _typedefs );
            if ( within )
               return within;
            AstDecl* c = _classes.find( nll::core::make_vector<mvv::Symbol>( name ), _typedefs );
            if ( c )
               return c;
            AstTypedef* t = _typedefs.find( name );
            if ( t )
            {
               return t;
            }
            return 0;
         }
      }

      virtual void operator()( AstBreak& e )
      {
         if ( _whiles.empty() )
         {
            impl::reportUndeclaredType( e.getLocation(), _context, "a break statement must be in the scope of a while or for loop" );
         } else {
            e.setLoop( _whiles.top() );
         }
      }

      virtual void operator()( AstWhile& e )
      {
         operator()( e.getCondition() );

         _whiles.push( &e );
         operator()( e.getStatements() );
         _whiles.pop();
      }

      virtual void operator()( AstStatements& e )
      {
         ++_scopeDepth;
         if ( _scopeDepth > 1 )
         {
            // we simulate the "global scope"
            _vars.beginScope();
         }

         for ( AstStatements::Statements::const_iterator it = e.getStatements().begin();
               it != e.getStatements().end();
               ++it )
         {
            operator()( **it );
         }

         if ( _scopeDepth > 1 )
         {
            _vars.endScope();
         }
         --_scopeDepth;
      }

      virtual void operator()( AstExpAssign& e )
      {
         // first visit the declaration, in case rvalue reference itself
         operator()( e.getLValue() );
         operator()( e.getValue() );
      }

      virtual void operator()( AstVarSimple& e )
      {
         // a variable can be simple var, or class constructor or global function
         // if none of these then we have a problem...
         AstDeclVar* var = _vars.find( e.getName() );
         if ( !var )
         {
            if ( _functionCallsNeeded )
            {
               // there is a callExp so check class/function
               SymbolTableFuncs::iterator it = _funcs.find( e.getName() );
               AstDecl* decl = findClassDecl( _defaultClassPath, _currentFieldList, e.getName() );
               if ( !decl && it == _funcs.end() )
               {
                  impl::reportUndeclaredType( e.getLocation(), _context, "undeclared function/class constructor call" );
               }
               //e.setReference( decl );
            } else {
               SymbolTableFuncs::iterator it = _funcs.find( e.getName() );
               if ( it == _funcs.end() )
               {
                  // in case we have: typedef string() fp_test; string test1(){return \"123\";} fp_test = test1;
                  // address of a function
                  impl::reportUndeclaredType( e.getLocation(), _context, "undeclared variable" );
               } else {
                  e.setFunctionAddress( true );
               }
            }
         } else {
            e.setReference( var );
         }
/*
         if ( !var )
         {
            // there is still the case of the function call: "func( 0 )" -> 
            // in case it is actually a global function
            if ( _functionCallsNeeded )
            {
               SymbolTableFuncs::iterator it = _funcs.find( e.getName() ); 
               if ( it == _funcs.end() )
               {
                  AstDeclClass* decl = findClassDecl( _defaultClassPath, _currentFieldList, e.getName() );
                  if ( decl )
                  {
                     e.setClassConstructorCall( decl );
                  } else {
                     impl::reportUndeclaredType( e.getLocation(), _context, "undeclared function/class constructor call" );
                  }
               } else {
                  std::cout << "function declared:" << e.getName() << std::endl;
                  e.setFunctionCall( true );
                  --_functionCallsNeeded;
               }
            } else {
               impl::reportUndeclaredType( e.getLocation(), _context, "undeclared variable" );
            }
         } else {
            e.setReference( var );
         }*/
      }

      virtual void operator()( AstVarField& e )
      {
         // do nothing: we need type value to check if the fields are correctly used
         operator()( e.getField() );

         // special case for "this"
         if ( e.getName() == mvv::Symbol::create( "this" ) )
         {
            if ( _defaultClassPath.size() == 0 )
            {
               impl::reportUndeclaredType( e.getLocation(), _context, "this can only be declared in a class scope" );
               return;
            }
            e.setReference( _classes.find( _defaultClassPath, _typedefs ) );
         }
      }

      virtual void operator()( AstType& e )
      {
         if ( e.getType() == AstType::SYMBOL )
         {
            AstDecl* decl = findClassDecl( _defaultClassPath, _currentFieldList, *e.getSymbol() );
            if ( !decl )
            {
               AstTypedef* f = _typedefs.find( *e.getSymbol() );     // TODO? this is fine: it should be only for typedef on basic types!
               if ( !f )
               {
                  impl::reportUndeclaredType( e.getLocation(), _context, "undeclared type" );
               } else {
                  e.setReference( f );
               }
            } else {
               e.setReference( decl );
            }
         }
      }

      virtual void operator()( AstDeclClass& e )
      {
         _typedefs.begin_scope( e.getName() );

         if ( _defaultClassPath.size() == 0 && _scopeDepth > 1 )
         {
            impl::reportUndeclaredType( e.getLocation(), _context, "a class can only be declared in the global scope/or nested in another class" );
         }

         ++_scopeDepth;
         _vars.beginScope( true );
         _defaultClassPath.push_back( e.getName() );

         // sort member definition:
         /*
         std::sort( e.getDeclarations().getDecls().begin(),
                    e.getDeclarations().getDecls().end() );
                    */
         operator()( e.getDeclarations() );
         _defaultClassPath.pop_back();
         _vars.endScope();
         --_scopeDepth;

         _typedefs.end_scope();
      }

      /**
       @brief Check the parameters with default init in function declaration are properly used
       @return true if correct
       */
      static bool checkDefaultInitizalizationInFunctionDeclaration( AstDeclFun& e )
      {
         bool mustHaveDefaultInit = false;
         for ( AstDeclVars::Decls::iterator it = e.getVars().getVars().begin(); it != e.getVars().getVars().end(); ++it )
         {
            if ( (*it)->getInit() )
            {
               mustHaveDefaultInit = true;
            } else {
               if ( mustHaveDefaultInit )
                  return false;
            }
         }
         return true;
      }

      static mvv::Symbol mangling( const AstDeclFun& e )
      {
         return mvv::Symbol::create( e.getName().getName() + nll::core::val2str( &e ) );
      }

      virtual void operator()( AstDeclFun& e ) 
      {
         ++_isInFunction;

         if ( _defaultClassPath.size() == 0 )
         {
            // if we are in class, it means it is a member function
            SymbolTableFuncs::iterator it = _funcs.find( e.getName() );
            if ( it == _funcs.end() )
            {
               impl::reportUndeclaredType( e.getLocation(), _context, "a function can only be declared in the global scope" );
            }
         }

         if ( _isInFunction > 1 )
         {
            impl::reportUndeclaredType( e.getLocation(), _context, "a member function can't be created inside another member function" );
         }

         if ( e.getType() )
         {
            operator()( *e.getType() );
         }
         
         if ( e.getBody() )
         {
            _typedefs.begin_scope( mangling( e ) );
            _currentFunc.push_back( &e );
            ++_scopeDepth;
            if ( _defaultClassPath.size () )
            {
               // it means we are in a  class, so don't create a blocking scope
               _vars.beginScope( false );
            } else {
               _vars.beginScope( true );
            }

            // read the vars in the body
            if ( e.getVars().getVars().size() )
            {
               _isInFunctionDeclaration = true;
               operator()( e.getVars() );
               _isInFunctionDeclaration = false;
            }

            _canReturn.push( true );
            operator()( *e.getBody() );
            _canReturn.pop();
            _vars.endScope();

            --_scopeDepth;
            _currentFunc.pop_back();
            _typedefs.end_scope();
         } else {
            // if there is no body, we still need to go through the arguments and bind them
            if ( e.getVars().getVars().size() )
            {
               _isInFunctionDeclaration = true;
               operator()( e.getVars() );
               _isInFunctionDeclaration = false;
            }
         }
         --_isInFunction;

         if ( !checkDefaultInitizalizationInFunctionDeclaration( e ) )
         {
            impl::reportUndeclaredType( e.getLocation(), _context, "default initialisation is not correctly used. (a parameter with default initialization cannot be followed by one without)" );
         }

         if ( e.getMemberOfClass() && e.getName() == e.getMemberOfClass()->getName() && e.getType() )
         {
            impl::reportUndeclaredType( e.getLocation(), _context, "a constructor can't have a return type" );
         }
      }

      virtual void operator()( AstThis& e )
      {
         if ( _defaultClassPath.size() == 0 )
         {
            impl::reportUndeclaredType( e.getLocation(), _context, "this can only be declared in a class scope" );
            return;
         } else {
            e.setReference( _classes.find( _defaultClassPath, _typedefs ) );
         }
      }

      virtual void operator()( AstReturn& e )
      {
         // check if we are in a function
         if ( !_canReturn.size() || !_canReturn.top() )
         {
            impl::reportUndeclaredType( e.getLocation(), _context, "return statements are only allowed in the body of a function" );
            return;
         }
         e.setFunction( *_currentFunc.rbegin() );
         if ( e.getReturnValue() )
         {
            operator()( *e.getReturnValue() );
         }
      }

      virtual void operator()( AstExpCall& e )
      {
         // nothing to do: i.e. a[ 0 ]( 5 ) => a will be checked part of "e.getName()"
         ++_functionCallsNeeded;
         operator()( e.getArgs() );
         operator()( e.getName() );
         --_functionCallsNeeded;

         // 
         AstVarSimple* var = dynamic_cast<AstVarSimple*>( &e.getName() );
         if ( var )
         {
            // if we only have a simple var, it means we are calling a global function, or global class, so link it!
            e.setSimpleName( var->getName() );

            // if the declaration is a class, store it!
            AstDeclVar* decl = dynamic_cast<AstDeclVar*>( var->getReference() );
            if ( decl )
            {
               // operator()
               AstDeclClass* c = dynamic_cast<AstDeclClass*>( decl->getType().getReference() );
               ensure( c, "compiler error: must be a class declaration (no typedef...)" );
               e.setInstanciation( c );
            } else {
               // construction of an object
               AstDecl* decl = findClassDecl( _defaultClassPath, _currentFieldList, var->getName() );
               if ( decl )
               {
                  AstDeclClass* declClass = dynamic_cast<AstDeclClass*>( decl );
                  if ( declClass )
                  {
                     // if this is a class, perfect!
                     e.setConstructed( declClass );
                  } else {
                     // else it has to be a typedef...
                     AstTypedef* declTypedef = dynamic_cast<AstTypedef*>( decl );
                     ensure( declTypedef, "should normally be a class declaration. TODO Check!" );
                     ensure( declTypedef->getType().getReference(), "compiler error: no ref" );
                     AstDeclClass* declClass  = dynamic_cast<AstDeclClass*>( declTypedef->getType().getReference() );
                     if ( declClass )
                        e.setConstructed( declClass );
                     else {
                        impl::reportUndeclaredType( declTypedef->getLocation(), _context, "this typedef must be a typedef on a type!" );
                     }
                  }
               } else {
                  // simple function call, nothing to do in the binding visitor
                  //impl::reportUndeclaredType( var->getLocation(), _context, "can't find construct an undefined class" );
               }
            }
         } else {
            e.setReference( e.getName().getReference() );
         }

      }

      virtual void operator()( AstDeclVar& e )
      {
         if ( _isInFunctionDeclaration )
         {
            e.setIsInFunctionPrototype();
         }

         // don't reference again the global scope...
         if ( _scopeDepth > 1 )
         {
            const AstDecl* decl = findClassDecl( _defaultClassPath, _currentFieldList, e.getName() );
            if ( decl )
            {
               impl::reportAlreadyDeclaredType( decl->getLocation(), e.getLocation(), _context, "a class has already been declared with this name" );
               return;
            }

            /*
            // function can have the same name, except at global scope, the priority is variable
            SymbolTableFuncs::iterator it = _funcs.find( e.getName() );
            if ( it != _funcs.end() )
            {
               // we know we have at lest 1 function of this name declared
               impl::reportAlreadyDeclaredType( it->second.list[ 0 ]->getLocation(), e.getLocation(), _context, "a function has already been declared with this name" );
               return;
            }*/

            if ( _vars.find_in_scope( e.getName() ) && !_isInFunctionDeclaration )
            {
               impl::reportAlreadyDeclaredType( _vars.find( e.getName() )->getLocation(), e.getLocation(), _context, "a variable has already been declared with this name" );
            } else {
               if ( !_isInFunction )
               {
                  // if we are in a class && not in a function, we must link variable to the class
                  AstDecl* current = _classes.find( _defaultClassPath, _typedefs );
                  if ( current )
                  {
                     AstDeclClass* currentClass = dynamic_cast<AstDeclClass*>( current );
                     ensure( currentClass, "it must be a class!" );
                     e.setClassMember( currentClass );
                  }
               }
               _vars.insert( e.getName(), &e );
            }
         } else {
            // we need to bind the classes level 1
         }


         if ( e.getInit() )
         {
            operator()( *e.getInit() );
         } else {
            // if a reference, then it must be initialized
            if ( !_isInFunctionDeclaration && e.getType().isAReference() && _defaultClassPath.size() == 0 && !e.getDeclarationList() ) // we don't check reference init if in function prototype -> must be done in the call // we can init a ref in a class at its first use
            {
               impl::reportUndeclaredType( e.getLocation(), _context, "type with reference must be initialized" );
               return;
            }

            if ( e.getDeclarationList() )
            {
               operator()( *e.getDeclarationList() );
            }
         }

         operator()( e.getType() );

         if ( e.getType().isArray() )
         {
            if ( e.getType().getSize() && e.getType().getSize()->size() > 0 )
            {
               for ( size_t n = 0; n < e.getType().getSize()->size(); ++n )
               {
                  operator()( *( (*e.getType().getSize())[ n ] ) );
               }
            } 
         }

         if ( e.getObjectInitialization() )
         {
            operator()( *e.getObjectInitialization() );
         }
      }

      virtual void operator()( AstTypeField& e )
      {
         _typedefs.begin_scope( e.getName() );
         
         
         AstDecl* decl = findClassDecl( _defaultClassPath, _currentFieldList, e.getName() );
         // the final type will be checked, we don't need to check each typefield...
         if ( !decl )
         {
            std::stringstream ss;
            ss << "can't find the declaration for this field: \"" << e.getName() << "\"";
            impl::reportUndeclaredType( e.getLocation(), _context, ss.str() );
         } else {
            e.setReference( decl );
         }
         

         _currentFieldList.push_back( e.getName() );
         operator()( e.getField() );
         _currentFieldList.pop_back();

         AstTypeT* t = dynamic_cast<AstTypeT*>( &e.getField() );
         if ( t )
         {
            // in case there is a error node -> we need to check we can cast it!
            e.setReference( t->getReference() );
         }
         _typedefs.end_scope();
      }

      virtual void operator()( AstImport& e )
      {
         if ( _scopeDepth != 1 )
         {
            impl::reportError( e.getLocation(), _context, "include statement can only be found in global scope" );
         }
      }

      virtual void operator()( AstInclude& e )
      {
         if ( _scopeDepth != 1 )
         {
            impl::reportError( e.getLocation(), _context, "include statement can only be found in global scope" );
         }
      }

      SymbolTableVars& getVars()
      {
         return _vars;
      }

      SymbolTableFuncs& getFuncs()
      {
         return _funcs;
      }

      SymbolTableClasses& getClasses()
      {
         return _classes;
      }

      virtual void operator()( AstExpTypename& e )
      {
         operator()( e.getType() );
         AstType* type = dynamic_cast<AstType*>( &e.getType() );
         AstTypeField* typefield = dynamic_cast<AstTypeField*>( &e.getType() );

         if ( type )
         {
            if ( type->getType() != AstType::SYMBOL )
            {
               impl::reportError( type->getLocation(), _context, "typename must only be used with class declaration" );
            } else {
               AstDeclClass* c = dynamic_cast<AstDeclClass*>( type->getReference() );
               ensure( c, "compiler error: must be a class declaration (no typename...)" );
               e.setReference( c );
            }
         } else if ( typefield )
         {
            AstDeclClass* c = dynamic_cast<AstDeclClass*>( typefield->getReference() );
            if ( c )
            {
               //ensure( c, "compiler error: should be a class declaration (no typename...)" );
               e.setReference( c );
            }
         }
      }

      
      virtual void operator()( AstTypedef& e )
      {
         if ( _isInFunction )
         {
            impl::reportError( e.getLocation(), _context, "a typedef cannot be created in a function" );
         }

         // check class & typedef name doesn't clash
         const AstDecl* decl = findClassDecl( _defaultClassPath, _currentFieldList, e.getName() );
         if ( decl && dynamic_cast<const AstDeclClass*>( decl ) ) // we only check there is no class with this name
         {
            impl::reportError( decl->getLocation(), _context, "typename: a class has already been declared with this name in this scope" );
         }
         operator()( e.getType() );
      }

   private:
      // disabled
      VisitorBind& operator=( const VisitorBind& );
      VisitorBind( const VisitorBind& );

   private:
      int                        _scopeDepth;
      std::stack<bool>           _canReturn;
      std::vector<mvv::Symbol>   _defaultClassPath;
      std::vector<mvv::Symbol>   _currentFieldList;
      std::vector<AstDeclFun*>   _currentFunc;
      std::stack<AstWhile*>      _whiles;
      int                        _functionCallsNeeded;
      int                        _isInFunction;
      bool                       _isInFunctionDeclaration; /* true if we are checking the function parameter declaration */

      ParserContext&      _context;
      SymbolTableVars&    _vars;
      SymbolTableFuncs&   _funcs;
      SymbolTableClasses& _classes;
      SymbolTableTypedef& _typedefs;
   };
}
}

#endif