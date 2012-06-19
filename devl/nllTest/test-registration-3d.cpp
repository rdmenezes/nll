#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

namespace nll
{
namespace algorithm
{
   namespace impl
   {
      /**
       @brief 3D Transformation estimator to be used by a Ransac optimizer
    
              It is just estimating a transformation of the form T(x) = s * R * x + t,
              where <s> is a scalar representing the isometric scaling, R a 3x3 rotation
              matrix and t a translation vector
    
       @note Internally the algorithm used is <code>EstimatorTransformSimilarityIsometric</code>
       */
      class SimilarityIsotropicTransformationEstimatorRansac3d
      {
      private:
         typedef SpeededUpRobustFeatures3d::Points PointsData;

         template <class PointsMatch>
         class PointsWrapper1
         {
         public:
            PointsWrapper1( const PointsData& points, const PointsMatch& matches ) : _points( points ), _matches( matches )
            {}

            ui32 size() const
            {
               return static_cast<ui32>( _matches.size() );
            }

            const core::vector3i& operator[]( ui32 index ) const
            {
               return _points[ _matches[ index ].index1 ].position;
            }

         private:
            // copy disabled
            PointsWrapper1& operator=( const PointsWrapper1& );

         private:
            const PointsData&     _points;
            const PointsMatch&    _matches;
         };

         template <class PointsMatch>
         class PointsWrapper2
         {
         public:
            PointsWrapper2( const PointsData& points, const PointsMatch& matches ) : _points( points ), _matches( matches )
            {}

            ui32 size() const
            {
               return static_cast<ui32>( _matches.size() );
            }

            const core::vector3i& operator[]( ui32 index ) const
            {
               return _points[ _matches[ index ].index2 ].position;
            }

         private:
            // copy disabled
            PointsWrapper2& operator=( const PointsWrapper2& );

         private:
            const PointsData&     _points;
            const PointsMatch&    _matches;
         };

      public:
         /**
          @brief 
          */
         SimilarityIsotropicTransformationEstimatorRansac3d( const SpeededUpRobustFeatures3d::Points& p1, const SpeededUpRobustFeatures3d::Points& p2,
                                                             double scale, double minimumScale, double maximumScale ) : _p1( p1 ), _p2( p2 ), _scale( scale ), _minimumScale( minimumScale ), _maximumScale( maximumScale )
         {
         }

         static ui32 minimumNumberOfPointsForEstimation()
         {
            return 2;
         }

         struct Model
         {
            Model() : tfm( core::identityMatrix< core::Matrix<double> >( 3 ) )
            {}

            void print( std::ostream& o ) const
            {
               o << " Transformation Translation only=";
               tfm.print( o );
            }

            core::Matrix<double> tfm;
         };

         typedef impl::FeatureMatcher::Match    Point;

         template <class Points>
         void estimate( const Points& points )
         {
            if ( points.size() < 2 )
               return;
           
            PointsWrapper1<Points> wrapperP1( _p1, points );
            PointsWrapper2<Points> wrapperP2( _p2, points );
            
            EstimatorTransformSimilarityIsometric affineEstimator( _scale, _minimumScale, _maximumScale );
            _model.tfm = affineEstimator.compute( wrapperP1, wrapperP2 );
         }

