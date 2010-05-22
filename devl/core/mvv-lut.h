#ifndef CORE_MVV_LUT_H_
# define CORE_MVV_LUT_H_

# include "core.h"
# include <mvvPlatform/resource-lut.h>
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

using namespace mvv::parser;
using namespace mvv;

class FunctionLutConstructor : public FunctionRunnable
{
public:
   typedef platform::ResourceLut Pointee;

public:
   FunctionLutConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 4 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      RuntimeValue& v4 = unref( *args[ 3 ] );
      if ( v2.type != RuntimeValue::CMP_FLOAT || v3.type != RuntimeValue::CMP_FLOAT || v4.type != RuntimeValue::TYPE )
      {
         throw RuntimeException( "wrong arguments: expecting 2 floats, 1 vector3i as arguments" );
      }

      nll::core::vector3i vals;
      getVector3iValues( v4, vals );

      float color[ 3 ] =
      {
         vals[ 0 ], vals[ 1 ], vals[ 2 ]
      };

      nll::imaging::LookUpTransformWindowingRGB lutImpl( v2.floatval, v3.floatval, 255, 3 );
      lutImpl.createColorScale( color );

      // construct the type
      Pointee* lut = new Pointee( lutImpl );


      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( lut ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }
};

class FunctionLutDestructor : public FunctionRunnable
{
   typedef platform::ResourceLut Pointee;

public:
   FunctionLutDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
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

class FunctionLutSetColorIndex : public FunctionRunnable
{
   typedef platform::ResourceLut Pointee;

public:
   FunctionLutSetColorIndex( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v2.type != RuntimeValue::CMP_INT || v3.type != RuntimeValue::CMP_INT || v4.type != RuntimeValue::CMP_INT || v5.type != RuntimeValue::CMP_INT )
      {
         throw RuntimeException( "wrong arguments: expecting 2 ints as arguments" );
      }

      if ( v3.intval > 255 || v3.intval < 0 ||
           v4.intval > 255 || v4.intval < 0 ||
           v5.intval > 255 || v5.intval < 0 ||
           v2.intval > 255 || v2.intval < 0 )
      {
         throw RuntimeException( "out of bound argument: must be in the range [0..255]" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* lut = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // update the value
      const f32 color[] = { v3.intval, v4.intval, v5.intval };
      lut->setIndex( v2.intval, color );
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionLutGetColorIndex : public FunctionRunnable
{
   typedef platform::ResourceLut Pointee;

public:
   FunctionLutGetColorIndex( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v2.type != RuntimeValue::CMP_INT  )
      {
         throw RuntimeException( "wrong arguments: expecting 1 int as arguments" );
      }

      if ( v2.intval > 255 || v2.intval < 0 )
      {
         throw RuntimeException( "out of bound argument: must be in the range [0..255]" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* lut = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // get the value
      const f32* color = lut->getIndex( v2.intval );
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3i( rt, static_cast<int>( color[ 0 ] ),
                          static_cast<int>( color[ 1 ] ),
                          static_cast<int>( color[ 2 ] ) );
      return rt;
   }
};

class FunctionLutTransform : public FunctionRunnable
{
   typedef platform::ResourceLut Pointee;

public:
   FunctionLutTransform( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v2.type != RuntimeValue::CMP_FLOAT  )
      {
         throw RuntimeException( "wrong arguments: expecting 1 float as arguments" );
      }

      if ( v2.intval > 255 || v2.intval < 0 )
      {
         throw RuntimeException( "out of bound argument: must be in the range [0..255]" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* lut = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // get the value
      const f32* color = lut->transform( v2.floatval );
      RuntimeValue rt( RuntimeValue::TYPE );
      createVector3i( rt, static_cast<int>( color[ 0 ] ),
                          static_cast<int>( color[ 1 ] ),
                          static_cast<int>( color[ 2 ] ) );
      return rt;
   }
};

#endif