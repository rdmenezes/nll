#ifndef MVV_PARSE_VISITOR_TYPE_H_
# define MVV_PARSE_VISITOR_TYPE_H_

# include "visitor.h"
# include "ast-files.h"
# include "type.h"

//
// TODO declaration order in class => should be independent
// TODO class and operator overloading: in a class, (this) not shown...
// TODO float operator+( int n, float nn); int n = 3; float f = 2.5; int nn = f + n; : function returns a float, but argument does not->need conversion or type error


namespace mvv
{
namespace parser
{
   /**
    @brief Defines a visitor visiting all the nodes and typing all typable nodes

    @note Because we want to remove forward declarations, in the case of includes that mutually
    need each other, we sometimes will need to start typing a file while some part of the ohter
    have not been typed, so we manually need to launch the typing on the nodes that are missing
    (the mutual dependencies). => we need to type the classes and function declarations!
    The scenario is: - use a function imported -> we need to type it! (when lookup the possible functions)
                       also, if the imported function uses types that have not been typed -> type them! (AstDeclVar)
                     - use a type declared in another file( which have not yet been parsed)
    */
   class VisitorType : public VisitorDefault
   {
   public:
      VisitorType( ParserContext& context,
                   SymbolTableVars& vars,
                   SymbolTableFuncs& funcs,
                   SymbolTableClasses& classes ) : _context( context ), _vars( vars ), _funcs( funcs ), _classes( classes )
      {
         // TODO: we actually want it when the visitor is run... not created
         _isInFunctionDeclaration = false;
         _currentFp = 0;
         _isFunctionBeingCalled = false;
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
         return new TypeArray( array->getDimentionality() - 1, array->getRoot(), array->isReference() );
      }

      // reutrn true if array<Type&>
      bool hasArrayInnerReference( Type& t )
      {
         ui32 depth = 0;
         const TypeArray* array = dynamic_cast<const TypeArray*>( &t );
         const TypeArray* previous = array;
         while ( array )
         {
            if ( array->isReference() && depth > 0 )
               return true;
            previous = array;
            array = dynamic_cast<const TypeArray*>( &array->getRoot() );
            ++depth;
         }
         if ( previous )
            return previous->getRoot().isReference();
         return false;
      }

