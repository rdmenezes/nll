#include "sliceMapper.h"

// Case7: fat
// Case12: very big lungs

using namespace nll;

namespace mvv
{
namespace mapper
{
   SliceBasicPreprocessing::Image
   SliceBasicPreprocessing::preprocessSlice( const SliceBasicPreprocessing::Volume& volume, unsigned sliceNumber ) const
   {
      // we use 2 luts: one with a lot of soft tissue to find the center of mass (with the bony LUT, it depends on the skeleton which is not smooth)
      // the other one is a bony LUT to compute the extent of the bounding box.

      // get the LUT transformed slice
      typedef nll::imaging::Slice<unsigned char> Slice;
      Slice sliceReal( nll::core::vector3ui( volume.getSize()[ 0 ], volume.getSize()[ 1 ], 1 ),
                       nll::core::vector3f( 1.0f, 0, 0 ),
                       nll::core::vector3f( 0, 1.0f, 0 ),
                       nll::core::vector3f( volume.getSize()[ 0 ] / 2.0f, volume.getSize()[ 1 ] / 2.0f, 0 ),
                       nll::core::vector2f( 1.0f, 1.0f ) );
      Image& slice = sliceReal.getStorage();
      Image sliceMask( volume.getSize()[ 0 ], volume.getSize()[ 1 ], 1, false );

      for ( ui32 y = 0; y < slice.sizey(); ++y )
      {
         for ( ui32 x = 0; x < slice.sizex(); ++x )
         {
            const float volVal = volume( x, y, sliceNumber );
            const float val = _lut.transform( volVal )[ 0 ];
            const float valMask = _lutMask.transform( volVal )[ 0 ];
            slice( x, y, 0 ) = (ui8)val;
            sliceMask( x, y, 0 ) = (ui8)valMask;
         }
      }

      // extract the biggest connected component that is not '0'
      typedef nll::algorithm::Labelize<nll::ui8,
                                       nll::core::IndexMapperRowMajorFlat2DColorRGBn,
                                       nll::algorithm::RegionPixelSpecific<nll::ui8> >  Labelize;

      ui8 black[] = { 0 };
      Labelize::DifferentPixel different( 1, black, 10 );

      Labelize l( different );
      Labelize::ComponentsInfo info = l.run( sliceMask, true );
      
      int max = 0;
      int maxIndex = -1;
      for ( ui32 n = 0; n < info.components.size(); ++n )
      {
         if ( info.components[ n ].size > (ui32)max &&
              sliceMask( info.components[ n ].posx, info.components[ n ].posy, 0 ) > 0 )
         {
            maxIndex = n;
            max = info.components[ n ].size;
         }
      }

      if ( maxIndex >= 0 )
      {
         slice = extract( slice, info.labels, info.components[ maxIndex ].id );
      }

      cropVertical( slice, _params.verticalCroppingRatio, 2 );
      cropHorizontal( slice, _params.horizontalCroppingRatio, 2 );
      core::rescaleBilinear( slice, _params.preprocessSizeX, _params.preprocessSizeY );
      return slice;
   }

   SliceBasicPreprocessing::Imagef
   SliceBasicPreprocessing::preprocessSlicef( const SliceBasicPreprocessing::Volume& volume, unsigned sliceNumber ) const
   {
      Image i = preprocessSlice( volume, sliceNumber );
      Imagef imf( i.sizex(), i.sizey(), 1 );
      for ( ui32 n = 0; n < i.size(); ++n )
      {
         imf[ n ] = i[ n ] / 256.0;
      }
      return imf;
   }

