#ifndef DETECT_TEST_H
# define DETECT_TEST_H

# include "compute-barycentre.h"

namespace nll
{
namespace detect
{
   class TestVolume
   {
      typedef algorithm::FeatureSelectionFilterPearson< core::Buffer1D<double> > FeatureSelection;

   public:
      struct Result
      {
         std::vector<ui32>    sliceIds;
         std::vector<f64>     probabilities;
      };

      struct ResultFinal
      {
         ResultFinal()
         {
            neckStart = -1;
            heartStart = -1;
            lungStart = -1;
            skullStart = -1;
            hipsStart = -1;
         }

         int   neckStart;
         int   heartStart;
         int   lungStart;
         int   skullStart;
         int   hipsStart;
      };

      typedef core::Buffer1D<double>                                 Point;
      typedef algorithm::Classifier<Point>                           Classifier;
      typedef algorithm::FeatureTransformationNormalization<Point>   Normalization;

      TestVolume( Classifier* classifier, const std::string& haarFeatures, const std::string& haarNormalization, const std::string& featureSelection ) : _selection( 0 )
      {
         _classifier = classifier;
         algorithm::Haar2dFeatures::read( _haar, haarFeatures );
         _normalization.read( haarNormalization );
         _selection.read( featureSelection );
      }

      ResultFinal test( const Volume& v )
      {
         return test( rawTest( v ) );
      }

      ResultFinal test( const Volume& v, Result& results )
      {
         results = rawTest( v );
         return test( results );
      }

      /**
       @brief from a set of slice results, guess the real locations
       */
      ResultFinal test( const Result& results )
      {
         ResultFinal r;

         core::Buffer1D<double>  max( NB_CLASS );
         core::Buffer1D<int>     maxPos( NB_CLASS );
         for ( ui32 n = 0; n < NB_CLASS; ++n )
            maxPos[ n ] = -1;

         // select the highest probability
         for ( ui32 n = 0; n < results.probabilities.size(); ++n )
         {
            if ( results.probabilities[ n ] > max[ results.sliceIds[ n ] ] )
            {
               max[ results.sliceIds[ n ] ] = results.probabilities[ n ];
               maxPos[ results.sliceIds[ n ] ] = n;
            }
         }

         // special rule for the head: we go up until can't find the same class
         bool cont = true;
         while ( cont )
         {
            int max = 0;
            cont = false;
            for ( int n = 1; n <= 20 && ( maxPos[ 4 ] + n ) < results.sliceIds.size(); ++n )
               if ( results.sliceIds[ maxPos[ 4 ] + n ] == 4 )
               {
                  max = n;
                  cont = true;
               }
            maxPos[ 4 ] += max;
         }
         
         r.neckStart =   maxPos[ 1 ];
         r.heartStart =  maxPos[ 2 ];
         r.lungStart =   maxPos[ 3 ];
         r.skullStart =  maxPos[ 4 ];
         r.hipsStart =   maxPos[ 5 ];
         return r;
      }

      /**
       @brief returns the class ID for each slice of the volume
       */
      Result rawTest( const Volume& volume )
      {
         Result results;
         std::vector<ui32> ids( volume.size()[ 2 ] );
         std::vector<f64> pbs( volume.size()[ 2 ] );
         for ( ui32 n = 0; n < volume.size()[ 2 ]; ++n )
         {
            core::Buffer1D<double> pb;
            Point features = getFeatures( volume, n );
            ids[ n ] = _classifier->test( features, pb );
            pbs[ n ] = pb[ ids[ n ] ];
            std::cout << "p=" << pbs[ n ] << "class=" << ids[ n ] << std::endl;
         }
         results.sliceIds = ids;
         results.probabilities = pbs;
         return results;
      }

      /**
       @param feature: haar normalized feature
       */
      ui32 rawTest( const Point& features, f64& probability )
      {
         core::Buffer1D<double> pb;
         ui32 c = _classifier->test( features, pb );
         probability = pb[ c ];
         return c;
      }

