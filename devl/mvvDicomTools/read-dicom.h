#ifndef MVV_DICOM_TOOLS_READ_DICOM
# define MVV_DICOM_TOOLS_READ_DICOM

# include <mvvPlatform/context-tools.h>
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include "utils.h"
# include "dicom-attributs.h"

using namespace mvv::platform;
using namespace mvv::parser;
using namespace mvv;

extern ui32 dicomVolumeId;

namespace mvv
{
   typedef std::map<mvv::Symbol, std::vector<size_t> >   MapSeriesUid;
   typedef std::vector<DcmFileFormat>                    DicomFiles;

   

   class FunctionReadDicomVolume : public FunctionRunnable
   {

   public:
      FunctionReadDicomVolume( const AstDeclFun* fun, mvv::platform::Context& context, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _context( context ), _e( e )
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
            const std::string volumeId = "DICOM" + nll::core::val2str( dicomVolumeId );
            volumes->volumes.insert( mvv::SymbolVolume::create( volumeId ), volume );

            const nll::core::Context& context = (*volume).getContext();

            // get the DICOM header
            ContextInstanceDicomInfo* contextDicom = 0;
            context.get( contextDicom );
            ensure( contextDicom, "DICOM volume must be associated with a DICOM context" );
            
            // translate it to a modifiable structure
            if ( v1.type != RuntimeValue::NIL && v1.type != RuntimeValue::EMPTY )
            {
               DicomAttributs::exportTagsToRuntime( v1, contextDicom->attributs );
            }

            ++dicomVolumeId;

            // create the volume ID
            RuntimeValue rt( RuntimeValue::TYPE );
            Type* ty = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("VolumeID") ) ) );
            ensure( ty, "can't find type VolumeID" );

            rt.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), ty, new RuntimeValues( 1 ) );
            (*rt.vals)[ 0 ].setType( RuntimeValue::STRING );
            (*rt.vals)[ 0 ].stringval = volumeId;
            return rt;
         } else {
            RuntimeValue rt( RuntimeValue::EMPTY );
            return rt;
         }
      }

   private:
      mvv::platform::Context&    _context;
      CompilerFrontEnd&          _e;
   };
}

#endif