#ifndef MVV_DICOM_TOOLS_READ_DICOM
# define MVV_DICOM_TOOLS_READ_DICOM

# include <mvvPlatform/context-tools.h>
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include "utils.h"

using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

extern ui32 dicomVolumeId;

namespace
{
   typedef std::map<mvv::Symbol, std::vector<size_t> >   MapSeriesUid;
   typedef std::vector<DcmFileFormat>                    DicomFiles;

   

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

         ContextVolumes* volumes = _context.get<ContextVolumes>();
         if ( !volumes )
         {
            throw std::runtime_error( "ContextVolumes context has not been loaded" );
         }

         if ( v0.type != RuntimeValue::STRING )
         {
            throw std::runtime_error( "expecting <string, DicomAttributs> as argument" );
         }

         DicomDatasets datasets;
         datasets.loadDicomDirectory( v0.stringval );

         if ( datasets.getSeriesUids().size() )
         {
            RefcountedTyped<Volume> volume( datasets.constructVolumeFromSeries<float>( 0 ) );

            ++dicomVolumeId;
            RuntimeValue rt( RuntimeValue::EMPTY );
            return rt;
         } else {
            RuntimeValue rt( RuntimeValue::EMPTY );
            return rt;
         }
      }

   private:
      mvv::platform::Context&    _context;
   };
}

#endif