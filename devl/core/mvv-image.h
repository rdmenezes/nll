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
   typedef nll::core::Image<nll::ui8> Pointee;

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

      Pointee* lut = new Pointee( v2.intval, v3.intval, v4.intval );


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
      rt.intval = p->sizex();
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
      rt.intval = p->sizey();
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
      rt.intval = p->getNbComponents();
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
      rt.intval = (*p)( v2.intval, v3.intval, v4.intval );
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
      (*p)( v3.intval, v3.intval, v4.intval ) = v2.intval;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

#endif