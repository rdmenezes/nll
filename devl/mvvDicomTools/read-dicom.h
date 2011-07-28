#ifndef MVV_DICOM_TOOLS_READ_DICOM
# define MVV_DICOM_TOOLS_READ_DICOM

# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include <Boost/filesystem.hpp>

using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

namespace
{
   class FunctionReadDicomVolume : public FunctionRunnable
   {

   public:
      FunctionReadDicomVolume( const AstDeclFun* fun, mvv::platform::Context& context ) : FunctionRunnable( fun ), _context( context )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 2 )
         {
            throw std::runtime_error( "unexpected number of arguments, expecting string, DicomAttributs" );
         }

         RuntimeValue& v0 = unref( *args[ 0 ] );
         RuntimeValue& v1 = unref( *args[ 1 ] );

         if ( v0.type != RuntimeValue::STRING )
         {
            throw std::runtime_error( "expecting <string, DicomAttributs> as argument" );
         }

         // get the list of files


         

         RuntimeValue rt( RuntimeValue::EMPTY );
         return rt;
      }

   private:
      mvv::platform::Context&    _context;
   };
}

#endif