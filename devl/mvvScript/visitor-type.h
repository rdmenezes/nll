#ifndef MVV_PARSE_VISITOR_TYPE_H_
# define MVV_PARSE_VISITOR_TYPE_H_

# include "visitor.h"
# include "ast-files.h"
# include "type.h"

//
// TODO node type must all be cloned when copied! else memory problems...
// TODO check int n[3] = {n[ 0 ]}; recursive...
// TODO int fn( int a = 0, int b ) => improve message error
// TODO declaration order in class
//

namespace mvv
{
namespace parser
{
   /**
    @brief Defines a visitor visiting all the nodes but doing nothing
    @note this is usefull if a visitor is only handling a limited number of nodes
    */
   class VisitorType : public VisitorDefault
   {
   public:
      VisitorType( ParserContext& context,
                   const SymbolTableVars& vars,
                   const SymbolTableFuncs& funcs,
                   const SymbolTableClasses& classes ) : _context( context ), _vars( vars ), _funcs( funcs ), _classes( classes )
      {
      }

      /**
       @brief Return the result of a dereferenced type, must be deallocated
       */
      static Type* dereference( Type& t )
      {
         TypeArray* array = dynamic_cast<TypeArray*>( &t );
         if ( !array )
         {
            return 0;
         }
         if ( array->getDimentionality() == 1 )
         {
            return array->getRoot().clone();
         }
         return new TypeArray( array->getDimentionality() - 1, array->getRoot() );
      }

      /**
       @brief Returns the member function of a class
       */
      static std::vector<AstDeclFun*> getFunctionsFromClass( AstDeclClass& c, const mvv::Symbol& s )
      {
         std::vector<AstDeclFun*> res;
         for ( AstDecls::Decls::iterator it = c.getDeclarations().getDecls().begin();
               it != c.getDeclarations().getDecls().end();
               ++it )
         {
            AstDeclFun* fn = dynamic_cast<AstDeclFun*>( *it );
            if ( (*it)->getName() == s && fn )
            {
               res.push_back( fn );
            }
         }
         return res;
      }

      /**
       @brief Given a set of functions, find the ones that are compatible with the args

       First check if a function match exactly the args - in this case exactly 1 func will be returned,
       else check all functions that are possible. If several, there is a conflict and an error must be issued.
       */
      static std::vector<AstDeclFun*> getMatchingFunctionsFromArgs( const std::vector<AstDeclFun*>& funcs, const AstArgs& args )
      {
         std::vector<AstDeclFun*> res;
         std::vector<AstDeclFun*> possible;
         for ( AstArgs::Args::const_iterator it = args.getArgs().begin();
               it != args.getArgs().end();
               ++it )
         {
            ensure( (*it)->getNodeType(), "can't type an expression" );
         }

         for ( size_t n = 0; n < funcs.size(); ++n )
         {
         lbl_continue:
            if ( n >= funcs.size() )
               break;

            bool succeeded = true;
            bool equal = true;

            // check all the arguments
            AstDeclVars::Decls::const_iterator decls = funcs[ n ]->getVars().getVars().begin();
            for ( AstArgs::Args::const_iterator it = args.getArgs().begin();
                  ;
               ++it, ++decls )
            {
               if ( it == args.getArgs().end() )
               {
                  // there are more decls than args, check the remaining ones have all an initialization
                  for ( ; decls != funcs[ n ]->getVars().getVars().end(); ++decls )
                  {
                     if ( !(*decls)->getInit() )
                     {
                        // if not init, just discard the function
                        ++n;
                        goto lbl_continue;
                     }
                  }
               } else {
                  // check if types are equal, compatible, incompatible
                  if ( (*it)->getNodeType()->isEqual( *(*decls)->getNodeType() ) )
                  {
                     // do nothing
                  } else if ( (*it)->getNodeType()->isEqual( *(*decls)->getNodeType() ) )
                  {
                     equal = false;
                  } else {
                     succeeded = false;
                     break;
                  }
               }
               if ( decls == funcs[ n ]->getVars().getVars().end() )
                  break;
            }

            if ( equal )
            {
               res.push_back( funcs[ n ] );
            } else if ( succeeded )
            {
               possible.push_back( funcs[ n ] );
            }
         }

         if ( res.size() == 1 )
         {
            return res;
         }
         if ( possible.size() == 1 )
         {
            return possible;
         }
         if ( res.size() > 1 )
            return res;
         return possible;

         // TODO use constructor...?
      }

