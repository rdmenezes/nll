#ifndef MVV_PARSE_EVALUATE_VISITOR_H_
# define MVV_PARSE_EVALUATE_VISITOR_H_

# include "visitor.h"
# include "ast-files.h"

namespace mvv
{
namespace parser
{
   class RuntimeException : public std::exception
   {
   public:
      RuntimeException( const char* c ) : std::exception( c )
      {}
   };

   namespace impl
   {
      inline void runtimeError( const YYLTYPE& loc, mvv::parser::ParserContext& context, const std::string& msg )
      {
         std::stringstream ss;
         ss << loc << msg << std::endl;
         context.getError() << ss.str() << mvv::parser::Error::RUNTIME;

         throw RuntimeException( ss.str().c_str() );
      }
   }

   /**
    @brief Defines a visitor visiting all the nodes but doing nothing
    @note this is usefull if a visitor is only handling a limited number of nodes
    @note TODO for class type efficiency: before doing any computations on a class,
          we must fetch all the member to the class declaration, do processing,
          then copy back the results, which is extremly inefficient for big structures...
    */
   class VisitorEvaluate : public Visitor
   {
   public:
      VisitorEvaluate( ParserContext& context,
                       SymbolTableVars& vars,
                       SymbolTableFuncs& funcs,
                       SymbolTableClasses& classes ) : _context( context ), _vars( vars ), _funcs( funcs ), _classes( classes )
      {
      }

      virtual void operator()( AstInt& e )
      {
         RuntimeValue& val = e.getRuntimeValue();
         val.setType( RuntimeValue::INT, e.getNodeType() );
         val.intval = e.getValue();
      }

      virtual void operator()( AstThis& e )
      {
         AstDeclClass* decl = dynamic_cast<AstDeclClass*>( e.getReference() );
         ensure( decl, "compiler error: this reference is incorrect" );
         e.getRuntimeValue() = decl->getRuntimeObjectSource();
      }

      virtual void operator()( AstNil& e )
      {
         RuntimeValue& val = e.getRuntimeValue();
         val.setType( RuntimeValue::NIL, e.getNodeType() );
      }

      virtual void operator()( AstFloat& e )
      {
         RuntimeValue& val = e.getRuntimeValue();
         val.setType( RuntimeValue::FLOAT, e.getNodeType() );
         val.floatval = e.getValue();
      }

      virtual void operator()( AstString& e )
      {
         RuntimeValue& val = e.getRuntimeValue();
         val.setType( RuntimeValue::STRING, e.getNodeType() );
         val.stringval = e.getValue();
      }

      virtual void operator()( AstOpBin& e )
      {
         operator()( e.getLeft() );
         operator()( e.getRight() );

         // special case: named type with operator == and !=, we need to check the refcount pointer
         TypeNamed* tl = dynamic_cast<TypeNamed*>( e.getLeft().getNodeType() );
         if ( tl )
         {
            if ( e.getOp() == AstOpBin::EQ )
            {
               e.getRuntimeValue().setType( RuntimeValue::INT, 0 );
               e.getRuntimeValue().intval = e.getLeft().getRuntimeValue().vals.getDataPtr() == e.getRight().getRuntimeValue().vals.getDataPtr();
               return;
            } else if ( e.getOp() == AstOpBin::NE )
            {
               e.getRuntimeValue().setType( RuntimeValue::INT, 0 );
               e.getRuntimeValue().intval = e.getLeft().getRuntimeValue().vals.getDataPtr() != e.getRight().getRuntimeValue().vals.getDataPtr();
               return;
            }

            // fetch object (in case A + B, with A a class and defined member operator+( B )
            assert( e.getFunctionCall() ); // if object but not operator == != and no compile error => we must call a function
            if ( e.getFunctionCall()->getMemberOfClass() )
            {
               e.getFunctionCall()->getMemberOfClass()->getRuntimeValue() = e.getLeft().getRuntimeValue();
            }
         }

         // other cases: we need to call a function
         AstArgs args( e.getLocation(), false );
         ensure( e.getFunctionCall(), "compiler error: no function to call" );
         if ( e.isBinOpInClass() )
         {
            // bin op declared in class, we need to fecth the source object, decrease one arg count
            assert( e.getFunctionCall()->getMemberOfClass() ); // must have a class ref as it is a class member
            e.getFunctionCall()->getMemberOfClass()->getRuntimeObjectSource() = e.getLeft().getRuntimeValue();
            _runtimeValueToClass( e.getLeft().getRuntimeValue(), *e.getFunctionCall()->getMemberOfClass() );
            args.insert( &e.getRight() );
         } else {
            args.insert( &e.getLeft() );
            args.insert( &e.getRight() );
         }
         _callFunction( e, args, *e.getFunctionCall() );
      }