         /**
            @brief Returns squared error. Transforms the source point to target point given a transformation,
                  and returns the squared sum of the differences
            */
         double error( const Point& point ) const
         {
            const SpeededUpRobustFeatures3d::Point& p1 = _p1[ point.index1 ];
            const SpeededUpRobustFeatures3d::Point& p2 = _p2[ point.index2 ];

            // transform the point
            const core::Matrix<double>& tfm = _model.tfm;


            const double px = tfm( 0, 2 ) + p1.position[ 0 ] * tfm( 0, 0 ) + p1.position[ 1 ] * tfm( 0, 1 ) + p1.position[ 2 ] * tfm( 0, 2 );
            const double py = tfm( 1, 2 ) + p1.position[ 0 ] * tfm( 1, 0 ) + p1.position[ 1 ] * tfm( 1, 1 ) + p1.position[ 2 ] * tfm( 1, 2 );
            const double pz = tfm( 2, 2 ) + p1.position[ 0 ] * tfm( 2, 0 ) + p1.position[ 1 ] * tfm( 2, 1 ) + p1.position[ 2 ] * tfm( 2, 2 );

            // we want a ratio of the error...
            const double lengthTfm = core::sqr( px - p2.position[ 0 ] ) +
                                     core::sqr( py - p2.position[ 1 ] ) +
                                     core::sqr( pz - p2.position[ 2 ] );
            const double errorVal = fabs( lengthTfm );
            return errorVal;
         }

         /**
            @brief Returns the current model
            */
         const Model& getModel() const
         {
            return _model;
         }

         const SpeededUpRobustFeatures3d::Points& getP1() const
         {
            return _p1;
         }

         const SpeededUpRobustFeatures3d::Points& getP2() const
         {
            return _p2;
         }

      private:
         // copy disabled
         SimilarityIsotropicTransformationEstimatorRansac3d& operator=( const SimilarityIsotropicTransformationEstimatorRansac3d& );

      private:
         Model    _model;
         const SpeededUpRobustFeatures3d::Points& _p1;
         const SpeededUpRobustFeatures3d::Points& _p2;
         const double   _scale;
         const double   _minimumScale;
         const double   _maximumScale;
      };

      /**
       @brief Factory for the <code>SimilarityIsotropicTransformationEstimatorRansac3d</code> estimator
       */
      class SurfEstimatorSimilarityIsotropicFactory3d
      {
      public:
         typedef SimilarityIsotropicTransformationEstimatorRansac3d Estimator;

         SurfEstimatorSimilarityIsotropicFactory3d( const SpeededUpRobustFeatures3d::Points& p1, const SpeededUpRobustFeatures3d::Points& p2,
                                                    double scale = 0, double minimumScale = 0.7, double maximumScale = 1.4 ) : _p1( &p1 ), _p2( &p2 ), _scale( scale ), _minimumScale( minimumScale ), _maximumScale( maximumScale )
         {}

         std::auto_ptr<Estimator> create() const
         {
            return std::auto_ptr<Estimator>( new Estimator( *_p1, *_p2, _scale, _minimumScale, _maximumScale ) );
         }

      private:
         // copy disabled
         const SurfEstimatorSimilarityIsotropicFactory3d& operator=( const SurfEstimatorSimilarityIsotropicFactory3d& );

      private:
         const SpeededUpRobustFeatures3d::Points* _p1;
         const SpeededUpRobustFeatures3d::Points* _p2;
         const double   _scale;
         const double   _minimumScale;
         const double   _maximumScale;
      };
   }

   /**
    @ingroup algorithm
    @brief Algorithm registering two clouds of 3D points
    */
   template <class TransformationEstimatorFactoryT = impl::SurfEstimatorSimilarityIsotropicFactory3d, class FeatureMatcherT = impl::FeatureMatcher>
   class RegistrationCloudOfPoints3d
   {
      typedef core::Matrix<double>                                Matrix;
      typedef algorithm::SpeededUpRobustFeatures3d::Point         Point;
      typedef algorithm::SpeededUpRobustFeatures3d::Points        Points;
      typedef TransformationEstimatorFactoryT                     TransformationEstimatorFactory;
      typedef typename TransformationEstimatorFactoryT::Estimator RansacTransformationEstimator;
      typedef FeatureMatcherT                                     FeatureMatcher;
      typedef std::vector< std::pair<Point, Point> >              PointPairs;

      struct Debug
      {
         typename FeatureMatcher::Matches matches;
         PointPairs inliers;
         ui32       nbSourcePoints;
         ui32       nbTargetPoints;
      };