      /**
       @check a class is constructible with 0 arguments
       */
      AstDeclFun* checkDefaultConstructible( const TypeNamed* ty, const YYLTYPE& loc )
      {
         if ( ty )
         {
            AstArgs args( loc, false );
            std::vector<AstDeclFun*> funcs = getMatchingFunctionsFromArgs(getFunctionsFromClass( *ty->getDecl(), ty->getDecl()->getName() ), args );
            if ( funcs.size() == 0 )
            {
               impl::reportTypeError( loc, _context, "the class is not constructible" );
               return 0;
            } else if ( funcs.size() > 1 )
            {
               impl::reportTypeError( loc, _context, "ambiguous constructor to construct a class in this array" );
               return 0;
            }

            return funcs[ 0 ];
         }
         return 0;
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
       @brief Returns the member function of a class
       */
      static std::vector<AstDeclFun*> getFunctionsFromGlobal( const SymbolTableFuncs& funcs, const mvv::Symbol& s )
      {
         SymbolTableFuncs::const_iterator it = funcs.find( s );
         if ( it != funcs.end() )
         {
            return it->second.list;
         }
         return std::vector<AstDeclFun*>();
      }

      /**
       @brief Given a set of functions, find the ones that are compatible with the args

       First check if a function match exactly the args - in this case exactly 1 func will be returned,
       else check all functions that are possible. If several, there is a conflict and an error must be issued.
       */
      std::vector<AstDeclFun*> getMatchingFunctionsFromArgs( const std::vector<AstDeclFun*>& funcs, const AstArgs& args )
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

            // check the function has been typed (in case mutual inclusion, this may not be true as we need to parse 1 file at a time)
            if ( funcs[ n ]->getNodeType() == 0 )
            {
               // manually type the function...
               VisitorType visitor( _context, _vars, _funcs, _classes );
               visitor( *funcs[ n ] );
            }

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
                  if ( decls == funcs[ n ]->getVars().getVars().end() )
                  {
                     // not enough decl field
                     ++n;
                     goto lbl_continue;
                  }

                  // check if types are equal, compatible, incompatible
                  if ( (*it)->getNodeType()->isEqual( *(*decls)->getNodeType() ) )
                  {
                     // do nothing
                  } else if ( (*it)->getNodeType()->isCompatibleWith( *(*decls)->getNodeType() ) )
                  {
                     equal = false;
                  } else {
                     succeeded = false;
                     equal = false;
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
      }

      /**
       @brief Find a declaration inside a class
       */
      static std::vector<AstDecl*> getDeclFromClass( AstDeclClass& declClass, const mvv::Symbol& name )
      {
         std::vector<AstDecl*> v;
         for ( AstDecls::Decls::iterator it = declClass.getDeclarations().getDecls().begin();
               it != declClass.getDeclarations().getDecls().end();
               ++it )
         {
            if ( (*it)->getName() == name )
               v.push_back( *it );
         }

         return v;
      }

      virtual void operator()( AstInt& e )
      {
         e.setNodeType( new TypeInt( false ) );
      }

      virtual void operator()( AstNil& e )
      {
         e.setNodeType( new TypeNil() );
      }

      virtual void operator()( AstFloat& e )
      {
         e.setNodeType( new TypeFloat( false ) );
      }

      virtual void operator()( AstString& e )
      {
         e.setNodeType( new TypeString( false ) );
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

         // if type && operator== operator!=, special case: check the adress and return an int
         TypeNamed* t = dynamic_cast<TypeNamed*>( e.getLeft().getNodeType() );
         if ( t &&
              e.getOp() == AstOpBin::EQ ||
              e.getOp() == AstOpBin::NE )
         {
            if ( !e.getRight().getNodeType()->isCompatibleWith( *e.getLeft().getNodeType() ) )
            {
               impl::reportTypeError( e.getLocation(), _context, "incompatible types");
               e.setNodeType( new TypeError() );
               return;
            } else {
               e.setNodeType( new TypeInt( e.getLeft().getNodeType()->isReference() ) );
            }
         }

         mvv::Symbol ops = impl::toSymbol( e.getOp() );
         // if it is a type, check the class has a specific member for this operator
         if ( t )
         {
            AstArgs args( e.getRight().getLocation(), false );
            args.insert_back( &e.getRight() );
            std::vector<AstDeclFun*> funs = getMatchingFunctionsFromArgs( getFunctionsFromClass( *t->getDecl(), ops ), args );
            if ( funs.size() == 1 )
            {
               e.setNodeType( funs[ 0 ]->getNodeType()->clone() );
               e.setFunctionCall( funs[ 0 ] );
               e.setIsBinOpInClass( true );
               return;
            } else {
               if ( funs.size() > 1 )
               {
                  // ambiguous call
                  impl::reportTypeError( e.getRight().getLocation(), _context, "ambiguous call to " + std::string( ops.getName() ) );
                  e.setNodeType( new TypeError() );
                  return;
               }
            }
         }

         // this is a basic type, or a type but operator is not member
         AstArgs args( e.getRight().getLocation(), false );
         args.insert_back( &e.getLeft() );
         args.insert_back( &e.getRight() );
         std::vector<AstDeclFun*> funs = getMatchingFunctionsFromArgs( getFunctionsFromGlobal( _funcs, ops ), args );
         if ( funs.size() == 1 )
         {
            e.setNodeType( funs[ 0 ]->getNodeType()->clone() );
            e.setFunctionCall( funs[ 0 ] );
                  return;
         } else if ( funs.size() > 1 )
         {
            // ambiguous call
            impl::reportTypeError( e.getRight().getLocation(), _context, "ambiguous call to " + std::string( ops.getName() ) );
            e.setNodeType( new TypeError() );
            return;
         } else {
            // no function found
            impl::reportTypeError( e.getLocation(), _context, "no function found associated with this operator");
            e.setNodeType( new TypeError() );
            return;
         }
      }

      virtual void operator()( AstIf& e )
      {
         operator()( e.getCondition() );

         ensure( e.getCondition().getNodeType(), "tree can't be typed" );
         if ( !TypeInt( false ).isEqual( *e.getCondition().getNodeType() ) )
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
         // save the current FP: in case: "int a; { int b; } int c;"
         _fp.push( _currentFp );

         for ( AstStatements::Statements::const_iterator it = e.getStatements().begin();
               it != e.getStatements().end();
               ++it )
         {
            operator()( **it );
         }

         // restaure the FP
         _currentFp = _fp.top();
         _fp.pop();
      }

      // we can't overload operator=, because all structures are refcounted and so is incompatible
      virtual void operator()( AstExpAssign& e )
      {
         operator()( e.getValue() );
         operator()( e.getLValue() );
         // TODO handle "var" here
         e.setNodeType( e.getValue().getNodeType()->clone() );

         ensure( e.getLValue().getNodeType(), "compiler error: cannot evaluate expression type" );
         if ( !e.getValue().getNodeType()->isCompatibleWith( *e.getLValue().getNodeType() ) )
         {
            impl::reportTypeError( e.getLocation(), _context, "incompatible types");
            e.setNodeType( new TypeError() );
         }
      }

      virtual void operator()( AstVarSimple& e )
      {
         if ( e.getIsFunctionAddress() )
         {
            std::vector<AstDeclFun*> funcs = getFunctionsFromGlobal( _funcs, e.getName() );
            if ( funcs.size() == 0 )
            {
               impl::reportTypeError( e.getLocation(), _context, "couldn't find the function's adress." );
               e.setNodeType( new TypeError() );
               return;
            }
            if ( funcs.size() > 1 )
            {
               impl::reportTypeError( e.getLocation(), _context, "ambiguous function address: at least 2 functions have the same name. See:" + funcs[ 0 ]->getLocation().toString() );
               e.setNodeType( new TypeError() );
               return;
            }
            e.setNodeType( createTypeFromFunction( funcs[ 0 ] ) );
            e.setFunctionAddress( funcs[ 0 ] );
         } else {
            if ( e.getReference() )
            {
               // if it is a simple variable, we will have a type
               if ( !e.getReference()->getNodeType() )   // check if the node has already been evaluated // TODO check why? - not from different includes: "class Test2{ Test2 tt; int ttt; Test2(){ttt=42;} } Test2 t; t.tt = t; int nn = t.tt.ttt;"
               {
                  operator()( *e.getReference() );
               }
               e.setNodeType( e.getReference()->getNodeType()->clone() );
            } // else we know it is a call exp, so don't do anything
         }
      }

      virtual void operator()( AstThis& e )
      {
         e.setNodeType( e.getReference()->getNodeType()->clone() );
      }

      virtual void operator()( AstVarArray& e )
      {
         operator()( e.getName() );
         operator()( e.getIndex() );
         ensure( e.getIndex().getNodeType(), "tree can't be typed" );
         ensure( e.getName().getNodeType(), "tree can't be typed" );
         if ( !TypeInt( false ).isEqual( *e.getIndex().getNodeType() ) )
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
                  e.setNodeType( funcs[ 0 ]->getNodeType()->clone() );
                  return;
               }

               // it means funcs.size() == 1
               if ( !funcs[ 0 ]->getNodeType() )
               {
                  // if type has not been visited yet... do it (function inclusion order...)
                  operator()( *funcs[ 0 ] );
               }
               e.setNodeType( funcs[ 0 ]->getNodeType()->clone() );
               e.setFunction( funcs[ 0 ] );
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

      bool match( const TypeFunctionPointer& fp, const AstArgs& args )
      {
         if ( fp.getArgs().size() != args.getArgs().size() )
            return false;
         for ( ui32 n = 0; n < fp.getArgs().size(); ++n )
         {
            if ( !args.getArgs()[ n ]->getNodeType()->isCompatibleWith( *fp.getArgs()[ n ] ) )
               return false;
         }
         return true;
      }

      // choices are:
      // - global function
      // - class instanciation
      // - varfield
      virtual void operator()( AstExpCall& e )
      {
         operator()( e.getArgs() );

         _isFunctionBeingCalled = true;
         operator()( e.getName() );
         _isFunctionBeingCalled = false;

         std::vector<AstDeclFun*> funcs;

         // it has to be a function pointer: unfortunately it is dynamic, so we don't know the address of the function yet!
         TypeFunctionPointer* fp = dynamic_cast<TypeFunctionPointer*>( e.getName().getNodeType() );
         if ( fp )
         {
            if ( match( *fp, e.getArgs() ) )
            {
               e.setNodeType( fp->getReturnType().clone() );
               e.setFunctionPointerCall( true );
               return;
            } else {
               impl::reportTypeError( e.getLocation(), _context, "the actual arguments don't mach the function pointer" );
               e.setNodeType( new TypeError() );
               return;
            }
         }


         if ( e.getSimpleName() )
         {
            // we are in the case where we are calling a global function/class constructor
            SymbolTableFuncs::iterator it = _funcs.find( *e.getSimpleName() );
            if ( it != _funcs.end() )
            {
               // use the simple name for the function
               funcs = getMatchingFunctionsFromArgs( it->second.list, e.getArgs() );
            } else {
               if ( e.getInstanciation() )
               {
                  // else we know it is an object, so operator() must be called
                  funcs = getMatchingFunctionsFromArgs( getFunctionsFromClass( *e.getInstanciation(), mvv::Symbol::create( "operator()" ) ), e.getArgs() );
               } else {
                  if ( e.getConstructed() )
                  {
                     // else, it is a class intanciated, check if the constructor is appropriate
                     funcs = getMatchingFunctionsFromArgs( getFunctionsFromClass( *e.getConstructed(), /**e.getSimpleName()*/ e.getConstructed()->getName() ), e.getArgs() );
                  } else {
                     ensure( 0, "compiler error: should not happen..." );
                  }
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

         //
         // TODO: in "construction" => there is a link to find the class in case a typedef on a class and instanciate it...
         // class Test{ int m; Test(int n ){ m = n; }} typedef Test TEST; TEST t = TEST(3); int n = t.m;
         //


         if ( funcs.size() == 0 )
         {
            impl::reportTypeError( e.getName().getLocation(), _context, "no function matching the arguments found" );
            e.setNodeType( new TypeVoid() );
            return;
         }

         if ( funcs.size() > 1 )
         {
            impl::reportTypeMultipleCallableFunction( funcs, _context );
            e.setNodeType( funcs[ 0 ]->getNodeType()->clone() );
            return;
         }

         e.setNodeType( funcs[ 0 ]->getNodeType()->clone() );
         e.setFunctionCall( funcs[ 0 ] );

         // check if the parameter must be referenced, it can be!
         AstArgs::Args::iterator itarg = e.getArgs().getArgs().begin(); // we know the number of parameters given is compatible with the one in the function decl.
         AstDeclVars::Decls::iterator it = funcs[ 0 ]->getVars().getVars().begin();
         for ( ; itarg != e.getArgs().getArgs().end(); ++it, ++itarg )
         {
            if ( ( *it )->getType().isAReference() && !checkValidReferenceInitialization( **itarg )  )
            {
               impl::reportTypeError( (*it)->getLocation(), _context, "argument cannot be referenced" );
            }
         }
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
         std::vector<AstDecl*> members = getDeclFromClass( *type->getDecl(), e.getName() );
         if ( members.size() == 0 )
         {
            impl::reportTypeError( e.getLocation(), _context, "error can't find member in class definition" );
            e.setNodeType( new TypeError() );
            return;
         }
         e.setPointee( members[ 0 ] );

         // handle function member pointer
         TypeNamed* vs = dynamic_cast<TypeNamed*>( e.getField().getNodeType() );
         AstDeclFun* fs = dynamic_cast<AstDeclFun*>( members[ 0 ] );
         if ( vs && fs && !_isFunctionBeingCalled )
         {
            if ( members.size() > 1 )
               impl::reportTypeError( e.getLocation(), _context, "member function pointer is ambiguous, see: " + members[ 0 ]->getLocation().toString() + " and " + members[ 1 ]->getLocation().toString() );

            // we have a function pointer!
            delete e.getField().getNodeType();
            e.getField().setNodeType( createTypeFromFunction( &fs[ 0 ], type ) );
            e.setNodeType( e.getField().getNodeType()->clone() );

            e.setMemberFunction( type->getDecl(), fs );
            /*
            vs->getDecl()->setClassMember( type->getDecl() );
            vs->getDecl()->setFunctionAddress( fs );
            vs->getDecl()->setIsFunctionAddress( true );
            */
         } else {
            e.setNodeType( members[ 0 ]->getNodeType()->clone() );
         }

         // Note: handle "this" in the interpreter
      }

      virtual void operator()( AstType& e )
      {
         if ( e.getNodeType() )
         {
            // this node has already been manually parsed before
            return;
         }

         switch ( e.getType() )
         {
         case AstType::CMP_FLOAT:
            e.setNodeType( new TypeFloat( e.isAReference() ) );
            break;

         case AstType::CMP_INT:
            e.setNodeType( new TypeInt( e.isAReference() ) );
            break;

         case AstType::STRING:
            e.setNodeType( new TypeString( e.isAReference() ) );
            break;

         case AstType::VOID:
            e.setNodeType( new TypeVoid() );
            break;

         case AstType::SYMBOL:
            ensure( e.getReference(), "compiler error: can't find a link on a symbol" );

            if ( e.getReference()->getNodeType() == 0)
            {
               // we manually need to type this node as it is due to mutual inclusion, with one
               // of the header that has not been parsed yet
               VisitorType visitor( _context, _vars, _funcs, _classes );
               visitor( *e.getReference() );
            }
            if ( e.getReference()->getNodeType() == 0)
            {
               impl::reportTypeError( e.getLocation(), _context, "error: can't type this symbol" );
               e.setNodeType( new TypeError() );
               return;
            }
            e.setNodeType( e.getReference()->getNodeType()->clone() );
            if ( e.isAReference() )
            {
               // we need to check if this is a reference
               e.setIsAReference( true );
               e.getNodeType()->setReference( true );
            }
            break;

         default:
            ensure( 0, "compiler error: primitive type not handled" );
         }

         if ( e.isArray() )
         {
            ui32 dim = 1;
            if ( e.getSize() )
            {
               // we have this case: int a[ 4 ][ 5 ] => dim = 2
               dim = (ui32)e.getSize()->size();
            } else {
      
               TypeArray* type = dynamic_cast<TypeArray*>( e.getNodeType() );
               if ( type )
               {
                  dim += type->getDimentionality();
                  e.setNodeType( new TypeArray( dim, type->getRoot(), false ) );
                  return;
               }
            }

            e.setNodeType( new TypeArray( dim, *e.getNodeType(), false ) );
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
         if ( e.getNodeType() )
         {
            // this node has been manually typed (i.e. manual inclusion)
            return;
         }

         _fp.push( _currentFp );
         _currentFp = 0;

         _defaultClassPath.push_back( e.getName() );
         e.setNodeType( new TypeNamed( &e, false ) );
         operator()( e.getDeclarations() );
         _defaultClassPath.pop_back();

         _currentFp = _fp.top();
         _fp.pop();

         // check if a class has cyclic dependencies
         std::set<AstDeclClass*> visited;
         bool check = _checkCyclicDependencies( &e, visited );
         if ( check )
         {
            impl::reportTypeError( e.getLocation(), _context, "instanciation of a member variable that has cyclic type dependencies" );
         }
      }

      virtual void operator()( AstDeclFun& e ) 
      {
         // beacuse we are modifying the FP at runtime when we enter a function, we need to 
         _fp.push( _currentFp );
         _currentFp = ( e.getMemberOfClass() != 0 );  // if we are in a class, we need to shift decl var as FP[0]=object

         if ( e.getNodeType() )
         {
            // this node has been manually typed (i.e. manual inclusion)
            return;
         }

         bool isAConstructor = false;
         if ( e.getType() )
         {
            operator()( *e.getType() );

            // TODO:REMOVE special rule if the type of a function is an array! int[] test() => type returned is int instead of int[], so we need to update it!
            /*if ( e.getType()->isArray() )
            {
               Type* type = e.getType()->getNodeType();  // don't need to clone, just a ref for an array
               TypeArray* typeA = new TypeArray(0, *type, false );
               e.setNodeType( typeA ); // TODO: add reference is this feature is added later on...
            } else {*/
               e.setNodeType( e.getType()->getNodeType()->clone() );
            //}
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
               e.setNodeType( e.getMemberOfClass()->getNodeType()->clone() );
               isAConstructor = true;

               if ( e.getName() == e.getMemberOfClass()->getDestructorName() )
               {
                  if ( e.getMemberOfClass()->getDestructor() )
                  {
                     // destructor already declared
                     impl::reportTypeError( e.getLocation(), _context, "a destructor has already been declared for this class" );
                     e.setNodeType( new TypeError() );
                     return;
                  }

                  // we found a destructor!
                  e.getMemberOfClass()->setDestructor( &e );

               } else if ( e.getName() != e.getMemberOfClass()->getName() )
               {
                  impl::reportTypeError( e.getLocation(), _context, "constructor and destructor must have the same name than class, or missing function type" );
                  e.setNodeType( new TypeError() );
                  return;
               }
            }
         }

         if ( e.getVars().getVars().size() )
         {
            _isInFunctionDeclaration = true;
            operator()( e.getVars() );
            _isInFunctionDeclaration = false;
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

         _currentFp = _fp.top();
         _fp.pop();
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
            e.setNodeType( e.getReturnValue()->getNodeType()->clone() );
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

         if ( e.getFunction()->getType()->isAReference() && e.getReturnValue() ) // if there is no return type, no ref!
         {
            // if function type is ref, check we have a valid expression
            bool isRef = checkValidReferenceInitialization( *e.getReturnValue() );
            if ( !isRef )
            {
               impl::reportTypeError( e.getLocation(), _context, "function return type must be a referencable value (variable, function with reference type)" );
            }
            e.getNodeType()->setReference( true );
         }
      }

      // check a variable with reference has a correct initialization
      static bool checkValidReferenceInitialization( AstExp& e )
      {
         //
         // TODO update the type system:
         //  "Type" class must have ConstInt, ConstFloat, ConstString, isNotComposed()...
         //

         AstVar* v = dynamic_cast<AstVar*>( &e );
         if ( v )
         {
            // we are aliasing a reference
            return true;
         }
         return e.getNodeType()->isReference();
      }

      virtual void operator()( AstDeclVar& e )
      {
         if ( e.getRuntimeIndex() == -1 )
         {
            e.setRuntimeIndex( _currentFp );
         }
         ++_currentFp;  // we don't reassign index, but we still need to take into account the space of this global

         // we first must visite the type!
         operator()( e.getType() );

         ensure( e.getType().getNodeType(), "tree can't be typed" );
         if ( TypeVoid().isCompatibleWith( *e.getType().getNodeType() ) )
         {
            impl::reportTypeError( e.getLocation(), _context, "variable type must not be void" );
         }

         if ( e.getType().isArray() )
         {
            // check the type held is not a reference...
            if ( !_isInFunctionDeclaration && hasArrayInnerReference( *e.getType().getNodeType() ) && _defaultClassPath.size() == 0 && !e.getDeclarationList() ) // we don't check reference init if in function prototype -> must be done in the call // we can init a ref in a class at its first use
            {
               impl::reportTypeError( e.getLocation(), _context, "type with reference must be initialized" );
               return;
            }

            // check there is a default contructor
            TypeArray* arrayType = dynamic_cast<TypeArray*>( e.getType().getNodeType() );
            ensure( arrayType, "compiler error: this is an array, so it must have array type!" );
            const TypeNamed* ty = dynamic_cast<const TypeNamed*>( &arrayType->getRoot() );
            if ( ty && !e.getDeclarationList() )
            {
               AstDeclFun* fn = checkDefaultConstructible( ty, e.getType().getLocation() );
               if ( fn )
               {
                  e.setConstructorCall( fn );
               }
            }
            
            // else carry on...
            if ( e.getType().getSize() && e.getType().getSize()->size() > 0 )
            {
               for ( size_t n = 0; n < e.getType().getSize()->size(); ++n )
               {
                  operator()( *( (*e.getType().getSize())[ n ] ) );

                  ensure( (*e.getType().getSize())[ n ]->getNodeType(), "must have a dimensionality >= 1" );
                  if ( !TypeInt( false ).isEqual( *(*e.getType().getSize())[ n ]->getNodeType() ) )
                  {
                     impl::reportTypeError( (*e.getType().getSize())[ n ]->getLocation(), _context, "variable type must be of integer type" );
                  }
               }
            }
            ensure( e.getType().getNodeType(), "can't type properly a tree" );
            //ui32 size = static_cast<ui32>( ( e.getType().getSize() && e.getType().getSize()->size() ) ? e.getType().getSize()->size() : 1 );

            Type& typeArray = *e.getType().getNodeType();
            e.setNodeType( typeArray.clone() );
         } else {
            e.setNodeType( e.getType().getNodeType()->clone() );
         }

         TypeNamed* ty = dynamic_cast<TypeNamed*>( e.getType().getNodeType() );
         if ( e.getInit() )
         {
            operator()( *e.getInit() );
            ensure( e.getInit()->getNodeType(), "tree can't be typed" );
            if ( !e.getInit()->getNodeType()->isCompatibleWith( *e.getNodeType() ) )
            {
               impl::reportTypeError( e.getInit()->getLocation(), _context, "variable and asignment types are not compatible" );
            }

            
            if ( e.isClassMember() && ty && !e.getInit()->getNodeType()->isEqual( TypeNil() ) )
            {
               // prepare cyclic dependency check
               e.isClassMember()->addInstanciatedType( ty->getDecl() );
            }
         } else {
            // if a reference, then it must be initialized
            if ( !_isInFunctionDeclaration && e.getType().getNodeType()->isReference() && _defaultClassPath.size() == 0 && !e.getDeclarationList() ) // we don't check reference init if in function prototype -> must be done in the call // we can init a ref in a class at its first use
            {
               impl::reportTypeError( e.getLocation(), _context, "type with reference must be initialized" );
               return;
            }

            if ( e.isClassMember() && ty )
            {
               // prepare cyclic dependency check
               e.isClassMember()->addInstanciatedType( ty->getDecl() );
            }

            if ( e.getObjectInitialization() )
            {
               // visit the arguments first
               operator()( *e.getObjectInitialization() );

               // if object initialization, check we can construct the object
               ensure( ty->getDecl(), "Internal compiler error" );

               std::vector<AstDeclFun*> funs = getMatchingFunctionsFromArgs( getFunctionsFromClass( *ty->getDecl(), ty->getDecl()->getName() ), *e.getObjectInitialization() );
               if ( funs.size() == 1 )
               {
                  e.setConstructorCall( funs[ 0 ] );
                  return;
               } else {
                  if ( funs.size() > 1 )
                  {
                     // ambiguous call
                     impl::reportTypeError( e.getLocation(), _context, "ambiguous constructor call to " + std::string( ty->getDecl()->getName().getName() ) );
                     e.setNodeType( new TypeError() );
                     return;
                  } else {
                     impl::reportTypeError( e.getLocation(), _context, "no constructor found in class definition that matches this prototype" );
                     e.setNodeType( new TypeError() );
                     return;
                  }
               }
            } else {
               if ( !_isInFunctionDeclaration )
               {
                  // if we are in the declaration of a function, we are not constructing an object... so we shouldn't test this
                  AstDeclFun* fn = checkDefaultConstructible( ty, e.getType().getLocation() );
                  if ( fn )
                  {
                     e.setConstructorCall( fn );
                  }
               }
            }

            if ( e.getDeclarationList() )
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
               } else if ( ty )
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

         if ( e.getInit() && e.getInit()->getNodeType() && e.getType().getNodeType()->isReference() ) // in case function declaration, there is no init, but this error is already catched in binding, so it is correct to not check
         {
            if ( !checkValidReferenceInitialization( *e.getInit() ) )
            {
               // wrong init of a variable with reference...
               impl::reportTypeError( e.getLocation(), _context, "variable with reference must be initialized with another variable or method returning reference" );
            }
         }

         // check if this class member need to be initialized before hand (array, default value & class+constructor)
         // in case of ref, the init will occur at the firs affectation
         if ( !e.getType().isAReference() && e.isClassMember() && ( e.getInit() || e.getConstructorCall() || e.getType().isArray() ) )
         {
            e.isClassMember()->addMemberToInitialize( &e );
         }
      }

      virtual void operator()( AstExpSeq& e )
      {
         operator()( e.getExp() );
         e.setNodeType( e.getExp().getNodeType()->clone() );
      }

      virtual void operator()( AstTypeField& e )
      {
         operator()( e.getField() );
         AstDeclClass* c = dynamic_cast<AstDeclClass*>( e.getReference() );
         if ( c )
         {
            e.setNodeType( new TypeNamed( c, false ) );
         } else {
            AstTypedef* t = dynamic_cast<AstTypedef*>( e.getReference() );
            if ( t )
            {
               e.setNodeType( t->getType().getNodeType()->clone() );
            } else {
               ensure( 0, "unreachable!!" );
            }
            /*
            ensure( t, "compiler error: precondition" );
            AstDeclClass* c2 = dynamic_cast<AstDeclClass*>( t->getType().getReference() );
            ensure( c2, "TODO: handle typedef on a typedef..." );
            e.setNodeType( new TypeNamed( c2, false ) );*/
         }
      }

      virtual void operator()( AstExpTypename& e )
      {
         operator()( e.getType() );
         if ( e.getArgs().getArgs().size() )
         {
            operator()( e.getArgs() );
         }
         e.setNodeType( new TypeNamed( e.getReference(), false ) );

         // check it can be constructed
         // if object initialization, check we can construct the object
         assert( e.getReference() );
         std::vector<AstDeclFun*> funs = getMatchingFunctionsFromArgs( getFunctionsFromClass( *e.getReference(), e.getReference()->getName() ), e.getArgs() );
         if ( funs.size() == 1 )
         {
            e.setConstructor( funs[ 0 ] );
            return;
         } else {
            if ( funs.size() > 1 )
            {
               // ambiguous call
               impl::reportTypeError( e.getLocation(), _context, "ambiguous constructor call to " + std::string( e.getReference()->getName().getName() ) );
               e.setNodeType( new TypeError() );
               return;
            } else {
               impl::reportTypeError( e.getLocation(), _context, "no matching constructor call for the class " + std::string( e.getReference()->getName().getName() ) );
               e.setNodeType( new TypeError() );
               return;
            }
         }
      }

      virtual void operator()( AstTypedef& e )
      {
         bool isVisited = e.getVisited();
         if ( isVisited )
         {
            // do nothing: we already typed this node
         } else {
            e.setVisited();
            operator()( e.getType() );

            // we don't want to have a typedef on a typedef... so unloop it!
            Type* type = e.getType().getNodeType();
            ensure( type, "compiler error: tree not typed!" );
            e.setNodeType( type->clone() );
         }
      }

      virtual void operator()( AstFunctionType& e )
      {
         operator()( e.getType() );
         operator()( e.getArgs() );

         std::vector<Type*> args( e.getArgs().getVars().size() );
         for ( ui32 n = 0; n < args.size(); ++n )
         {
            args[ n ] = e.getArgs().getVars()[ n ]->getNodeType();
         }

         e.setNodeType( new TypeFunctionPointer( e.isAReference(), e.getType().getNodeType(), args ) );
      }

      /**
       @brief check if a class has cyclic dependencies
       */
      static bool _checkCyclicDependencies( AstDeclClass* currentClass, std::set<AstDeclClass*>& visited )
      {
         if ( visited.find( currentClass ) != visited.end() )  // we already visited it...
            return true;
         visited.insert( currentClass );


         bool res = false;
         const std::set<AstDeclClass*>& classes = currentClass->getInstanciatedType();
         for ( std::set<AstDeclClass*>::const_iterator it = classes.begin(); it != classes.end(); ++it )
         {
            res = res | _checkCyclicDependencies( *it, visited );
         }  
         return res;
      }

      virtual void operator()( Ast& e )
      {
         e.accept( *this );
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

   private:
      // disabled
      VisitorType& operator=( VisitorType& );
      VisitorType( const VisitorType& );


   private:
      std::vector<const Type*>   _returnType;


      ParserContext&      _context;
      SymbolTableVars&    _vars;
      SymbolTableFuncs&   _funcs;
      SymbolTableClasses& _classes;
      bool                _isInFunctionDeclaration;
      bool                _isFunctionBeingCalled;
      std::vector<mvv::Symbol>   _defaultClassPath;

      std::stack<ui32>    _fp;   // we locally need to compute the frame pointer & update reference variable for all declared variables in class & function (this is local)
      ui32                _currentFp;
   };
}
}

#endif