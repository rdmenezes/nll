#ifndef CORE_MVV_IMAGEF_H_
# define CORE_MVV_IMAGEF_H_

# include "core.h"
# include "mvv-image.h"
# include "mvv-lut.h"
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>


using namespace mvv::parser;
using namespace mvv;

class FunctionImagefConstructor : public FunctionRunnable
{
public:
   typedef mvv::platform::ResourceImagef Pointee;

public:
   FunctionImagefConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 4 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );
      if ( v2.type != RuntimeValue::CMP_INT || v3.type != RuntimeValue::CMP_INT || v4.type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "expecting int x 3" );
      }

      Pointee* lut = new Pointee();
      lut->getValue() = Pointee::value_type( v2.intval, v3.intval, v4.intval );

      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( lut ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }
};

class FunctionImagefDestructor : public FunctionRunnable
{
   typedef FunctionImagefConstructor::Pointee Pointee;

public:
   FunctionImagefDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* lut = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // deallocate data
      delete lut;
      (*v1.vals)[ 0 ].ref = 0;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionImagefGetSizex : public FunctionRunnable
{
public:
   typedef FunctionImagefConstructor::Pointee Pointee;

public:
   FunctionImagefGetSizex( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = p->getValue().sizex();
      return rt;
   }
};

class FunctionImagefGetSizey : public FunctionRunnable
{
public:
   typedef FunctionImagefConstructor::Pointee Pointee;

public:
   FunctionImagefGetSizey( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = p->getValue().sizey();
      return rt;
   }
};

class FunctionImagefGetSizec : public FunctionRunnable
{
public:
   typedef FunctionImagefConstructor::Pointee Pointee;

public:
   FunctionImagefGetSizec( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = p->getValue().getNbComponents();
      return rt;
   }
};

class FunctionImagefGet : public FunctionRunnable
{
public:
   typedef FunctionImagefConstructor::Pointee Pointee;

public:
   FunctionImagefGet( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 4 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );

      if ( v2.type != RuntimeValue::CMP_INT || v3.type != RuntimeValue::CMP_INT || v4.type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "expecting int x 3" );
      }

      
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = p->getValue()( v2.intval, v3.intval, v4.intval );
      return rt;
   }
};

class FunctionImagefSet : public FunctionRunnable
{
public:
   typedef FunctionImagefConstructor::Pointee Pointee;

public:
   FunctionImagefSet( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 5 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );
      RuntimeValue& v5 = unref( *args[ 4 ] );

      if ( v2.type != RuntimeValue::CMP_FLOAT || v3.type != RuntimeValue::CMP_INT || v4.type != RuntimeValue::CMP_INT || v5.type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "expecting float, int x 3" );
      }

      
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      p->notify();
      (p->getValue())( v3.intval, v4.intval, v5.intval ) = v2.floatval;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionImagefClone : public FunctionRunnable
{
public:
   typedef FunctionImagefConstructor::Pointee Pointee;
   typedef FunctionImagefConstructor::Pointee PointeeImage;

public:
   FunctionImagefClone( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
     
      // clone the image
      PointeeImage* pi = new PointeeImage();
      pi->getValue().clone( p->getValue() );
      
      
      RuntimeValue rt( RuntimeValue::TYPE );
      Type* t = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      if ( !t )
      {
         throw std::runtime_error( "internal error: cannot instanciate Image type" );
      }

      RuntimeValues* vals = new RuntimeValues( 1 );
      (*vals)[ 0 ] = RuntimeValue( RuntimeValue::PTR );
      (*vals)[ 0 ].ref = reinterpret_cast<RuntimeValue*>( pi );
      rt.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), t, vals );
      return rt;
   }

private:
   CompilerFrontEnd&    _e;
};


class FunctionImagefSetImage : public FunctionRunnable
{
public:
   typedef FunctionImagefConstructor::Pointee Pointee;

public:
   FunctionImagefSetImage( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
            
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      Pointee* p2 = reinterpret_cast<Pointee*>( (*v2.vals)[ 0 ].ref );

      p->getValue().clone( p2->getValue() );
      p->notify();
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionApplyLutImagef : public FunctionRunnable
{
public:
   typedef FunctionImagefConstructor::Pointee   Pointee;
   typedef FunctionLutConstructor::Pointee      PointeeLut;
   typedef FunctionImageConstructor::Pointee    PointeeImage;

public:
   FunctionApplyLutImagef( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
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
            
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      PointeeLut* p2 = reinterpret_cast<PointeeLut*>( (*v2.vals)[ 0 ].ref );

      if ( p->getValue().getNbComponents() != 1 )
      {
         throw std::runtime_error( "apply: only 1-channel component is handled by this function" );
      }

      // apply the lut
      PointeeImage* pi = new PointeeImage();
      pi->setValue( PointeeImage::value_type( p->getValue().sizex(), p->getValue().sizey(), 3 ) );

      PointeeImage::value_type::iterator itI = pi->getValue().begin();
      Pointee::value_type::iterator itf = p->getValue().begin();
      Pointee::value_type::iterator itfe = p->getValue().end();

      for ( ; itf != itfe; ++itf )
      {
         const float* color = p2->getValue().lut.transform( *itf );
         *itI++ = color[ 0 ];
         *itI++ = color[ 1 ];
         *itI++ = color[ 2 ];
      }
      
      RuntimeValue rt( RuntimeValue::TYPE );
      Type* t = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      if ( !t )
      {
         throw std::runtime_error( "internal error: cannot instanciate Image type" );
      }

      RuntimeValues* vals = new RuntimeValues( 1 );
      (*vals)[ 0 ] = RuntimeValue( RuntimeValue::PTR );
      (*vals)[ 0 ].ref = reinterpret_cast<RuntimeValue*>( pi );
      rt.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), t, vals );
      return rt;
   }

private:
   CompilerFrontEnd&    _e;
};

#endif