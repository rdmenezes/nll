#ifndef MVV_PARSE_EVALUATE_VISITOR_H_
# define MVV_PARSE_EVALUATE_VISITOR_H_

# include "visitor.h"
# include "ast-files.h"
# include "visitor-default.h"

namespace mvv
{
namespace parser
{
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

         throw std::runtime_error( ss.str().c_str() );
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

    lvalue: we need a ref runtime value to handle them as we need to modify them

    When an object is constructed: first, initalize automatic variables (member variable (init, class construction, array))
    then call the constructor.


    */
   class VisitorEvaluate : public VisitorDefault
   {
   public:
      /**
       @param destructorEvaluator this is the evaluator used to run the destuctor
       */
      VisitorEvaluate( ParserContext& context,
                       SymbolTableVars& vars,
                       SymbolTableFuncs& funcs,
                       SymbolTableClasses& classes,
                       RuntimeEnvironment& env,
                       VisitorEvaluate* destructorEvaluator = 0 ) : _context( context ), _vars( vars ), _funcs( funcs ), _classes( classes ), _env( env ), _destructorEvaluator( destructorEvaluator )
      {
         // TODO: set to zero FP only when run, not constructed (i.e. run multiple times...)
         
         init();

         // if no source evaluator, then assign itself
         if ( destructorEvaluator == 0 )
            _destructorEvaluator = this;
      }

      /**
       @brief prepare the evaluator to run
       */
      void init()
      {
         _env.framePointer = 0;
         _level = 0;
         _mustBreak = false;

         // reinit the stack frame
         _env.stackFrame = std::stack<ui32>();
         _env.stackFrame.push( 0 );

         _env.stackUnstack = std::stack<ui32>();
      }

      static void _debug( RuntimeValue& val )
      {
         if ( val.type == RuntimeValue::CMP_INT )
         {
            std::cout << "CMP_INT:" << val.intval << std::endl;
            return;
         }

         if ( val.type == RuntimeValue::CMP_FLOAT )
         {
            std::cout << "CMP_FLOAT:" << val.floatval << std::endl;
            return;
         }

         if ( val.type == RuntimeValue::STRING )
         {
            std::cout << "STRING:" << val.stringval << std::endl;
            return;
         }

         if ( val.type == RuntimeValue::REF )
         {
            std::cout << "REF:" << val.ref << std::endl;
            _debug( *val.ref );
            return;
         }

         if ( val.type == RuntimeValue::EMPTY )
         {
            std::cout << "EMPTY" << std::endl;
            return;
         }

         if ( val.type == RuntimeValue::TYPE )
         {
            std::cout << "TYPE:" << val.vals.getDataPtr() << std::endl;
            
            for ( ui32 n = 0; n < (*val.vals).size(); ++n )
            {
               RuntimeValue& ptr = ( (*val.vals.getDataPtr())[ n ] );
               if ( &unref( ptr ) == &unref( val ) )
               {
                  std::cout << "  SELF" << std::endl;
                  // recursive check...
                  continue;
               }
               std::cout << "  ";
               _debug( (*val.vals)[ n ] );
            }
            return;
         }

         if ( val.type == RuntimeValue::PTR )
         {
            std::cout << "PTR:" << val.ref << std::endl;
            return;
         }

         if ( val.type == RuntimeValue::FUN_PTR )
         {
            std::cout << "FUN_PTR:" << val.functionPointer << std::endl;
            return;
         }

         std::cout << "UNDEFINED TYPE" << std::endl;
      }

      virtual void operator()( AstInt& e )
      {
         _env.resultRegister.setType( RuntimeValue::CMP_INT );
         _env.resultRegister.intval = e.getValue();
      }

      virtual void operator()( AstThis& )
      {
         // "this" is stored at FP[0]
         _env.resultRegister.setType( RuntimeValue::TYPE );
         _env.resultRegister = _env.stack[ _env.framePointer ];
      }

      virtual void operator()( AstNil& )
      {
         _env.resultRegister.setType( RuntimeValue::NIL );
      }