   SliceBasicPreprocessing::Database
   SliceBasicPreprocessing::createPreprocessedDatabase( const SliceMapperDataset& datasets ) const
   {
      Database database;
      for ( unsigned caseid = 0; caseid < datasets.size(); ++caseid )
      {
         std::cout << "preprocessing case=" << caseid << " out of=" << datasets.size() << std::endl;

         Database::Sample::Type type = (Database::Sample::Type)datasets.getSampleType( caseid );
         std::auto_ptr<Volume> v = datasets.loadData( caseid );

         // prepare a quick slice type lookup indicating the landmark
         std::vector<int> sliceType( v->getSize()[ 2 ], -1 );
         for ( ui32 index = 0; index < datasets[ caseid ].landmarks.size(); ++index )
         {
            const SliceMapperDataset::Dataset& dataset = datasets[ caseid ];
            if ( dataset.landmarks[ index ][ 0 ] >= 0 && // only the position with positive coordinates are right
                 dataset.landmarks[ index ][ 1 ] >= 0 &&
                 dataset.landmarks[ index ][ 2 ] >= 0 )
            {
               const float zSlicePos = dataset.landmarks[ index ][ 2 ];
               const unsigned sliceIndex = core::round( zSlicePos );
               sliceType[ sliceIndex ] = static_cast<int>( index );
            }
         }

         // now go through all the slices and extract them using simple preprocessing. This processing
         // level is relatively independent to the classification step (i.e., the features used by the classifier)
         // so that it is regenerated much less frequently, as this step is relatively long

         // first precompute the samples, as we want to keep the slices ordered!
         std::vector<Database::Sample> samples( v->getSize()[ 2 ] );
         #pragma omp parallel for
         for ( int slice = 0; slice < (int)v->getSize()[ 2 ]; ++slice )
         {
            std::cout << ".";

            Imagef i = preprocessSlicef( *v, slice );
            
            Database::Sample::Input input = i;
            Database::Sample::Output output = 0;
            Database::Sample::String str = nll::core::make_buffer1D_from_string( nll::core::val2str( caseid ) );

            if ( sliceType[ slice ] != -1 )
            {
               output = sliceType[ slice ];
            }

            Database::Sample sample( input, output, (Database::Sample::Type)type );
            samples[ slice ] = sample;
         }

         // finally, add them to the database
         for ( unsigned slice = 0; slice < v->getSize()[ 2 ]; ++slice )
         {
            database.add( samples[ slice ] );
         }
      }

      return database;
   }

   SlicePreprocessingClassifierInput::Point
   SlicePreprocessingClassifierInput::preprocess( const IntegralImage& preprocessedSlice, unsigned classifierId ) const
   {
      Point haar = _featuresByType[ classifierId ].process( preprocessedSlice );
      const Pca& pca = _featureReductionByType[ classifierId ];

      Point features = pca.process( haar );
      return features;
   }


   void SlicePreprocessingClassifierInput::computeClassifierFeatures( const SlicePreprocessingClassifierInput::Database& preprocessedSliceDatabase )
   {
      // first create the corresponding haar features
      _createHaarFeatures( preprocessedSliceDatabase );

      std::vector<Database> dats = _sortAndSelectDatabaseByClassifier( preprocessedSliceDatabase );

      // extract the haar features
      #pragma omp parallel for
      for ( int datid = 0; datid < (int)dats.size(); ++datid )
      {
         for ( ui32 n = 0; n < dats[ datid ].size(); ++n )
         {
            nll::core::Buffer1D<double> buffer = dats[ datid ][ n ].input;
            Imagef bufferAsImage( buffer,
                                  _paramsPreprocessing.preprocessSizeX,
                                  _paramsPreprocessing.preprocessSizeY,
                                  1 );
            IntegralImage integralImage;
            integralImage.process( bufferAsImage );

            Point haar = _featuresByType[ datid ].process( integralImage );
            dats[ datid ][ n ].input = haar;
         }
      }

      // compute the PCA transform
      _featureReductionByType = std::vector<Pca>( dats.size() ); //.clear();
      #pragma omp parallel for
      for ( int n = 0; n < (int)dats.size(); ++n )
      {
         std::cout << "compute PCA projector:" << n << " dataSize=" << dats[ n ].size() << " toNbComponents=" <<  _params.nbFinalFeatures << std::endl;
         const ui32 nbClasses = nll::core::getNumberOfClass( dats[ n ] );
         ensure(  nbClasses == 2, "this must be a binary classification: class or not class" );

         typedef nll::core::DatabaseInputAdapterRead<Database> Points;
         Points adapter( dats[ n ] );
    
         // now reduce the feature size using PCA
         nll::algorithm::PrincipalComponentAnalysis<Points> pca;
         bool computed = pca.compute( adapter, _params.nbFinalFeatures );
         ensure( computed, "PCA failed for landmark=" + nll::core::val2str( n ) );

         _featureReductionByType[ n ] = Pca( pca );
      }
   }

