#ifndef MVV_PARSE_EVALUATE_VISITOR_H_
# define MVV_PARSE_EVALUATE_VISITOR_H_

# include "visitor.h"
# include "ast-files.h"
# include "visitor-default.h"

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

    lvalue: we need a ref runtime value to handle them as we need to modify them
    */
   class VisitorEvaluate : public VisitorDefault
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

         _level = 0;
         _mustBreak = false;

         // reinit the stack frame
         env.stackFrame = std::stack<ui32>();
         env.stackFrame.push( 0 );

      }

      static void _debug( RuntimeValue& val )
      {
         if ( val.type == RuntimeValue::INT )
         {
            std::cout << "INT:" << val.intval << std::endl;
            return;
         }

         if ( val.type == RuntimeValue::FLOAT )
         {
            std::cout << "FLOAT:" << val.floatval << std::endl;
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
      }

      virtual void operator()( AstInt& e )
      {
         _env.resultRegister.setType( RuntimeValue::INT );
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
         if ( ur0.type == RuntimeValue::TYPE  )
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
         operator()( e.getCondition() );

         assert( _env.resultRegister.type == RuntimeValue::INT ); // only INT should be handled
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

         operator()( e.getLValue() );

         const bool isRef = e.getLValue().getNodeType()->isReference();

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
            if ( isRef )
            {
               // TODO is this ok?
               assert( _env.resultRegister.type == RuntimeValue::REF );
               *_env.resultRegister.ref = valRef;
            } else {
               *_env.resultRegister.ref = val;
            }
         }
      }

      virtual void operator()( AstVarSimple& e )
      {
         AstDeclVar* v = dynamic_cast<AstDeclVar*>( e.getReference() );
         assert( v ); // compiler error if this is not a decl

         // for a class, always create a ref...
         if ( v->isClassMember() )
         {
            ui32 index = _env.framePointer;
            assert( index < _env.stack.size() ); // compiler error if the stack size is wrong
            assert( unref(_env.stack[ index ]).type == RuntimeValue::TYPE );

            // if class member, the result must be in the class itself
            RuntimeValue& src = (*unref(_env.stack[ index ]).vals)[ v->getRuntimeIndex() ];
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
               // avoid the ref
               dst.ref = src.ref;
            } else {
               dst.ref = &src;
            }
         }
      }

      virtual void operator()( AstVarArray& e )
      {
         operator()( e.getIndex() );
         assert( _env.resultRegister.type == RuntimeValue::INT );
         int index = _env.resultRegister.intval;

         operator()( e.getName() );
         RuntimeValue& array = unref( _env.resultRegister );

         if ( array.type == RuntimeValue::EMPTY )
         {
            throw RuntimeException( "uninitialized array" );
         }
         assert( array.type == RuntimeValue::TYPE );

         if ( index >= static_cast<int>( (*array.vals).size() ) || index < 0 )
         {
            throw RuntimeException( "out of bound exception" );
         }

         // a L-value must return a ref
         _createRef( _env.resultRegister, (*array.vals)[ index ], false );
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

         assert( dynamic_cast<AstVarSimple*>( last ) || dynamic_cast<AstThis*>( last ) || dynamic_cast<AstVarArray*>( last ) ); // expect only simple variable decl for now // TODO array?
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
                  if ( val.type == RuntimeValue::EMPTY )
                  {
                     throw RuntimeException( "null reference" );
                  }
                  assert( 0 );   // we are expecting a null ref in case user made a mistake, else compiler error
               }

               _createRef( _env.resultRegister, (*val.vals)[ var->getRuntimeIndex() ], false );
            } else {
               // if this is not a variable, it must be a cuntion call
               assert( dynamic_cast<AstDeclFun*>( fields[ n ]->getPointee() ) );
            }
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
         // run through the names as it can be a varfield, which will store
         // the object to be invoked in the result register
         if ( dynamic_cast<AstVarField*>( &e.getName() ) )
         {
            operator()( e.getName() );
         }


         AstArgs::Args& args = e.getArgs().getArgs();

         ui32 tab = ( e.getFunctionCall()->getMemberOfClass() != 0 );   // we need to put the object adress...
         std::vector<RuntimeValue> vals( args.size() + tab );
         if ( tab )
         {
            if ( e.getConstructed() )
            {
               // because it is constructed, we need to allocate a temporary, start constructor, move the object
               // in the result register, unstack the object
               vals[ 0 ] = RuntimeValue( RuntimeValue::TYPE );
               vals[ 0 ].vals = RuntimeValue::RefcountedValues( this, e.getNodeType(), new RuntimeValues( e.getConstructed()->getMemberVariableSize() ) );
            } else  {
               vals[ 0 ] = _env.resultRegister;
            }
         }
         for ( size_t n = 0; n < args.size(); ++n )
         {
            operator()( *args[ n ] );
            vals[ n + tab ] = _env.resultRegister;
         }

         _callFunction( *e.getFunctionCall(), vals );

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
            src.vals = RuntimeValue::RefcountedValues( this, 0, new RuntimeValues( size[ currentIndex ] ) );

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
               (*src.vals)[ n ].vals = RuntimeValue::RefcountedValues( this, 0 /* TODO TODO: send the destructor if class...*/ , new RuntimeValues( constructor->getMemberOfClass()->getMemberVariableSize() ) );

               // call the constructor
               RuntimeValues init( 1 );
               init[ 0 ] = (*src.vals)[ n ];
               _callFunction( *constructor, init );
            }
         }
      }

      virtual void operator()( AstDeclVar& e )
      {
         if ( e.getInit() )
         {
            operator()( *e.getInit() );
            if ( e.getType().isAReference() )
            {
               _env.stack.push_back( _env.resultRegister );
            } else {
               // when you do int& fn(); int a = fn(); => we want to copy the value only!
               _env.stack.push_back( unref( _env.resultRegister ) );
            }
            return;
         } else if ( e.getDeclarationList() )
         {
            AstArgs::Args& args = e.getDeclarationList()->getArgs();
            _env.stack.push_back( RuntimeValue( RuntimeValue::TYPE ) );
            _env.stack.rbegin()->vals = RuntimeValue::RefcountedValues( this, e.getNodeType(), new RuntimeValues( args.size() ) );
            for ( ui32 n = 0; n < args.size(); ++n )
            {
               operator()( *args[ n ] );
               (*_env.stack.rbegin()->vals)[ n ] = unref( _env.resultRegister ); // copy the value, not a ref!
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
                  assert( _env.resultRegister.type == RuntimeValue::INT );
                  vals[ vals.size() - 1 - n ] = unref( _env.resultRegister ).intval; // we need to unref: in case we use ++operator
               }

               // create the root of the array
               _env.stack.push_back( RuntimeValue( RuntimeValue::TYPE ) );
               createArray( *_env.stack.rbegin(), vals, 0, e.getConstructorCall() );
               return;
            }
         } else {
            // if not an array & constructor call
            if ( e.getConstructorCall() )
            {
               // construct an object
               _env.stack.push_back( RuntimeValue( RuntimeValue::TYPE ) );
               assert( e.getConstructorCall()->getMemberOfClass() ); // if constructor, it must have a ref on the class def!
               _env.stack.rbegin()->vals = RuntimeValue::RefcountedValues( this, e.getNodeType(), new RuntimeValues( e.getConstructorCall()->getMemberOfClass()->getMemberVariableSize() ) );
               if ( e.getObjectInitialization() )
               {
                  AstArgs::Args& args = e.getObjectInitialization()->getArgs();
                  RuntimeValues vals( args.size() + 1 );

                  vals[ 0 ] = *_env.stack.rbegin();
                  for ( size_t n = 0; n < args.size(); ++n )
                  {
                     operator()( *args[ n ] );
                     vals[ n + 1 ] = _env.resultRegister;
                  }

                  _callFunction( *e.getConstructorCall(), vals );
               } else {
                  RuntimeValues vals( 1 );
                  vals[ 0 ] = *_env.stack.rbegin();
                  _callFunction( *e.getConstructorCall(), vals );
               }
               return;
            }
         }

         // uninitialized variable
         _env.stack.push_back( RuntimeValue( RuntimeValue::EMPTY ) );
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
         exp.setNodeType( e.getNodeType()->clone() );
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
            assert( _env.resultRegister.type == RuntimeValue::INT ); // the condition must be an int

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
      ui32                 _level;     // scope depth
      bool                 _mustBreak; // true if we must break the current loop
   };
}
}

#endif