      virtual void operator()( AstFloat& e )
      {
         _env.resultRegister.setType( RuntimeValue::CMP_FLOAT );
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
         if ( ur0.type == RuntimeValue::TYPE && !e.getFunctionCall() )
         {
            if ( e.getOp() == AstOpBin::EQ )
            {
               _env.resultRegister.type = RuntimeValue::CMP_INT;
               if ( ur1.type != RuntimeValue::NIL )
               {
                  _env.resultRegister.intval = ur0.vals.getDataPtr() == ur1.vals.getDataPtr();
               } else { // if the RHS is NIL and the LHS is not...
                  _env.resultRegister.intval = 0;
               }
               return;
            } else if ( e.getOp() == AstOpBin::NE )
            {
               _env.resultRegister.type = RuntimeValue::CMP_INT;
               if ( ur1.type != RuntimeValue::NIL )
               {
                  _env.resultRegister.intval = ur0.vals.getDataPtr() != ur1.vals.getDataPtr();
               } else { // if the RHS is NIL and the LHS is not...
                  _env.resultRegister.intval = 1;
               }
               return;
            }
         }

         // in case the dynamic type is NIL
         if ( ur0.type == RuntimeValue::NIL && !e.getFunctionCall() )
         {
            const int test = ( ur1.type == RuntimeValue::NIL ) /* || 0 == ur1.vals.getDataPtr() */;
            if ( e.getOp() == AstOpBin::EQ )
            {
               _env.resultRegister.type = RuntimeValue::CMP_INT;
               _env.resultRegister.intval = test;
               return;
            } else if ( e.getOp() == AstOpBin::NE )
            {
               _env.resultRegister.type = RuntimeValue::CMP_INT;
               _env.resultRegister.intval = !test;
               return;
            }
         }

         ensure( e.getFunctionCall(), "compiler error: no function to call in binary operator" );
         _callFunction( *e.getFunctionCall(), vals );
      }

      virtual void operator()( AstIf& e )
      {
         operator()( e.getCondition() );

         assert( _env.resultRegister.type == RuntimeValue::CMP_INT ); // only CMP_INT should be handled
         if ( _env.resultRegister.intval )
         {
            operator()( e.getThen() );
         } else {
            if ( e.getElse() )
            {
               operator()( *e.getElse() );
            }
         }
      }

      virtual void operator()( AstStatements& e )
      {
         // when we need to break, we need to exit any statements, until the loop has been found
         if ( _mustBreak )
            return;

         ++_level;

         if ( _level > 1 )
         {
            // update the stackframe
            _env.stackUnstack.push( static_cast<ui32>( _env.stack.size() ) );
         }

         for ( AstStatements::Statements::const_iterator it = e.getStatements().begin();
               it != e.getStatements().end();
               ++it )
         {
            operator()( **it );

            if ( _mustBreak )
               break;
         }

         if ( _level > 1 )
         {
            ui32 origSize = _env.stackUnstack.top();

            // unstack all the temporaries, except for global
            while ( _env.stack.size() > origSize )
               _env.stack.pop_back();
            _env.stackUnstack.pop();
         }

         --_level;
      }

      virtual void operator()( AstExpAssign& e )
      {
         operator()( e.getValue() );
         RuntimeValue val = unref( _env.resultRegister );   // when a 'value' is copyied, this is always by value, so we need to remove ref... except if the type of lvalue is a reference
         RuntimeValue valRef = _env.resultRegister;   // in case it is a ref we need to save it...

         bool forceCopyValue = false;
         _forceUnref = false;
         operator()( e.getLValue() );
         forceCopyValue = _forceUnref;

         const bool isRef = e.getLValue().getNodeType()->isReference();

         /*
         std::cout << "right=" << std::endl;
         _debug( val );

         std::cout << "left=" << std::endl;
         _debug( unref(_env.resultRegister) );
         */
         
         /*
         if ( isRef && _env.resultRegister.type == RuntimeValue::EMPTY )
         {
         // TODO? doesn't seem to be necessary
            // reference not initialized
            std::cout << "haha" << std::endl;
         }*/

         assert( _env.resultRegister.type == RuntimeValue::TYPE || _env.resultRegister.type == RuntimeValue::REF && _env.resultRegister.ref ); // compiler error, we are expecting a reference

         if ( _env.resultRegister.type == RuntimeValue::TYPE )
         {
            _env.resultRegister = val;
         } else {
            if ( isRef && unref( _env.resultRegister ).type == RuntimeValue::EMPTY && !forceCopyValue )   // if we have a ref and this ref doesn't have value, then copy the ref, else we copy by value
            {
               assert( _env.resultRegister.type == RuntimeValue::REF );
               *_env.resultRegister.ref = valRef;
            } else {
               *_env.resultRegister.ref = val;
            }
         }

         //_debug( unref( _env.resultRegister ) );
      }