      /**
       @brief Find a declaration inside a class
       */
      static AstDecl* getDeclFromClass( AstDeclClass& declClass, const mvv::Symbol& name )
      {
         for ( AstDecls::Decls::iterator it = declClass.getDeclarations().getDecls().begin();
               it != declClass.getDeclarations().getDecls().end();
               ++it )
         {
            if ( (*it)->getName() == name )
               return *it;
         }
         return 0;
      }

      virtual void operator()( AstInt& e )
      {
         e.setNodeType( new TypeInt() );
      }

      virtual void operator()( AstNil& e )
      {
         e.setNodeType( new TypeNil() );
      }

      virtual void operator()( AstFloat& e )
      {
         e.setNodeType( new TypeFloat() );
      }

      virtual void operator()( AstString& e )
      {
         e.setNodeType( new TypeString() );
      }

      virtual void operator()( AstOpBin& e )
      {
         // first evaluate the operand type
         operator()( e.getLeft() );
         operator()( e.getRight() );

         if ( !e.getLeft().getNodeType() )
         {
            impl::reportTypeError( e.getLeft().getLocation(), _context, "expression cannot be typed");
            e.setNodeType( new TypeError() );
            return;
         }

         if ( !e.getRight().getNodeType() )
         {
            impl::reportTypeError( e.getRight().getLocation(), _context, "expression cannot be typed");
            e.setNodeType( new TypeError() );
            return;
         }

         // types must be compatible
         // TODO update this as it is not true if default operators are overloaded...
         if ( !e.getRight().getNodeType()->isCompatibleWith( *e.getLeft().getNodeType() ) )
         {
            impl::reportTypeError( e.getLocation(), _context, "incompatible types");
            e.setNodeType( new TypeError() );
            return;
         }

         if ( e.getOp() == AstOpBin::GE ||
              e.getOp() == AstOpBin::EQ ||
              e.getOp() == AstOpBin::LE ||
              e.getOp() == AstOpBin::NE ||
              e.getOp() == AstOpBin::LT ||
              e.getOp() == AstOpBin::GT ||
              e.getOp() == AstOpBin::AND||
              e.getOp() == AstOpBin::OR )
         {
            e.setNodeType( new TypeInt() );
         } else {
            e.setNodeType( e.getLeft().getNodeType() );
         }

         // TODO additionally, first, 
         // - first if TypeNamed, look up for a function in the class (operator...), else
         // - check the list of global function (with operator+, ...)
         // if true then set the type for these functions
         // if funtcion not found, issue an error
      }

      virtual void operator()( AstIf& e )
      {
         operator()( e.getCondition() );

         ensure( e.getCondition().getNodeType(), "tree can't be typed" );
         if ( !TypeInt().isEqual( *e.getCondition().getNodeType() ) )
         {
            impl::reportTypeError( e.getCondition().getLocation(), _context, "if condition type must be an int" );
         }

         operator()( e.getThen() );

         if ( e.getElse() )
         {
            operator()( *e.getElse() );
         }
      }

      virtual void operator()( AstStatements& e )
      {
         for ( AstStatements::Statements::const_iterator it = e.getStatements().begin();
               it != e.getStatements().end();
               ++it )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstExpAssign& e )
      {
         operator()( e.getValue() );
         operator()( e.getLValue() );
         // TODO handle "var" here
         // TODO handle operator= here
         e.setNodeType( e.getValue().getNodeType() );

         ensure( e.getLValue().getNodeType(), "compiler error: cannot evaluate expression type" );
         if ( !e.getValue().getNodeType()->isCompatibleWith( *e.getLValue().getNodeType() ) )
         {
            impl::reportTypeError( e.getLocation(), _context, "incompatible types");
            e.setNodeType( new TypeError() );
         }
      }

      virtual void operator()( AstVarSimple& e )
      {
         if ( e.getReference() )
         {
            // if it is a simple variable, we will have a type
            e.setNodeType( e.getReference()->getNodeType() );
         } // else we know it is a call exp, so don't do anything
      }