   void SlicePreprocessingClassifierInput::_createHaarFeatures( const SlicePreprocessingClassifierInput::Database& dat )
   {
      typedef HaarFeatures::Feature Feature;

      // create a pyramid like feature detector, with number of levels = <nbScales>, and nb features=<nbFeaturesFirstLevel> for the first level
      // for each following level, the number of features is decreased by 2
      const ui32 nbScales = 3;
      const ui32 nbFeaturesFirstLevel = 12;

      // compute the number of features
      ui32 nbFeatures = 0;
      ui32 nbFeaturesPerLevelTmp = nbFeaturesFirstLevel;
      for ( ui32 n = 0; n < nbScales; ++n )
      {
         nbFeatures += nbFeaturesFirstLevel * nbFeaturesFirstLevel;
         nbFeaturesPerLevelTmp /= 2;
      }
      ensure( nbFeatures > 1, "not enough features!" );

      std::cout << "preprocessing slice: nbHaarFeatures=" << nbFeatures * 2 << std::endl;

      // now instanciate the haar features
      const ui32 nbClasses = nll::core::getNumberOfClass( dat );
      ensure( nbFeatures >= 2, "not enough classes in the database, 0 is not considered as a class!!!" );
      _featuresByType = std::vector<HaarFeatures>( nbClasses );

      for ( ui32 type = 0; type < nbClasses; ++type )
      {
         nbFeaturesPerLevelTmp = nbFeaturesFirstLevel;
         for ( ui32 level = 0; level < nbScales; ++level )
         {
            ensure( nbFeaturesPerLevelTmp > 0, "too many levels and not enough nbFeaturesFirstLevel" );
            const double step = 1.0 / nbFeaturesPerLevelTmp;
            for ( ui32 x = 0; x < nbFeaturesPerLevelTmp; ++x )
            {
               for ( ui32 y = 0; y < nbFeaturesPerLevelTmp; ++y )
               {
                  const double centerx = x * step;
                  const double centery = y * step;

                  _featuresByType[ type ].add( Feature( Feature::VERTICAL,
                                                        core::vector2d( centerx, centery ), 
                                                        core::vector2d( centerx + step, centery + step ) ) );
                  _featuresByType[ type ].add( Feature( Feature::HORIZONTAL,
                                                        core::vector2d( centerx, centery ), 
                                                        core::vector2d( centerx + step, centery + step ) ) );
               }
            }
         }

         nbFeaturesPerLevelTmp /= 2;
      }
   }

   std::vector<SlicePreprocessingClassifierInput::Point>
   SlicePreprocessingClassifierInput::_computeHaarFeatures( const SlicePreprocessingClassifierInput::IntegralImage& input )
   {
      std::vector<Point> points( _featuresByType.size() );
      for ( size_t n = 0; n < _featuresByType.size(); ++n )
      {
         points[ n ] = _featuresByType[ n ].process( input );
      }
      return points;
   }

   std::vector<SlicePreprocessingClassifierInput::Database>
   SlicePreprocessingClassifierInput::_sortAndSelectDatabaseByClassifier( const SlicePreprocessingClassifierInput::Database& preprocessedSliceDatabase ) const
   {
      const ui32 nbClasses = nll::core::getNumberOfClass( preprocessedSliceDatabase );
      std::vector<Database> datsInput;
      for ( ui32 datid = 1; datid < nbClasses; ++datid )
      {
         int lastNonSliceOfInterestIndex = std::numeric_limits<int>::max();
         Database dat;
         int lastLandmarkOfInterest = std::numeric_limits<int>::max();
         int currentId = -1;
         for ( ui32 n = 0; n < preprocessedSliceDatabase.size(); ++n )
         {
            int sliceId = nll::core::str2val<int>( nll::core::string_from_Buffer1D( preprocessedSliceDatabase[ n ].debug ) );
            if ( sliceId != currentId )
            {
               // when we change of volumes, we also must reinit the min slice distances
               currentId = sliceId;
               lastNonSliceOfInterestIndex = std::numeric_limits<int>::max();
               lastLandmarkOfInterest = std::numeric_limits<int>::max();
            }
            Database::Sample sample;

            sample.input = preprocessedSliceDatabase[ n ].input;
            sample.type = preprocessedSliceDatabase[ n ].type;
            if ( datid == preprocessedSliceDatabase[ n ].output ) // if the landmark id is the same than the datid, this is a sample of interest for this classifier
            {
               sample.output = 1;
               lastLandmarkOfInterest = n;
            } else {
               sample.output = 0;

               // now skip the slice if necessary
               ui32 dist = std::abs( lastNonSliceOfInterestIndex - (int)n );
               if ( dist < _params.skipSliceInterval )
               {
                  continue;
               } else {
                  lastNonSliceOfInterestIndex = n;
               }
            }

            if ( sample.output == 0 )
            {
               int dist = abs( lastLandmarkOfInterest - (int)n );
               if ( (ui32)dist < _params.minDistanceBetweenSliceOfInterest )
               {
                  continue;
               }
            }
            dat.add( sample );
         }

         datsInput.push_back( dat );
      }

      return datsInput;
   }

