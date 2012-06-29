/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_ALGORITHM_RANSAC_H_
# define NLL_ALGORITHM_RANSAC_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Factory for creating the estimator with no parameter
    */
   template <class Estimator>
   class GenericEstimatorFactory
   {
   public:
      std::auto_ptr<Estimator> create() const
      {
         return std::auto_ptr<Estimator>( new Estimator() );
      }
   };

   /**
    @ingroup algorithm
    @brief RANSAC is an iterative method to estimate parameters of a mathematical model, it is particularly suited when
           the model is simple (#feature << #sample) and the number of outliers is very high

           RANSAC, or "RANdom SAmple Consensus" is an iterative method to estimate parameters of a mathematical model
           from a set of observed data which contains outliers. It is a non-deterministic algorithm in the sense that
           it produces a reasonable result only with a certain probability, with this probability increasing as more
           iterations are allowed.

           A basic assumption is that the data consists of "inliers", i.e., data whose distribution can be explained
           by some set of model parameters, and "outliers" which are data that do not fit the model. In addition to this,
           the data can be subject to noise. The outliers can come, e.g., from extreme values of the noise or from
           erroneous measurements or incorrect hypotheses about the interpretation of data. RANSAC also assumes that,
           given a (usually small) set of inliers, there exists a procedure which can estimate the parameters of a model
           that optimally explains or fits this data.

    @param Estimator must define <Mode> and <Point> types, as well as methods:
           - template <class Points> void estimate( const Points& points ) MUST BE THREADSAFE
           - const Model& getModel() const
           - double error( const Point& point ) const, returns the % of error compared to the predicted value

           Model must define:
           - void print( std::ostream& o ) const

    @note the <Estimator> must be guarded against degenerated model, as the subset selection doesn't
          ensure unicity of the data selected...
    @note Estimator::estimate must be threadsafe
    */
   template < class Estimator, class EstimatorFactoryM = GenericEstimatorFactory<Estimator> >
   class Ransac
   {
   public:
      typedef typename Estimator::Model   Model;
      typedef typename Estimator::Point   Point;
      typedef EstimatorFactoryM           EstimatorFactory;

      Ransac( EstimatorFactory estimatorFactory = EstimatorFactory() ) : _estimatorFactory( estimatorFactory )
      {
      }

      /**
       @brief Run the algorithm and estimate model parameters
       @param points the set of points, with possibly a highratio of outliers
       @param minimalSample the number of points from which the model will be initially estimated, it must be a somewhat minimal
              number of points from which the model can be computed (e.g., for a line estimator =2, 3)
       @param numberOfSubset the number of sets that will be tested. This must be increased exponentially with the ratio of outliers
       @param maxError the maximum error a test point to have to be considered an inlier, specified in %
       @param weights if weights.size() == 0, no weighting is done, else one-one matching between weight and point
              This will be used in RANSAC where it will maximize the sum of the weighted inliers and not anymore the number of inliers
       */
      template <class Points>
      Model estimate( const Points& points, size_t minimalSample, size_t numberOfSubsets, double maxError, const core::Buffer1D<float>& weights )
      {
         enum Value{ value = core::Equal<typename Points::value_type, Point>::value };
         STATIC_ASSERT( value ); // "the points must be identitcal"
         ensure( weights.size() == 0 || weights.size() == points.size(), "Weights must be empty or have the same dimension as the points" );

         {
            std::stringstream ss;
            ss << "Ransac estimation:" << std::endl <<
                  " numberOfPoints=" << points.size() << std::endl << 
                  " minimalSampleSize=" << minimalSample << std::endl <<
                  " numberOfSubsets=" << numberOfSubsets << std::endl <<
                  " maxError=" << maxError;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         double                  bestError = -1000;
         Model                   bestModel;
         std::vector<size_t>       bestSubset;      // just save a reference, faster than copy the actual point...
         float                   bestWeight = 0;

         const size_t nbPoint = static_cast<size_t>( points.size() );

         //#ifndef NLL_NOT_MULTITHREADED
         //# pragma omp parallel for
         //#endif
         for ( int n = 0; n < (int)numberOfSubsets; ++n )
         {
            std::auto_ptr<Estimator> estimator;
            std::vector<size_t>       currentSubset;

            {
               currentSubset.reserve( points.size() );
               estimator = _estimatorFactory.create();
            }

            core::ConstCollectionWrapper<Points> initialSubset( points );
            initialSubset.reserve( minimalSample );
            for ( size_t nn = 0; nn < minimalSample; ++nn )
            {
               // randomly select a subset of point. As it is random, maybe the point
               // will be selected several times, and this is fine: the model will be degenerated
               // and so will be discarded...
               const size_t index = rand() % nbPoint;
               initialSubset.insertRef( index );
            }

            try
            {
               estimator->estimate( initialSubset );  // the estimator may throw, we don't want to cancel all computations just because one failed...
               double meanError = 0;
               float weightedInliers = 0;
               for ( size_t nn = 0; nn < nbPoint; ++nn )
               {
                  // compute the subset of inliers
                  const double err = estimator->error( points[ nn ] );
                  if ( err < maxError )
                  {
                     currentSubset.push_back( nn );
                     meanError += err;
                     weightedInliers += weights.size() ? weights[ nn ] : 1;
                  }
               }

               meanError /= currentSubset.size();

               if ( weightedInliers > bestWeight )
               {
                  //#ifndef NLL_NOT_MULTITHREADED
                  //# pragma omp critical
                  //#endif
                  {
                     //if ( weightedInliers > bestWeight )    // double locking mecanism, the first one is unsafe,
                     //                                       // filtering most of them, second one is safe, but slower
                     {
                        // save the model as it agrees with more points
                        bestModel = estimator->getModel();
                        bestSubset = currentSubset;
                        bestError = meanError;
                        bestWeight = weightedInliers;
                     }
                  }
               }
            } catch (...)
            {
               // nothing to do...
            }
         }

         // now recompute the model parameters with the inlier subset
         Points inliers;
         inliers.reserve( bestSubset.size() );
         const size_t nbInliers = static_cast<size_t>( bestSubset.size() );
         for ( size_t n = 0; n < nbInliers; ++n )
         {
            inliers.push_back( points[ bestSubset[ n ] ] );
         }
         std::auto_ptr<Estimator> estimator = _estimatorFactory.create();

         try
         {
            estimator->estimate( inliers );
         } catch (...)
         {
            // error in estimation...
         }

         {
            std::stringstream ss;
            ss << " best subset:" << std::endl <<
                  "  weighted inliers=" << bestWeight << std::endl <<
                  "  inlier size=" << inliers.size() << std::endl <<
                  "  inlier error=" << bestError << std::endl;

            estimator->getModel().print( ss );
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }
         _nbInliers = static_cast<f32>( inliers.size() );
         _inlierId = bestSubset;
         return estimator->getModel();
      }

      f32 getNbInliers() const
      {
         return _nbInliers;
      }

      const std::vector<size_t>& getInliers() const
      {
         return _inlierId;
      }

   private:
      EstimatorFactory  _estimatorFactory;
      f32               _nbInliers;
      std::vector<size_t> _inlierId;
   };

   /**
    @brief Example of line estimator to be used with RANSAC
    */
   template <class PointM>
   class LineEstimator
   {
   public:
      struct Model
      {
         double a;
         double b;

         void print( std::ostream& o ) const
         {
            o << "  line model: a=" << a << " b=" << b;
         }
      };

      typedef PointM Point;

      template <class Points>
      void estimate( const Points& points )
      {
         algorithm::PrincipalComponentAnalysis<Points> pca;
         bool success = pca.compute( points, 2 );
         if (success)
         {
            // y = ax + b
            const size_t maxIndex = pca.getEigenValues()[ 0 ] > pca.getEigenValues()[ 1 ] ? 0 : 1;
            const double dx = pca.getEigenVectors()( 0, maxIndex );
            const double dy = pca.getEigenVectors()( 1, maxIndex );
            if ( fabs( dx ) < 1e-5 )
            {
               _model.a = 0;
            } else {
               _model.a = dy / dx;
            }
            _model.b = pca.getMean()[ 1 ] - pca.getMean()[ 0 ] * _model.a;
            return;
         }

         // else leave the estimation as null
         _model.a = 0;
         _model.b = 0;
      }

      /**
       @brief Returns squared error, assuming a model y = ax + b, measure the error between y estimate and real value / | x |
       */
      double error( const Point& point ) const
      {
         const double yestimate = _model.a * point[ 0 ] + _model.b;
         const double err = ( point[ 1 ] - yestimate ) / ( fabs( point[ 1 ] ) + 1e-4 );
         return err * err;
      }

      /**
       @brief Returns the current model
       */
      const Model& getModel() const
      {
         return _model;
      }

   private:
      Model    _model;
   };
}
}