      virtual void operator()( AstVarArray& e )
      {
         operator()( e.getName() );
         operator()( e.getIndex() );
         ensure( e.getIndex().getNodeType(), "tree can't be typed" );
         ensure( e.getName().getNodeType(), "tree can't be typed" );
         if ( !TypeInt().isEqual( *e.getIndex().getNodeType() ) )
         {
            impl::reportTypeError( e.getLocation(), _context, "index must be of type int" );
         }

         TypeNamed* ty = dynamic_cast<TypeNamed*>( e.getName().getNodeType() );
         if ( ty )
         {
            // if it is a type, just find an operator[]
            std::vector<AstDeclFun*> funcs = getFunctionsFromClass( *ty->getDecl(), mvv::Symbol::create( "operator[]" ) );
            if ( funcs.size() == 0 )
            {
               impl::reportTypeError( e.getName().getLocation(), _context, "operator[] could not be found in class" );
               e.setNodeType( new TypeError() );
               return;
            } else {
               if ( funcs.size() > 1 )
               {
                  impl::reportTypeMultipleCallableFunction( funcs, _context );
                  e.setNodeType( funcs[ 0 ]->getNodeType() );
                  return;
               }
               e.setNodeType( funcs[ 0 ]->getNodeType() );
            }
         } else {
            Type* deref = dereference( *e.getName().getNodeType() );
            if ( !deref )
            {
               impl::reportTypeError( e.getName().getLocation(), _context, "dereferencement of a variable incorrect" );
               e.setNodeType( new TypeError() );
               return;
            } else {
               e.setNodeType( deref );
            }
         }
      }

      // choices are:
      // - global function
      // - class instanciation
      // - varfield
      virtual void operator()( AstExpCall& e )
      {
         operator()( e.getArgs() );
         operator()( e.getName() );

         std::vector<AstDeclFun*> funcs;
         if ( e.getSimpleName() )
         {
            // we are in the case where we are calling a global function/class constructor
            SymbolTableFuncs::iterator it = _funcs.find( *e.getSimpleName() );
            if ( it != _funcs.end() )
            {
               // use the simple name for the function
               funcs = getMatchingFunctionsFromArgs( it->second, e.getArgs() );
            } else {
               if ( e.getInstanciation() )
               {
                  // else we know it is an object, so operator() must be called
                  funcs = getMatchingFunctionsFromArgs( getFunctionsFromClass( *e.getInstanciation(), mvv::Symbol::create( "operator()" ) ), e.getArgs() );
               } else {
                  ensure( e.getConstructed(), "error" );
                  // else, it is a class intanciated, check if the constructor is appropriate
                  funcs = getMatchingFunctionsFromArgs( getFunctionsFromClass( *e.getConstructed(), *e.getSimpleName() ), e.getArgs() );
               }
            }
         } else {
            AstVarField* field = dynamic_cast<AstVarField*>( &e.getName() );
            if ( field )
            {
               AstDeclClass* decl = dynamic_cast<AstDeclClass*>( field->getReference() );
               ensure( decl, "field must be of class type" );
               funcs = getMatchingFunctionsFromArgs( getFunctionsFromClass( *decl, field->getName() ), e.getArgs() );
            } else {
               ensure( 0, "Error: unexpected node type..." );
            }
         }

         if ( funcs.size() == 0 )
         {
            impl::reportTypeError( e.getName().getLocation(), _context, "no function matching the arguments found" );
            e.setNodeType( new TypeVoid() );
            return;
         }

         if ( funcs.size() > 1 )
         {
            impl::reportTypeMultipleCallableFunction( funcs, _context );
            e.setNodeType( funcs[ 0 ]->getNodeType() );
            return;
         }

         e.setNodeType( funcs[ 0 ]->getNodeType() );
      }

      virtual void operator()( AstVarField& e )
      {
         // we are storing the field in reverse order, so first run node, then set up the type
         operator()( e.getField() );

         if ( !e.getField().getNodeType() )
         {
            impl::reportTypeError( e.getLocation(), _context, "cannot evaluate type of an expression" );
            e.setNodeType( new TypeError() );
            return;
         }

         TypeNamed* type = dynamic_cast<TypeNamed*>( e.getField().getNodeType() );
         if ( !type )
         {
            impl::reportTypeError( e.getLocation(), _context, "field must be of class type" );
            e.setNodeType( new TypeError() );
            return;
         }

         e.setReference( type->getDecl() );
         AstDecl* member = getDeclFromClass( *type->getDecl(), e.getName() );
         if ( !member )
         {
            impl::reportTypeError( e.getLocation(), _context, "error can't find member in class definition" );
            e.setNodeType( new TypeError() );
            return;
         }
         e.setNodeType( member->getNodeType() );
      }