   std::vector<SlicePreprocessingClassifierInput::Database>
   SlicePreprocessingClassifierInput::createClassifierInputDatabases( const SlicePreprocessingClassifierInput::Database& preprocessedSliceDatabase ) const
   {
      ensure( _featuresByType.size() && _featureReductionByType.size(), "first: preprocess the database to compute the haar features" );
      ensure( nll::core::getNumberOfClass( preprocessedSliceDatabase ) == _featuresByType.size(), "error: number of classes doens't match" );

      // this can be greatly improved by storing an index of the sorted list and compute only one time the <IntegralImage> per slice
      std::vector<Database> sortedSlice = _sortAndSelectDatabaseByClassifier( preprocessedSliceDatabase );
      for ( ui32 datid = 0; datid < sortedSlice.size(); ++datid )
      {
         for ( ui32 n = 0; n < sortedSlice[ datid ].size(); ++n )
         {
            nll::core::Buffer1D<double> buffer = sortedSlice[ datid ][ n ].input;
            Imagef bufferAsImage( buffer,
                                  _paramsPreprocessing.preprocessSizeX,
                                  _paramsPreprocessing.preprocessSizeY,
                                  1 );
            IntegralImage integralImage;
            integralImage.process( bufferAsImage );
            sortedSlice[ datid ][ n ].input = preprocess( integralImage, datid );
         }
      }
      return sortedSlice;
   }

   void
   SliceMapperClassifierSvm::destroy()
   {
      for ( size_t n = 0; n < _classifiers.size(); ++n )
      {
         delete _classifiers[ n ];
      }
      _classifiers.clear();
   }

   SliceMapperClassifierSvm::~SliceMapperClassifierSvm()
   {
      destroy();
   }

   void
   SliceMapperClassifierSvm::learn( const SliceMapperClassifierSvmParameters& params, const std::vector<SliceMapperClassifierSvm::Database>& databases )
   {
      _classifiers = std::vector< Classifier* >( databases.size() );

      #pragma omp parallel for
      for ( int n = 0; n < (int)databases.size(); ++n )
      {
         if ( databases[ n ].size() )
         {
            std::cout << "Learning classifier=" << n << " nbInputFeatures=" << databases[ n ][ 0 ].input.size() << std::endl;
         }

         std::auto_ptr<Classifier> c( new Classifier( true, true ) );
         const Classifier::Database& dat = databases[ n ];
         c->learn( dat, nll::core::make_buffer1D<double>( params.gamma, params.costMargin ) );

         _classifiers[ n ] = c.release();
      }
   }
}
}



