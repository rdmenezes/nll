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

#ifndef NLL_ALGORITHM_REGISTRATION_2D_H_
# define NLL_ALGORITHM_REGISTRATION_2D_H_

namespace nll
{
namespace algorithm
{
   namespace impl
   {
      /**
       @ingroup algorithm
       @brief Find match in two sets of points using a simple kd-tree
       */
      class FeatureMatcher
      {
      public:
         struct Match
         {
            Match( ui32 i1, ui32 i2, double d ) : index1( i1 ), index2( i2 ), dist( d )
            {}

            Match()
            {}

            bool operator<( const Match& m ) const
            {
               return dist < m.dist;
            }

            ui32     index1;
            ui32     index2;
            double   dist;
         };
         typedef std::vector< Match >  Matches;

         /**
          @param nbPairsToBeConsidered During the nearest neighbour search, we select the <nbPairsToBeConsidered> neighbours
          */
         FeatureMatcher( ui32 nbPairsToBeConsidered = 1 ) : _nbPairsToBeConsidered( nbPairsToBeConsidered )
         {}

         template <class Points>
         void findMatch( const Points& points1, const Points& points2, Matches& matches ) const
         {
            typedef typename Points::value_type    Point;
            typedef algorithm::KdTree< Point, MetricEuclidian<Point>, 5, Points >  KdTree;
            matches.clear();

            if ( points1.size() <= 1 + _nbPairsToBeConsidered || 
                 points2.size() <= 1 + _nbPairsToBeConsidered )
               return;
            const ui32 nbFeatures = (ui32)points1[ 0 ].size();

            // find a match from the smaller set to the bigger one
            if ( points2.size() > points1.size() )
            {
               matches.resize( points1.size() * _nbPairsToBeConsidered );

               KdTree tree;
               tree.build( points2, nbFeatures );

               #ifndef NLL_NOT_MULTITHREADED
               # pragma omp parallel for
               #endif
               for ( int n = 0; n < (int)points1.size(); ++n )
               {
                  typename KdTree::NearestNeighborList list = tree.findNearestNeighbor( points1[ n ], _nbPairsToBeConsidered );
                  if ( list.size() )
                  {
                     size_t nn = 0;
                     for (typename KdTree::NearestNeighborList::const_iterator i = list.begin(); i != list.end(); ++i, ++nn)
                     {
                        const float d = i->dist;
                        const size_t index = n * _nbPairsToBeConsidered + nn;
                        matches[ index ] = Match( n, i->id, d );
                     }
                  }
               }
            } else {

               matches.resize( points2.size() * _nbPairsToBeConsidered );

               KdTree tree;
               tree.build( points1, nbFeatures );

               #ifndef NLL_NOT_MULTITHREADED
               # pragma omp parallel for
               #endif
               for ( int n = 0; n < (int)points2.size(); ++n )
               {
                  typename KdTree::NearestNeighborList list = tree.findNearestNeighbor( points2[ n ], _nbPairsToBeConsidered );
                  if ( list.size() )
                  {
                     size_t nn = 0;
                     for (typename KdTree::NearestNeighborList::const_iterator i = list.begin(); i != list.end(); ++i, ++nn)
                     {
                        const float d = i->dist;
                        const size_t index = n * _nbPairsToBeConsidered + nn;
                        matches[ index ] = Match( i->id, n, d );
                     }
                  }
               }
            }

            std::sort( matches.begin(), matches.end() );
         }

      private:
         ui32  _nbPairsToBeConsidered;
      };

      class SimilarityIsotropicTransformationEstimatorRansac
      {
      private:
         typedef SpeededUpRobustFeatures::Points PointsData;

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

            const core::vector2i& operator[]( ui32 index ) const
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

            const core::vector2i& operator[]( ui32 index ) const
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
         SimilarityIsotropicTransformationEstimatorRansac( const SpeededUpRobustFeatures::Points& p1, const SpeededUpRobustFeatures::Points& p2,
                                                       double scale = 0, double minimumScale = 0.8, double maximumScale = 1.2 ) : _p1( p1 ), _p2( p2 ), _scale( scale ), _minimumScale( minimumScale ), _maximumScale( maximumScale )
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

