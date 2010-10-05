#ifndef CORE_MVV_IMAGE_H_
# define CORE_MVV_IMAGE_H_

# include "core.h"
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

using namespace mvv::parser;
using namespace mvv;

class FunctionImageConstructor : public FunctionRunnable
{
public:
   typedef mvv::platform::ResourceImageuc Pointee;

public:
   FunctionImageConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionImageDestructor : public FunctionRunnable
{
   typedef FunctionImageConstructor::Pointee Pointee;

public:
   FunctionImageDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionImageGetSizex : public FunctionRunnable
{
public:
   typedef FunctionImageConstructor::Pointee Pointee;

public:
   FunctionImageGetSizex( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionImageGetSizey : public FunctionRunnable
{
public:
   typedef FunctionImageConstructor::Pointee Pointee;

public:
   FunctionImageGetSizey( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionImageGetSizec : public FunctionRunnable
{
public:
   typedef FunctionImageConstructor::Pointee Pointee;

public:
   FunctionImageGetSizec( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionImageGet : public FunctionRunnable
{
public:
   typedef FunctionImageConstructor::Pointee Pointee;

public:
   FunctionImageGet( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      
      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = p->getValue()( v2.intval, v3.intval, v4.intval );
      return rt;
   }
};

class FunctionImageSet : public FunctionRunnable
{
public:
   typedef FunctionImageConstructor::Pointee Pointee;

public:
   FunctionImageSet( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v2.type != RuntimeValue::CMP_INT || v3.type != RuntimeValue::CMP_INT || v4.type != RuntimeValue::CMP_INT || v5.type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "expecting int x 4" );
      }

      
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      p->notify();
      (p->getValue())( v3.intval, v4.intval, v5.intval ) = v2.intval;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

template <class Image>
inline void inverColor( Image& m )
{
   const ui32 nbc = m.getNbComponents();
   for ( ui32 y = 0; y < m.sizey(); ++y )
      for ( ui32 x = 0; x < m.sizex(); ++x )
         for ( ui32 c = 0; c < nbc / 2; ++c )
            std::swap( m( x, y, c ), m( x, y, nbc - 1 - c ) );
}

class FunctionWriteBmp : public FunctionRunnable
{
public:
   typedef FunctionImageConstructor::Pointee Pointee;

public:
   FunctionWriteBmp( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      Pointee::value_type im;
      im.clone( p->getValue() );
      inverColor( im );

      bool success = nll::core::writeBmp( im, v2.stringval );
      if ( !success )
      {
         throw std::runtime_error( "can't write image to:" + v2.stringval );
      }
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionReadBmp : public FunctionRunnable
{
public:
   typedef FunctionImageConstructor::Pointee Pointee;

public:
   FunctionReadBmp( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      Pointee* p = new Pointee();
      bool success = nll::core::readBmp( p->getValue(), v1.stringval );
      if ( !success )
      {
         throw std::runtime_error( "can't read image from:" + v1.stringval );
      }
      inverColor( p->getValue() );
      
      RuntimeValue rt( RuntimeValue::TYPE );
      Type* t = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create( "Image" ) ) ) );
      if ( !t )
      {
         throw std::runtime_error( "internal error: cannot instanciate Image type" );
      }

      RuntimeValues* vals = new RuntimeValues( 1 );
      (*vals)[ 0 ] = RuntimeValue( RuntimeValue::PTR );
      (*vals)[ 0 ].ref = reinterpret_cast<RuntimeValue*>( p );
      rt.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), t, vals );
      return rt;
   }

private:
   CompilerFrontEnd& _e;
};

class FunctionImageHolderConstructor : public FunctionRunnable
{
public:
   typedef mvv::platform::ResourceImageuc Pointee;

public:
   FunctionImageHolderConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!

      Pointee* im = new Pointee();

      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( im ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }
};

class FunctionImageHolderDestructor : public FunctionRunnable
{
   typedef FunctionImageHolderConstructor::Pointee Pointee;

public:
   FunctionImageHolderDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionImageHolderSet : public FunctionRunnable
{
public:
   typedef FunctionImageHolderConstructor::Pointee Pointee;
   typedef FunctionImageConstructor::Pointee PointeeImage;

public:
   FunctionImageHolderSet( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      PointeeImage* pi = reinterpret_cast<PointeeImage*>( (*v2.vals)[ 0 ].ref );
      p->getValue().clone( pi->getValue() );
      p->notify();
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionImageHolderGet : public FunctionRunnable
{
public:
   typedef FunctionImageHolderConstructor::Pointee Pointee;
   typedef FunctionImageConstructor::Pointee PointeeImage;

public:
   FunctionImageHolderGet( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      
      //Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      return v1;
   }

private:
   CompilerFrontEnd&    _e;
};

class FunctionImageClone : public FunctionRunnable
{
public:
   typedef FunctionImageConstructor::Pointee Pointee;
   typedef FunctionImageConstructor::Pointee PointeeImage;

public:
   FunctionImageClone( const AstDeclFun* fun, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _e( e )
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


class FunctionImageSetImage : public FunctionRunnable
{
public:
   typedef FunctionImageConstructor::Pointee Pointee;

public:
   FunctionImageSetImage( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

class FunctionImageDecolor : public FunctionRunnable
{
public:
   typedef FunctionImageConstructor::Pointee Pointee;

public:
   FunctionImageDecolor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      nll::core::decolor( p->getValue() );
      p->notify();
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionImageExtend : public FunctionRunnable
{
public:
   typedef FunctionImageConstructor::Pointee Pointee;

public:
   FunctionImageExtend( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      nll::core::extend( p->getValue(), v2.intval );
      p->notify();
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionImageCrop : public FunctionRunnable
{
public:
   typedef FunctionImageConstructor::Pointee Pointee;

public:
   FunctionImageCrop( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 3 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );

      nll::core::vector2i p1;
      nll::core::vector2i p2;

      parser::getVector2iValues( v2, p1 );
      parser::getVector2iValues( v3, p2 );
            
      Pointee* p = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      Pointee::value_type& image = p->getValue();
      if ( p1[ 0 ] < 0 || p1[ 1 ] < 0 ||
           p2[ 0 ] >= (int)image.sizex() || p2[ 1 ] >= (int)image.sizey() )
      {
         throw std::runtime_error( "out of bound image cropping" );
      }

      if ( p1[ 0 ] > p2 [ 0 ] || p1[ 1 ] > p2[ 1 ] )
      {
         throw std::runtime_error( "min must be bottom-left max cropping point" );
      }

      nll::core::extract( image, p1[ 0 ], p1[ 1 ], p2[ 0 ], p2[ 1 ] );
      p->setValue( image );
      p->notify();
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

#endif