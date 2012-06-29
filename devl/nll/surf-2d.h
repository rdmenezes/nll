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

#ifndef NLL_ALGORITHM_SURF_2D_H_
# define NLL_ALGORITHM_SURF_2D_H_

// if defined, SURF will use a global angle detection mecanism which seems more efficient/robust
#define NLL_SURF_USE_FEATURE_ANGLE_UPGRADE

#define NLL_SURF_2D_NB_AREA_PER_FEATURE         4.0
#define NLL_SURF_2D_NB_AREA_PER_FEATURE_SIZE    20.0
#define NLL_SURF_2D_NB_SUBAREA_PER_AREA         5.0

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Implementation of the Speeded Up Robust Features or SURF algorithm

           Feature detector which intend to be robust to scaling, illumination and rotation

    @see http://sites.google.com/site/chrisevansdev/files/opensurf.pdf for good notes on the algorithm
         http://www.vision.ee.ethz.ch/~surf/eccv06.pdf reference paper
    */
   class SpeededUpRobustFeatures
   {
      typedef double                     value_type;
      typedef core::Matrix<value_type>   Matrix;
      typedef core::ProbabilityDistributionFunctionGaussian<value_type> GaussianPdf;

      struct LocalPoint
      {
         LocalPoint( value_type a, value_type x, value_type y ) : angle( a ), dx( x ), dy( y )
         {}

         value_type angle;
         value_type dx;
         value_type dy;
      };

   public:
      struct Point
      {
         //typedef core::Buffer1D<value_type> Features;
         typedef std::vector<value_type> Features;
         Point( core::vector2i p, size_t s ) : position( p ), scale( s ), features( (size_t)(4 * NLL_SURF_2D_NB_AREA_PER_FEATURE * NLL_SURF_2D_NB_AREA_PER_FEATURE) ), weight( 1 )
         {}

         Point() : features( (size_t)(4 * NLL_SURF_2D_NB_AREA_PER_FEATURE * NLL_SURF_2D_NB_AREA_PER_FEATURE) )   
         {}

         Features                   features;
         value_type                 orientation;
         core::vector2i             position;
         size_t                       scale;
         float                      weight;        // this will be used to weight the points in the <AffineRegistrationPointBased2d>. By default the algorithm will set it to 1, but a third party could change this value to give more weight to specific points

         void write( std::ostream& o ) const
         {
            core::write<Features>( features, o );
            core::write<value_type>( orientation, o );
            position.write( o );
            core::write<size_t>( scale, o );
            core::write<float>( weight, o );
         }

         void read( std::istream& i )
         {
            core::read<Features>( features, i );
            core::read<value_type>( orientation, i );
            position.read( i );
            core::read<size_t>( scale, i );
            core::read<float>( weight, i );
         }
      };

      typedef std::vector<Point> Points;
      //typedef core::Buffer1D<Point> Points;

      /**
       @brief Expose the <point>'s <features> array as if it was stored as an array only
       */
      class PointsFeatureWrapper
      {
      public:
         typedef std::vector<SpeededUpRobustFeatures::value_type>  value_type;

      public:
         PointsFeatureWrapper( const Points& points ) : _points( points )
         {}

         size_t size() const
         {
            return static_cast<size_t>( _points.size() );
         }

         const std::vector<SpeededUpRobustFeatures::value_type>& operator[]( size_t n ) const
         {
            return _points[ n ].features;
         }

      private:
         PointsFeatureWrapper( const PointsFeatureWrapper& );
         PointsFeatureWrapper& operator=( const PointsFeatureWrapper& );

      private:
         const Points& _points;
      };

      /**
       @brief Construct SURF
       @param octaves the number of octaves to analyse. This increases the range of filters exponentially
       @param intervals the number of intervals per octave This increase the filter linearly
       @param threshold the minimal threshold of the hessian. The lower, the more features (but less robust) will be detected
       */
      SpeededUpRobustFeatures( size_t octaves = 5, size_t intervals = 4, size_t init_step = 2, value_type threshold = 0.00012 ) : _threshold( threshold )
      {
         size_t step = init_step;
         for ( size_t o = 1; o <= octaves; ++o )
         {
            for ( size_t i = 1; i <= intervals; ++i )
            {
               const size_t filterSize = core::round( 3 * ( std::pow( 2.0, (int)o ) * i + 1 ) );
               
               if ( _filterSizes.size() == 0 || *_filterSizes.rbegin() < filterSize )
               {
                  _filterSizes.push_back( filterSize );
                  _filterSteps.push_back( step );
               }
            }
            step *= 2;
         }
      }

      /**
       @param this computes only points of interest (position and scaling) but doesn't compute the features (orientation, feature vector)
       */
      template <class T, class Mapper, class Alloc>
      Points computesPoints( const core::Image<T, Mapper, Alloc>& i )
      {
         std::stringstream ss;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "SURF point detection started..." );
         FastHessianDetPyramid2d pyramid;
         Points points = _computesPoints( i, pyramid );

         ss << "Number of points detected=" << points.size();
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         return points;
      }

      /**
       @brief Compute the SURF features
       */
      template <class T, class Mapper, class Alloc>
      Points computesFeatures( const core::Image<T, Mapper, Alloc>& i )
      {
         std::stringstream ss;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "SURF feature detection started..." );

         FastHessianDetPyramid2d pyramid;
         Points points = _computesPoints( i, pyramid );
         ss << "Number of points detected=" << points.size() << std::endl;

         core::Timer timeOrientation;
         _computeAngle( pyramid.getIntegralImage(), points );
         ss << "Orientation time=" << timeOrientation.getCurrentTime() << std::endl;

         core::Timer timeFeeatures;
         _computeFeatures( pyramid.getIntegralImage(), points );
         ss << "Features time=" << timeFeeatures.getCurrentTime();

         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         return points;
      }

      /**
       @brief Compute the SURF features given a user supplied list of points
       @note features and orientation will be assigned for each point
       */
      template <class T, class Mapper, class Alloc>
      void computesFeatures( const core::Image<T, Mapper, Alloc>& i, Points& points )
      {
         IntegralImage integral;
         integral.process( i );

         _computeAngle( integral, points );
         _computeFeatures( integral, points );
      }

   private:
      template <class T, class Mapper, class Alloc>
      Points _computesPoints( const core::Image<T, Mapper, Alloc>& i, FastHessianDetPyramid2d& pyramid )
      {
         // this constant is to find the gaussian's sigma
         // we know that a filter 9*9 corresponds to a gaussian's sigma = 1.2
         // so for a filter of size X, sigma = 1.2 / 9 * X
         static const value_type scaleFactor = 1.2 / 9;

         size_t nbPoints = 0;

         // each thread can work independently, so allocate an array og points that are not shared
         // between threads
         #ifndef NLL_NOT_MULTITHREADED
            const size_t maxNumberOfThread = omp_get_max_threads();
         #else
            const size_t maxNumberOfThread = 1;
         #endif
         std::vector< std::vector<Point> > bins( maxNumberOfThread );


         core::Timer timePyramid;
         pyramid.construct( i, _filterSizes, _filterSteps );

         {
            std::stringstream ss;
            ss << "Pyramid construction time=" << timePyramid.getCurrentTime();
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         timePyramid.start();

         ensure( pyramid.getPyramidDetHessian().size() > 1, "too small!" );
         const size_t nbFilters = static_cast<size_t>( pyramid.getPyramidDetHessian().size() ) - 1; // we don't want the last filter, it will never be "maximal"
         for ( size_t filter = 1; filter < nbFilters; ++filter )
         {
            const Matrix& f = pyramid.getPyramidDetHessian()[ filter ];
            const int sizex = static_cast<int>( f.sizex() ) - 1;
            const int sizey = static_cast<int>( f.sizey() ) - 1;

            #ifndef NLL_NOT_MULTITHREADED
            # pragma omp parallel for reduction(+ : nbPoints)
            #endif
            for ( int y = 0; y < sizey; ++y )
            {
               for ( int x = 0; x < sizex; ++x )
               {
                  const value_type val = f( y, x );
                  if ( val > _threshold )
                  {
                     bool isMax = pyramid.isDetHessianMax( val, x, y, filter, filter )     &&
                                  pyramid.isDetHessianMax( val, x, y, filter, filter + 1 ) &&
                                  pyramid.isDetHessianMax( val, x, y, filter, filter - 1 );
                     if ( isMax )
                     {
                        
                        core::vector3d hessianGradient = pyramid.getHessianGradient( x, y, filter );
                        
                        Matrix hessianHessian = pyramid.getHessianHessian( x, y, filter );
                        
                        const bool inverted = core::inverse3x3( hessianHessian );
                        core::StaticVector<double, 3> interpolatedPoint = core::mat3Mulv( hessianHessian, hessianGradient ) * -1;
                        if ( inverted && fabs( interpolatedPoint[ 0 ] ) < 0.5 &&
                                         fabs( interpolatedPoint[ 1 ] ) < 0.5 &&
                                         fabs( interpolatedPoint[ 2 ] ) < 0.5 )
                        {
                           const int size = static_cast<int>( _filterSizes[ filter ] );
                           // here we need to compute the step between the two scales (i.e., their difference in size and not the step as for the position)
                           const int filterStep = static_cast<int>( _filterSizes[ filter + 1 ] - _filterSizes[ filter ] );

                           const core::vector2f index = pyramid.getPositionPyramid2Integral( x + (float)interpolatedPoint[ 0 ], y + (float)interpolatedPoint[ 1 ], filter );

                           int px    = core::round( index[ 0 ] );
                           int py    = core::round( index[ 1 ] );
                           int scale = core::round( ( size   + interpolatedPoint[ 2 ]   * filterStep ) * scaleFactor );
                           if ( scale <= 0 )
                              continue;   // should not happen, but just in case!

                           #ifndef NLL_NOT_MULTITHREADED
                           size_t threadId = omp_get_thread_num();
                           #else
                           size_t threadId = 0;
                           #endif
                           bins[ threadId ].push_back( Point( core::vector2i( px, py ), scale ) );

                           ++nbPoints;
                        }
                     }
                  }
               }
            }
         }

         {
            std::stringstream ss;
            ss << "non maximal suppression and localization=" << timePyramid.getCurrentTime();
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         Points points( nbPoints );
         size_t cur = 0;
         for ( size_t bin = 0; bin < bins.size(); ++bin )
         {
            for ( size_t p = 0; p < bins[ bin ].size(); ++p )
            {
               points[ cur++ ] = bins[ bin ][ p ];
            }
         }
         return points;
      }
      
      void _computeFeatures( const IntegralImage& image, Points& points ) const
      {
         const int nbPoints = static_cast<int>( points.size() );
         const value_type area_size = NLL_SURF_2D_NB_AREA_PER_FEATURE_SIZE / NLL_SURF_2D_NB_AREA_PER_FEATURE;
         const value_type area_pos_min = - NLL_SURF_2D_NB_AREA_PER_FEATURE / 2 * area_size;
         const value_type area_pos_max =   NLL_SURF_2D_NB_AREA_PER_FEATURE / 2 * area_size;
         const value_type area_pos_center =   area_pos_min + area_size / 2;
         const value_type dd5x5 = (area_size / NLL_SURF_2D_NB_SUBAREA_PER_AREA);

         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int n = 0; n < nbPoints; ++n )
         {
            Point& point = points[ n ];

            const value_type scale = static_cast<value_type>( point.scale );

            const value_type x = point.position[ 0 ];
            const value_type y = point.position[ 1 ];

            const Rotate rotate( point.orientation );
            const Rotate rotateInv( -point.orientation );

            const int size = (int)core::sqr( 2 * scale );

            Matrix cov1( 2, 2 );
            cov1( 0, 0 ) = core::sqr( 2.5 * scale * 2 );
            cov1( 1, 1 ) = core::sqr( 2.5 * scale * 2  );
            GaussianPdf gauss1( cov1, core::make_buffer1D<value_type>( 0, 0 ) );

            Matrix cov2( 2, 2 );
            cov2( 0, 0 ) = core::sqr( 3.3 * scale * 2  );
            cov2( 1, 1 ) = core::sqr( 3.3 * scale * 2  );
            GaussianPdf gauss2( cov2, core::make_buffer1D<value_type>( 0, 0 ) );

            size_t count = 0;
            value_type len = 0;

            // (i, j) the bottom left corners of the 4x4 area, in the unrotated space
            // (cx, cy) the centers of the 4x4 area, in the unrotated space
            value_type cy = area_pos_center;
            for ( value_type j = area_pos_min; j < area_pos_max; j += area_size, cy += area_size )
            {
               value_type cx = area_pos_center;
               for ( value_type i = area_pos_min; i < area_pos_max; i += area_size, cx += area_size )
               {
                  // the feature for each 4x4 region
                  value_type dx = 0;
                  value_type dy = 0;
                  value_type mdx = 0;
                  value_type mdy = 0;

                  // now compute the 5x5 points for each 4x4 region
                  for ( value_type dj = j; dj < j + area_size; dj += dd5x5 )
                  {
                     for ( value_type di = i; di < i + area_size; di += dd5x5 )
                     {
                        // center on the rotated axis
                        const core::vector2d pointInRtotatedGrid = rotate.transform( core::vector2d( di * scale, dj * scale ) );
                        const core::vector2i center( static_cast<int>( x + pointInRtotatedGrid[ 0 ] ),
                                                     static_cast<int>( y + pointInRtotatedGrid[ 1 ] ) );

                        //Get the gaussian weighted x and y responses
                        const value_type gauss_s1 = gauss1.eval( core::make_buffer1D<value_type>( di - cx, dj - cy ) );

                        core::vector2i bl( static_cast<int>( center[ 0 ] - scale ),
                                           static_cast<int>( center[ 1 ] - scale ) );
                        core::vector2i tr( static_cast<int>( center[ 0 ] + scale ),
                                           static_cast<int>( center[ 1 ] + scale ) );
                        if ( bl[ 0 ] >= 0 && bl[ 1 ] >= 0 &&
                             tr[ 0 ] < static_cast<int>( image.sizex() ) &&
                             tr[ 1 ] < static_cast<int>( image.sizey() ) )
                        {
                           const value_type dry = HaarFeatures2d::getValue( HaarFeatures2d::HORIZONTAL,
                                                                            image,
                                                                            center,
                                                                            static_cast<int>( 2 * scale + 1 ) ) / size;
                           const value_type drx = HaarFeatures2d::getValue( HaarFeatures2d::VERTICAL,
                                                                            image,
                                                                            center,
                                                                            static_cast<int>( 2 * scale + 1 ) ) / size;

                           //Get the gaussian weighted x and y responses on the unrotated axis
                           const core::vector2d rotatedInvFeature = rotateInv.transform( core::vector2d( drx, dry ) );
                           const value_type rrx = gauss_s1 * rotatedInvFeature[ 0 ];
                           const value_type rry = gauss_s1 * rotatedInvFeature[ 1 ];

                           dx += rrx;
                           dy += rry;
                           mdx += fabs( rrx );
                           mdy += fabs( rry );
                        }
                     }
                  }

                  //Add the values to the descriptor vector
                  const value_type gauss_s2 = gauss2.eval( core::make_buffer1D<value_type>( cx, cy ) );
                  point.features[ count++ ] = dx * gauss_s2;
                  point.features[ count++ ] = dy * gauss_s2;
                  point.features[ count++ ] = mdx * gauss_s2;
                  point.features[ count++ ] = mdy * gauss_s2;

                  len += ( dx * dx + dy * dy + mdx * mdx + mdy * mdy ) * gauss_s2 * gauss_s2;
               }
            }

            //Convert to Unit Vector
            len = (sqrt( len ) + 1e-7);
            for( size_t i = 0; i < point.features.size(); ++i )
               point.features[ i ] /= len;
         }
      }

      /**
       @brief assign a repeable orientation for each point       

         The dominant orientation is determined for each SURF 
         key point in order to guarantee the invariance of feature 
         description to image rotatio
       */
      static void _computeAngle( const IntegralImage& i, Points& points )
      {
         const int nbPoints = static_cast<int>( points.size() );

         Matrix cov1( 2, 2 );
         cov1( 0, 0 ) = core::sqr( 2.5 );
         cov1( 1, 1 ) = core::sqr( 2.5 );
         GaussianPdf gaussEval( cov1, core::make_buffer1D<value_type>( 0, 0 ) );

         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int n = 0; n < nbPoints; ++n )
         {
            const Point& point = points[ n ];
            const int scale = static_cast<int>( point.scale );

            std::vector<LocalPoint> localPoints;
            localPoints.reserve( 109 );

            for ( int v = -6; v <= 6; ++v )
            {
               for ( int u = -6; u <= 6; ++u )
               {
                  if ( u * u + v * v < 36 )
                  {
                     // we need to weight the response so that it is more tolerant to the noise. Indeed, the further
                     // away from the centre, the more likely it is to be noisier
                     //const value_type gauss = gaussian( u, v, 2.5 );
                     const value_type gauss = gaussEval.eval( core::make_buffer1D<value_type>( u, v ) );
                     const int x = point.position[ 0 ] + u * scale;
                     const int y = point.position[ 1 ] + v * scale;
                     const core::vector2i center( x, y );

                     const core::vector2i bl( x - 2 * scale, y - 2 * scale );
                     const core::vector2i tr( x + 2 * scale, y + 2 * scale );
                     if ( bl[ 0 ] >= 0 && bl[ 1 ] >= 0 &&
                          tr[ 0 ] < static_cast<int>( i.sizex() ) &&
                          tr[ 1 ] < static_cast<int>( i.sizey() ) )
                     {
                        const value_type dy = gauss * HaarFeatures2d::getValue( HaarFeatures2d::HORIZONTAL,
                                                                                i,
                                                                                center,
                                                                                static_cast<int>( 4 * scale + 1 ) );
                        const value_type dx = gauss * HaarFeatures2d::getValue( HaarFeatures2d::VERTICAL,
                                                                                i,
                                                                                center,
                                                                                static_cast<int>( 4 * scale + 1 ) );
                        const value_type angle = core::getAngle( dx, dy );
                        localPoints.push_back( LocalPoint( angle, dx, dy ) );
                     }
                  }
               }
            }
           
            const int nbLocalPoints = static_cast<int>( localPoints.size() );

            
            //
            // simple averaging seems better than the technique described in the original paper...
            //  
# ifdef NLL_SURF_USE_FEATURE_ANGLE_UPGRADE
            double dx = 0;
            double dy = 0;
            for ( int nn = 0; nn < nbLocalPoints; ++nn )
            {
               dx += localPoints[ nn ].dx;
               dy += localPoints[ nn ].dy;
            }
            if ( nbLocalPoints )
            {
               dx /= nbLocalPoints;
               dy /= nbLocalPoints;
            }
            points[ n ].orientation = core::getAngle( dx, dy );
#  else
            const double step = 0.15;
            const double window = 0.3 * core::PI;
            double best_angle = 0;
            double best_norm = 0;
            for ( double angleMin = 0 ; angleMin < core::PI * 2; angleMin += step )
            {
               double dx = 0;
               double dy = 0;
               size_t nbPoints = 0;
            

               const double angleMax = ( angleMin > 2 * core::PI - window ) ? ( angleMin + window - 2 * core::PI ) : ( angleMin + window );

               for ( int nn = 0; nn < nbLocalPoints; ++nn )
               {
                  const double startAngle = localPoints[ nn ].angle;
                  if ( _isInside( startAngle, angleMin, angleMax ) )
                  {
                     dx += localPoints[ nn ].dx;
                     dy += localPoints[ nn ].dy;
                     ++nbPoints;
                  }
               }

               // finally compute the mean direction and save the best one
               if ( nbPoints )
               {
                  dx /= nbPoints;
                  dy /= nbPoints;
               }
               const double norm = dx * dx + dy * dy;
               if ( norm > best_norm )
               {
                  best_norm = norm;
                  best_angle = core::getAngle( dx, dy );
               }
            }

            // assign the angle
            if ( best_norm > 0 )
            {
               points[ n ].orientation = best_angle;
            }
# endif
         }
      }

      static bool _isInside( double angle, double min, double max )
      {
         if ( max < min )
         {
            // means we are at the angle disconuity point
            if ( angle < core::PI )
            {
               // the angle is 0-PI, so it means min is in PI-2PI
               min -= 2 * core::PI;
               return angle <= max && angle >= min;
            } else {
               max += 2 * core::PI;
               return angle <= max && angle >= min;
            }
         } else {
            // default case
            return angle <= max && angle >= min;
         }
      }

      class Rotate : public core::NonCopyable
      {
      public:
         Rotate( double angleRad ) : _cosx( std::cos( angleRad ) ), _sinx( std::sin( angleRad ) )
         {}

         core::vector2d transform( const core::vector2d& p ) const
         {
            return core::vector2d( _cosx * p[ 0 ] - _sinx * p[ 1 ],
                                   _sinx * p[ 0 ] + _cosx * p[ 1 ] );
         }

      private:
         const double   _cosx;
         const double   _sinx;
      };

   private:
      std::vector<size_t> _filterSizes;
      std::vector<size_t> _filterSteps;
      value_type        _threshold;
   };
}
}

#endif