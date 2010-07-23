#ifndef CORRECTION4_H_
# define CORRECTION4_H_

# include "globals.h"
# include "read-result.h"
# include "regionDetection.h"
# include <nll/nll.h>
# include <map>

# ifndef UNDEFINED_NB
#  define UNDEFINED_NB    99999.0f
# endif

namespace nll
{
namespace detect
{
   class CorrectPosition4
   {
   public:
      typedef RegionResult::Measure                Measure;
      typedef std::vector<RegionResult::Measure>   Measures;
      typedef core::Buffer1D<float>                Vector;
      typedef core::Matrix<float>                  Matrix;

      typedef algorithm::ClassifierSvm<Vector>                       Classifier;
      typedef Classifier::Database                                   Database;
      typedef algorithm::FeatureTransformationNormalization<Vector>  Normalization;

   public:
      CorrectPosition4()
      {
         _classifiers = 0;
      }

      void learn( const Measures& test )
      {
         // settings for the sample generation
         const double meanBigDeviation     = 0;       // mean of the generated error in mm
         const double varBigDeviation      = 200;     // variance of the generated error in mm
         const double outlierThreshold     = 30;      // threshold indicating it should be detected as outlier, between smallThreshold-outlierThreshold is considered ok to detect it as outlier or miss it...
         const ui32 nbSamplePerTest        = 100;
         const double pb1RoiMissing        = 0.1;     // probability that one of the ROI is missing
         const double noBigError           = 0.1;     // probability that one of the ROI is missing


         const double means[ NB_CLASS ] =
         {
            0, 6.89f, 7.29f, 2.65f, 8.95f, 4.74f
         };

         const double vars[ NB_CLASS ] =
         {
            0, 9.56f, 6.00f, 4.22f, 9.89f, 5.68f
         };

         CorrectPosition3::Database dat;
         for ( ui32 testid = 0; testid < test.size(); ++testid )
         {
            for ( ui32 sample = 0; sample < nbSamplePerTest; ++sample )
            {
               Vector distances = test[ testid ].toArray();
               Vector distancesOrig = test[ testid ].toArray();

               int errorRoi;
               float maxError = 0;
               while ( 1 )
               {
                  // select randomly a ROI
                  errorRoi = ( rand() % (NB_CLASS - 1) ) + 1;
                  if ( distances[ errorRoi ] > 0 )
                     break;
               }

               // generate a default error to test robustness
               for ( ui32 n = 1; n < NB_CLASS; ++n )
                  if ( distances[ n ] > 0 )
                  {
                     const float err = (float)core::generateGaussianDistribution( means[ n ] * core::generateSign(), vars[ n ] );
                     //std::cout << " add err=" << err << std::endl;
                     distances[ n ] += err;
                     maxError = std::max<float>( maxError, err );
                  }


               // generate a missing ROI
               if ( core::generateUniformDistribution( 0, 1 ) < pb1RoiMissing )
               {
                  std::map<float, ui32> missing;
                  for ( ui32 n = 1; n < NB_CLASS; ++n )
                     if ( distances[ n ] > 0 )
                        missing[ core::generateUniformDistribution( 0, 1 ) ] = n;

                  if ( missing.size() )
                  {
                     distances[ missing.rbegin()->second ] = -1;
                  }
               }

               // generate an error, and detect the outlier
               if ( noBigError < core::generateUniformDistribution( 0, 1 ) )
               {
                  float error = (float)core::generateGaussianDistribution( meanBigDeviation * core::generateSign(), varBigDeviation );
                  distances[ errorRoi ] += error;
                  maxError = std::max<float>( maxError, error );
               }

               CorrectPosition3::Database::Sample::Type type = (CorrectPosition3::Database::Sample::Type)( sample % 2 );
               ui32 classid = fabs( maxError ) > outlierThreshold;
               dat.add( CorrectPosition3::Database::Sample( distances, classid, type ) );
            }
         }

         // train the classifiers
         Database datTraining;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            datTraining.add( Database::Sample( transformToRatios( dat[ n ].input ), dat[ n ].output, dat[ n ].type ) );
         }

         _normalizations.compute( datTraining );
         Database normalized = _normalizations.transform( datTraining );

         core::Buffer1D<double> params = core::make_buffer1D<double>( 0.1, 100 );

         if ( _classifiers )
            delete _classifiers;
         _classifiers = new Classifier( false, true );
         _classifiers->learn( normalized, params );
         _classifiers->test( normalized );
      }

      Vector transformToRatios( const Vector& distances )
      {
         Vector testRatios = _getFullRatios( _getFullDistances( distances ) );
         for ( ui32 n = 0; n < testRatios.size(); ++n )
            if ( testRatios[ n ] >= UNDEFINED_NB )
               testRatios[ n ] = 0;
         return testRatios;
      }

      bool detectOutliers( const Vector& testing )
      {
         ui32 nbRoi = 0;

         // get the number of ROI
         for ( ui32 n = 1; n < NB_CLASS; ++n )
         {
            if ( testing[ n ] > 0 )
               ++nbRoi;
         }

         // we can't do anything without at least 3 ROI
         if ( nbRoi < 3 )
            return false;
         Vector ratios = transformToRatios( testing );
         Vector point = _normalizations.process( ratios );
         return _classifiers->test( point ) == 1;
      }


   private:
      static Matrix _getFullDistances( const core::Buffer1D<float>& labels )
      {
         Matrix ref( NB_CLASS, NB_CLASS );
         for ( ui32 n = 0; n < ref.size(); ++n )
            ref[ n ] = UNDEFINED_NB;

         for ( ui32 n = 2; n < NB_CLASS; ++n )  // start a 1, we don't care about the 0 label
         {
            for ( ui32 m = 1; m < n; ++m )
            {
               if ( labels[ n ] > 0 && labels[ m ] > 0 )
               {
                  ref( n, m ) = labels[ m ] - labels[ n ];
               }
            }
         }
         return ref;
      }

      static Vector _getFullRatios( const Matrix& interdistances )
      {
         const ui32 actualNbClass = NB_CLASS - 1;
         const ui32 nbPossibleDist = ( actualNbClass * ( actualNbClass - 1 ) ) / 2;
         Vector ref( nbPossibleDist * ( nbPossibleDist ) );
         for ( ui32 n = 0; n < ref.size(); ++n )
            ref[ n ] = UNDEFINED_NB;

         ui32 nn = 0;
         for ( ui32 n = 2; n < NB_CLASS; ++n )  // start a 1, we don't care about the 0 label
         {
            for ( ui32 m = 1; m < n; ++m )
            {
               for ( ui32 o = 2; o < NB_CLASS; ++o )  // start a 1, we don't care about the 0 label
               {
                  for ( ui32 p = 1; p < o; ++p )
                  {
                     if ( o != n || p != m )
                     {
                        if ( interdistances( n, m ) < UNDEFINED_NB &&
                             interdistances( o, p ) < UNDEFINED_NB )
                        {
                           const float v1 = interdistances( n, m );
                           const float v2 = interdistances( o, p );
                           if ( fabs( v2 ) < 1e-3 )
                              ref[ nn ] = UNDEFINED_NB;
                           else
                              ref[ nn ] = v1 / v2;
                        } else {
                           ref[ nn ] = UNDEFINED_NB;
                        }
                     }
                     ++nn;
                  }
               }
            }
         }
         return ref;
      }
   private:
      Classifier*                   _classifiers;
      Normalization                 _normalizations;
   };
}
}
#endif