      virtual void operator()( AstIf& e )
      {
         operator()( e.getCondition() );
         operator()( e.getThen() );

         if ( e.getElse() )
         {
            operator()( *e.getElse() );
         }

         // TODO
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

         // we need to find the reference of the lvalue and update it's runtime value
         ensure( e.getLValue().getReference(), "compiler error: lvalue not linked to a reference" );
         e.getRuntimeValue() = e.getValue().getRuntimeValue();
         e.getLValue().getReference()->getRuntimeValue() = e.getValue().getRuntimeValue();

         // if type or array, we must update the structure
         TypeNamed* decl = dynamic_cast<TypeNamed*>( e.getLValue().getReference()->getNodeType() );
         if ( decl )
         {
            // update the result
            //_classToRuntimeValue( *decl->getDecl(), e.getLValue().getReference()->getRuntimeValue() );
         }
      }

      virtual void operator()( AstVarSimple& e )
      {
         ensure( e.getReference(), "compiler error: can't find variable declaration" );
         e.getRuntimeValue() = e.getReference()->getRuntimeValue();
      }

      virtual void operator()( AstVarArray& e )
      {
         operator()( e.getIndex() );
         operator()( e.getName() );

         if ( e.getName().getRuntimeValue().vals.getDataPtr() == 0 )
         {
            throw RuntimeException( "uninitialized array" );
         }

         // ensure the variable is an array... else we missed something in the type visitor!
         assert( e.getName().getRuntimeValue().type == RuntimeValue::ARRAY ); // "compiler error: must be an array" 
         int index = e.getIndex().getRuntimeValue().intval;

         if ( index < 0 || static_cast<unsigned int>( index ) >= (*e.getName().getRuntimeValue().vals).size() )
         {
            // out of bound error
            std::stringstream msg;
            msg << "out of bound index: array size=" << (*e.getName().getRuntimeValue().vals).size() << " index=" << index;
            impl::runtimeError( e.getIndex().getLocation(), _context, msg.str() );
         }

         // just get the value
         e.getRuntimeValue() = (*e.getName().getRuntimeValue().vals)[ index ];
      }

      virtual void operator()( AstVarField& e )
      {
         AstDeclClass* c = dynamic_cast<AstDeclClass*>( e.getReference() );
         assert( c ); /* Compiler error: the reference must be a class! */

         ui32 index = 0;
         AstDecls::Decls::iterator it = c->getDeclarations().getDecls().begin();
         for ( ; it != c->getDeclarations().getDecls().end(); ++it )
         {
            if ( (*it)->getName() == e.getName() )
            {
               break;
            }
            if ( dynamic_cast<AstDeclVar*>( *it ) )
            {
               ++index;
            }
         }
         assert( it != c->getDeclarations().getDecls().end() ); // compiler error: we must be able to find the var name!
         /*
         // TODO check assert
         assert( c->getRuntimeValue().type == RuntimeValue::TYPE );
         assert( c->getRuntimeValue().vals.getDataPtr() && ( *c->getRuntimeValue().vals ).size() > index );
         */
         e.getRuntimeValue() = ( *e.getField().getReference()->getRuntimeValue().vals )[ index ];
         operator()( e.getField() );
      }

      virtual void operator()( AstType& )
      {
         // the definition has been hadnled before...
      }

      virtual void operator()( AstDecls& )
      {
         // the definition has been hadnled before...
      }

      virtual void operator()( AstDeclVars& )
      {
         // the definition has been hadnled before...
      }

