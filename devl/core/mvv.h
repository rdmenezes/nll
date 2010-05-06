#ifndef CORE_MVV_H_
# define CORE_MVV_H_

# include "core.h"
# include <mvvLauncher/init.h>
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

using namespace mvv::parser;
using namespace mvv;

static ui32 volumeId = 0;

/**
 the MVV function linkage is assuming the correct context are in place such as
 ContextTools, ContextVolumes, ContextSegments
 */
class FunctionLoadVolumeAsynchronous : public FunctionRunnable
{
public:
   FunctionLoadVolumeAsynchronous( const AstDeclFun* fun, mvv::platform::Context& context, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _context( context ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      ++volumeId;
      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::STRING )
      {
         throw RuntimeException( "wrong arguments: expecting 1 string as arguments" );
      }

      ContextTools* tools = _context.get<ContextTools>();
      if ( !tools )
      {
         throw RuntimeException( "ContextTools context has not been loaded" );
      }
      tools->loadVolume( v1.stringval, mvv::SymbolVolume::create( nll::core::val2str( volumeId ) ) );


      Type* ty = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "VolumeID" ) ) ) );

      RuntimeValue rt( RuntimeValue::TYPE );
      rt.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), ty, new RuntimeValues( 1 ) );
      (*rt.vals)[ 0 ].setType( RuntimeValue::STRING );
      (*rt.vals)[ 0 ].stringval = nll::core::val2str( volumeId );
      std::cout << "created volume:" << volumeId << std::endl;
      return rt;
   }

private:
   mvv::platform::Context&  _context;
   CompilerFrontEnd&        _e;
};

/**
 @brief returns a volume from an ID, throw runtime exception if volume not found
 */
class FunctionGetVolumeId : public FunctionRunnable
{
public:
   // We need these variables to be able to run the destructor
   // volumeClass: the class declaration
   // eval: the evaluator
   FunctionGetVolumeId( const AstDeclFun* fun, mvv::platform::Context& context, VisitorEvaluate* eval, Type* volumeClass ) : FunctionRunnable( fun ), _context( context ), _eval( eval ), _volumeClass( volumeClass )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::TYPE && (*v1.vals).size() == 1 && (*v1.vals)[ 0 ].type == RuntimeValue::STRING )
      {
         throw RuntimeException( "invalid argument" );
      }

      ContextTools* tools = _context.get<ContextTools>();
      if ( !tools )
      {
         throw RuntimeException( "ContextTools context has not been loaded" );
      }

      // it is guaranteed we have a volume
      mvv::platform::RefcountedTyped<Volume> vol = tools->getVolume( mvv::SymbolVolume::create( (*v1.vals)[ 0 ].stringval ) );

      RuntimeValue rt( RuntimeValue::TYPE );
      rt.vals = RuntimeValue::RefcountedValues( 0, 0, new RuntimeValues( 1 ) );

      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( vol.getDataPtr() ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*rt.vals)[ 0 ] = field;
      return rt;
   }

private:
   mvv::platform::Context&    _context;
   VisitorEvaluate*           _eval;
   Type*                      _volumeClass;
};

/**
 @brief synchronously load volumes
 */
class FunctionLoadVolumeMF2 : public FunctionRunnable
{
public:
   FunctionLoadVolumeMF2( const AstDeclFun* fun, mvv::platform::Context& context, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _context( context ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::STRING )
      {
         throw RuntimeException( "wrong arguments: expecting 1 string as arguments" );
      }

      ContextVolumes* volumes = _context.get<ContextVolumes>();
      if ( !volumes )
      {
         throw RuntimeException( "ContextVolumes context has not been loaded" );
      }

      ++volumeId;

      // load and store the type
      RefcountedTyped<Volume> volume( new Volume() );
      bool loaded = nll::imaging::loadSimpleFlatFile( v1.stringval, *volume );
      if ( !loaded )
      {
         throw RuntimeException( "can't find the volume" );
      }
      volumes->volumes.insert( mvv::SymbolVolume::create( nll::core::val2str( volumeId ) ), volume );
      
      // create the volume ID
      RuntimeValue rt( RuntimeValue::TYPE );
      Type* ty = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("VolumeID") ) ) );
      assert( ty );

      rt.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), ty, new RuntimeValues( 1 ) );
      (*rt.vals)[ 0 ].setType( RuntimeValue::STRING );
      (*rt.vals)[ 0 ].stringval = nll::core::val2str( volumeId );

      std::cout << "created volume:" << volumeId << std::endl;
      return rt;
   }

private:
   mvv::platform::Context&  _context;
   CompilerFrontEnd&        _e;
};