      virtual void operator()( AstVarSimple& e )
      {
         AstDeclVar* v = dynamic_cast<AstDeclVar*>( e.getReference() );
         
         if ( v && v->getIsGlobalVariable() && !v->getIsPreinit() )
         {
            // we are using a global variable that has not been initialized, so init it!
            // it can happens in case of "include" that are recursively need the init of a GV
            RuntimeValue save = _env.resultRegister;
            operator()( *v );
            _env.resultRegister = save;
         }

         if ( e.getIsFunctionAddress() )
         {
            RuntimeValue save = _env.resultRegister;

            _env.resultRegister = RuntimeValue( RuntimeValue::FUN_PTR );
            _env.resultRegister.functionPointer =  e.getFunctionAddress();
         } else {
            assert( v ); // compiler error if this is not a decl
            if ( v->getIsInFunctionPrototype() )
               _forceUnref = true;  // in case a variable is in function prototype, if this var is a reference, we must force to copy the value in the assignment

            // for a class, always create a ref...
            if ( v->isClassMember() )
            {
               ui32 index = _env.framePointer;
               assert( index < _env.stack.size() ); // compiler error if the stack size is wrong
               //assert( unref(_env.stack[ index ]).type == RuntimeValue::TYPE ); // in case member function pointer, we need to hold a function pointer & type ref so it can be both

               // if class member, the result must be in the class itself
               ensure( unref(_env.stack[ index ]).type == RuntimeValue::TYPE ||
                       unref(_env.stack[ index ]).type == RuntimeValue::FUN_PTR ||
                       unref(_env.stack[ index ]).type == RuntimeValue::PTR , "Compiler problem: not a type" );
               RuntimeValues& values = (*unref(_env.stack[ index ]).vals);
               ensure( v->getRuntimeIndex() < values.size(), "Compiler problem: out of bound index!!!" );
               RuntimeValue& src = values[ v->getRuntimeIndex() ];
               _createRef( _env.resultRegister, src, src.type == RuntimeValue::TYPE );
            } else {
               // else, just compute its position on the stack
               ui32 index = _env.framePointer + v->getRuntimeIndex();
               assert( index < _env.stack.size() ); // compiler error if the stack size is wrong

               // we create a ref on the declaration
               RuntimeValue& src = _env.stack[ index ];
               _createRef( _env.resultRegister, src, src.type == RuntimeValue::TYPE );
            }
         }
      }

      /**
       @brief We never want to have a ref of a ref (it can be cyclic...)
       */
      void _createRef( RuntimeValue& dst, RuntimeValue& src, bool forceRef )
      {
         if ( src.type == RuntimeValue::TYPE )
         {
            if ( forceRef )
            {
               dst.setType( RuntimeValue::REF );   // ref so we don't update to the real type
               dst.ref = &src;
            } else {
               dst.setType( RuntimeValue::TYPE );
               dst.vals = src.vals;
            }
         } else {
            dst.type = RuntimeValue::REF;
            if ( forceRef || src.type == RuntimeValue::REF )
            {
               if ( src.type == RuntimeValue::REF )
               {
                  // avoid the ref
                  dst.ref = src.ref;
               } else {
                  dst.ref = &src;
               }
            } else {
               dst.ref = &src;
            }
         }
      }

      virtual void operator()( AstVarArray& e )
      {
         operator()( e.getIndex() );
         RuntimeValue& indexVal = unref( _env.resultRegister );
         assert( indexVal.type == RuntimeValue::CMP_INT );
         int index = indexVal.intval;

         operator()( e.getName() );
         RuntimeValue& array = unref( _env.resultRegister );

         if ( e.getFunction() )
         {
            // user defined array
            RuntimeValues vals( 2 );
            vals[ 0 ] = array;
            vals[ 1 ] = RuntimeValue( RuntimeValue::CMP_INT );
            vals[ 1 ].intval = index;
            _callFunction( *e.getFunction(), vals );
         } else {
            // regular array
            if ( array.type == RuntimeValue::EMPTY || array.type == RuntimeValue::NIL )
            {
               throw std::runtime_error( "uninitialized array (see " + e.getLocation().toString() + ")" );
            }
            //_debug( array );
            assert( array.type == RuntimeValue::TYPE );

            if ( array.vals.getDataPtr() == 0 || index >= static_cast<int>( (*array.vals).size() ) || index < 0 )
            {
               throw std::runtime_error( "out of bound exception (see " + e.getLocation().toString() + ")" );
            }

            // a L-value must return a ref
            _createRef( _env.resultRegister, (*array.vals)[ index ], true );
         }
      }