      /**
       @brief export on a XZ mpr the classification result
       */
      /*
      core::Image<ui8> exportTest( const Volume& volume )
      {
         typedef nll::imaging::LookUpTransformWindowingRGB  Lut;

         // classifiy the slices
         std::vector<ui32> r = rawTest( volume ).sliceIds;

         // extract the slice on XZ plan
         Lut lut( REGION_DETECTION_BARYCENTRE_LUT_MIN, REGION_DETECTION_BARYCENTRE_LUT_MAX, 256 );
         lut.createGreyscale();

         const core::vector3f centre = computeBarycentre( volume, lut );
         const core::vector3f centreVoxel = volume.positionToIndex( centre );
         const core::vector3f mprCentre1 = volume.indexToPosition( core::vector3f( static_cast<f32>( volume.size()[ 0 ] ) / 2,
                                                                                   centreVoxel[ 1 ],
                                                                                   static_cast<f32>( volume.size()[ 2 ] ) / 2 ) );

         const core::vector3f vector_x( volume.getPst()( 0, 0 ) / volume.getSpacing()[ 0 ],
                                        volume.getPst()( 1, 0 ) / volume.getSpacing()[ 0 ],
                                        volume.getPst()( 2, 0 ) / volume.getSpacing()[ 0 ]);

         const core::vector3f vector_z( volume.getPst()( 0, 2 ) / volume.getSpacing()[ 2 ],
                                        volume.getPst()( 1, 2 ) / volume.getSpacing()[ 2 ],
                                        volume.getPst()( 2, 2 ) / volume.getSpacing()[ 2 ]);


         ui32 sx1 = static_cast<ui32>( volume.size()[ 0 ] * volume.getSpacing()[ 0 ] / volume.getSpacing()[ 2 ] );

         core::Image<ui8> sliceTfm(sx1, volume.size()[ 2 ], 3 );

         typedef imaging::Mpr< Volume, imaging::InterpolatorTriLinear<Volume> >  Mpr;
         typedef Mpr::Slice                                                      Slice;

         Slice slice( core::vector3ui( sliceTfm.sizex(), sliceTfm.sizey(), 3 ),
                      vector_x,
                      vector_z,
                      mprCentre1,
                      core::vector2f( volume.getSpacing()[ 2 ], volume.getSpacing()[ 2 ] ) );

         Mpr mpr( volume );
         mpr.getSlice( slice );
         for ( ui32 y = 0; y < sliceTfm.sizey(); ++y )
         {
            for ( ui32 x = 0; x < sliceTfm.sizex(); ++x )
            {
               const float* value = lut.transform( slice( x, y, 0 ) );

               ui8* pos = sliceTfm.point( x, y );
               pos[ 0 ] = static_cast<ui8>( NLL_BOUND( value[ 0 ], 0, 255 ) );
               pos[ 1 ] = static_cast<ui8>( NLL_BOUND( value[ 1 ], 0, 255 ) );
               pos[ 2 ] = static_cast<ui8>( NLL_BOUND( value[ 2 ], 0, 255 ) );
            }
         }

         // annotate the slice
         ensure( r.size() == sliceTfm.sizey(), "size must match!" );
         ui8 colors[ 5 ][ 3 ] = 
         {
            { 0, 0, 0},
            { 255, 255, 255 },
            { 255, 0, 0 },
            { 0, 255, 0 },
            { 0, 255, 255 }
         };
         for ( ui32 n = 0; n < r.size(); ++n )
         {
            for ( ui32 nn = 0; nn < std::min<ui32>( sliceTfm.sizex(), 10 ); ++nn )
            {
               ui8* p = sliceTfm.point( nn, n );
               p[ 0 ] = colors[ r[ n ] ][ 0 ];
               p[ 1 ] = colors[ r[ n ] ][ 1 ];
               p[ 2 ] = colors[ r[ n ] ][ 2 ];
            }
         }
         return sliceTfm;
      }
      */

      /**
       @brief Create normalized haar features from raw mpr
       */
      Point getFeatures( core::Image<ui8>& mpr_xy )
      {
         // convert to a f32 image
         Point sliceFeature( mpr_xy.size(), false );
         for ( ui32 n = 0; n < sliceFeature.size(); ++n )
         {
            sliceFeature[ n ] = static_cast<Point::value_type>( mpr_xy[ n ] ) / 256.0;
         }

         // extract 2D Haar features
         core::Image<Point::value_type> mprf( sliceFeature, mpr_xy.sizex(), mpr_xy.sizey(), 1 );
         Point haarFeature = algorithm::Haar2dFeatures::process( _haar, mprf );

         // normalize
         return _selection.process( _normalization.process( haarFeature ) );
      }

      /**
       @brief Create normalized Haar features from a volume & slice index
       */
      Point getFeatures( const Volume& volume, ui32 sliceIndex )
      {
         // extract MPR
         core::vector3f center = volume.indexToPosition( core::vector3f( volume.size()[ 0 ] / 2.0f,
                                                                         volume.size()[ 1 ] / 2.0f,
                                                                         static_cast<f32>( sliceIndex ) ) );
         core::Image<ui8> mpr_xy = extractSlice( volume, center[ 2 ] );
         //std::cout << "mpr_xy size=" << mpr_xy.sizex() << " " << mpr_xy.sizey() << std::endl;
         return getFeatures( mpr_xy );
      }

   private:
      Normalization                          _normalization;
      algorithm::Haar2dFeatures::Features    _haar;
      Classifier*                            _classifier;
      FeatureSelection                       _selection;
   };
}
}

#endif