#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"
#include "utils.h"

using namespace nll;

/**
 @brief This class is testing all intensity registration algorithms using synthetic cases where optimal registration and tolerance error are perfectly
        known
 */
class TestIntensityBasedRegistrationSynthetic
{
public:
   typedef imaging::VolumeSpatial<ui8>                                         Volume;
   typedef imaging::VolumeSpatial<f32>                                         Volumef;
   typedef Volume::Matrix                                                      Matrix;
   typedef algorithm::RegistrationEvaluatorHelper<Volume>::EvaluatorSimilarity RegistrationEvaluator;
   typedef algorithm::RegistrationAlgorithmIntensity<Volume::value_type, Volume::VoxelBuffer> RegistrationAlgorithmIntensity;
   typedef algorithm::RegistrationGradientEvaluatorFiniteDifference<Volume::value_type, Volume::VoxelBuffer> GradientEvaluator;

   template <class Volume>
   void createSphere( Volume& v, const core::vector3ui& position, double radius, typename Volume::value_type valueForeground )
   {
      const double radius2 = core::sqr( radius );
      for ( size_t z = 0; z < v.sizez(); ++z )
      {
         for ( size_t y = 0; y < v.sizey(); ++y )
         {
            for ( size_t x = 0; x < v.sizex(); ++x )
            {
               const double d = core::sqr( (double)position[ 0 ] - (double)x ) +
                                core::sqr( (double)position[ 1 ] - (double)y ) +
                                core::sqr( (double)position[ 2 ] - (double)z );
               if ( d <= radius2 )
               {
                  v( x, y, z ) = valueForeground;
               }
            }
         }
      }
   }

   template <class Volume>
   void createBox( Volume& v, const core::vector3ui& position, const core::vector3f& radius, typename Volume::value_type valueForeground )
   {
      for ( size_t z = 0; z < v.sizez(); ++z )
      {
         for ( size_t y = 0; y < v.sizey(); ++y )
         {
            for ( size_t x = 0; x < v.sizex(); ++x )
            {
               const bool inside = x >= position[ 0 ] && x <= position[ 0 ] + radius[ 0 ] &&
                                   y >= position[ 1 ] && y <= position[ 1 ] + radius[ 1 ] &&
                                   z >= position[ 2 ] && z <= position[ 2 ] + radius[ 2 ];
               if ( inside )
               {
                  v( x, y, z ) = valueForeground;
               }
            }
         }
      }
   }