      // this node will only produce a.b.c, then they will be interpreted function call: ->(a.b)<- .c[(something)]
      virtual void operator()( AstVarField& e )
      {
         // we have initialized the varfield by a SimpleVar which is in the result register
         std::vector<AstVarField*> fields; // we must change the traversal order as we retrieve the results using a result register
         
         // sort the fields; // TODO: we can save this list for future evaluations...
         AstVarField* cur = &e;
         AstVar* last = 0;
         while ( cur )
         {
            fields.push_back( cur );
            last = &cur->getField();
            cur = dynamic_cast<AstVarField*>( &cur->getField() );
         }

         assert( dynamic_cast<AstExpCall*>( last ) || dynamic_cast<AstVarSimple*>( last ) || dynamic_cast<AstThis*>( last ) || dynamic_cast<AstVarArray*>( last ) ); // expect only simple variable decl for now // TODO array?
         operator()( *last );

         // evaluate the field
         for ( int n = static_cast<int>( fields.size() ) - 1; n >= 0; --n )
         {
            AstDeclVar* var = dynamic_cast<AstDeclVar*>( fields[ n ]->getPointee() );
            if ( var )
            {
               RuntimeValue& val = unref( _env.resultRegister );
               RuntimeValue::TypeEnum type = val.type;
               if ( type != RuntimeValue::TYPE )
               {
                  if ( val.type == RuntimeValue::EMPTY || val.type == RuntimeValue::NIL )
                  {
                     throw std::runtime_error( "null reference in:" + e.getLocation().toString() );
                  }
                  assert( 0 );   // we are expecting a null ref in case user made a mistake, else compiler error
               }

               _createRef( _env.resultRegister, (*val.vals)[ var->getRuntimeIndex() ], true );
            } else {
               // if this is not a variable, it must be a cuntion call
               assert( dynamic_cast<AstDeclFun*>( fields[ n ]->getPointee() ) );
            }
         }

         // if returns a member funtion pointer
         if ( e.getMemberFun() )
         {
            RuntimeValue& type = unref( _env.resultRegister ); // because it is a member pointer, we need to keep track of the class structure, we need to unref it as it may be a reference
            assert( type.type == RuntimeValue::TYPE );
            _env.resultRegister.type = RuntimeValue::FUN_PTR;
            _env.resultRegister.functionPointer =  e.getMemberFun();
            _env.resultRegister.vals = type.vals;
            /*
            RuntimeValue save = _env.resultRegister;
            _env.resultRegister = RuntimeValue( RuntimeValue::TYPE );
            _env.resultRegister.functionPointer =  e.getMemberFun();
            _env.resultRegister.vals = RuntimeValue::RefcountedValues( 0, 0, new RuntimeValues( 1 ) );
            (*_env.resultRegister.vals)[ 0 ] = save;

            _debug( _env.resultRegister );*/
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
         if ( e.getReturnValue() )
         {
            operator()( *e.getReturnValue() );
            // we have nothing to do. The return value is stored in the result register already!
            // however, because it is returned as a reference, we must dereference it
            // indeed, it is a reference on the stack!
            if ( _env.resultRegister.type == RuntimeValue::REF && !e.getNodeType()->isReference() )
            {
               // if the result is already a ref or the type is a ref this is safe to just do nothing
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

         ui32 isClass = fun.getMemberOfClass() != 0;
         ui32 stackframeSize = static_cast<ui32>( vars.size() + isClass ); // if it is a member decl, then we need to add object on the stack, so +1
         const ui32 initVectorSize = static_cast<ui32>( vals.size() );

         // prepare the stack, FP and restaure points
         _env.stackFrame.push( _env.framePointer );

         // we move the frame pointer to the end of the stack
         _env.framePointer = static_cast<ui32>( _env.stack.size() );

         // populate the stack
         for ( ui32 n = 0; n < vals.size(); ++n )
         {
            _env.stack.push_back( vals[ n ] );
         }

         // we need to compute the default parameter that have not beend transmitted
         // we need to remove 1 if a class!
         for ( size_t t = vals.size() - isClass; t < vars.size(); ++t )
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
            throw std::runtime_error( "unable to find function implementation:" + std::string( fun.getName().getName() ) );
         }

         // restaure FP, frame pointer & stack
         _env.framePointer = _env.stackFrame.top();
         _env.stackFrame.pop();
         unstack( _env.stack, stackframeSize );
      }
      


      virtual void operator()( AstExpCall& e )
      {
         // run through the names as it can be a varfield, which will store
         // the object to be invoked in the result register
         if ( dynamic_cast<AstVarField*>( &e.getName() ) )
         {
            operator()( e.getName() );
         }

         AstDeclFun* functionToCall = e.getFunctionCall();
         if ( e.getFunctionPointerCall() )
         {
            operator()( e.getName() ); // we need to run the node to compute the function address...
            functionToCall = unref(_env.resultRegister).functionPointer;   // unref as we always return a ref
         }

         if ( !functionToCall )
            throw std::runtime_error("Function to call has null pointer");


         AstArgs::Args& args = e.getArgs().getArgs();

         ui32 tab = ( functionToCall->getMemberOfClass() != 0 );   // we need to put the object adress...
         std::vector<RuntimeValue> vals( args.size() + tab );
         if ( tab )
         {
            if ( e.getInstanciation() )
            {
               // we are calling operator() of a class, just fetch the object
               // get the position of the variable on the stack
               vals[ 0 ] = _env.stack[ _env.framePointer + e.getName().getRuntimeIndex() ];
            } else if ( e.getConstructed() )
            {
               // because it is constructed, we need to allocate a temporary, start constructor, move the object
               // in the result register, unstack the object
               vals[ 0 ] = RuntimeValue( RuntimeValue::TYPE );
               vals[ 0 ].vals = RuntimeValue::RefcountedValues( _destructorEvaluator, e.getNodeType(), new RuntimeValues( e.getConstructed()->getMemberVariableSize() ) );

               // init some of the values
               _initObject( *functionToCall->getMemberOfClass(), vals[ 0 ] );
            } else  {
               if ( e.getSimpleName() )
               {
                  // compute the address of the object from the framePointer and not result register
                  vals[ 0 ] = _env.stack[ _env.framePointer + e.getName().getRuntimeIndex() ];
               } else {
                  // when we do a.b.c a[ 3 ]
                  vals[ 0 ] = _env.resultRegister;
               }
            }
         }
         for ( size_t n = 0; n < args.size(); ++n )
         {
            operator()( *args[ n ] );
            vals[ n + tab ] = _env.resultRegister;
         }

         _callFunction( *functionToCall, vals );

         if ( e.getConstructed() )
         {
            _env.resultRegister = vals[ 0 ];
         }
      }

      /**
       @brief Recursively populate the array
       */
      void createArray( RuntimeValue& src, const std::vector<ui32>& size, ui32 currentIndex, AstDeclFun* constructor )
      {
         if ( currentIndex > size.size() )
            return;
         if ( currentIndex < size.size() )
         {
            // if we are not at the last level, just allocate the next level...
            src.type = RuntimeValue::TYPE;
            src.vals = RuntimeValue::RefcountedValues( _destructorEvaluator, 0, new RuntimeValues( size[ currentIndex ] ) );

            // recursively populates the other dimensions
            for ( ui32 n = 0; n < size[ currentIndex ]; ++n )
            {
               createArray( (*src.vals)[ n ], size, currentIndex + 1, constructor );
            }
         }
         
         if ( currentIndex + 1 == size.size() && constructor )
         {
            // if last level & type, initialize the type
            // initialize the objects
            for ( ui32 n = 0; n < size[ currentIndex ]; ++n )
            {
               // create the object
               (*src.vals)[ n ].type = RuntimeValue::TYPE;
               (*src.vals)[ n ].vals = RuntimeValue::RefcountedValues( _destructorEvaluator, constructor->getMemberOfClass()->getNodeType(), new RuntimeValues( constructor->getMemberOfClass()->getMemberVariableSize() ) );
               _initObject( *constructor->getMemberOfClass(), (*src.vals)[ n ] );

               // call the constructor
               RuntimeValues init( 1 );
               init[ 0 ] = (*src.vals)[ n ];
               _callFunction( *constructor, init );
            }
         }
      }

      static bool isUnrefIsARef( const Type& t )
      {
         const TypeArray* ty = dynamic_cast<const TypeArray*>( &t );
         if ( ty )
            return ty->getRoot().isReference();
         return false;
      }

      RuntimeValue& addVariable( const RuntimeValue& val, AstDeclVar& e )
      {
         if ( e.getIsGlobalVariable() )
         {
            // in case it is a global variable, the memory should have been allocated by the compiler
            // so that when an "include" is done, we can recursively access the value of the variables
            RuntimeValue& store = _env.stack[ e.getRuntimeIndex() ];
            store = val;
            return store;
         } else {
            _env.stack.push_back( val );
            return *_env.stack.rbegin();
         }
      }

      virtual void operator()( AstDeclVar& e )
      {
         // TODO: variable may be initialized several times...
         if ( e.getIsGlobalVariable() && e.getIsPreinit() ) // already initialized...
            return;
         
         
         e.setIsPreinit( true );

         if ( e.getInit() )
         {
            operator()( *e.getInit() );
            if ( e.getType().getNodeType()->isReference() )
            {
               addVariable( _env.resultRegister, e );
            } else {
               // when you do int& fn(); int a = fn(); => we want to copy the value only!
               addVariable( unref( _env.resultRegister ), e );
            }
            return;
         } else if ( e.getDeclarationList() )
         {
            AstArgs::Args& args = e.getDeclarationList()->getArgs();
            RuntimeValue& value = addVariable( RuntimeValue( RuntimeValue::TYPE ), e );
            value.vals = RuntimeValue::RefcountedValues( _destructorEvaluator, e.getNodeType(), new RuntimeValues( args.size() ) );
            bool mustCreateRef = isUnrefIsARef( *e.getNodeType() );  // if the type held is a reference, keep a reference and not the value!
            for ( ui32 n = 0; n < args.size(); ++n )
            {
               operator()( *args[ n ] );
               if ( mustCreateRef )
                  (*value.vals)[ n ] = _env.resultRegister;
               else
                  (*value.vals)[ n ] = unref( _env.resultRegister ); // copy the value, not a ref!
            }
            return;
         } 

         if ( e.getType().isArray() )
         {
            
            if ( e.getType().getSize() && e.getType().getSize()->size() > 0 )
            {
               std::vector<ui32> vals( e.getType().getSize()->size() );
               for ( size_t n = 0; n < e.getType().getSize()->size(); ++n )
               {
                  operator()( *( (*e.getType().getSize())[ n ] ) );
                  RuntimeValue& val = unref( _env.resultRegister );
                  assert( val.type == RuntimeValue::CMP_INT );
                  vals[ vals.size() - 1 - n ] = val.intval; // we need to unref: in case we use ++operator
               }

               // create the root of the array
               RuntimeValue& value = addVariable( RuntimeValue( RuntimeValue::TYPE ), e );
               createArray( value, vals, 0, e.getConstructorCall() );
               return;
            }
         } else {
            // if not an array & constructor call
            if ( e.getConstructorCall() )
            {
               RuntimeValue& value = addVariable( RuntimeValue( RuntimeValue::TYPE ), e );
               //_env.stack.push_back( RuntimeValue( RuntimeValue::TYPE ) );
               assert( e.getConstructorCall()->getMemberOfClass() ); // if constructor, it must have a ref on the class def!
               //_env.stack.rbegin()->vals = RuntimeValue::RefcountedValues( _destructorEvaluator, e.getNodeType(), new RuntimeValues( e.getConstructorCall()->getMemberOfClass()->getMemberVariableSize() ) );
               value.vals = RuntimeValue::RefcountedValues( _destructorEvaluator, e.getNodeType(), new RuntimeValues( e.getConstructorCall()->getMemberOfClass()->getMemberVariableSize() ) );

               // init the var that need to!
               //
               //
               //
               if ( e.getIsGlobalVariable() )
               {
                  _env.stack.push_back( value );
                  _initObject( *e.getConstructorCall()->getMemberOfClass(), value ); // populate the object if necessary
                  _env.stack.pop_back();
               } else {
                  _initObject( *e.getConstructorCall()->getMemberOfClass(), value ); // populate the object if necessary
               }

               if ( e.getObjectInitialization() )
               {
                  AstArgs::Args& args = e.getObjectInitialization()->getArgs();
                  RuntimeValues vals( args.size() + 1 );

                  vals[ 0 ] = value;
                  for ( size_t n = 0; n < args.size(); ++n )
                  {
                     operator()( *args[ n ] );
                     vals[ n + 1 ] = _env.resultRegister;
                  }

                  _callFunction( *e.getConstructorCall(), vals );
               } else {
                  RuntimeValues vals( 1 );
                  vals[ 0 ] = value;
                  _callFunction( *e.getConstructorCall(), vals );
               }
               return;
            }
         }

         // uninitialized variable
         // check if we need default initialization?
         RuntimeValue rt( RuntimeValue::EMPTY );
         if ( e.getNodeType()->isCompatibleWith( TypeFloat( false ) ) )
         {
            rt.type = RuntimeValue::CMP_FLOAT;
            rt.floatval = 0;
         } else if ( e.getNodeType()->isCompatibleWith( TypeInt( false ) ) )
         {
            rt.type = RuntimeValue::CMP_INT;
            rt.intval = 0;
         } else if ( e.getNodeType()->isCompatibleWith( TypeString( false ) ) )
         {
            rt.type = RuntimeValue::STRING;
         }
         addVariable( rt, e );
      }

      /**
       @brief Initialize an object that is on top of the stack that must be automatically constructed (so member variable in a class such as array/other class!)
       */
      void _initObject( AstDeclClass& e, RuntimeValue& dst )
      {
         // for all members, instanciate the variable, then copy it in result dir, finally restore the stack
         const std::vector<AstDeclVar*>& toInit = e.getMemberToInit();
         for ( size_t n = 0; n < toInit.size(); ++n )
         {
            operator()( *toInit[ n ] );

            _env.resultRegister = *_env.stack.rbegin();
            _env.stack.pop_back();

            (*dst.vals)[ toInit[ n ]->getRuntimeIndex() ] = _env.resultRegister;
         }
      }

      virtual void operator()( AstExpSeq& e )
      {
         operator()( e.getExp() );
      }

      virtual void operator()( AstExpTypename& e )
      {
         // construct an AstExpCall and eval as we want to do exactly the same thing!
         AstVarSimple var( e.getLocation(), mvv::platform::Symbol::create("TMP"), false );
         AstExpCall exp( e.getLocation(), &var, &e.getArgs(), false );
         exp.setNodeType( e.getNodeType() ); // no clone as we don't destroy it!
         exp.setFunctionCall( e.getConstructor() );
         exp.setConstructed( e.getReference() );

         operator()( exp );
      }

      virtual void operator()( AstBreak& )
      {
         _mustBreak = true;
      }

      virtual void operator()( AstWhile& e )
      {
         while ( 1 )
         {
            operator()( e.getCondition() );
            assert( _env.resultRegister.type == RuntimeValue::CMP_INT ); // the condition must be an int
            if ( !_env.resultRegister.intval )
               break;

            operator()( e.getStatements() );
            if ( _mustBreak )
            {
               _mustBreak = false;
               break;
            }
         }
      }


      virtual void operator()( Ast& e )
      {
         e.accept( *this );
      }

      virtual void operator()( AstTypeField& )
      {
         // nothing to do
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

      RuntimeEnvironment&  _env;

   private:
      // disabled copy
      VisitorEvaluate( const VisitorEvaluate& );
      VisitorEvaluate& operator=( const VisitorEvaluate& );


   private:
      ParserContext&       _context;
      SymbolTableVars&     _vars;
      SymbolTableFuncs&    _funcs;
      SymbolTableClasses&  _classes;
      ui32                 _level;     // scope depth
      bool                 _mustBreak; // true if we must break the current loop
      VisitorEvaluate*     _destructorEvaluator;
      bool                 _forceUnref;
   };
}
}

#endif