#ifndef CORE_MVV_FILE_H_
# define CORE_MVV_FILE_H_

# include "core.h"
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>

using namespace mvv::parser;
using namespace mvv;

class FunctionOFStreamConstructor : public FunctionRunnable
{
public:
   typedef std::ofstream Pointee;

public:
   FunctionOFStreamConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 3 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      if ( v2.type != RuntimeValue::STRING || v3.type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 string, 1 int" );
      }

      if ( v3.intval < 0 || v3.intval > 1 )
      {
         throw std::runtime_error( "OFStream() : mode is expected to be 0 (text) or 1 (binary)" );
      }

      Pointee* file;
      file = new Pointee( v2.stringval.c_str(), ( v3.intval == 0 ) ? std::ios::out : std::ios::binary | std::ios::out );
      if ( !file->good() )
         throw std::runtime_error( "OFStream() : cannot open the requested file" );


      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( file ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }
};

class FunctionOFStreamDestructor : public FunctionRunnable
{
   typedef FunctionOFStreamConstructor::Pointee Pointee;

public:
   FunctionOFStreamDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      Pointee* file = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // deallocate data
      delete file;
      (*v1.vals)[ 0 ].ref = 0;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionOFStreamWriteString : public FunctionRunnable
{
   typedef FunctionOFStreamConstructor::Pointee Pointee;

public:
   FunctionOFStreamWriteString( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v2.type != RuntimeValue::STRING )
         throw std::runtime_error( "OFStream::write() expected 1 string" );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* file = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      ( *file ) << v2.stringval;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionOFStreamWriteInt : public FunctionRunnable
{
   typedef FunctionOFStreamConstructor::Pointee Pointee;

public:
   FunctionOFStreamWriteInt( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v2.type != RuntimeValue::CMP_INT )
         throw std::runtime_error( "OFStream::write() expected 1 int" );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* file = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      ( *file ) << v2.intval;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionOFStreamWriteFloat : public FunctionRunnable
{
   typedef FunctionOFStreamConstructor::Pointee Pointee;

public:
   FunctionOFStreamWriteFloat( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v2.type != RuntimeValue::CMP_FLOAT )
         throw std::runtime_error( "OFStream::write() expected 1 int" );

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* file = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      ( *file ) << v2.floatval;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionOFStreamClose : public FunctionRunnable
{
   typedef FunctionOFStreamConstructor::Pointee Pointee;

public:
   FunctionOFStreamClose( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      Pointee* file = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      
      file->close();
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};


class FunctionIFStreamConstructor : public FunctionRunnable
{
public:
   typedef std::ifstream Pointee;

public:
   FunctionIFStreamConstructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
   {
   }

   virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
   {
      if ( args.size() != 3 )
      {
         throw std::runtime_error( "unexpected number of arguments" );
      }

      RuntimeValue& v1 = unref( *args[ 0 ] ); // we need to use this and not creating a new type as the destructor reference is already in place!
      RuntimeValue& v2 = unref( *args[ 1 ] );
      RuntimeValue& v3 = unref( *args[ 2 ] );
      if ( v2.type != RuntimeValue::STRING || v3.type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 string, 1 int" );
      }

      if ( v3.intval < 0 || v3.intval > 1 )
      {
         throw std::runtime_error( "IFStream() : mode is expected to be 0 (text) or 1 (binary)" );
      }

      Pointee* file;
      file = new Pointee( v2.stringval.c_str(), ( v3.intval == 0 ) ? std::ios::out : std::ios::binary | std::ios::out );
      if ( !file->good() )
         throw std::runtime_error( "IFStream() : cannot open the requested file" );


      RuntimeValue field( RuntimeValue::PTR );
      field.ref = reinterpret_cast<RuntimeValue*>( file ); // we are not interested in the pointer type! just a convenient way to store a pointer without having to create another field saving storage & speed
      (*v1.vals).resize( 1 );    // resize the original field
      (*v1.vals)[ 0 ] = field;

      return v1;  // return the original object!
   }
};

class FunctionIFStreamDestructor : public FunctionRunnable
{
   typedef FunctionIFStreamConstructor::Pointee Pointee;

public:
   FunctionIFStreamDestructor( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      Pointee* file = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      // deallocate data
      delete file;
      (*v1.vals)[ 0 ].ref = 0;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionIFStreamReadInt : public FunctionRunnable
{
   typedef FunctionIFStreamConstructor::Pointee Pointee;

public:
   FunctionIFStreamReadInt( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v2.type != RuntimeValue::CMP_INT )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 int" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* file = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      int n = 0;
      ( *file ) >> n;
      v2.intval = n;
      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionIFStreamReadFloat : public FunctionRunnable
{
   typedef FunctionIFStreamConstructor::Pointee Pointee;

public:
   FunctionIFStreamReadFloat( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      if ( v2.type != RuntimeValue::CMP_FLOAT )
      {
         throw std::runtime_error( "wrong arguments: expecting 1 float" );
      }

      // check we have the data
      assert( (*v1.vals)[ 0 ].type == RuntimeValue::PTR ); // it must be 1 field, PTR type
      Pointee* file = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );


      float n = 0;
      ( *file ) >> n;
      v2.floatval = n;

      
      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};

class FunctionIFStreamEof : public FunctionRunnable
{
   typedef FunctionIFStreamConstructor::Pointee Pointee;

public:
   FunctionIFStreamEof( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      Pointee* file = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      RuntimeValue rt( RuntimeValue::CMP_INT );
      rt.intval = file->eof();
      return rt;
   }
};

class FunctionIFStreamGetline : public FunctionRunnable
{
   typedef FunctionIFStreamConstructor::Pointee Pointee;

public:
   FunctionIFStreamGetline( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      Pointee* file = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );

      RuntimeValue rt( RuntimeValue::STRING );
      std::getline( *file, rt.stringval );
      return rt;
   }
};

class FunctionIFStreamClose : public FunctionRunnable
{
   typedef FunctionIFStreamConstructor::Pointee Pointee;

public:
   FunctionIFStreamClose( const AstDeclFun* fun ) : FunctionRunnable( fun )
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
      Pointee* file = reinterpret_cast<Pointee*>( (*v1.vals)[ 0 ].ref );
      file->close();

      RuntimeValue rt( RuntimeValue::EMPTY );
      return rt;
   }
};


#endif