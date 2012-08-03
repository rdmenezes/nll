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

#ifndef NLL_ALGORITHM_REGISTRATION_3D_H_
# define NLL_ALGORITHM_REGISTRATION_3D_H_

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

            size_t size() const
            {
               return static_cast<size_t>( _matches.size() );
            }

            const core::vector3i& operator[]( size_t index ) const
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

            size_t size() const
            {
               return static_cast<size_t>( _matches.size() );
            }

            const core::vector3i& operator[]( size_t index ) const
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

         static size_t minimumNumberOfPointsForEstimation()
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
         size_t       nbSourcePoints;
         size_t       nbTargetPoints;
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
                      size_t maxErrorInPixel = 20,
                      size_t minimumInliers = 20,
                      size_t nbRansacIterations = 35000,
                      size_t nbMaxRansacPairs = 1000 ) const
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
         typename FeatureMatcher::Matches matchesTrimmed( matches.begin(), matches.begin() + std::min<size_t>( nbMaxRansacPairs, (size_t)matches.size() - 1 ) );

         // estimate the transformation
         typedef algorithm::Ransac<RansacTransformationEstimator, TransformationEstimatorFactory> Ransac;

         TransformationEstimatorFactory estimatorFactory( sourcePoints, targetPoints );
         Ransac ransac( estimatorFactory );

         core::Buffer1D<float> weights( matchesTrimmed.size() );
         for ( size_t n = 0; n < static_cast<size_t>( weights.size() ); ++n )
         {
            const size_t indexTarget = matchesTrimmed[ n ].index2;
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

#endif