      virtual void operator()( AstType& e )
      {
         switch ( e.getType() )
         {
         case AstType::FLOAT:
            e.setNodeType( new TypeFloat() );
            break;

         case AstType::INT:
            e.setNodeType( new TypeInt() );
            break;

         case AstType::STRING:
            e.setNodeType( new TypeString() );
            break;

         case AstType::VOID:
            e.setNodeType( new TypeVoid() );
            break;

         case AstType::SYMBOL:
            ensure( e.getReference(), "compiler error: can't find a link on a symbol" );
            e.setNodeType( e.getReference()->getNodeType() );
            break;

         default:
            ensure( 0, "compiler error: primitive type not handled" );
         }
      }

      virtual void operator()( AstDecls& e )
      {
         for ( AstDecls::Decls::const_iterator it = e.getDecls().begin(); it != e.getDecls().end(); ++it )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstDeclVars& e )
      {
         for ( AstDeclVars::Decls::const_iterator it = e.getVars().begin(); it != e.getVars().end(); ++it )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstDeclClass& e )
      {
         e.setNodeType( new TypeNamed( &e ) );
         operator()( e.getDeclarations() );
      }

      virtual void operator()( AstDeclFun& e ) 
      {
         bool isAConstructor = false;
         if ( e.getType() )
         {
            operator()( *e.getType() );
            e.setNodeType( e.getType()->getNodeType() );
         } else {
            // if not in class, type can't be empty
            if ( !e.getMemberOfClass() )
            {
               e.setNodeType( new TypeVoid() );
               impl::reportTypeError( e.getLocation(), _context, "missing type in function prototype" );
               return;
            } else {
               // else it is a constructor and must have the same name than the class
               // a constructor returns the same type than the class it is nested
               e.setNodeType( e.getMemberOfClass()->getNodeType() );
               isAConstructor = true;

               if ( e.getName() != e.getMemberOfClass()->getName() )
               {
                  impl::reportTypeError( e.getLocation(), _context, "constructor must have the same name than class, or missing function type" );
                  e.setNodeType( new TypeError() );
                  return;
               }
            }
         }

         if ( e.getVars().getVars().size() )
         {
            operator()( e.getVars() );
         }
         
         if ( e.getBody() )
         {
            ensure( e.getNodeType(), "error: a function doesn't have a return type" );
            _returnType.push_back( e.getNodeType() );
            operator()( *e.getBody() );
            _returnType.pop_back();

            // check return has been called, ( else it must be a void type or class constructor)
            if ( !isAConstructor && !e.getExpectedFunctionType() && !TypeVoid().isCompatibleWith( *e.getNodeType() )  )
            {
               impl::reportTypeError( e.getLocation(), _context, "return type statement has not been called in a function returning a value" );
               return;
            }

            // return should have checked incorrect types... so don't check it again
         }
      }

      virtual void operator()( AstArgs& e )
      {
         for ( AstArgs::Args::const_iterator it = e.getArgs().begin(); it != e.getArgs().end(); ++it )
         {
            operator()( **it );
         }
      }

      virtual void operator()( AstReturn& e )
      {
         ensure( e.getFunction(), "unknown error" );
         if ( e.getReturnValue() )
         {
            operator()( *e.getReturnValue() );
            e.setNodeType( e.getReturnValue()->getNodeType() );
         } else {
            e.setNodeType( new TypeVoid() );
         }
         e.getFunction()->setExpectedFunctionType( e.getNodeType() );

         ensure( _returnType.size(), "error: return allowed outside of a function..." );
         if ( !e.getNodeType()->isCompatibleWith( **_returnType.rbegin() ) )
         {
            impl::reportTypeError( e.getLocation(), _context, "function return type and actual type are incompatible" );
            e.setNodeType( new TypeError() );
         }
      }