   void utilTestSphereSameRadius( const RegistrationAlgorithmIntensity::Base& registrationalgorithm, const algorithm::TransformationParametrized& tfmInit )
   {
      const size_t joinHistogramNbBins = 2;

      const size_t size = 60;
      const core::vector3ui sizeVolume( size, size, size );
      const double radius = core::generateUniformDistribution( 10, 20 );

      const core::vector3ui positionSource( core::generateUniformDistributioni( radius, size - radius ),
                                            core::generateUniformDistributioni( radius, size - radius ),
                                            core::generateUniformDistributioni( radius, size - radius ) );

      core::vector3ui positionTarget( core::generateUniformDistributioni( radius, size - radius ),
                                      core::generateUniformDistributioni( radius, size - radius ),
                                      core::generateUniformDistributioni( radius, size - radius ) );

      while ( (positionTarget - positionSource).norm2() > radius / 2 )
      {
         positionTarget = core::vector3ui( core::generateUniformDistributioni( radius, size - radius ),
                                           core::generateUniformDistributioni( radius, size - radius ),
                                           core::generateUniformDistributioni( radius, size - radius ) );
      }

      std::cout << "source=" << positionSource << std::endl;
      std::cout << "target=" << positionTarget << std::endl;
      std::cout << "radius=" << radius << std::endl;

      const core::vector3d expectedTfm( - (double)positionSource[ 0 ] + (double)positionTarget[ 0 ],
                                        - (double)positionSource[ 1 ] + (double)positionTarget[ 1 ],
                                        - (double)positionSource[ 2 ] + (double)positionTarget[ 2 ] );

      std::cout << "expected=" << expectedTfm << std::endl;

      // data set up
      Volume source( sizeVolume, core::identityMatrix<Matrix>( 4 ) );
      createSphere( source, positionSource, radius, joinHistogramNbBins - 1 );

      Volume target( sizeVolume, core::identityMatrix<Matrix>( 4 ) );
      createSphere( target, positionTarget, radius, joinHistogramNbBins - 1 );

      imaging::LookUpTransformWindowingRGB lut( 0, joinHistogramNbBins - 1, 256, 3 );
      lut.createGreyscale();

      {
         core::Image<ui8> mpr1 = test::GetMprForDisplay( source,
                                                         core::vector3ui( 60, 60, 1 ), 
                                                         core::vector3f( 1, 0, 0 ),
                                                         core::vector3f( 0, 1, 0 ),
                                                         core::vector3f( 30, 30, positionSource[ 2 ] ),
                                                         lut );
         core::writeBmp( mpr1, "c:/tmp2/sphereSource.bmp" );
      }

      {
         core::Image<ui8> mpr2 = test::GetMprForDisplay( target,
                                                         core::vector3ui( 60, 60, 1 ), 
                                                         core::vector3f( 1, 0, 0 ),
                                                         core::vector3f( 0, 1, 0 ),
                                                         core::vector3f( 30, 30, positionTarget[ 2 ] ),
                                                         lut );
         core::writeBmp( mpr2, "c:/tmp2/sphereTarget.bmp" );
      }

      std::shared_ptr<algorithm::TransformationParametrized> tfm = registrationalgorithm.evaluate( source, target, tfmInit );
      core::Buffer1D<double> resultd = tfm->getParameters();

      std::cout << "result=" << resultd << std::endl;

      TESTER_ASSERT( core::equal<double>( resultd[ 0 ], expectedTfm[ 0 ], 1 ) );
      TESTER_ASSERT( core::equal<double>( resultd[ 1 ], expectedTfm[ 1 ], 1 ) );
      TESTER_ASSERT( core::equal<double>( resultd[ 2 ], expectedTfm[ 2 ], 1 ) );
   }

   // position randomly a sphere in each volume with some overlap. Find the optimal reigstration which is aligning both centers
   // maximum error should be 1 spacing
   void testSphereRigidGradient()
   {
      const size_t joinHistogramNbBins = 2;

      // set up the algorithm
      core::Buffer1D<double> seed = core::make_buffer1D<double>( 0, 0, 0 );
      algorithm::TransformationCreatorTranslation c;
      algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
      algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
      std::shared_ptr<GradientEvaluator> gradientEvaluator( new GradientEvaluator( core::make_buffer1D<double>( 1, 1, 1 ), true ) );
      RegistrationEvaluator evaluator( similarity, histogramMaker, joinHistogramNbBins, gradientEvaluator, false );
      std::shared_ptr<algorithm::TransformationParametrized> initTfm = c.create( seed );

      algorithm::StopConditionStable stopCondition( 10 );
      algorithm::OptimizerGradientDescent optimizer( stopCondition, 0.0, true, 1, core::make_buffer1D<double>( 1, 1, 1 ),
                                                                                  core::make_buffer1D<double>( 0.5, 0.5, 0.5 ) );
      RegistrationAlgorithmIntensity registration( c, evaluator, optimizer );

      // run and check
      srand( 0 );
      for ( int n = 0; n < 50; ++n )
      {
         utilTestSphereSameRadius( registration, *initTfm );
      }
   }