   public:
      RegistrationCloudOfPoints3d( FeatureMatcher matcher = FeatureMatcher() ) : _matcher( matcher )
      {}

      /**
       @brief Compute a registration between two clouds of points

       For generality's sake, we take only clouds of points as argument, but not how these were generated.
       */
      Matrix process( const Points& sourcePoints,
                      const Points& targetPoints,
                      ui32 maxErrorInPixel = 20,
                      ui32 minimumInliers = 20,
                      ui32 nbRansacIterations = 35000,
                      ui32 nbMaxRansacPairs = 1000 ) const
      {
         nll::core::Timer timerFull;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "starting RegistrationCloudOfPoints3d..." );

         ensure( sourcePoints.size() > 0 && targetPoints.size() > 0, "register an empty set!" );
         ensure( sourcePoints[ 0 ].position.size() == 3, "we are expecting 3D points!" );
         
         // match points
         algorithm::SpeededUpRobustFeatures3d::PointsFeatureWrapper p1Wrapper( sourcePoints );
         algorithm::SpeededUpRobustFeatures3d::PointsFeatureWrapper p2Wrapper( targetPoints );

         nll::core::Timer timerMatcher;
         typename FeatureMatcher::Matches matches;
         _matcher.findMatch( p1Wrapper, p2Wrapper, matches );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "point matching time=" + core::val2str( timerMatcher.getCurrentTime() ) );

         // take only the best subset...
         ensure( matches.size(), "No match found!" );
         typename FeatureMatcher::Matches matchesTrimmed( matches.begin(), matches.begin() + std::min<ui32>( nbMaxRansacPairs, (ui32)matches.size() - 1 ) );

         // estimate the transformation
         typedef algorithm::Ransac<RansacTransformationEstimator, TransformationEstimatorFactory> Ransac;

         TransformationEstimatorFactory estimatorFactory( sourcePoints, targetPoints );
         Ransac ransac( estimatorFactory );

         core::Buffer1D<float> weights( matchesTrimmed.size() );
         for ( ui32 n = 0; n < static_cast<ui32>( weights.size() ); ++n )
         {
            const ui32 indexTarget = matchesTrimmed[ n ].index2;
            weights[ n ] = targetPoints[ indexTarget ].weight;
         }

         core::Timer ransacOptimTimer;
         typename RansacTransformationEstimator::Model model = ransac.estimate( matchesTrimmed,
                                                                                RansacTransformationEstimator::minimumNumberOfPointsForEstimation(),
                                                                                nbRansacIterations,
                                                                                maxErrorInPixel * maxErrorInPixel,
                                                                                weights );

         std::cout << "inliers=" << ransac.getInliers().size() << std::endl;

         ensure( ransac.getNbInliers() > minimumInliers, "Error: inliers are too small" );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "RANSAC optimization time=" + nll::core::val2str( ransacOptimTimer.getCurrentTime() ) );

         _debug.inliers.clear();
         for ( size_t n = 0; n < ransac.getInliers().size(); ++n )
         {
            typedef typename Points::value_type PointM;
            const typename FeatureMatcher::Matches::value_type& match = matchesTrimmed[ ransac.getInliers()[ n ] ];
            _debug.inliers.push_back( std::pair<PointM, PointM>( sourcePoints[ match.index1 ], targetPoints[ match.index2 ] ) );
         }
         _debug.matches = matchesTrimmed;
         _debug.nbSourcePoints = sourcePoints.size();
         _debug.nbTargetPoints = targetPoints.size();

         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "3D RegistrationCloudOfPoints3d successful..." );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "3D RegistrationCloudOfPoints3d total time=" + core::val2str( timerFull.getCurrentTime() ) );

         return model.tfm;
      }

      /**
       @brief Returns the debug info associated to the last call of <process>
       */
      const Debug& getDebugInfo() const
      {
         return _debug;
      }

   private:
      FeatureMatcher _matcher;
      mutable Debug  _debug;
   };
}
}