#endif


/* // it seems there is a concurrency problems when running thousands of registration with RANSAC... below is safe
 //     #ifndef NLL_NOT_MULTITHREADED
    //     # pragma omp parallel for
    //     #endif
         for ( int n = 0; n < (int)numberOfSubsets; ++n )
         {
            std::auto_ptr<Estimator> estimator;
            std::vector<size_t>       currentSubset;

            #ifndef NLL_NOT_MULTITHREADED
            # pragma omp critical
            #endif
            {
               currentSubset.reserve( points.size() );
               estimator = _estimatorFactory.create();
            }

            core::ConstCollectionWrapper<Points> initialSubset( points );
            initialSubset.reserve( minimalSample );
            for ( size_t nn = 0; nn < minimalSample; ++nn )
            {
               // randomly select a subset of point. As it is random, maybe the point
               // will be selected several times, and this is fine: the model will be degenerated
               // and so will be discarded...
               const size_t index = rand() % nbPoint;
               initialSubset.insertRef( index );
            }

   //         #ifndef NLL_NOT_MULTITHREADED
   //         # pragma omp critical
   //         #endif
            {
               estimator->estimate( initialSubset );
            }


            double meanError = 0;

   //         #ifndef NLL_NOT_MULTITHREADED
   //         # pragma omp critical
   //         #endif
            {
               for ( size_t nn = 0; nn < nbPoint; ++nn )
               {
                  // compute the subset of inliers
                  const double err = estimator->error( points[ nn ] );
                  if ( err < maxError )
                  {
                     currentSubset.push_back( nn );
                     meanError += err;
                  }
               }
            }
            meanError /= currentSubset.size();

            #ifndef NLL_NOT_MULTITHREADED
            # pragma omp critical
            #endif
            {
               if ( currentSubset.size() > bestSubset.size() )
               {
                  // save the model as it agrees with more points
                  bestModel = estimator->getModel();
                  bestSubset = currentSubset;
                  bestError = meanError;
               }
            }
         }
*/