   // position randomly a sphere in each volume with some overlap. Find the optimal reigstration which is aligning both centers
   // maximum error should be 1 spacing
   void testSphereRigidGradientMutualInformation()
   {
      const size_t joinHistogramNbBins = 2;

      // set up the algorithm
      core::Buffer1D<double> seed = core::make_buffer1D<double>( 0, 0, 0 );
      algorithm::TransformationCreatorTranslation c;
      algorithm::SimilarityFunctionMutualInformation similarity;
      algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
      std::shared_ptr<GradientEvaluator> gradientEvaluator( new GradientEvaluator( core::make_buffer1D<double>( 1, 1, 1 ), true ) );
      RegistrationEvaluator evaluator( similarity, histogramMaker, joinHistogramNbBins, gradientEvaluator, false );
      std::shared_ptr<algorithm::TransformationParametrized> initTfm = c.create( seed );

      algorithm::StopConditionStable stopCondition( 10 );
      algorithm::OptimizerGradientDescent optimizer( stopCondition, 0.0, true, 1, core::make_buffer1D<double>( 1, 1, 1 ),
                                                                                  core::make_buffer1D<double>( 0.5, 0.5, 0.5 ) );
      RegistrationAlgorithmIntensity registration( c, evaluator, optimizer );

      // run and check
      srand( 0 );
      for ( int n = 0; n < 50; ++n )
      {
         utilTestSphereSameRadius( registration, *initTfm );
      }
   }

   // position randomly a sphere in each volume with some overlap. Find the optimal reigstration which is aligning both centers
   // maximum error should be 1 spacing
   void testSphereRigidGradientPyramid()
   {
      const size_t joinHistogramNbBins = 2;

      // set up the algorithm
      core::Buffer1D<double> seed = core::make_buffer1D<double>( 0, 0, 0 );
      algorithm::TransformationCreatorTranslation c;
      algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
      algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
      std::shared_ptr<GradientEvaluator> gradientEvaluator( new GradientEvaluator( core::make_buffer1D<double>( 1, 1, 1 ), true ) );
      RegistrationEvaluator evaluator( similarity, histogramMaker, joinHistogramNbBins, gradientEvaluator, false );
      std::shared_ptr<algorithm::TransformationParametrized> initTfm = c.create( seed );

      algorithm::StopConditionStable stopCondition( 10 );
      algorithm::OptimizerGradientDescent optimizer( stopCondition, 0.0, true, 1, core::make_buffer1D<double>( 1, 1, 1 ),
                                                                                  core::make_buffer1D<double>( 0.5, 0.5, 0.5 ) );
      RegistrationAlgorithmIntensity registration( c, evaluator, optimizer );
      algorithm::RegistrationAlgorithmIntensityPyramid<Volume::value_type, Volume::VoxelBuffer> pyramidRegistration( registration );

      // run and check
      srand( 0 );
      for ( int n = 0; n < 50; ++n )
      {
         utilTestSphereSameRadius( pyramidRegistration, *initTfm );
      }
   }

   void testSimilarity()
   {
      Volume v( core::vector3ui( 8, 8, 8 ), core::identityMatrix<Matrix>( 4 ) );
      createBox( v, core::vector3ui( 3, 3, 3 ), core::vector3f( 2, 2, 2 ), 1 );
      const size_t joinHistogramNbBins = 2;

      // construct the evaluator
      algorithm::TransformationCreatorTranslation transformationCreator;
      algorithm::SimilarityFunctionSumOfSquareDifferences similarity;
      algorithm::HistogramMakerTrilinearPartial<Volume::value_type, Volume::VoxelBuffer> histogramMaker;
      RegistrationEvaluator evaluator( v, v, similarity, transformationCreator, histogramMaker, joinHistogramNbBins, std::shared_ptr<GradientEvaluator>(), false );

      // get the similarity. We expect it to be decreasing until we reach 0, then increasing
      RegistrationEvaluator::SimilarityPlot plot = evaluator.returnSimilarityAlongParameter( core::make_buffer1D<double>( 0.1,  0,  0 ), 0, -0.01, 22 );
      for ( size_t n = 0; n < plot.size() - 1; ++n )
      {
         if ( plot[ n ].first > 0 )
         {
            TESTER_ASSERT( plot[ n + 1 ].second < plot[ n ].second );
         } else {
            TESTER_ASSERT( plot[ n + 1 ].second > plot[ n ].second );
         }
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestIntensityBasedRegistrationSynthetic);
 TESTER_TEST(testSphereRigidGradient);
 TESTER_TEST(testSphereRigidGradientMutualInformation);
 TESTER_TEST(testSphereRigidGradientPyramid);
 TESTER_TEST(testSimilarity);
TESTER_TEST_SUITE_END();
#endif