      virtual void operator()( AstDeclClass& )
      {
         // the definition has been hadnled before...
      }

      virtual void operator()( AstDeclFun& ) 
      {
         // the definition has been hadnled before...
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
         if ( e.getReturnValue() )
         {
            operator()( *e.getReturnValue() );
            e.getFunction()->getRuntimeValue() = e.getReturnValue()->getRuntimeValue();
         }
      }

      virtual void operator()( AstImport& )
      {
         // nothing to do
      }

      virtual void operator()( AstInclude& )
      {
         // nothing to do
      }

      /**
       @brief Evaluate the function fun, with arguments args and save the result in nodeToEval
       @note if the function to evalaute is a member function, the source object must be
             fecthed beforehand
       */
      void _callFunction( Typable& nodeToEval, AstArgs& args, AstDeclFun& fun, bool skipFirstArg = false )
      {
         // fetch the arguments
         AstDeclVars::Decls::iterator itVar = fun.getVars().getVars().begin();
         AstArgs::Args::iterator itArg = args.getArgs().begin();
         if ( skipFirstArg )
         {
            // skip the first argument (for example is bin op declared in class)
            ++itArg;
         }

         for ( ; itArg != args.getArgs().end(); ++itVar, ++itArg )
         {
            assert( itVar != fun.getVars().getVars().end() ); // compiler error: number of arguments must be compatible with function declaration
            (*itVar)->getRuntimeValue() = (*itArg)->getRuntimeValue();
         }
         while ( itVar != fun.getVars().getVars().end() )
         {
            // if we are not done, we must first evaluate the default value and fetch it to the argument
            ensure( (*itVar)->getInit(), "compiler error: there must be a default value!" );
            operator()( *(*itVar)->getInit() );
            (*itVar)->getRuntimeValue() = (*itVar)->getInit()->getRuntimeValue();

            ++itVar;
         }

         // call the function
         if ( fun.getBody() )
         {
            operator()( *fun.getBody() );
            nodeToEval.getRuntimeValue() = fun.getRuntimeValue();
         } else if ( fun.getImportedFunction() )
         {
            // construct arg list
            std::vector<RuntimeValue*> vals( fun.getVars().getVars().size() );
            int n = 0;
            for ( AstDeclVars::Decls::iterator it = fun.getVars().getVars().begin(); it != fun.getVars().getVars().end(); ++it, ++n )
            {
               vals[ n ] = &((*it)->getRuntimeValue());
            }
            nodeToEval.getRuntimeValue() = fun.getImportedFunction()->run( vals );
         } else {
            throw RuntimeException( "unable to find function implementation" );
         }
      }


      virtual void operator()( AstExpCall& e )
      {
         // first, evaluate the arguments
         operator()( e.getArgs() );

         /*
         // if the function is actually a member function, fetch the object
         ensure( e.getFunctionCall(), "compiler error: function call not set" );
         if ( e.getConstructed() )
         {
            // TODO
            //e.getConstructed()->getRuntimeObjectSource() = construct the object from the members...
         } else if ( e.getFunctionCall() )
         {
            // TODO
         }*/

         _callFunction( e, e.getArgs(), *e.getFunctionCall() );
      }

      /**
       @brief Construct an array with the specific size
       */
      static void _buildArray( Typable& nodeToType, std::vector<AstExp*>& size )
      {
#ifndef NDEBUG
         for ( ui32 n = 0; n < size.size(); ++n )
         {
            assert( size[ n ]->getRuntimeValue().type == RuntimeValue::INT );
         }
#endif

         // TODO build array
      }

