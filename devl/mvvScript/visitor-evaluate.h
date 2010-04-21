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

   inline void unstack( RuntimeValues& stack, ui32 nb )
   {
      while ( nb-- )
      {
         stack.pop_back();
      }
   }

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
    @brief Evaluate a fully annotated Ast.

    ResultRegister: is used to store indermediate results

    Frame pointer: points to the current frame. example : int ( int, int )
                  => stack growing direction
                   int int
                   ^
                   |
                   FP
                   It is only changed when a function is called
    Stack frame: When a function run/finish we need to push the current FP/then pop

    AstDeclVar runtimeIndex is relative to the current frame pointer, ex: runtimeIndex = 3, FP = 100, index in frame = FP + runtimeIndex
    */
   class VisitorEvaluate : public Visitor
   {
   public:
      VisitorEvaluate( ParserContext& context,
                       SymbolTableVars& vars,
                       SymbolTableFuncs& funcs,
                       SymbolTableClasses& classes,
                       RuntimeEnvironment& env ) : _context( context ), _vars( vars ), _funcs( funcs ), _classes( classes ), _env( env )
      {
         // TODO: set to zero FP only when run, not constructed (i.e. run multiple times...)
         env.framePointer = 0;

         // reinit the stack frame
         env.stackFrame = std::stack<ui32>();
         env.stackFrame.push( 0 );

      }

      virtual void operator()( AstInt& e )
      {
         _env.resultRegister.setType( RuntimeValue::INT );
         _env.resultRegister.intval = e.getValue();
      }

      virtual void operator()( AstThis& e )
      {
         // "this" is stored at FP[0]
         _env.resultRegister.setType( RuntimeValue::TYPE );
         _env.resultRegister = _env.stack[ _env.framePointer ];
      }

      virtual void operator()( AstNil& e )
      {
         _env.resultRegister.setType( RuntimeValue::NIL );
      }

      virtual void operator()( AstFloat& e )
      {
         _env.resultRegister.setType( RuntimeValue::FLOAT );
         _env.resultRegister.floatval = e.getValue();
      }

      virtual void operator()( AstString& e )
      {
         _env.resultRegister.setType( RuntimeValue::STRING );
         _env.resultRegister.stringval = e.getValue();
      }

      virtual void operator()( AstOpBin& e )
      {
         RuntimeValues vals( 2 );
         
         operator()( e.getLeft() );
         vals[ 0 ] = _env.resultRegister;

         operator()( e.getRight() );
         vals[ 1 ] = _env.resultRegister;


         // check we have a specific operator== or !=
         RuntimeValue& ur0 = unref( vals[ 0 ] );
         RuntimeValue& ur1 = unref( vals[ 1 ] );
         if ( ur0.type == RuntimeValue::TYPE || ur0.type == RuntimeValue::ARRAY )
         {
            if ( e.getOp() == AstOpBin::EQ )
            {
               _env.resultRegister.type = RuntimeValue::INT;
               _env.resultRegister.intval = ur0.vals.getDataPtr() == ur1.vals.getDataPtr();
               return;
            } else if ( e.getOp() == AstOpBin::NE )
            {
               _env.resultRegister.type = RuntimeValue::INT;
               _env.resultRegister.intval = ur0.vals.getDataPtr() != ur1.vals.getDataPtr();
               return;
            }
         }

         _callFunction( *e.getFunctionCall(), vals );
      }

      virtual void operator()( AstIf& e )
      {
         /*
         operator()( e.getCondition() );
         operator()( e.getThen() );

         if ( e.getElse() )
         {
            operator()( *e.getElse() );
         }

         // TODO
         */
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
         RuntimeValue val = _env.resultRegister;

         operator()( e.getLValue() );

         assert( _env.resultRegister.type == RuntimeValue::REF && _env.resultRegister.ref ); // compiler error, we are expecting a reference

         if ( e.getLValue().getNodeType()->isReference() )
         {
            // derefernce double...
            assert( _env.resultRegister.ref->type == RuntimeValue::REF );
            *_env.resultRegister.ref->ref = val;
         } else {
            *_env.resultRegister.ref = val;
         }
      }

      virtual void operator()( AstVarSimple& e )
      {
         AstDeclVar* v = dynamic_cast<AstDeclVar*>( e.getReference() );
         assert( v ); // compiler error if this is not a decl
         ui32 index = _env.framePointer + v->getRuntimeIndex();
         assert( index < _env.stack.size() ); // compiler error if the stack size is wrong
         
         // we create a ref on the declaration
         _env.resultRegister.type = RuntimeValue::REF;
         _env.resultRegister.ref = &_env.stack[ index ];
      }

      virtual void operator()( AstVarArray& e )
      {
         /*
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
         */
      }

      virtual void operator()( AstVarField& e )
      {
         /*
         AstDeclClass* c = dynamic_cast<AstDeclClass*>( e.getReference() );
         assert( c ); 

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
         
         e.getRuntimeValue() = ( *e.getField().getReference()->getRuntimeValue().vals )[ index ];
         operator()( e.getField() );
         */
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
            // we have nothing to do. The return value is stored in the result register already!
            // however, because it is returned as a reference, we must dereference it
            // indeed, it is a reference on the stack!
            if ( _env.resultRegister.type == RuntimeValue::REF && !e.getNodeType()->isReference() )
            {
               _env.resultRegister = *_env.resultRegister.ref;
            }
         }
      }

      

      /**
       @brief evaluate a function/memberfunction
       @note if memberfunction, just put the object in vals[0]
       @note vals can me modified...
       */
      void _callFunction( AstDeclFun& fun, std::vector<RuntimeValue>& vals )
      {
         AstDeclVars::Decls& vars = fun.getVars().getVars();

         ui32 stackframeSize = static_cast<ui32>( vars.size() );
         const ui32 initVectorSize = static_cast<ui32>( vals.size() );

         // prepare the stack, FP and restaure points
         _env.stackFrame.push( _env.framePointer );
         for ( ui32 n = 0; n < vals.size(); ++n )
         {
            _env.stack.push_back( vals[ n ] );
         }
         _env.framePointer += static_cast<ui32>( stackframeSize );

         // we need to compute the default parameter that have not beend transmitted
         for ( size_t t = vals.size(); t < vars.size(); ++t )
         {
            AstDeclVar* var = dynamic_cast<AstDeclVar*>( vars[ t ] );
            assert( var && var->getInit() );
            operator()( *var->getInit() );

            // the result should be located in the result register
            _env.stack.push_back( _env.resultRegister );
         }

         // run the function
         if ( fun.getBody() )
         {
            operator()( *fun.getBody() );
            // we expect the result to be in the resultRegister
         } else if ( fun.getImportedFunction() )
         {
            // construct arg list
            std::vector<RuntimeValue*> valsPtr( stackframeSize );
            for ( ui32 n = 0; n < initVectorSize; ++n )
            {
               valsPtr[ n ] = ( &vals[ n ] );
            }

            if ( initVectorSize != stackframeSize )
            {   
               for ( ui32 n = initVectorSize; n < stackframeSize; ++n )
               {
                  valsPtr[ n ] = ( &_env.stack[ _env.framePointer + n ] );
               }
            }
            _env.resultRegister = fun.getImportedFunction()->run( valsPtr );
         } else {
            throw RuntimeException( "unable to find function implementation" );
         }

         // restaure FP, frame pointer & stack
         _env.framePointer = _env.stackFrame.top();
         _env.stackFrame.pop();
         unstack( _env.stack, stackframeSize );
      }
      


      virtual void operator()( AstExpCall& e )
      {
         AstArgs::Args& args = e.getArgs().getArgs();
         std::vector<RuntimeValue> vals( args.size() );
         for ( size_t n = 0; n < args.size(); ++n )
         {
            operator()( *args[ n ] );
            vals[ n ] = _env.resultRegister;
         }

         _callFunction( *e.getFunctionCall(), vals );
      }



      virtual void operator()( AstDeclVar& e )
      {
         if ( e.getInit() )
         {
            operator()( *e.getInit() );
            _env.stack.push_back( _env.resultRegister );
            return;
         } else if ( e.getDeclarationList() )
         {
            /*
            operator()( *e.getDeclarationList() );

            
            for ( AstArgs::Args::iterator itArg = e.getDeclarationList()->getArgs().begin(); itArg != e.getDeclarationList()->getArgs().end(); ++itArg, ++n )
            {
               (*e.getRuntimeValue().vals)[ n ] = (*itArg)->getRuntimeValue();
            }*/
            return;
         }

         if ( e.getType().isArray() )
         {
            /*
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
            */
         }

         if ( e.getConstructorCall() )
         {
            /*
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
            */
         }

         // uninitialized variable
         _env.stack.push_back( RuntimeValue( RuntimeValue::EMPTY ) );
      }

      /**
       @brief assumes the runtime r has enough space to store the values
       */
      /*
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
      }*/

      /**
       @brief fetch the runtime value of a class to its definition
       */
      /*
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
      }*/

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

      //
      // cut the tree...
      //
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

      virtual void operator()( AstImport& )
      {
         // nothing to do
      }

      virtual void operator()( AstInclude& )
      {
         // nothing to do
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
      RuntimeEnvironment&  _env;
   };
}
}

#endif