/*
// PREVIOUS VERSION

namespace mvv
{
namespace mapper
{
// from the center of mass:
   // scan in left, right, up and down direction until desired ratio is reached. If in a direction there is a line that doesn't have enough points, we impose the limit on the extent
   void getExtent( const SliceMapper::Image& slice, const nll::core::vector2i& center, double ratiox, double ratioy, const std::vector<double>& linex, const std::vector<double>& liney, double totalPixels, nll::core::vector2i& minBB, nll::core::vector2i& maxBB )
   {
      minBB = nll::core::vector2i( -1, -1 );
      maxBB = nll::core::vector2i( -1, -1 );

      double nbPixels = 0;
      int n = 0;
      for ( ; n < (int)slice.sizex() / 2; ++n )
      {
         const int x1 = (int)center[ 0 ] + n + 1;
         const int x2 = (int)center[ 0 ] - n;
         if ( x1 + 1 >= (int)slice.sizex() || x2 <= 0 )
            break;
         nbPixels += liney[ x1 ] + liney[ x2 ];
         const double ratio = nbPixels / totalPixels;
         if ( ratio >= ratiox )
            break;

         // now check we have enough voxel on the line we are checking, else constaint the extent within X
         // for this we want the points close to the center to be very likely to be inside, then less and less
         const double ratiox1 = liney[ x1 ] / slice.sizex();
         const double ratiox2 = liney[ x2 ] / slice.sizex();
         const double ratioPos = (double)n / ( slice.sizex() / 2 );
         const double ratioxToMatch = exp( ratioPos / 2 ) - 1;

         if ( maxBB[ 0 ] < 0 && ratiox1 < ratioxToMatch )
         {
            maxBB[ 0 ] = center[ 0 ] + n;
         }
         if ( minBB[ 0 ] < 0 && ratiox2 < ratioxToMatch )
         {
            minBB[ 0 ] = center[ 0 ] - n;
         }
      }

      if ( maxBB[ 0 ] < 0 )
         maxBB[ 0 ] = center[ 0 ] + n;
      if ( minBB[ 0 ] < 0 )
         minBB[ 0 ] = center[ 0 ] - n;

      nbPixels = 0;
      n = 0;
      for ( ; n < (int)slice.sizey() / 2; ++n )
      {
         const int y1 = (int)center[ 1 ] + n + 1;
         const int y2 = (int)center[ 1 ] - n;
         if ( y1 + 1 >= (int)slice.sizey() || y2 <= 0 )
            break;
         nbPixels += linex[ y1 ] + linex[ y2 ];
         const double ratio = nbPixels / totalPixels;
         if ( ratio >= ratioy )
            break;

         // now check we have enough voxel on the line we are checking, else constaint the extent within
         const double ratioy1 = linex[ y1 ] / slice.sizey();
         const double ratioy2 = linex[ y2 ] / slice.sizey();
         const double ratioyToMatch = ratioy / 8;
         if ( maxBB[ 1 ] < 0 && ratioy1 < ratioyToMatch )
         {
            maxBB[ 1 ] = center[ 1 ] + n;
         }
         if ( minBB[ 1 ] < 0 && ratioy2 < ratioyToMatch )
         {
            minBB[ 1 ] = center[ 1 ] - n;
         }
      }

      if ( maxBB[ 1 ] < 0 )
         maxBB[ 1 ] = center[ 1 ] + n;
      if ( minBB[ 1 ] < 0 )
         minBB[ 1 ] = center[ 1 ] - n;

      // we want a symetry in X (in case one arm is up, the other down), so do the average in X
      const double sizex = ( abs( maxBB[ 0 ] - center[ 0 ] ) +
                             abs( minBB[ 0 ] - center[ 0 ] ) ) / 2;
      const double sizey = ( abs( maxBB[ 1 ] - center[ 1 ] ) +
                             abs( minBB[ 1 ] - center[ 1 ] ) ) / 2;
      minBB[ 0 ] = center[ 0 ] - sizex;
      maxBB[ 0 ] = center[ 0 ] + sizex;
   }

   SliceMapper::Image SliceMapper::preprocessSlice( const SliceMapper::Volume& volume, unsigned sliceNumber ) const
   {
      // we use 2 luts: one with a lot of soft tissue to find the center of mass (with the bony LUT, it depends on the skeleton which is not smooth)
      // the other one is a bony LUT to compute the extent of the bounding box.

      // get the LUT transformed slice and compute a barycenter, compute the sums for each vertical/horizontal line
      typedef nll::imaging::Slice<unsigned char> Slice;
      Slice sliceReal( nll::core::vector3ui( volume.getSize()[ 0 ], volume.getSize()[ 1 ], 1 ),
                       nll::core::vector3f( 1, 0, 0 ),
                       nll::core::vector3f( 0, 1, 0 ),
                       nll::core::vector3f( volume.getSize()[ 0 ] / 2, volume.getSize()[ 1 ] / 2, 0 ),
                       nll::core::vector2f( 1, 1 ) );
      Image& slice = sliceReal.getStorage();
      Image sliceMask( volume.getSize()[ 0 ], volume.getSize()[ 1 ], 1, false );

      double dx = 0;
      double dy = 0;
      double nbPointsCenter = 0;
      double nbPoints = 0;
      std::vector<double> lineSumX( slice.sizey() );
      std::vector<double> lineSumY( slice.sizex() );
      for ( ui32 y = 0; y < slice.sizey(); ++y )
      {
         double sumx = 0;
         for ( ui32 x = 0; x < slice.sizex(); ++x )
         {
            const float volVal = volume( x, y, sliceNumber );
            const float val = _lut.transform( volVal )[ 0 ];
            const float valMask = _lutMask.transform( volVal )[ 0 ];
            slice( x, y, 0 ) = val;
            sliceMask( x, y, 0 ) = valMask;

            const double bound = 1;

            // compute the center
            if ( val > 0 )
            {
               dx += x * bound;
               dy += y * bound;
               nbPointsCenter += bound;
            }

            // compute the line sums
            if ( valMask > 0 )
            {
               sumx += bound;
               lineSumY[ x ] += bound;
               nbPoints += bound;
            }
         }
         lineSumX[ y ] = sumx;
      }


      const nll::core::vector2f center( dx / nbPointsCenter, dy / nbPointsCenter ); 

      // compute the distance we need to crop, retain X% in a direction from the center
      nll::core::vector2i minBB;
      nll::core::vector2i maxBB;
      getExtent( sliceMask, nll::core::vector2i( center[ 0 ], center[ 1 ] ), 0.85, 0.85, lineSumX, lineSumY, nbPoints, minBB, maxBB );
      const nll::core::vector2f centerI( slice.sizex() / 2 - center[ 0 ],
                                         slice.sizey() / 2 - center[ 1 ] );

      for ( int n = 0; n < 3; ++n )
      {
         sliceMask( minBB[ 0 ] + n, minBB[ 1 ], 0 ) = 255;
         sliceMask( minBB[ 0 ] - n, minBB[ 1 ], 0 ) = 255;
         sliceMask( minBB[ 0 ], minBB[ 1 ] + n, 0 ) = 255;
         sliceMask( minBB[ 0 ], minBB[ 1 ] - n, 0 ) = 255;
      }

      for ( int n = 0; n < 3; ++n )
      {
         sliceMask( center[ 0 ] + n , center[ 1 ]     , 0 ) = 255;
         sliceMask( center[ 0 ] - n , center[ 1 ]     , 0 ) = 255;
         sliceMask( center[ 0 ]     , center[ 1 ] + n , 0 ) = 255;
         sliceMask( center[ 0 ]     , center[ 1 ] - n , 0 ) = 255;
      }

      for ( int n = 0; n < 3; ++n )
      {
         sliceMask( maxBB[ 0 ] + n, maxBB[ 1 ], 0 ) = 255;
         sliceMask( maxBB[ 0 ] - n, maxBB[ 1 ], 0 ) = 255;
         sliceMask( maxBB[ 0 ], maxBB[ 1 ] + n, 0 ) = 255;
         sliceMask( maxBB[ 0 ], maxBB[ 1 ] - n, 0 ) = 255;
      }
      return sliceMask;

      // finally resample the image with the correct center and size
      Slice sliceResampled( nll::core::vector3ui( maxBB[ 0 ] - minBB[ 0 ] + 1, maxBB[ 1 ] - minBB[ 1 ] + 1, 1 ),
                            nll::core::vector3f( 1, 0, 0 ),
                            nll::core::vector3f( 0, 1, 0 ),
                            nll::core::vector3f( ( maxBB[ 0 ] + minBB[ 0 ] ) / 2, ( maxBB[ 1 ] + minBB[ 1 ] ) / 2, 0 ),
                            nll::core::vector2f( 1, 1 ) );

      typedef nll::core::InterpolatorNearestNeighbor2D<unsigned char, Image::IndexMapper, Image::Allocator> Interpolator;
      Interpolator interpolator( slice );
      nll::imaging::resampling<unsigned char, Interpolator> ( sliceReal, sliceResampled );

      // finally resample to the final size. Note, this step could be performed at the same time as the resampling...
      core::rescaleBilinear( sliceResampled.getStorage(), _params.preprocessSizeX, _params.preprocessSizeY );
      return sliceResampled.getStorage();
   }
}
}*/