         typedef FeatureMatcher::Match    Point;

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
            
            const SpeededUpRobustFeatures::Point& p1 = _p1[ point.index1 ];
            const SpeededUpRobustFeatures::Point& p2 = _p2[ point.index2 ];

            // transform the point
            const core::Matrix<double>& tfm = _model.tfm;
            double px = tfm( 0, 2 ) + p1.position[ 0 ] * tfm( 0, 0 ) + p1.position[ 1 ] * tfm( 0, 1 );
            double py = tfm( 1, 2 ) + p1.position[ 0 ] * tfm( 1, 0 ) + p1.position[ 1 ] * tfm( 1, 1 );

            // we want a ratio of the error...
            const double lengthTfm = core::sqr( px - p2.position[ 0 ] ) +
                                     core::sqr( py - p2.position[ 1 ] );
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

         const SpeededUpRobustFeatures::Points& getP1() const
         {
            return _p1;
         }

         const SpeededUpRobustFeatures::Points& getP2() const
         {
            return _p2;
         }

      private:
         // copy disabled
         SimilarityIsotropicTransformationEstimatorRansac& operator=( const SimilarityIsotropicTransformationEstimatorRansac& );

      private:
         Model    _model;
         const SpeededUpRobustFeatures::Points& _p1;
         const SpeededUpRobustFeatures::Points& _p2;
         const double   _scale;
         const double   _minimumScale;
         const double   _maximumScale;
      };

      class SimilarityNonIsotropicTransformationEstimatorRansac
      {
      private:
         typedef SpeededUpRobustFeatures::Points PointsData;

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

            const core::vector2i& operator[]( ui32 index ) const
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

            const core::vector2i& operator[]( ui32 index ) const
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
         SimilarityNonIsotropicTransformationEstimatorRansac( const SpeededUpRobustFeatures::Points& p1, const SpeededUpRobustFeatures::Points& p2,
                                                          double minimumScale = 0.8, double maximumScale = 1.2 ) : _p1( p1 ), _p2( p2 ), _minimumScale( minimumScale ), _maximumScale( maximumScale )
         {
         }

