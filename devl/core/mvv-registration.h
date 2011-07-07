#ifndef CORE_MVV_REGISTRATION_H_
# define CORE_MVV_REGISTRATION_H_

# include "mvv-affine-registration.h"

using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

class FunctionRegistrationAffineIstropicPlanar : public FunctionRunnable
{
public:
   typedef FunctionAffineRegistrationConstructor::Pointee   Pointee;

   FunctionRegistrationAffineIstropicPlanar( const AstDeclFun* fun, mvv::platform::Context& context, VisitorEvaluate* eval, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _context( context ), _eval( eval ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::TYPE || (*v1.vals).size() != 1 || (*v1.vals)[ 0 ].type != RuntimeValue::STRING )
      {
         throw std::runtime_error( "invalid argument" );
      }

      if ( v2.type != RuntimeValue::TYPE || (*v2.vals).size() != 1 || (*v2.vals)[ 0 ].type != RuntimeValue::STRING )
      {
         throw std::runtime_error( "invalid argument" );
      }

      ContextTools* tools = _context.get<ContextTools>();
      if ( !tools )
      {
         throw std::runtime_error( "ContextTools context has not been loaded" );
      }

      // it is guaranteed we have a volume
      mvv::platform::RefcountedTyped<Volume> vol1 = tools->getVolume( mvv::SymbolVolume::create( (*v1.vals)[ 0 ].stringval ) );
      mvv::platform::RefcountedTyped<Volume> vol2 = tools->getVolume( mvv::SymbolVolume::create( (*v2.vals)[ 0 ].stringval ) );

      typedef nll::algorithm::AffineRegistrationCT3d<> Registration;
      Registration registration;
      nll::core::Matrix<double> tfm;
      Registration::Result result = registration.process( *vol1, *vol2, tfm, false );
      if ( result == Registration::FAILED_TOO_LITTLE_INLIERS )
      {
         throw std::runtime_error( "Registration failed: Insufficient inliers" );
      }
      if ( result != Registration::SUCCESS )
      {
         throw std::runtime_error( "Registration failed!" );
      }

      nll::core::Matrix<float> tfmFloat( 4, 4 );
      for ( ui32 y = 0; y < 4; ++y )
      {
         for ( ui32 x = 0; x < 4; ++x )
         {
            tfmFloat( y, x ) = static_cast<float>( tfm( y, x ) );
         }
      }

      Pointee* p = new Pointee();
      p->setValue( tfmFloat );

      // create a runtime value with a destructor
      RuntimeValue rt( RuntimeValue::TYPE );
      Type* t = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "AffineRegistration" ) ) ) );
      if ( !t )
      {
         throw std::runtime_error( "internal error: cannot instanciate AffineRegistration type" );
      }

      RuntimeValues* vals = new RuntimeValues( 1 );
      (*vals)[ 0 ] = RuntimeValue( RuntimeValue::PTR );
      (*vals)[ 0 ].ref = reinterpret_cast<RuntimeValue*>( p );
      rt.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), t, vals );
      return rt;
   }

private:
   mvv::platform::Context&    _context;
   VisitorEvaluate*           _eval;
   CompilerFrontEnd&          _e;
};


class FunctionRegistrationAffinePlanar : public FunctionRunnable
{
public:
   typedef FunctionAffineRegistrationConstructor::Pointee   Pointee;

   FunctionRegistrationAffinePlanar( const AstDeclFun* fun, mvv::platform::Context& context, VisitorEvaluate* eval, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _context( context ), _eval( eval ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );

      if ( v1.type != RuntimeValue::TYPE || (*v1.vals).size() != 1 || (*v1.vals)[ 0 ].type != RuntimeValue::STRING )
      {
         throw std::runtime_error( "invalid argument" );
      }

      if ( v2.type != RuntimeValue::TYPE || (*v2.vals).size() != 1 || (*v2.vals)[ 0 ].type != RuntimeValue::STRING )
      {
         throw std::runtime_error( "invalid argument" );
      }

      ContextTools* tools = _context.get<ContextTools>();
      if ( !tools )
      {
         throw std::runtime_error( "ContextTools context has not been loaded" );
      }

      // it is guaranteed we have a volume
      mvv::platform::RefcountedTyped<Volume> vol1 = tools->getVolume( mvv::SymbolVolume::create( (*v1.vals)[ 0 ].stringval ) );
      mvv::platform::RefcountedTyped<Volume> vol2 = tools->getVolume( mvv::SymbolVolume::create( (*v2.vals)[ 0 ].stringval ) );

      typedef nll::algorithm::AffineRegistrationCT3d<nll::algorithm::impl::SurfEstimatorAffineFactory> Registration;
      Registration registration;
      nll::core::Matrix<double> tfm;
      Registration::Result result = registration.process( *vol1, *vol2, tfm, false );
      if ( result == Registration::FAILED_TOO_LITTLE_INLIERS )
      {
         throw std::runtime_error( "Registration failed: Insufficient inliers" );
      }
      if ( result != Registration::SUCCESS )
      {
         throw std::runtime_error( "Registration failed!" );
      }

      nll::core::Matrix<float> tfmFloat( 4, 4 );
      for ( ui32 y = 0; y < 4; ++y )
      {
         for ( ui32 x = 0; x < 4; ++x )
         {
            tfmFloat( y, x ) = static_cast<float>( tfm( y, x ) );
         }
      }

      Pointee* p = new Pointee();
      p->setValue( tfmFloat );

      // create a runtime value with a destructor
      RuntimeValue rt( RuntimeValue::TYPE );
      Type* t = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "AffineRegistration" ) ) ) );
      if ( !t )
      {
         throw std::runtime_error( "internal error: cannot instanciate AffineRegistration type" );
      }

      RuntimeValues* vals = new RuntimeValues( 1 );
      (*vals)[ 0 ] = RuntimeValue( RuntimeValue::PTR );
      (*vals)[ 0 ].ref = reinterpret_cast<RuntimeValue*>( p );
      rt.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), t, vals );
      return rt;
   }

private:
   mvv::platform::Context&    _context;
   VisitorEvaluate*           _eval;
   CompilerFrontEnd&          _e;
};

#endif