      virtual void operator()( AstDeclVar& e )
      {
         // we first must visite the type!
         operator()( e.getType() );
         ensure( e.getType().getNodeType(), "tree can't be typed" );
         if ( TypeVoid().isCompatibleWith( *e.getType().getNodeType() ) )
         {
            impl::reportTypeError( e.getLocation(), _context, "variable type must not be void" );
         }

         if ( e.getType().isArray() )
         {
            if ( e.getType().getSize() && e.getType().getSize()->size() > 0 )
            {
               for ( size_t n = 0; n < e.getType().getSize()->size(); ++n )
               {
                  operator()( *( (*e.getType().getSize())[ n ] ) );

                  ensure( (*e.getType().getSize())[ n ]->getNodeType(), "must have a dimensionality >= 1" );
                  if ( !TypeInt().isEqual( *(*e.getType().getSize())[ n ]->getNodeType() ) )
                  {
                     impl::reportTypeError( (*e.getType().getSize())[ n ]->getLocation(), _context, "variable type must be of integer type" );
                  }
               }
            }
            ensure( e.getType().getNodeType(), "can't type properly a tree" );
            ui32 size = static_cast<ui32>( ( e.getType().getSize() && e.getType().getSize()->size() ) ? e.getType().getSize()->size() : 1 );
            e.setNodeType( new TypeArray( size, *e.getType().getNodeType() ) );
         } else {
            e.setNodeType( e.getType().getNodeType() );
         }

         if ( e.getInit() )
         {
            operator()( *e.getInit() );
            ensure( e.getInit()->getNodeType(), "tree can't be typed" );
            if ( !e.getInit()->getNodeType()->isCompatibleWith( *e.getNodeType() ) )
            {
               impl::reportTypeError( e.getInit()->getLocation(), _context, "variable and asignment types are not compatible" );
            }
         } else if ( e.getDeclarationList() )
         {
            operator()( *e.getDeclarationList() );

            Type* deref = dereference( *e.getNodeType() );
            if ( deref )
            {
               // all the values must be compatible with
               for ( AstArgs::Args::iterator it = e.getDeclarationList()->getArgs().begin();
                     it != e.getDeclarationList()->getArgs().end();
                     ++it )
               {
                  if ( !deref->isCompatibleWith( *(*it)->getNodeType() ) )
                  {
                     impl::reportTypeError( (*it)->getLocation(), _context, "type in initialization list is not compatible with te variable type" );
                  }
               }
            } else if ( dynamic_cast<TypeNamed*>( e.getType().getNodeType() ) )
            {
               impl::reportTypeError( e.getLocation(), _context, "only array can have initialization list" );
               // TODO: is it worth adding classes with init list?

               /*
               AstDeclClass* c = dynamic_cast<TypeNamed*>( e.getType().getNodeType() )->getDecl();
               ensure( c, "critical error" );

               AstDecls::Decls::iterator itDecl = c->getDeclarations().getDecls().begin()
               for ( AstArgs::Args::iterator it = e.getDeclarationList()->getArgs().begin();
                     it != e.getDeclarationList()->getArgs().end();
                     ++it, ++itDecl )
               {
                  if ( itDecl == c->getDeclarations().getDecls().end() )
                  {
                     impl::reportTypeError( (*it)->getLocation(), _context, "too many field in initialization list for this type" );
                     break;
                  }

                  AstDeclVar* v = dynamic_cast<AstDeclVar*>( *itDecl );
                  if ( v )
                  {

                  }
               }*/
            } else {
               impl::reportTypeError( e.getDeclarationList()->getLocation(), _context, "variable must be an array or a class to use initialization list" );
            }

            delete deref;
         }
      }

      virtual void operator()( AstExpSeq& e )
      {
         operator()( e.getExp() );
         e.setNodeType( e.getExp().getNodeType() );
      }

      virtual void operator()( AstTypeField& e )
      {
         operator()( e.getField() );
         e.setNodeType( new TypeNamed( e.getReference() ) );
      }

      virtual void operator()( AstExpTypename& e )
      {
         operator()( e.getType() );
         if ( e.getArgs().getArgs().size() )
         {
            operator()( e.getArgs() );
         }
         e.setNodeType( new TypeNamed( e.getReference() ) );
      }

      virtual void operator()( Ast& e )
      {
         e.accept( *this );
      }

   private:
      // disabled
      VisitorType& operator=( VisitorType& );
      VisitorType( const VisitorType& );


   private:
      std::vector<const Type*>   _returnType;


      ParserContext&      _context;
      SymbolTableVars     _vars;
      SymbolTableFuncs    _funcs;
      SymbolTableClasses  _classes;
   };
}
}

#endif