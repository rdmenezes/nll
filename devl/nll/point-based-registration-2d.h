/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
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
       @brief Find match in two sets of points using a simple kd-tree
       */
      class FeatureMatcher
      {
      public:
         struct Match
         {
            Match( ui32 i1, ui32 i2, double d ) : index1( i1 ), index2( i2 ), dist( d )
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

         template <class Points>
         void findMatch( const Points& points1, const Points& points2, Matches& matches ) const
         {
            typedef typename Points::value_type    Point;
            typedef algorithm::KdTree< Point, MetricEuclidian<Point>, 5, Points >  KdTree;
            matches.clear();

            if ( points1.size() == 0 )
               return;
            const ui32 nbFeatures = points1[ 0 ].size();

            // find a match from the smaller set to the bigger one
            if ( points2.size() > points1.size() )
            {
               KdTree tree;
               tree.build( points2, nbFeatures );

               for ( ui32 n = 0; n < points1.size(); ++n )
               {
                  KdTree::NearestNeighborList list = tree.findNearestNeighbor( points1[ n ], 1 );
                  if ( list.size() )
                  {
                     const float d = list.begin()->dist;
                     matches.push_back( Match( n, list.begin()->id, d ) );
                  }
               }
            } else {
               KdTree tree;
               tree.build( points1, nbFeatures );

               for ( ui32 n = 0; n < points2.size(); ++n )
               {
                  KdTree::NearestNeighborList list = tree.findNearestNeighbor( points2[ n ], 1 );
                  if ( list.size() )
                  {
                     const float d = list.begin()->dist;
                     matches.push_back( Match( list.begin()->id, n, d ) );
                  }
               }
            }

            std::sort( matches.begin(), matches.end() );
         }
      };

      class TranslationTransformationEstimatorRansac
      {
      public:
         TranslationTransformationEstimatorRansac( const SpeededUpRobustFeatures::Points& p1, const SpeededUpRobustFeatures::Points& p2 ) : _p1( p1 ), _p2( p2 )
         {
         }

         struct Model
         {
            Model() : tfm( core::identityMatrix< core::Matrix< double > >( 3 ) )
            {}

            void print( std::ostream& o ) const
            {
               o << " Transformation Translation only=" << tfm( 0, 2 ) << " " << tfm( 1, 2 );
            }

            core::Matrix< double > tfm;
         };

         typedef FeatureMatcher::Match    Point;

         template <class Points>
         void estimate( const Points& points )
         {
            double dx = 0;
            double dy = 0;

            const ui32 nbPoints = static_cast<ui32>( points.size() );
            for ( ui32 n = 0; n < nbPoints; ++n )
            {
               const SpeededUpRobustFeatures::Point& p1 = _p1[ points[ n ].index1 ];
               const SpeededUpRobustFeatures::Point& p2 = _p2[ points[ n ].index2 ];

               dx += p2.position[ 0 ] - p1.position[ 0 ];
               dy += p2.position[ 1 ] - p1.position[ 1 ];
            }

            dx /= nbPoints;
            dy /= nbPoints;

            _model.tfm( 0, 2 ) = dx;
            _model.tfm( 1, 2 ) = dy;
         }

         /**
          @brief Returns squared error, assuming a model y = ax + b, measure the error between y estimate and real value / | x |
          */
         double error( const Point& point ) const
         {
            const SpeededUpRobustFeatures::Point& p1 = _p1[ point.index1 ];
            const SpeededUpRobustFeatures::Point& p2 = _p2[ point.index2 ];
            double px = p1.position[ 0 ] + _model.tfm( 0, 2 );
            double py = p1.position[ 1 ] + _model.tfm( 1, 2 );
            return core::sqr( ( px - p2.position[ 0 ] ) / ( p2.position[ 0 ] + 1e-2 ) ) +
                   core::sqr( ( py - p2.position[ 1 ] ) / ( p2.position[ 1 ] + 1e-2 ) );
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
         TranslationTransformationEstimatorRansac& operator=( const TranslationTransformationEstimatorRansac& );

      private:
         Model    _model;
         const SpeededUpRobustFeatures::Points& _p1;
         const SpeededUpRobustFeatures::Points& _p2;
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
            const PointsData     _points;
            const PointsMatch    _matches;
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
            const PointsData     _points;
            const PointsMatch    _matches;
         };

      public:
         AffineTransformationEstimatorRansac( const SpeededUpRobustFeatures::Points& p1, const SpeededUpRobustFeatures::Points& p2 ) : _p1( p1 ), _p2( p2 )
         {
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
            
            EstimatorTransformAffine2D affineEstimator;
            _model.tfm = affineEstimator.compute( wrapperP1, wrapperP2 );
         }

         /**
          @brief Returns squared error, assuming a model y = ax + b, measure the error between y estimate and real value / | x |
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
            return core::sqr( ( px - p2.position[ 0 ] ) / ( p2.position[ 0 ] ) ) +
                   core::sqr( ( py - p2.position[ 1 ] ) / ( p2.position[ 1 ] ) );
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
      };

      template <class EstimatorT>
      class SurfEstimatorFactory
      {
      public:
         typedef EstimatorT Estimator;
         SurfEstimatorFactory( const SpeededUpRobustFeatures::Points& p1, const SpeededUpRobustFeatures::Points& p2 ) : _p1( &p1 ), _p2( &p2 )
         {}

         Estimator create() const
         {
            return Estimator( *_p1, *_p2 );
         }

      private:
         const SpeededUpRobustFeatures::Points* _p1;
         const SpeededUpRobustFeatures::Points* _p2;
      };
   }

   /**
    @brief Point based registration, internally using SURF to create the interest points,
           RANSAC to estimate the best transformation, and <EstimatorTransformAffine2D> to estimate the
           local transformation

    @note this class is just an example and will likely need a lot of customization by the user to fit
          the given registration problem
    */
   template <class FeatureMatcher = impl::FeatureMatcher>
   class AffineRegistrationPointBased2d
   {
   public:
      typedef impl::AffineTransformationEstimatorRansac   RansacTransformationEstimator;
      typedef core::Matrix<double> Matrix;

      AffineRegistrationPointBased2d( ui32 surfNumberOfOctaves = 5, ui32 surfNumberOfIntervals = 6, double surfThreshold = 0.00011, FeatureMatcher matcher = FeatureMatcher() ) : _surfNumberOfOctaves( surfNumberOfOctaves ), _surfNumberOfIntervals( surfNumberOfIntervals ), _surfThreshold( surfThreshold ), _matcher( matcher )
      {}

   public:
      template <class T, class Mapper, class Alloc>
      Matrix compute( const core::Image<T, Mapper, Alloc>& source, const core::Image<T, Mapper, Alloc>& target ) const
      {
         nll::core::Timer timer;
         algorithm::SpeededUpRobustFeatures surf( _surfNumberOfOctaves, _surfNumberOfIntervals, 2, _surfThreshold );

         algorithm::SpeededUpRobustFeatures::Points points1 = surf.computesFeatures( source );
         algorithm::SpeededUpRobustFeatures::Points points2 = surf.computesFeatures( target );

         // match points
         algorithm::SpeededUpRobustFeatures::PointsFeatureWrapper p1Wrapper( points1 );
         algorithm::SpeededUpRobustFeatures::PointsFeatureWrapper p2Wrapper( points2 );

         typename FeatureMatcher::Matches matches;
         _matcher.findMatch( p1Wrapper, p2Wrapper, matches );

         // take only the best subset...
         algorithm::impl::FeatureMatcher::Matches matchesTrimmed( matches.begin(), matches.begin() + std::min<ui32>( 100, (ui32)matches.size() - 1 ) );

         // estimate the transformation
         typedef algorithm::impl::SurfEstimatorFactory<RansacTransformationEstimator> SurfEstimatorFactory;
         typedef algorithm::Ransac<RansacTransformationEstimator, SurfEstimatorFactory> Ransac;

         SurfEstimatorFactory estimatorFactory( points1, points2 );
         Ransac ransac( estimatorFactory );

         core::Timer ransacOptimTimer;
         RansacTransformationEstimator::Model model = ransac.estimate( matchesTrimmed, 5, 50000, 0.01 );
         return model.tfm;
     }

  private:
     FeatureMatcher                 _matcher;
     ui32                           _surfNumberOfOctaves;
     ui32                           _surfNumberOfIntervals;
     double                         _surfThreshold;
  };
}
}

#endif