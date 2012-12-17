#ifndef MVV_DICOM_TOOLS_READ_DICOM
# define MVV_DICOM_TOOLS_READ_DICOM

# include <mvvPlatform/context-tools.h>
# include <mvvScript/function-runnable.h>
# include <mvvScript/compiler-helper.h>
# include "utils.h"
# include "dicom-attributs.h"
# include <nll/nll.h>

using namespace mvv::platform;
using namespace mvv::parser;

extern mvv::ui32 dicomVolumeId;

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
            try
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
            } catch (...)
            {
               std::cerr << "Volume creation from DICOM series failed" << std::endl;
            }
         } 

         // empty or failed
         RuntimeValue rt( RuntimeValue::NIL );
         return rt;
      }

   private:
      mvv::platform::Context&    _context;
      CompilerFrontEnd&          _e;
   };


   class FunctionReadDicomVolumes : public FunctionRunnable
   {

   public:
      FunctionReadDicomVolumes( const AstDeclFun* fun, mvv::platform::Context& context, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _context( context ), _e( e )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 3 )
         {
            throw std::runtime_error( "unexpected number of arguments, expecting string, DicomAttributs" );
         }

         RuntimeValue& v0 = unref( *args[ 0 ] );
         RuntimeValue& v1 = unref( *args[ 1 ] );
         RuntimeValue& v2 = unref( *args[ 2 ] );

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
            // fill the Study index
            if ( v2.type != RuntimeValue::NIL )
            {
               v2.type = RuntimeValue::TYPE;
               if ( datasets.getStudyUids().size() > 1 )
               {
                  TypeArray* arrayint = new TypeArray( 0, *new TypeInt( false ), false );
                  v2.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), arrayint, new RuntimeValues( datasets.getStudyUids().size() - 1 ) );
                  int accum = 0;
                  for ( ui32 n = 0; n < datasets.getStudyUids().size() - 1; ++n )
                  {
                     accum += (int)datasets.getStudyUids()[ n ].size();
                     RuntimeValue v( RuntimeValue::CMP_INT );
                     v.intval = accum - 1;
                     (*v2.vals)[ n ] = v;
                  }
               }
            }

            // create the volume array
            RuntimeValue returnValue( RuntimeValue::TYPE );
            Type* ty = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("VolumeID") ) ) );
            ensure( ty, "can't find type VolumeID" );
            TypeArray* arrayty = new TypeArray( 0, *ty, false );
            returnValue.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), arrayty, new RuntimeValues( datasets.getSeriesUids().size() ) );

            // create the array of DicomAttributs if necessary
            if ( v1.type != RuntimeValue::NIL )
            {
               Type* ty2 = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("DicomAttributs") ) ) );
               ensure( ty2, "can't find type VolumeID" );
               TypeArray* arrayty2 = new TypeArray( 0, *ty2, false );

               v1.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), arrayty2, new RuntimeValues( datasets.getSeriesUids().size() ) );
               v1.type = RuntimeValue::TYPE;
            }

            ui32 index = 0;
            for ( ui32 study = 0; study < datasets.getStudyUids().size(); ++study )
            {
               for ( ui32 series = 0; series < datasets.getStudyUids()[ study ].size(); ++series )
               {
                  try
                  {
                     const size_t n = datasets.getStudyUids()[ study ][ series ];
                     RefcountedTyped<Volume> volume( datasets.constructVolumeFromSeries<float>( (ui32)n ) );
                     const std::string volumeId = "DICOM" + nll::core::val2str( dicomVolumeId );
                     volumes->volumes.insert( mvv::SymbolVolume::create( volumeId ), volume );

                     const nll::core::Context& context = (*volume).getContext();

                     // get the DICOM header
                     ContextInstanceDicomInfo* contextDicom = 0;
                     context.get( contextDicom );
                     ensure( contextDicom, "DICOM volume must be associated with a DICOM context" );
                     
                     // translate it to a modifiable structure
                     if ( v1.type != RuntimeValue::NIL )
                     {
                        std::cout << "Fill attribut=" << n << std::endl;
                        DicomAttributs::exportTagsToRuntime( (*v1.vals)[ index ], contextDicom->attributs );
                     }

                     ++dicomVolumeId;

                     // create the volume ID
                     RuntimeValue rt( RuntimeValue::TYPE );
                     Type* ty = const_cast<Type*>( _e.getType( nll::core::make_vector<mvv::Symbol>( mvv::Symbol::create("VolumeID") ) ) );
                     ensure( ty, "can't find type VolumeID" );
                     rt.vals = RuntimeValue::RefcountedValues( &_e.getEvaluator(), ty, new RuntimeValues( 1 ) );
                     (*rt.vals)[ 0 ].setType( RuntimeValue::STRING );
                     (*rt.vals)[ 0 ].stringval = volumeId;

                     (*returnValue.vals)[ index ] = rt;
                     std::cout << "INDEX=" << index << " TYPE=" << contextDicom->attributs.modality << std::endl;
                  } catch (...)
                  {
                     std::cerr << "Volume creation from DICOM series failed, index=" << index << std::endl;
                     RuntimeValue rt( RuntimeValue::NIL );
                  }
                  ++index;
               }
            }
            return returnValue;
         } else {
            RuntimeValue rt( RuntimeValue::EMPTY );
            return rt;
         }
      }

   private:
      mvv::platform::Context&    _context;
      CompilerFrontEnd&          _e;
   };

   class FunctionSizeDicomSlice : public FunctionRunnable
   {

   public:
      FunctionSizeDicomSlice( const AstDeclFun* fun ) : FunctionRunnable( fun )
      {
      }

      virtual RuntimeValue run( const std::vector<RuntimeValue*>& args )
      {
         if ( args.size() != 1 )
         {
            throw std::runtime_error( "unexpected number of arguments, expecting string, DicomAttributs" );
         }

         RuntimeValue& v0 = unref( *args[ 0 ] );

         RuntimeValue rt( RuntimeValue::CMP_INT );
         if ( v0.vals.getDataPtr() == 0 )
         {
            rt.intval = 0;
         } else {
            rt.intval = (int)(*v0.vals).size();
         }
         return rt;
      }
   };
}

#endif