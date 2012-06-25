#ifndef MVV_DICOM_TOOLS_WRITE_DICOM
# define MVV_DICOM_TOOLS_WRITE_DICOM

# include "utils.h"
# include "dicom-attributs.h"

namespace mvv
{
   template <class T, class Buffer>
   void writeDicomVolume( const nll::imaging::VolumeSpatial<T, Buffer>& volume, const std::string& directory )
   {
      mvv::ContextInstanceDicomInfo* dicomTags = NULL;
      volume.getContext().get( dicomTags );
      ensure( dicomTags, "no ContextInstanceDicomInfo found in volume context." ); // TODO: create a minimal context to save the volume?

      writeDicomVolume( volume, directory, dicomTags->attributs );
   }

   template <class T, class Buffer>
   void writeDicomVolume( const nll::imaging::VolumeSpatial<T, Buffer>& volume,
                          const std::string& directory,
                          const DicomAttributs& additionalAttributs )
   {
      typedef nll::imaging::VolumeSpatial<T, Buffer> Volume;

      const T minVal = *std::min_element( volume.begin(), volume.end() );
      const T maxVal = *std::max_element( volume.begin(), volume.end() );

      const double size = ( (ui32)1 << 16 ) - 1;
      const double slope = ( maxVal - minVal ) / size;
      const double intercept = minVal;

      std::shared_ptr<ui16> sliceBuf( new ui16[ volume.getSize()[ 0 ] * volume.getSize()[ 1 ] ] );

      // first fill up the DICOM info using the <additionalAttributs>
      DcmDataset dataset;
      DicomAttributs::exportTagsToDataset( additionalAttributs, dataset );
      DicomWrapper dicomWrapper( dataset );

      // then for each slice, fill up the correct info
      const nll::core::vector3f dx( volume.getPst()( 0, 0 ) / volume.getSpacing()[ 0 ],
                                    volume.getPst()( 1, 0 ) / volume.getSpacing()[ 0 ],
                                     volume.getPst()( 2, 0 ) / volume.getSpacing()[ 0 ] );
      const nll::core::vector3f dy( volume.getPst()( 0, 1 ) / volume.getSpacing()[ 1 ],
                                    volume.getPst()( 1, 1 ) / volume.getSpacing()[ 1 ],
                                    volume.getPst()( 2, 1 ) / volume.getSpacing()[ 1 ] );
      const nll::core::vector3f dz( volume.getPst()( 0, 2 ),     // here we want the spacing!
                                    volume.getPst()( 1, 2 ),
                                    volume.getPst()( 2, 2 ) );
      for ( ui32 slice = 0; slice < volume.getSize()[ 2 ]; ++slice )
      {
         // write volume info
#ifdef ORIENTATION_FOR_MATLAB_COMPABILITY
         nll::core::vector3f position = nll::core::vector3f( volume.getOrigin()[ 0 ],
                                                            -volume.getOrigin()[ 1 ],
                                                             volume.getOrigin()[ 2 ] ) + dz * slice;
#else
         nll::core::vector3f position = volume.getOrigin() + dz * slice;
#endif
         dicomWrapper.setImagePositionPatient( position );
         dicomWrapper.setImageOrientationPatient( dx, dy );
         dicomWrapper.setPixelSpacing( nll::core::vector2f( volume.getSpacing()[ 0 ], volume.getSpacing()[ 1 ] ) );
         dicomWrapper.setRescaleIntercept( intercept );
         dicomWrapper.setRescaleSlope( slope );
         dicomWrapper.setColumns( volume.getSize()[ 0 ] );
         dicomWrapper.setRows( volume.getSize()[ 1 ] );
         dicomWrapper.setPixelRepresentation( 0 ); // unsigned
         dicomWrapper.setBitsAllocated( 16 );
         dicomWrapper.setBitsStored( 16 );
         dicomWrapper.setHighBit( 15 );
         dicomWrapper.setSamplesPerPixel( 1 );

         for ( ui32 y = 0; y < volume.getSize()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < volume.getSize()[ 0 ]; ++x )
            {
               const double val = ( volume( x, y, slice ) - intercept ) / slope;
               assert( val >= 0 && val <= size );
               const ui16 vali = static_cast<ui16>( val );

               sliceBuf.get()[ x + y * volume.getSize()[ 0 ] ] = vali;
            }
         }
         dicomWrapper.setPixelData( sliceBuf.get() );

         std::stringstream ss;
         ss << directory;
         if ( *directory.rbegin() != '/' && *directory.rbegin() != '\\' )
         {
            ss << "/";
         }
         ss << dicomWrapper.getSeriesInstanceUid() << "-" << slice << ".dcm";
         OFCondition result = dataset.saveFile( ss.str().c_str(), EXS_LittleEndianImplicit );
         ensure( result.good(), "the DICOM file could not be created." );
      }
   }


   class FunctionWriteDicomVolume : public FunctionRunnable
   {

   public:
      FunctionWriteDicomVolume( const AstDeclFun* fun, mvv::platform::Context& context, CompilerFrontEnd& e ) : FunctionRunnable( fun ), _context( context ), _e( e )
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

         if ( ( v1.type != RuntimeValue::TYPE && (*v1.vals).size() == 1 && (*v1.vals)[ 0 ].type == RuntimeValue::STRING ) ||
                v2.type != RuntimeValue::STRING )
         {
            throw std::runtime_error( "invalid argument: expected: VolumeID, string" );
         }

         ContextTools* tools = _context.get<ContextTools>();
         if ( !tools )
         {
            throw std::runtime_error( "ContextTools context has not been loaded" );
         }

         // it is guaranteed we have a volume
         mvv::platform::RefcountedTyped<Volume> vol = tools->getVolume( mvv::SymbolVolume::create( (*v1.vals)[ 0 ].stringval ) );
         ensure( !vol.isEmpty(), "volume exists but can't be found!" );

         if ( v2.type == RuntimeValue::NIL )
         {
            writeDicomVolume( *vol, v1.stringval );
         } else {
            DicomAttributs dcmAttributs;
            DicomAttributs::exportTagsToRuntime( v2, dcmAttributs );
            writeDicomVolume( *vol, v1.stringval, dcmAttributs );
         }

         RuntimeValue rt( RuntimeValue::EMPTY );
         return rt;
      }

   private:
      mvv::platform::Context&    _context;
      CompilerFrontEnd&          _e;
   };
}

#endif