class FunctionRunnableVolumeGetSize : public FunctionRunnable
{
public:
   FunctionRunnableVolumeGetSize( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::TYPE   )
      {
         throw RuntimeException( "wrong arguments: expecting 1 volume as arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Volume* volume = reinterpret_cast<Volume*>( (*v1.vals)[ 0 ].ref );


      // create a vector3i
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3i( rt, volume->getSize()[ 0 ], volume->getSize()[ 1 ], volume->getSize()[ 2 ] );
      return rt;
   }
};

class FunctionRunnableVolumeGetSpacing : public FunctionRunnable
{
public:
   FunctionRunnableVolumeGetSpacing( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::TYPE   )
      {
         throw RuntimeException( "wrong arguments: expecting 1 volume as arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Volume* volume = reinterpret_cast<Volume*>( (*v1.vals)[ 0 ].ref );


      // create a vector3i
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, volume->getSpacing()[ 0 ], volume->getSpacing()[ 1 ], volume->getSpacing()[ 2 ] );
      return rt;
   }
};

class FunctionRunnableVolumeGetOrigin : public FunctionRunnable
{
public:
   FunctionRunnableVolumeGetOrigin( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::TYPE   )
      {
         throw RuntimeException( "wrong arguments: expecting 1 volume as arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Volume* volume = reinterpret_cast<Volume*>( (*v1.vals)[ 0 ].ref );


      // create a vector3i
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3f( rt, volume->getOrigin()[ 0 ], volume->getOrigin()[ 1 ], volume->getOrigin()[ 2 ] );
      return rt;
   }
};

class FunctionRunnableVolumeGetRotation : public FunctionRunnable
{
public:
   FunctionRunnableVolumeGetRotation( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      if ( v1.type != RuntimeValue::TYPE   )
      {
         throw RuntimeException( "wrong arguments: expecting 1 volume as arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Volume* volume = reinterpret_cast<Volume*>( (*v1.vals)[ 0 ].ref );


      // create a vector3i
      RuntimeValue rt( RuntimeValue::TYPE );

      nll::core::Matrix<float> rotation = volume->getRotation();
      createMatrix3f( rt, rotation );
      return rt;
   }
};

class FunctionRunnableVolumeSetOrigin : public FunctionRunnable
{
public:
   FunctionRunnableVolumeSetOrigin( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      VisitorEvaluate::_debug( v2 );

      if ( v1.type != RuntimeValue::TYPE   )
      {
         throw RuntimeException( "wrong arguments: expecting 1 volume as arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Volume* volume = reinterpret_cast<Volume*>( (*v1.vals)[ 0 ].ref );

      assert( v2.type == RuntimeValue::TYPE && (*v2.vals).size() == 1 );   // we are expecting a vector3f
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::TYPE && (*(*v2.vals)[ 0 ].vals).size() == 3 );   // we are expecting a vector3f

      RuntimeValue& vector3f = (*v2.vals)[ 0 ];
      nll::core::vector3f origin( ( *vector3f.vals )[ 0 ].floatval,
                                  ( *vector3f.vals )[ 1 ].floatval,
                                  ( *vector3f.vals )[ 2 ].floatval );
      volume->setOrigin( origin );

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionRunnableVolumeSetSpacing : public FunctionRunnable
{
public:
   FunctionRunnableVolumeSetSpacing( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      VisitorEvaluate::_debug( v2 );

      if ( v1.type != RuntimeValue::TYPE   )
      {
         throw RuntimeException( "wrong arguments: expecting 1 volume as arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Volume* volume = reinterpret_cast<Volume*>( (*v1.vals)[ 0 ].ref );

      assert( v2.type == RuntimeValue::TYPE && (*v2.vals).size() == 1 );   // we are expecting a vector3f
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::TYPE && (*(*v2.vals)[ 0 ].vals).size() == 3 );   // we are expecting a vector3f

      RuntimeValue& vector3f = (*v2.vals)[ 0 ];
      nll::core::vector3f value ( ( *vector3f.vals )[ 0 ].floatval,
                                  ( *vector3f.vals )[ 1 ].floatval,
                                  ( *vector3f.vals )[ 2 ].floatval );
      volume->setSpacing( value );

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

/**
 @brief synchronously load volumes
 */
class FunctionVolumeIDDestructor : public FunctionRunnable
{
public:
   FunctionVolumeIDDestructor( const AstDeclFun* fun, mvv::platform::Context& context ) : FunctionRunnable( fun ), _context( context )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      ContextVolumes* volumes = _context.get<ContextVolumes>();
      if ( !volumes )
      {
         throw RuntimeException( "ContextVolumes context has not been loaded" );
      }

      //std::string vol = (*args[ 0 ]->vals)[ 0 ].stringval;

      std::cout << "destroy volume:" << (*v1.vals)[ 0 ].stringval << std::endl;
      RefcountedTyped<Volume> vol;
      volumes->volumes.erase( mvv::SymbolVolume::create( (*v1.vals)[ 0 ].stringval.c_str() ) );

      //std::cout << "TODO: implement VolumeID destructor:" << vol << std::endl;
      //std::cout << "@vals=" << &(*args[ 0 ]->vals) << std::endl;
      //
      // TODO: remove the reference from the mvv context
      //

      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }

private:
   mvv::platform::Context&  _context;
};


#endif