class TestRegistration3d
{
   typedef core::Matrix<double>           Matrix;

   typedef core::Matrix<float>                     Matrixf;
   typedef imaging::VolumeSpatial<float>           Volume;
   typedef imaging::InterpolatorTriLinear<Volume>  Interpolator;
   typedef imaging::Mpr<Volume, Interpolator>      Mpr;
   typedef imaging::Slice<float>                   Slicef;
   typedef imaging::Slice<ui8>                     Slicec;

public:
   static Slicec getRgbSlice( const imaging::LookUpTransformWindowingRGB& lut, const Slicef& slice )
   {
      core::vector3ui sizeRgb( slice.size()[ 0 ], slice.size()[ 1 ], 3 );
      Slicec sliceRgb( sizeRgb, slice.getAxisX(), slice.getAxisY(), slice.getOrigin(), slice.getSpacing() );
      std::vector< imaging::BlendSliceInfof<imaging::LookUpTransformWindowingRGB> > sliceInfos;
      sliceInfos.push_back( imaging::BlendSliceInfof<imaging::LookUpTransformWindowingRGB>( slice, 0.9, lut ) );
      imaging::blendDummy( sliceInfos, sliceRgb );
      return sliceRgb;
   }

   // get a RGB slice of a preprocessed volume
   static Slicec getRgbSlice( const Volume& v, const core::vector3f& position )
   {
      imaging::LookUpTransformWindowingRGB lutId( 0, 255, 256 );
      lutId.createGreyscale();

      Mpr mpr( v );
      Slicef slice( core::vector3ui( 512, 512, 1 ), core::vector3f( 1, 0, 0 ), core::vector3f( 0, 1, 0 ), position, core::vector2f( 1, 1 ) );
      mpr.getSlice( slice );

      core::vector3ui sizeRgb;
      Slicec sliceRgb = getRgbSlice( lutId, slice );
      return sliceRgb;
   }

   // compute registration between two normalized volumes
   template <class Registrator>
   static Matrix computeRegistration( const Registrator& registrator, const Volume& source, const Volume& target, const imaging::LookUpTransformWindowingRGB& lut )
   {
      // extract 3D points
      algorithm::SpeededUpRobustFeatures3d surf( 5, 4, 2, 0.00005 );
      algorithm::SpeededUpRobustFeatures3d::Points sourcePoints = surf.computesFeatures( source );
      algorithm::SpeededUpRobustFeatures3d::Points targetPoints = surf.computesFeatures( target );

      std::cout << "nbPointsSource=" << sourcePoints.size() << std::endl;
      std::cout << "nbPointsTarget=" << targetPoints.size() << std::endl;

      Matrix tfm = registrator.process( targetPoints, sourcePoints );
      return tfm;
   }

   // normalize a volume
   Volume prepareVolume( const Volume& input, const imaging::LookUpTransformWindowingRGB& lut )
   {
      // we ABSOLUTELY need to use the same spacing [1 1 1] for the volume to get the normal transformation
      // also we need to be in the same geometric space!!
      const core::Matrix<float> targetOriginalPst = core::createTranslation4x4( input.getOrigin() ) *
                                                    core::createScaling4x4( core::vector3f( 1, 1, 1 ) );
      Volume target( core::vector3ui( 350, 350, 256 ), targetOriginalPst );
      imaging::resampleVolumeTrilinear( input, target );
      target.setOrigin( core::vector3f( 0, 0, 0 ) );

      for ( ui32 n = 0; n < target.getStorage().size(); ++n )
      {
         float& value = target.getStorage()[ n ];
         value = lut.transform( value )[ 0 ];
      }

      return target;
   }

