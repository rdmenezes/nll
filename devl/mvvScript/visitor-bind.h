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
                   const SymbolTableVars& vars,
                   const SymbolTableFuncs& funcs,
                   const SymbolTableClasses& classes ) : _context( context ), _vars( vars ), _funcs( funcs ), _classes( classes )
      {
         _scopeDepth = 0;
         _functionCallsNeeded = 0;
         _isInFunction = false;
      }

      AstDeclClass* findClassDecl( const std::vector<mvv::Symbol>& path, const std::vector<mvv::Symbol>& field, const mvv::Symbol& name )
      {
         // A::D  B C
         // first: - check the local path A::D, look for C from here
         //        - check if declared within source class: check from D if B::C, else check from A if B::C
         //        - else check in global scope B::C
         //

         if ( field.size() )
         {
            // local
            // concatenate the field to the path
            std::vector<mvv::Symbol> up( path );
            for ( size_t n = 0; n < field.size(); ++n )
            {
               up.push_back( field[ n ] );
            }
            AstDeclClass* current = _classes.find_in_class( up, name );
            if ( current )
               return current;

            // check within class
            std::vector<mvv::Symbol> up2( field );
            up2.push_back( name );
            AstDeclClass* within = _classes.find_within_scope( path, up2 );
            if ( within )
               return within;

            // finally check global scope
            return _classes.find( up2 );
         } else {
            // current class
            AstDeclClass* current = _classes.find_in_class( _defaultClassPath, name );
            if ( current )
               return current;
            AstDeclClass* within = _classes.find_within_scope( _defaultClassPath, name );
            if ( within )
               return within;
            return _classes.find( nll::core::make_vector<mvv::Symbol>( name ) );
         }
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
         AstDeclVar* var = _vars.find( e.getName() );
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
         }
      }

      virtual void operator()( AstVarField& e )
      {
         // do nothing: we need type value to check if the fields are correctly used
         operator()( e.getField() );
      }

      virtual void operator()( AstType& e )
      {
         if ( e.getType() == AstType::SYMBOL )
         {
            AstDeclClass* decl = findClassDecl( _defaultClassPath, _currentFieldList, *e.getSymbol() );
            if ( !decl )
            {
               impl::reportUndeclaredType( e.getLocation(), _context, "undeclared type" );
            } else {
               e.setReference( decl );
            }
         }
      }

      virtual void operator()( AstDeclClass& e )
      {
         if ( _defaultClassPath.size() == 0 && _scopeDepth > 1 )
         {
            impl::reportUndeclaredType( e.getLocation(), _context, "a class can only be declared in the global scope/or nested in another class" );
         }

         ++_scopeDepth;
         _vars.beginScope( true );
         _defaultClassPath.push_back( e.getName() );
         operator()( e.getDeclarations() );
         _defaultClassPath.pop_back();
         _vars.endScope();
         --_scopeDepth;
      }

      virtual void operator()( AstDeclFun& e ) 
      {
         _isInFunction = true;
         if ( _defaultClassPath.size() == 0 )
         {
            // if we are in class, it means it is a member function
            SymbolTableFuncs::iterator it = _funcs.find( e.getName() );
            if ( it == _funcs.end() )
            {
               impl::reportUndeclaredType( e.getLocation(), _context, "a function can only be declared in the global scope" );
            }
         }

         if ( e.getType() )
         {
            operator()( *e.getType() );
         }
         
         if ( e.getBody() )
         {
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
               operator()( e.getVars() );
            }

            _canReturn.push( true );
            operator()( *e.getBody() );
            _canReturn.pop();
            _vars.endScope();

            --_scopeDepth;
         } else {
            // if there is no body, we still need to go through the arguments and bind them
            if ( e.getVars().getVars().size() )
            {
               operator()( e.getVars() );
            }
         }
         _isInFunction = false;
      }

      virtual void operator()( AstReturn& e )
      {
         // check if we are in a function
         if ( !_canReturn.size() || !_canReturn.top() )
         {
            impl::reportUndeclaredType( e.getLocation(), _context, "return statements are only allowed in the body of a function" );
         }
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

         /*
         // TODO check conflicts?
         if ( _functionCallsNeeded != 0 )
         {
            // safeguard to check that functions have been called exactly the expected number of times
            impl::reportUndeclaredType( e.getName().getLocation(), _context, "function call not declared as such" );
         }*/
      }

      virtual void operator()( AstDeclVar& e )
      {
         // don't reference again the global scope...
         if ( _scopeDepth > 1 )
         {
            const AstDeclClass* decl = findClassDecl( _defaultClassPath, _currentFieldList, e.getName() );
            if ( decl )
            {
               impl::reportAlreadyDeclaredType( decl->getLocation(), e.getLocation(), _context, "a class has already been declared with this name" );
               return;
            }

            SymbolTableFuncs::iterator it = _funcs.find( e.getName() );
            if ( it != _funcs.end() )
            {
               // we know we have at lest 1 function of this name declared
               impl::reportAlreadyDeclaredType( it->second[ 0 ]->getLocation(), e.getLocation(), _context, "a function has already been declared with this name" );
               return;
            }

            if ( _vars.find_in_scope( e.getName() ) )
            {
               impl::reportAlreadyDeclaredType( _vars.find( e.getName() )->getLocation(), e.getLocation(), _context, "a variable has already been declared with this name" );
            } else {
               if ( !_isInFunction )
               {
                  // if we are in a class && not in a function, we must link variable to the class
                  AstDeclClass* currentClass = _classes.find( _defaultClassPath );
                  if ( currentClass )
                  {
                     e.setClassMember( currentClass );
                  }
               }
               _vars.insert( e.getName(), &e );
            }
         }


         if ( e.getInit() )
         {
            operator()( *e.getInit() );
         } else if ( e.getDeclarationList() )
         {
            operator()( *e.getDeclarationList() );
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
      }

      virtual void operator()( AstTypeField& e )
      {
         _currentFieldList.push_back( e.getName() );
         AstDeclClass* decl = findClassDecl( _defaultClassPath, _currentFieldList, e.getName() );
         /*
         // the final type will be checked, we don't need to check each typefield...
         if ( !decl )
         {
            std::stringstream ss;
            ss << "can't find the declaration for this field: \"" << e.getName() << "\"";
            impl::reportUndeclaredType( e.getLocation(), _context, ss.str() );
         } else {
            e.setReference( decl );
         }*/

         operator()( e.getField() );
         _currentFieldList.pop_back();

         AstTypeField* field = dynamic_cast<AstTypeField*>( &e.getField() );
         AstType* isTernimal = dynamic_cast<AstType*>( &e.getField() );
         //
         // TODO error here -> terminal is the next one, not this one...
         //
         if ( isTernimal )
         {
            // we are the last node of a typefield, we need to save the final class it is pointing to!
            e.setFinalReference( isTernimal->getReference() );
         } else {
            // else propagate the type to earlier node
            e.setFinalReference( e.getFinalReference() );
         }
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

      const SymbolTableVars& getVars() const
      {
         return _vars;
      }

      const SymbolTableFuncs& getFuncs() const
      {
         return _funcs;
      }

      const SymbolTableClasses& getClasses() const
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
               e.setReference( type->getReference() );
            }
         } else if ( typefield )
         {
            e.setReference( typefield->getFinalReference() );
         }
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
      int                        _functionCallsNeeded;
      bool                       _isInFunction;

      ParserContext&      _context;
      SymbolTableVars     _vars;
      SymbolTableFuncs    _funcs;
      SymbolTableClasses  _classes;
   };
}
}

#endif