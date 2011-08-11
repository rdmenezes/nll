#include "mvvForms.h"
#include <mvvScript/compiler-helper.h>

using namespace mvv::parser;
using namespace mvv;

namespace mvv
{
   class FunctionGetWorkingDirectory : public FunctionRunnable
   {

   public:
      FunctionGetWorkingDirectory( const AstDeclFun* fun ) : FunctionRunnable( fun )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 0 )
         {
            throw std::runtime_error( "unexpected number of arguments, expexted nothing" );
         }

         RuntimeValue rt( RuntimeValue::STRING );
         rt.stringval = getWorkingDirectory();
         return rt;
      }
   };

   class FunctionSetWorkingDirectory : public FunctionRunnable
   {

   public:
      FunctionSetWorkingDirectory( const AstDeclFun* fun ) : FunctionRunnable( fun )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 1 )
         {
            throw std::runtime_error( "unexpected number of arguments, expexted string" );
         }

         RuntimeValue& v0 = unref( *args[ 0 ] );
         if ( v0.type != RuntimeValue::STRING )
         {
            throw std::runtime_error( "expecting 1 string" );
         }

         setWorkingDirectory( v0.stringval );

         RuntimeValue rt( RuntimeValue::EMPTY );
         return rt;
      }
   };

   class FunctionBoxOpenFiles : public FunctionRunnable
   {

   public:
      FunctionBoxOpenFiles( const AstDeclFun* fun ) : FunctionRunnable( fun )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 1 )
         {
            throw std::runtime_error( "unexpected number of arguments, expexted string" );
         }

         RuntimeValue& v0 = unref( *args[ 0 ] );
         if ( v0.type != RuntimeValue::STRING )
         {
            throw std::runtime_error( "expecting 1 string" );
         }

         const std::vector< std::string > files = openFiles( v0.stringval );

         RuntimeValue rt( RuntimeValue::TYPE );
         createFields( rt, (ui32)files.size() );
         for ( size_t n = 0; n < files.size(); ++n )
         {
            RuntimeValue v( RuntimeValue::STRING );
            v.stringval = files[ n ];
            (*rt.vals)[ n ] = v;
         }
         return rt;
      }
   };

   class FunctionBoxOpenFolder : public FunctionRunnable
   {

   public:
      FunctionBoxOpenFolder( const AstDeclFun* fun ) : FunctionRunnable( fun )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 1 )
         {
            throw std::runtime_error( "unexpected number of arguments, expexted string" );
         }

         RuntimeValue& v0 = unref( *args[ 0 ] );
         if ( v0.type != RuntimeValue::STRING )
         {
            throw std::runtime_error( "expecting 1 string" );
         }

         RuntimeValue rt( RuntimeValue::STRING );
         rt.stringval = openFolder( v0.stringval );
         return rt;
      }
   };

   class FunctionBoxError : public FunctionRunnable
   {

   public:
      FunctionBoxError( const AstDeclFun* fun ) : FunctionRunnable( fun )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 2 )
         {
            throw std::runtime_error( "unexpected number of arguments, expected string, string" );
         }

         RuntimeValue& v0 = unref( *args[ 0 ] );
         RuntimeValue& v1 = unref( *args[ 1 ] );
         if ( v0.type != RuntimeValue::STRING ||
              v1.type != RuntimeValue::STRING )
         {
            throw std::runtime_error( "expecting 2 string" );
         }

         createMessageBoxError( v0.stringval, v1.stringval );

         RuntimeValue rt( RuntimeValue::EMPTY );
         return rt;
      }
   };

   class FunctionBoxQuestion : public FunctionRunnable
   {

   public:
      FunctionBoxQuestion( const AstDeclFun* fun ) : FunctionRunnable( fun )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 2 )
         {
            throw std::runtime_error( "unexpected number of arguments, expected string, string" );
         }

         RuntimeValue& v0 = unref( *args[ 0 ] );
         RuntimeValue& v1 = unref( *args[ 1 ] );
         if ( v0.type != RuntimeValue::STRING ||
              v1.type != RuntimeValue::STRING )
         {
            throw std::runtime_error( "expecting 2 string" );
         }

         bool res = createMessageBoxQuestion( v0.stringval, v1.stringval );

         RuntimeValue rt( RuntimeValue::CMP_INT );
         rt.intval = res == true;
         return rt;
      }
   };
}

void importFunctions( mvv::parser::CompilerFrontEnd& e, mvv::platform::Context&  )
{
   std::cout << "mvvForms.dll function importing..." << std::endl;
   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "setWorkingDirectory" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      ensure( fn, "can't find the function declaration in mvvForms.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionSetWorkingDirectory( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "getWorkingDirectory" ) ), std::vector<const Type*>() );
      ensure( fn, "can't find the function declaration in mvvForms.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionGetWorkingDirectory( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "boxOpenFiles" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      ensure( fn, "can't find the function declaration in mvvForms.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionBoxOpenFiles( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "boxOpenFolder" ) ), nll::core::make_vector<const Type*>( new TypeString( false ) ) );
      ensure( fn, "can't find the function declaration in mvvForms.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionBoxOpenFolder( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "boxError" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), new TypeString( false ) ) );
      ensure( fn, "can't find the function declaration in mvvForms.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionBoxError( fn ) ) );
   }

   {
      const AstDeclFun* fn = e.getFunction( nll::core::make_vector<platform::Symbol>( platform::Symbol::create( "boxQuestion" ) ), nll::core::make_vector<const Type*>( new TypeString( false ), new TypeString( false ) ) );
      ensure( fn, "can't find the function declaration in mvvForms.dll" );
      e.registerFunctionImport( platform::RefcountedTyped<FunctionRunnable>( new FunctionBoxQuestion( fn ) ) );
   }
   std::cout << "mvvForms.dll function importing done..." << std::endl;
}