   void testIdentity()
   {
      imaging::LookUpTransformWindowingRGB lut( 00, 5000, 255 );
      lut.createGreyscale();

      // load a volume
      Volume target;
      const bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/pet-NAC.mf2", target );
      ensure( loaded, "Error!" );

      typedef algorithm::RegistrationCloudOfPoints3d<> Registrator3d;
      Registrator3d registrator;
      const Matrix tfm = computeRegistration( registrator, target, target, lut );

      TESTER_ASSERT( tfm.equal( core::identityMatrix<Matrix>( 4 ), 1e-4 ) );
      TESTER_ASSERT( registrator.getDebugInfo().inliers.size() >= 0.99 * registrator.getDebugInfo().nbSourcePoints );
   }

   void testSimpleTfm()
   {
      imaging::LookUpTransformWindowingRGB lut( 0, 1000, 255 );
      lut.createGreyscale();

      srand( 1 );
      for ( ui32 n = 0; n < 10; ++n )
      {
         std::cout << "case="<< n << std::endl;
         /*
         core::vector3f translation( core::generateUniformDistribution( -30, 30 ),
                                     core::generateUniformDistribution( -30, 30 ),
                                     core::generateUniformDistribution( -30, 30 ) );
                                     */

         /*
         core::vector3f translation( core::generateUniformDistribution( -30, 30 ),
                                     0,
                                     0 );
           */                          
         core::vector3f translation( 29, 0, 0 );
         core::vector3f scaling( 1, 1, 1 );
         core::vector3f rotation( 0, 0, 0 );

         const core::Matrix<float> tfmMatrixExpected = core::createTranslation4x4( translation ) *
                                                       core::getRotation4Zf( rotation[ 2 ] ) *
                                                       core::getRotation4Zf( rotation[ 1 ] ) *
                                                       core::getRotation4Zf( rotation[ 0 ] ) *
                                                       core::createScaling4x4( scaling );

         // load a volume
         Volume targetOriginal;
         const bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/MR-1.mf2", targetOriginal );
         ensure( loaded, "Error!" );

         const Volume target = prepareVolume( targetOriginal, lut );

         Volume targetResampled( target.size(), target.getPst(), target.getBackgroundValue() );
         imaging::TransformationAffine tfmExpected( tfmMatrixExpected );
         imaging::resampleVolumeTrilinear( target, tfmExpected, targetResampled );

         {
            const core::vector3f targetCenter = target.indexToPosition( core::vector3f( target.size()[ 0 ] / 2, target.size()[ 1 ] / 2, target.size()[ 2 ] / 2 ) );
            Slicec slice = getRgbSlice( target, targetCenter );
            core::writeBmp( slice.getStorage(), "c:/tmp2/target-"+ core::val2str( n ) + ".bmp" );
            slice = getRgbSlice( targetResampled, targetCenter );
            core::writeBmp( slice.getStorage(), "c:/tmp2/targetResampled-"+ core::val2str( n ) + ".bmp" );
         }

         std::cout << "TfmExpected="<< std::endl;
         tfmExpected.getAffineMatrix().print( std::cout );

         typedef algorithm::RegistrationCloudOfPoints3d<> Registrator3d;
         Registrator3d registrator;
         const Matrix tfm = computeRegistration( registrator, target, targetResampled, lut );

         std::cout << "TfmFound="<< std::endl;
         tfm.print( std::cout );

         std::cout << "VolumeSize=" << target.size();

         Matrix tfmExpectedd;
         tfmExpectedd.import( tfmExpected.getAffineMatrix() );

         const Matrix tfmRot         = core::getRotationAndSpacing( tfm );
         const Matrix tfmRotExpected = core::getRotationAndSpacing( tfmExpectedd );

         const double tolTranslation = targetOriginal.getSpacing()[ 0 ];
         TESTER_ASSERT( tfmRot.equal( tfmRotExpected, 0.05 ) );
         TESTER_ASSERT( core::equal<double>( tfmExpectedd( 0, 3 ), tfm( 0, 3 ), tolTranslation ) );
         TESTER_ASSERT( core::equal<double>( tfmExpectedd( 1, 3 ), tfm( 1, 3 ), tolTranslation ) );
         TESTER_ASSERT( core::equal<double>( tfmExpectedd( 2, 3 ), tfm( 2, 3 ), tolTranslation ) );
      }
   }

