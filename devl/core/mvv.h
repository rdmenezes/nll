#ifndef CORE_MVV_H_
# define CORE_MVV_H_

# include "core.h"
//# include <mvvLauncher/init.h>
# include <mvvPlatform/context-tools.h>
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

using namespace mvv::platform;
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

class FunctionRunnableVolumeGetPst : public FunctionRunnable
{
public:
   FunctionRunnableVolumeGetPst( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      nll::core::Matrix<float> rotation = volume->getPst();
      createMatrix4f( rt, rotation );
      return rt;
   }
};

class FunctionRunnableVolumeSetRotation : public FunctionRunnable
{
public:
   FunctionRunnableVolumeSetRotation( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::TYPE   )
      {
         throw RuntimeException( "wrong arguments: expecting 1 volume as arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Volume* volume = reinterpret_cast<Volume*>( (*v1.vals)[ 0 ].ref );
      RuntimeValues& value = *(*v2.vals)[ 0 ].vals;

      assert( v2.type == RuntimeValue::TYPE && (*v2.vals).size() == 1 );   // we are expecting a vector3f
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::TYPE && value.size() == 9 );   // we are expecting a vector3f

      nll::core::Matrix<float> rot( 3, 3 );
      rot( 0, 0 ) = value[ 0 ].floatval;
      rot( 0, 1 ) = value[ 1 ].floatval;
      rot( 0, 2 ) = value[ 2 ].floatval;

      rot( 1, 0 ) = value[ 3 ].floatval;
      rot( 1, 1 ) = value[ 4 ].floatval;
      rot( 1, 2 ) = value[ 5 ].floatval;

      rot( 2, 0 ) = value[ 6 ].floatval;
      rot( 2, 1 ) = value[ 7 ].floatval;
      rot( 2, 2 ) = value[ 8 ].floatval;

      volume->setRotation( rot );

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionRunnableVolumeSetValue : public FunctionRunnable
{
public:
   FunctionRunnableVolumeSetValue( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 5 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );
      RuntimeValue& v5 = unref( *args[ 4 ] );

      const int x = v2.intval;
      const int y = v3.intval;
      const int z = v4.intval;
      const float val = v5.floatval;

      if ( v1.type != RuntimeValue::TYPE  || v2.type != RuntimeValue::CMP_INT || v3.type != RuntimeValue::CMP_INT || v4.type != RuntimeValue::CMP_INT || v5.type != RuntimeValue::CMP_FLOAT )
      {
         throw RuntimeException( "wrong arguments: expecting 1 volume and 3 int as arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Volume* volume = reinterpret_cast<Volume*>( (*v1.vals)[ 0 ].ref );
      if ( !volume->inside( x, y, z ) )
      {
         throw RuntimeException( "voxel index out of bound" );
      }

      (*volume)( x, y, z ) = val;
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionRunnableVolumeGetValue : public FunctionRunnable
{
public:
   FunctionRunnableVolumeGetValue( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 4 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );

      NLL_ALIGN_16 float pos[] = { v2.floatval, v3.floatval, v4.floatval, 0 };   // we need to correctly align the memory!
      if ( v1.type != RuntimeValue::TYPE  || v2.type != RuntimeValue::CMP_FLOAT || v3.type != RuntimeValue::CMP_FLOAT || v4.type != RuntimeValue::CMP_FLOAT )
      {
         throw RuntimeException( "wrong arguments: expecting 1 volume and 3 int as arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Volume* volume = reinterpret_cast<Volume*>( (*v1.vals)[ 0 ].ref );
      
      nll::imaging::InterpolatorTriLinear<Volume> interpolator( *volume );
      interpolator.startInterpolation();
      float val = interpolator( pos );
      interpolator.endInterpolation();

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = val;
      return rt;
   }
};

class FunctionRunnableVolumeSetPst : public FunctionRunnable
{
public:
   FunctionRunnableVolumeSetPst( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v1.type != RuntimeValue::TYPE   )
      {
         throw RuntimeException( "wrong arguments: expecting 1 volume as arguments" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Volume* volume = reinterpret_cast<Volume*>( (*v1.vals)[ 0 ].ref );
      RuntimeValues& value = *(*v2.vals)[ 0 ].vals;

      assert( v2.type == RuntimeValue::TYPE && (*v2.vals).size() == 1 );   // we are expecting a vector3f
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::TYPE && value.size() == 16 );   // we are expecting a vector3f

      nll::core::Matrix<float> rot( 4, 4 );
      rot( 0, 0 ) = value[ 0 ].floatval;
      rot( 0, 1 ) = value[ 1 ].floatval;
      rot( 0, 2 ) = value[ 2 ].floatval;
      rot( 0, 3 ) = value[ 3 ].floatval;

      rot( 1, 0 ) = value[ 4 ].floatval;
      rot( 1, 1 ) = value[ 5 ].floatval;
      rot( 1, 2 ) = value[ 6 ].floatval;
      rot( 1, 3 ) = value[ 7 ].floatval;

      rot( 2, 0 ) = value[ 8 ].floatval;
      rot( 2, 1 ) = value[ 9 ].floatval;
      rot( 2, 2 ) = value[ 10 ].floatval;
      rot( 2, 3 ) = value[ 11 ].floatval;

      rot( 3, 0 ) = value[ 12 ].floatval;
      rot( 3, 1 ) = value[ 13 ].floatval;
      rot( 3, 2 ) = value[ 14 ].floatval;
      rot( 3, 3 ) = value[ 15 ].floatval;

      
      volume->setPst( rot );

      RuntimeValue rt( RuntimeValue::EMPTY );
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
