#ifndef CORE_MVV_MATRIX_H_
# define CORE_MVV_MATRIX_H_

# include "core.h"
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

class FunctionRunnableMatrixfConstructor : public FunctionRunnable
{
public:
   typedef nll::core::Matrix<float> Pointee;

public:
   FunctionRunnableMatrixfConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 3 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      if ( v2.type != RuntimeValue::CMP_INT || v3.type != RuntimeValue::CMP_INT )
      {
         throw RuntimeException( "expecting (int, int)" );
      }

      Pointee* pointee = new Pointee( v2.intval, v3.intval );
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( pointee ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }
};

class FunctionRunnableMatrixfDestructor: public FunctionRunnable
{
public:
   typedef FunctionRunnableMatrixfConstructor::Pointee Pointee;

public:
   FunctionRunnableMatrixfDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // deallocate data
      delete pointee;
      (*v1.vals)[ 0 ].ref = 0;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionMarixfSet : public FunctionRunnable
{
public:
   typedef FunctionRunnableMatrixfConstructor::Pointee Pointee;

public:
   FunctionMarixfSet( const AstDeclFun* fun ) : FunctionRunnable( fun )
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

      if ( v2.type != RuntimeValue::CMP_FLOAT || v3.type != RuntimeValue::CMP_INT || v4.type != RuntimeValue::CMP_INT )
      {
         throw RuntimeException( "wrong arguments" );
      }

      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      if ( v3.intval < 0 || v3.intval >= (int)pointee->sizey() ||
           v4.intval < 0 || v4.intval >= (int)pointee->sizex() )
      {
         throw RuntimeException( "out of bound matrix access" );
      }
      (*pointee)( v3.intval, v4.intval ) = v2.floatval;

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};


class FunctionMarixfGet : public FunctionRunnable
{
public:
   typedef FunctionRunnableMatrixfConstructor::Pointee Pointee;

public:
   FunctionMarixfGet( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 3 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );

      if ( v2.type != RuntimeValue::CMP_INT || v3.type != RuntimeValue::CMP_INT )
      {
         throw RuntimeException( "wrong arguments" );
      }

      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      if ( v2.intval < 0 || v2.intval >= (int)pointee->sizey() ||
           v3.intval < 0 || v3.intval >= (int)pointee->sizex() )
      {
         throw RuntimeException( "out of bound matrix access" );
      }

      RuntimeValue rt( RuntimeValue::CMP_FLOAT );
      rt.floatval = (*pointee)( v2.intval, v3.intval );
      return rt;
   }
};

class FunctionMarixfInvert : public FunctionRunnable
{
public:
   typedef FunctionRunnableMatrixfConstructor::Pointee Pointee;

public:
   FunctionMarixfInvert( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      Pointee* p2 = new Pointee();
      p2->clone( *pointee );

      bool inverted = nll::core::inverse( *p2 );
      if ( !inverted )
         throw RuntimeException( "inverse: matrix is singular!" );
      

      RuntimeValue matrix( RuntimeValue::TYPE );
      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( p2 ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*matrix.vals).resize( 1 );    // resize the original field
      (*matrix.vals)[ 0 ] = field;
      return matrix;
   }
};

class FunctionMarixfDet : public FunctionRunnable
{
public:
   typedef FunctionRunnableMatrixfConstructor::Pointee Pointee;

public:
   FunctionMarixfDet( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      Pointee* p2 = new Pointee();
      p2->clone( *pointee );

      float d = 0;
      bool inverted = nll::core::inverse( *p2, &d );
      if ( !inverted )
         throw RuntimeException( "inverse: matrix is singular!" );
      

      RuntimeValue det( RuntimeValue::CMP_FLOAT );
      det.floatval = d;
      return det;
   }
};

class FunctionMarixfMul : public FunctionRunnable
{
public:
   typedef FunctionRunnableMatrixfConstructor::Pointee Pointee;

public:
   FunctionMarixfMul( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 2 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );
      RuntimeValue& v2 = unref( *args[ 0 ] );

      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      assert( (*v2.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      

      Pointee* pointeeA = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      Pointee* pointeeB = reinterpret_cast<Pointee*>( (*v2.vals)[ 0 ].ref );

      if ( pointeeA->sizex() != pointeeB->sizey() )
         throw RuntimeException( "matrix size error for multiplication: a.sizex == b.sizey");


      RuntimeValue p( RuntimeValue::TYPE );
      createFields( p, 1 );
      (*p.vals)[ 0 ].ref = reinterpret_cast<RuntimeValue*>( new Pointee( *pointeeA * *pointeeB ) ); 
      return p;
   }
};

class FunctionMatrixfTranspose : public FunctionRunnable
{
public:
   typedef FunctionRunnableMatrixfConstructor::Pointee Pointee;

public:
   FunctionMatrixfTranspose( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      

      Pointee* pointeeA = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      Pointee* pointeeB = new Pointee();
      pointeeB->clone( *pointeeA );
      nll::core::transpose( *pointeeB );

      RuntimeValue p( RuntimeValue::TYPE );
      createFields( p, 1 );
      (*p.vals)[ 0 ].ref = reinterpret_cast<RuntimeValue*>( pointeeB ); 
      return p;
   }
};

class FunctionMatrixfSizex : public FunctionRunnable
{
public:
   typedef FunctionRunnableMatrixfConstructor::Pointee Pointee;

public:
   FunctionMatrixfSizex( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      

      RuntimeValue det( RuntimeValue::CMP_INT );
      det.intval = pointee->sizex();
      return det;
   }
};

class FunctionMatrixfSizey : public FunctionRunnable
{
public:
   typedef FunctionRunnableMatrixfConstructor::Pointee Pointee;

public:
   FunctionMatrixfSizey( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 1 )
      {
         throw RuntimeException( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] );

      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      
      Pointee* pointee = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      

      RuntimeValue det( RuntimeValue::CMP_INT );
      det.intval = pointee->sizey();
      return det;
   }
};
#endif 