         static ui32 minimumNumberOfPointsForEstimation()
         {
            return 3;
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

         typedef FeatureMatcher::Match    Point;

         template <class Points>
         void estimate( const Points& points )
         {
            if ( points.size() < 3 )
               return;
           
            PointsWrapper1<Points> wrapperP1( _p1, points );
            PointsWrapper2<Points> wrapperP2( _p2, points );
            
            EstimatorAffineSimilarityNonIsotropic2d estimator( _minimumScale, _maximumScale );
            _model.tfm = estimator.compute( wrapperP1, wrapperP2 );
            ensure( estimator.getLastResult() == EstimatorAffineSimilarityNonIsotropic2d::OK, "the estimator failed to estimate the 2D transformation" );
         }

         /**
          @brief Returns squared error. Transforms the source point to target point given a transformation,
                 and returns the squared sum of the differences
          */
         double error( const Point& point ) const
         {
            
            const SpeededUpRobustFeatures::Point& p1 = _p1[ point.index1 ];
            const SpeededUpRobustFeatures::Point& p2 = _p2[ point.index2 ];

            // transform the point
            const core::Matrix<double>& tfm = _model.tfm;
            double px = tfm( 0, 2 ) + p1.position[ 0 ] * tfm( 0, 0 ) + p1.position[ 1 ] * tfm( 0, 1 );
            double py = tfm( 1, 2 ) + p1.position[ 0 ] * tfm( 1, 0 ) + p1.position[ 1 ] * tfm( 1, 1 );

            // we want a ratio of the error...
            const double lengthTfm = core::sqr( px - p2.position[ 0 ] ) +
                                     core::sqr( py - p2.position[ 1 ] );
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

         const SpeededUpRobustFeatures::Points& getP1() const
         {
            return _p1;
         }

         const SpeededUpRobustFeatures::Points& getP2() const
         {
            return _p2;
         }

      private:
         // copy disabled
         SimilarityNonIsotropicTransformationEstimatorRansac& operator=( const SimilarityNonIsotropicTransformationEstimatorRansac& );

      private:
         Model    _model;
         const SpeededUpRobustFeatures::Points& _p1;
         const SpeededUpRobustFeatures::Points& _p2;
         const double   _minimumScale;
         const double   _maximumScale;
      };

      class AffineTransformationEstimatorRansac
      {
      private:
         typedef SpeededUpRobustFeatures::Points PointsData;

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

            const core::vector2i& operator[]( ui32 index ) const
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

            const core::vector2i& operator[]( ui32 index ) const
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
         AffineTransformationEstimatorRansac( const SpeededUpRobustFeatures::Points& p1, const SpeededUpRobustFeatures::Points& p2,
                                              double minimumScale = 0.8, double maximumScale = 1.2, double maxShearing = 0.1 ) : _p1( p1 ), _p2( p2 ), _minimumScale( minimumScale ), _maximumScale( maximumScale ), _maxShearing( maxShearing )
         {
         }

         static ui32 minimumNumberOfPointsForEstimation()
         {
            return 4;
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

         typedef FeatureMatcher::Match    Point;

         template <class Points>
         void estimate( const Points& points )
         {
            if ( points.size() < 2 )
               return;
           
            PointsWrapper1<Points> wrapperP1( _p1, points );
            PointsWrapper2<Points> wrapperP2( _p2, points );
            
            EstimatorTransformAffine2dDlt affineEstimator( _minimumScale, _maximumScale, _maxShearing );
            _model.tfm = affineEstimator.compute( wrapperP1, wrapperP2 );
         }

         /**
          @brief Returns squared error. Transforms the source point to target point given a transformation,
                 and returns the squared sum of the differences
          */
         double error( const Point& point ) const
         {
            const SpeededUpRobustFeatures::Point& p1 = _p1[ point.index1 ];
            const SpeededUpRobustFeatures::Point& p2 = _p2[ point.index2 ];

            // transform the point
            const core::Matrix<double>& tfm = _model.tfm;
            double px = tfm( 0, 2 ) + p1.position[ 0 ] * tfm( 0, 0 ) + p1.position[ 1 ] * tfm( 0, 1 );
            double py = tfm( 1, 2 ) + p1.position[ 0 ] * tfm( 1, 0 ) + p1.position[ 1 ] * tfm( 1, 1 );

            const double lengthTfm = core::sqr( px - p2.position[ 0 ] ) +
                                     core::sqr( py - p2.position[ 1 ] );
            return fabs( lengthTfm );
         }

         /**
          @brief Returns the current model
          */
         const Model& getModel() const
         {
            return _model;
         }

      private:
         // copy disabled
         AffineTransformationEstimatorRansac& operator=( const AffineTransformationEstimatorRansac& );

      private:
         Model    _model;
         const SpeededUpRobustFeatures::Points& _p1;
         const SpeededUpRobustFeatures::Points& _p2;
         const double   _minimumScale;
         const double   _maximumScale;
         const double   _maxShearing;
      };

      class SurfEstimatorSimilarityIsotropicFactory
      {
      public:
         typedef SimilarityIsotropicTransformationEstimatorRansac Estimator;
         SurfEstimatorSimilarityIsotropicFactory( const SpeededUpRobustFeatures::Points& p1, const SpeededUpRobustFeatures::Points& p2,
                                              double scale = 0, double minimumScale = 0.8, double maximumScale = 1.2 ) : _p1( &p1 ), _p2( &p2 ), _scale( scale ), _minimumScale( minimumScale ), _maximumScale( maximumScale )
         {}

         std::auto_ptr<Estimator> create() const
         {
            return std::auto_ptr<Estimator>( new Estimator( *_p1, *_p2, _scale, _minimumScale, _maximumScale ) );
         }

      private:
         // copy disabled
         const SurfEstimatorSimilarityIsotropicFactory& operator=( const SurfEstimatorSimilarityIsotropicFactory& );

      private:
         const SpeededUpRobustFeatures::Points* _p1;
         const SpeededUpRobustFeatures::Points* _p2;
         const double   _scale;
         const double   _minimumScale;
         const double   _maximumScale;
      };

      class SurfEstimatorSimilarityNonIsotropicFactory
      {
      public:
         typedef SimilarityNonIsotropicTransformationEstimatorRansac Estimator;
         SurfEstimatorSimilarityNonIsotropicFactory( const SpeededUpRobustFeatures::Points& p1, const SpeededUpRobustFeatures::Points& p2,
                                                 double minimumScale = 0.4, double maximumScale = 2 ) : _p1( &p1 ), _p2( &p2 ), _minimumScale( minimumScale ), _maximumScale( maximumScale )
         {}

         std::auto_ptr<Estimator> create() const
         {
            return std::auto_ptr<Estimator>( new Estimator( *_p1, *_p2, _minimumScale, _maximumScale ) );
         }

      private:
         // copy disabled
         const SurfEstimatorSimilarityNonIsotropicFactory& operator=( const SurfEstimatorSimilarityNonIsotropicFactory& );

      private:
         const SpeededUpRobustFeatures::Points* _p1;
         const SpeededUpRobustFeatures::Points* _p2;
         const double   _minimumScale;
         const double   _maximumScale;
      };

      class SurfEstimatorAffineFactory
      {
      public:
         typedef AffineTransformationEstimatorRansac Estimator;
         SurfEstimatorAffineFactory( const SpeededUpRobustFeatures::Points& p1, const SpeededUpRobustFeatures::Points& p2,
                                     double minimumScale = 0.5, double maximumScale = 1.6, double maximumShearing = 10 ) : _p1( &p1 ), _p2( &p2 ), _minimumScale( minimumScale ), _maximumScale( maximumScale ), _maximumShearing( maximumShearing )
         {}

         std::auto_ptr<Estimator> create() const
         {
            return std::auto_ptr<Estimator>( new Estimator( *_p1, *_p2, _minimumScale, _maximumScale, _maximumShearing ) );
         }

      private:
         SurfEstimatorAffineFactory& operator=( const SurfEstimatorAffineFactory& );

      private:
         const SpeededUpRobustFeatures::Points* _p1;
         const SpeededUpRobustFeatures::Points* _p2;
         const double   _minimumScale;
         const double   _maximumScale;
         const double   _maximumShearing;
      };
   }

   /**
    @ingroup algorithm
    @brief Point based registration, internally using SURF to create the interest points,
           RANSAC to estimate the best transformation, and <EstimatorTransformAffine2D> to estimate the
           local transformation
    
    Target is the moving volume.

    Compute the transformation in the form source->target to apply on the target volume.

    @note this class is just an example and will likely need a lot of customization by the user to fit
          the given registration problem
    */
   template <class FactoryT = impl::SurfEstimatorSimilarityIsotropicFactory, class FeatureMatcherT = impl::FeatureMatcher>
   class AffineRegistrationPointBased2d
   {
   public:
      typedef FeatureMatcherT                                  FeatureMatcher;
      typedef typename FactoryT::Estimator                     RansacTransformationEstimator;
      typedef FactoryT                                         Factory;
      typedef core::Matrix<double>                             Matrix;
      typedef typename FeatureMatcher::Matches::value_type     Match;
      typedef algorithm::SpeededUpRobustFeatures::Point        Point;
      typedef std::vector< std::pair<Point, Point> >           PointPairs;

      AffineRegistrationPointBased2d( ui32 surfNumberOfOctaves = 5, ui32 surfNumberOfIntervals = 6, double surfThreshold = 0.00011, FeatureMatcher matcher = FeatureMatcher() ) : _surfNumberOfOctaves( surfNumberOfOctaves ), _surfNumberOfIntervals( surfNumberOfIntervals ), _surfThreshold( surfThreshold ), _matcher( matcher )
      {}

   public:
      template <class T, class Mapper, class Alloc>
      Matrix compute( const core::Image<T, Mapper, Alloc>& source,
                      const core::Image<T, Mapper, Alloc>& target,
                      const core::vector2i& minBoundingBoxSource = core::vector2i(),
                      const core::vector2i& maxBoundingBoxSource = core::vector2i(),
                      const core::vector2i& minBoundingBoxTarget = core::vector2i(),
                      const core::vector2i& maxBoundingBoxTarget = core::vector2i(),
                      ui32 maxErrorInPixel = 15,
                      ui32 minimumInliers = 20,
                      ui32 nbRansacIterations = 25000,
                      ui32 nbMaxRansacPairs = 1000 )
      {
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "starting 2D registration..." );
         {
            std::stringstream ss;
            ss << " source size=" << source.sizex() << " " << source.sizey() << std::endl;
            ss << " target size=" << target.sizex() << " " << target.sizey() << std::endl;
            ss << " source bounding box=" << minBoundingBoxSource[ 0 ] << " " << minBoundingBoxSource[ 1 ] << " " << "|| "
                                          << maxBoundingBoxSource[ 0 ] << " " << maxBoundingBoxSource[ 1 ] << " " << std::endl;
            ss << " target bounding box=" << minBoundingBoxTarget[ 0 ] << " " << minBoundingBoxTarget[ 1 ] << " " << "|| "
                                          << maxBoundingBoxTarget[ 0 ] << " " << maxBoundingBoxTarget[ 1 ] << " " << std::endl;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         algorithm::SpeededUpRobustFeatures surf( _surfNumberOfOctaves, _surfNumberOfIntervals, 2, _surfThreshold );

         algorithm::SpeededUpRobustFeatures::Points points1 = surf.computesFeatures( source );
         algorithm::SpeededUpRobustFeatures::Points points2 = surf.computesFeatures( target );
         return compute( points1, points2, minBoundingBoxSource, maxBoundingBoxSource,
                                           minBoundingBoxTarget, maxBoundingBoxTarget,
                         maxErrorInPixel, minimumInliers, nbRansacIterations, nbMaxRansacPairs );
      }

      /**
       @note sourcePoints for each source point, find the closest target point and run ransac on it. Additionally,
             the points will be can be weighted using the targetPoints' weights
       */
      Matrix compute( const algorithm::SpeededUpRobustFeatures::Points& sourcePoints,
                      const algorithm::SpeededUpRobustFeatures::Points& targetPoints,
                      const core::vector2i& minBoundingBoxSource = core::vector2i(),
                      const core::vector2i& maxBoundingBoxSource = core::vector2i(),
                      const core::vector2i& minBoundingBoxTarget = core::vector2i(),
                      const core::vector2i& maxBoundingBoxTarget = core::vector2i(),
                      ui32 maxErrorInPixel = 15,
                      ui32 minimumInliers = 20,
                      ui32 nbRansacIterations = 25000,
                      ui32 nbMaxRansacPairs = 1000 )
      {
         nll::core::Timer timerFull;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "starting 2D registration..." );
         {
            std::stringstream ss;
            ss << " source bounding box=" << minBoundingBoxSource[ 0 ] << " " << minBoundingBoxSource[ 1 ] << " " << "|| "
                                          << maxBoundingBoxSource[ 0 ] << " " << maxBoundingBoxSource[ 1 ] << " " << std::endl;
            ss << " target bounding box=" << minBoundingBoxTarget[ 0 ] << " " << minBoundingBoxTarget[ 1 ] << " " << "|| "
                                          << maxBoundingBoxTarget[ 0 ] << " " << maxBoundingBoxTarget[ 1 ] << " " << std::endl;
            ss << " surfNumberOfOctaves=" << _surfNumberOfOctaves << " surfNumberOfIntervals=" << _surfNumberOfIntervals << " surfThreshold=" << _surfThreshold;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         algorithm::SpeededUpRobustFeatures::Points points1 = sourcePoints;
         _originalPoints1 = points1;
         points1 = trimPoints( points1, minBoundingBoxSource, maxBoundingBoxSource );
         algorithm::SpeededUpRobustFeatures::Points points2 = targetPoints;
         _originalPoints2 = points2;
         points2 = trimPoints( points2, minBoundingBoxTarget, maxBoundingBoxTarget );

         // match points
         algorithm::SpeededUpRobustFeatures::PointsFeatureWrapper p1Wrapper( points1 );
         algorithm::SpeededUpRobustFeatures::PointsFeatureWrapper p2Wrapper( points2 );

         nll::core::Timer timerMatcher;
         typename FeatureMatcher::Matches matches;
         _matcher.findMatch( p1Wrapper, p2Wrapper, matches );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "point matching time=" + core::val2str( timerMatcher.getCurrentTime() ) );

         // take only the best subset...
         ensure( matches.size(), "No match found!" );
         typename FeatureMatcher::Matches matchesTrimmed( matches.begin(), matches.begin() + std::min<ui32>( nbMaxRansacPairs, (ui32)matches.size() - 1 ) );

         // estimate the transformation
         typedef algorithm::Ransac<RansacTransformationEstimator, Factory> Ransac;

         Factory estimatorFactory( points1, points2 );
         Ransac ransac( estimatorFactory );

         core::Buffer1D<float> weights( matchesTrimmed.size() );
         for ( ui32 n = 0; n < static_cast<ui32>( weights.size() ); ++n )
         {
            ui32 indexTarget = matchesTrimmed[ n ].index2;
            weights[ n ] = points2[ indexTarget ].weight;
         }

         core::Timer ransacOptimTimer;
         typename RansacTransformationEstimator::Model model = ransac.estimate( matchesTrimmed,
                                                                                RansacTransformationEstimator::minimumNumberOfPointsForEstimation(),
                                                                                nbRansacIterations,
                                                                                maxErrorInPixel * maxErrorInPixel,
                                                                                weights );

         ensure( ransac.getNbInliers() > minimumInliers, "Error: inliers are too small" );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "RANSAC optimization time=" + nll::core::val2str( ransacOptimTimer.getCurrentTime() ) );

         _inliers.clear();
         _inliers.reserve( ransac.getNbInliers() );
         const std::vector<ui32>& inliers = ransac.getInliers();
         for ( ui32 n = 0; n < (ui32)inliers.size(); ++n )
         {
            const ui32 inlierId = ransac.getInliers()[ n ];
            const Point& p1 = points1[ matchesTrimmed[ inlierId ].index1 ];
            const Point& p2 = points2[ matchesTrimmed[ inlierId ].index2 ];
            _inliers.push_back( std::make_pair( p1, p2 ) );
         }
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "2D registration successful..." );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "2D registration total time=" + core::val2str( timerFull.getCurrentTime() ) );
         return model.tfm;
      }