      virtual void operator()( AstDeclVar& e )
      {
         if ( e.getInit() )
         {
            operator()( *e.getInit() );
            e.getRuntimeValue() = e.getInit()->getRuntimeValue();
            return;
         } else if ( e.getDeclarationList() )
         {
            operator()( *e.getDeclarationList() );

            // check we have an array
            ensure( e.getType().isArray(), "compiler error: current implementation: it must be an array" );

            // copy the value from the initialization list
            e.getRuntimeValue().setType( RuntimeValue::ARRAY, e.getNodeType() );
            e.getRuntimeValue().vals = platform::RefcountedTyped< RuntimeValues >( new RuntimeValues( e.getDeclarationList()->getArgs().size() ) );

            ui32 n = 0;
            for ( AstArgs::Args::iterator itArg = e.getDeclarationList()->getArgs().begin(); itArg != e.getDeclarationList()->getArgs().end(); ++itArg, ++n )
            {
               (*e.getRuntimeValue().vals)[ n ] = (*itArg)->getRuntimeValue();
            }
         }

         if ( e.getType().isArray() )
         {
            if ( e.getType().getSize() && e.getType().getSize()->size() > 0 )
            {
               for ( size_t n = 0; n < e.getType().getSize()->size(); ++n )
               {
                  operator()( *( (*e.getType().getSize())[ n ] ) );
               }
            }

            if ( e.getType().getSize() )
            {
               _buildArray( e, *e.getType().getSize() );
            }
            return;
         }

         if ( e.getConstructorCall() )
         {
            if ( e.getObjectInitialization() )
            {
               operator()( *e.getObjectInitialization() );
               _callFunction( e, *e.getObjectInitialization(), *e.getConstructorCall() );
            } else {
               AstArgs emptyArgs( e.getLocation() );
               _callFunction( e, emptyArgs, *e.getConstructorCall() );
            }
            assert( e.getConstructorCall()->getMemberOfClass() ); // error

            e.getRuntimeValue().vals = platform::RefcountedTyped<RuntimeValues>( new RuntimeValues( e.getConstructorCall()->getMemberOfClass()->getMemberVariableSize() ) );
            _classToRuntimeValue( *e.getConstructorCall()->getMemberOfClass(), e.getRuntimeValue() );
            return;
         }
      }

      /**
       @brief assumes the runtime r has enough space to store the values
       */
      static void _classToRuntimeValue( AstDeclClass& c, RuntimeValue& r )
      {
         r.setType( RuntimeValue::TYPE, c.getNodeType() );
         if ( (*r.vals).size() != c.getMemberVariableSize() )
         {
            (*r.vals).resize( c.getMemberVariableSize() );
         }

         ui32 index = 0;
         for ( AstDecls::Decls::iterator it = c.getDeclarations().getDecls().begin(); it != c.getDeclarations().getDecls().end(); ++it )
         {
            if ( dynamic_cast<AstDeclVar*>( *it ) )
            {
               (*r.vals)[ index ] = (*it)->getRuntimeValue();
               ++index;
            }
         }
      }

      /**
       @brief fetch the runtime value of a class to its definition
       */
      static void _runtimeValueToClass( RuntimeValue& r, AstDeclClass& c )
      {
         ui32 index = 0;
         assert( (*r.vals).size() == c.getMemberVariableSize() ); // compiler problem, the object must be of the same memory layout
         for ( AstDecls::Decls::iterator it = c.getDeclarations().getDecls().begin(); index != c.getMemberVariableSize(); ++it )
         {
            assert( it != c.getDeclarations().getDecls().end() ); // compiler problem
            if ( dynamic_cast<AstDeclVar*>( *it ) )
            {
               (*it)->getRuntimeValue() = (*r.vals)[ index ];
               ++index;
            }
         }
      }

      virtual void operator()( AstExpSeq& e )
      {
         operator()( e.getExp() );
      }

      virtual void operator()( AstTypeField& e )
      {
         operator()( e.getField() );
      }

      virtual void operator()( AstExpTypename& e )
      {
         operator()( e.getType() );
         if ( e.getArgs().getArgs().size() )
         {
            operator()( e.getArgs() );
         } 
      }

      virtual void operator()( Ast& e )
      {
         e.accept( *this );
      }

   private:
      // disabled copy
      VisitorEvaluate( const VisitorEvaluate& );
      VisitorEvaluate& operator=( const VisitorEvaluate& );


   private:
      ParserContext&       _context;
      SymbolTableVars&     _vars;
      SymbolTableFuncs&    _funcs;
      SymbolTableClasses&  _classes;
   };
}
}

#endif