   // here we give exact points. We want to test that the point matching mecanism works well
   void testSimpleTfmGivenPoints()
   {
      imaging::LookUpTransformWindowingRGB lut( 0, 1000, 255 );
      lut.createGreyscale();

      srand( 1 );
      for ( ui32 n = 0; n < 10; ++n )
      {
         std::cout << "case="<< n << std::endl;

         // create a random transformation
         /*
         core::vector3f translation( core::generateUniformDistribution( -30, 30 ),
                                     core::generateUniformDistribution( -30, 30 ),
                                     core::generateUniformDistribution( -30, 30 ) );
                                     */
         core::vector3f translation( 29.1, 0, 0 );
         core::vector3f scaling( 1, 1, 1 );
         core::vector3f rotation( 0, 0, 0 );
         const core::Matrix<float> tfmMatrixExpected = core::createTranslation4x4( translation ) *
                                                       core::getRotation4Zf( rotation[ 2 ] ) *
                                                       core::getRotation4Zf( rotation[ 1 ] ) *
                                                       core::getRotation4Zf( rotation[ 0 ] ) *
                                                       core::createScaling4x4( scaling );

         core::Matrix<float> tfmMatrixExpectedInv;
         tfmMatrixExpectedInv.clone( tfmMatrixExpected );
         core::inverse( tfmMatrixExpectedInv );

         // load a volume
         Volume targetOriginal;
         const bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/MR-1.mf2", targetOriginal );
         ensure( loaded, "Error!" );

         // create the source and target volumes
         const Volume target = prepareVolume( targetOriginal, lut );

         Volume targetResampled( target.size(), target.getPst(), target.getBackgroundValue() );
         imaging::TransformationAffine tfmExpected( tfmMatrixExpected );
         imaging::resampleVolumeTrilinear( target, tfmExpected, targetResampled );

         {
            const core::vector3f targetCenter = target.indexToPosition( core::vector3f( target.size()[ 0 ] / 2, target.size()[ 1 ] / 2, target.size()[ 2 ] / 2 ) );
            Slicec slice = getRgbSlice( target, targetCenter );
            core::writeBmp( slice.getStorage(), "c:/tmp2/simple-target-"+ core::val2str( n ) + ".bmp" );
            slice = getRgbSlice( targetResampled, targetCenter );
            core::writeBmp( slice.getStorage(), "c:/tmp2/simple-targetResampled-"+ core::val2str( n ) + ".bmp" );
         }

         {
            const core::vector3f sourceCenter = target.indexToPosition( core::vector3f( 168, 192, 153 ) );
            const core::vector3f targetCenter = targetResampled.indexToPosition( core::vector3f( 138, 192, 153 ) );
            Slicec slice = getRgbSlice( target, sourceCenter );
            core::writeBmp( slice.getStorage(), "c:/tmp2/point-target-"+ core::val2str( n ) + ".bmp" );
            slice = getRgbSlice( targetResampled, targetCenter );
            core::writeBmp( slice.getStorage(), "c:/tmp2/point-targetResampled-"+ core::val2str( n ) + ".bmp" );
         }

         // get the points in the target and create the corresponding points in the targetResampled
         algorithm::SpeededUpRobustFeatures3d surf( 5, 4, 2, 0.00001 );
         algorithm::SpeededUpRobustFeatures3d::Points sourcePoints = surf.computesFeatures( target );

         std::cout << "nbPoints=" << sourcePoints.size() << std::endl;

         algorithm::SpeededUpRobustFeatures3d::Points targetPoints( sourcePoints.size() );
         for ( ui32 n = 0; n < sourcePoints.size(); ++n )
         {
            algorithm::SpeededUpRobustFeatures3d::Point point;
            const core::vector3f input( sourcePoints[ n ].position[ 0 ],
                                        sourcePoints[ n ].position[ 1 ],
                                        sourcePoints[ n ].position[ 2 ] );
            const core::vector3f output = core::transf4( tfmMatrixExpectedInv, input );

            point.position = core::vector3i( output[ 0 ], output[ 1 ], output[ 2 ] );
            point.scale = sourcePoints[ n ].scale;
            point.weight = sourcePoints[ n ].weight;
            targetPoints[ n ] = point;
         }

         surf.computeFeatures( targetResampled, targetPoints );

         std::cout << "v=" << target( 152, 258, 14 ) << std::endl;
         //sourcePoints[ 0 ].print( std::cout );
         //sourcePoints[ 466 ].print( std::cout );
         //sourcePoints[ 468 ].print( std::cout );
         //sourcePoints[ 459 ].print( std::cout );
         //sourcePoints[ 444 ].print( std::cout );
         
         //targetPoints[ 0 ].print( std::cout );
         //targetPoints[ 466 ].print( std::cout );
         //targetPoints[ 468 ].print( std::cout );
         //targetPoints[ 459 ].print( std::cout );
         //targetPoints[ 444 ].print( std::cout );

         // now we have the full points, so perform the registration. Ideally the matching will be perfect!
         typedef algorithm::RegistrationCloudOfPoints3d<> Registrator3d;
         Registrator3d registrator;
         const Matrix tfm = registrator.process( targetPoints, sourcePoints, 64 /* i.e. 4*4*4 voxels */, 20, 10000, 5000 );

         // check the matches
         ui32 nbGoodMatches = 0;
         ui32 nbZeroLength = 0;
         for ( size_t n = 0; n < registrator.getDebugInfo().matches.size(); ++n )
         {
            if ( registrator.getDebugInfo().matches[ n ].index1 == registrator.getDebugInfo().matches[ n ].index2 )
            {
               ++nbGoodMatches;
            }
         }

         for ( size_t n = 0; n < sourcePoints.size(); ++n )
         {
            if ( core::norm2( sourcePoints[ n ].features ) < 1e-8 )
            {
               ++nbZeroLength;
            }
         }

         std::cout << "zeroLength=" << nbZeroLength << std::endl;

         const double goodMatchRatio = nbGoodMatches / (double)  registrator.getDebugInfo().matches.size();
         std::cout << "goodMatchRatio=" << goodMatchRatio << std::endl;

         const double inlierRatio =  registrator.getDebugInfo().inliers.size() / (double)targetPoints.size();
         std::cout << "inlierRatio=" << inlierRatio << std::endl;

         tfmExpected.getAffineMatrix().print( std::cout );
         tfm.print( std::cout );

         Matrix tfmExpectedd;
         tfmExpectedd.import( tfmExpected.getAffineMatrix() );

         const Matrix tfmRot         = core::getRotationAndSpacing( tfm );
         const Matrix tfmRotExpected = core::getRotationAndSpacing( tfmExpectedd );

         const double tolTranslation = targetOriginal.getSpacing()[ 0 ];
         TESTER_ASSERT( tfmRot.equal( tfmRotExpected, 0.05 ) );
         TESTER_ASSERT( core::equal<double>( tfmExpectedd( 0, 3 ), tfm( 0, 3 ), tolTranslation ) );
         TESTER_ASSERT( core::equal<double>( tfmExpectedd( 1, 3 ), tfm( 1, 3 ), tolTranslation ) );
         TESTER_ASSERT( core::equal<double>( tfmExpectedd( 2, 3 ), tfm( 2, 3 ), tolTranslation ) );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestRegistration3d);
//TESTER_TEST(testIdentity);
//TESTER_TEST(testSimpleTfm);
TESTER_TEST(testSimpleTfmGivenPoints);
TESTER_TEST_SUITE_END();
#endif