      const PointPairs& getInliers() const
      {
         return _inliers;
      }

      const SpeededUpRobustFeatures::Points& getPoints1() const
      {
         return _originalPoints1;
      }

      const SpeededUpRobustFeatures::Points& getPoints2() const
      {
         return _originalPoints2;
      }

   private:
      // remove the points not within the bounding box
      algorithm::SpeededUpRobustFeatures::Points trimPoints( const algorithm::SpeededUpRobustFeatures::Points& points,
                                                             const core::vector2i& minBoundingBox,
                                                             const core::vector2i& maxBoundingBox )
      {
         if ( minBoundingBox == maxBoundingBox )
            return points;

         std::vector<ui32> indexes;
         for ( ui32 n = 0; n < points.size(); ++n )
         {
            if ( points[ n ].position[ 0 ] >= minBoundingBox[ 0 ] &&
                 points[ n ].position[ 1 ] >= minBoundingBox[ 1 ] &&
                 points[ n ].position[ 0 ] <= maxBoundingBox[ 0 ] &&
                 points[ n ].position[ 1 ] <= maxBoundingBox[ 1 ] )
            {
               indexes.push_back( n );
            }
         }

         const ui32 size = static_cast<ui32>( indexes.size() );
         algorithm::SpeededUpRobustFeatures::Points trimmedPoints( size );
         for ( ui32 n = 0; n < size; ++n )
         {
            trimmedPoints[ n ] = points[ indexes[ n ] ];
         }

         return trimmedPoints;
      }


      

  private:
     FeatureMatcher                    _matcher;
     ui32                              _surfNumberOfOctaves;
     ui32                              _surfNumberOfIntervals;
     double                            _surfThreshold;
     PointPairs                        _inliers;
     SpeededUpRobustFeatures::Points   _originalPoints1;
     SpeededUpRobustFeatures::Points   _originalPoints2;
  };
}
}

#endif