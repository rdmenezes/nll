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

#ifndef NLL_ALGORITHM_SURF_2D_H_
# define NLL_ALGORITHM_SURF_2D_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Implementation of the Speeded Up Robust Features or SURF algorithm

           Feature detector which intend to be robust to scaling, illumination and rotation

    @see http://sites.google.com/site/chrisevansdev/files/opensurf.pdf for good notes on the algorithm
         http://www.vision.ee.ethz.ch/~surf/eccv06.pdf reference paper
    */
   class SpeededUpRobustFeatures
   {
      typedef double                     value_type;
      typedef core::Matrix<value_type>   Matrix;

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
         typedef core::Buffer1D<value_type> Features;
         Point( core::vector2i p, ui32 s ) : position( p ), scale( s ), features( 64 )
         {}

         Features                   features;
         value_type                 orientation;
         core::vector2i             position;
         ui32                       scale;
      };

      typedef core::Buffer1D<Point> Points;

      /**
       @brief Expose the <point>'s <features> array as if it was stored as an array only
       */
      class PointsFeatureWrapper
      {
      public:
         typedef core::Buffer1D<SpeededUpRobustFeatures::value_type>  value_type;

      public:
         PointsFeatureWrapper( Points& points ) : _points( points )
         {}

         ui32 size() const
         {
            return _points.size();
         }

         core::Buffer1D<SpeededUpRobustFeatures::value_type>& operator[]( ui32 n )
         {
            return _points[ n ].features;
         }

         const core::Buffer1D<SpeededUpRobustFeatures::value_type>& operator[]( ui32 n ) const
         {
            return _points[ n ].features;
         }

      private:
         PointsFeatureWrapper( const PointsFeatureWrapper& );
         PointsFeatureWrapper& operator=( const PointsFeatureWrapper& );

      private:
         Points& _points;
      };

      /**
       @brief Construct SURF
       @param octaves the number of octaves to analyse. This increases the range of filters exponentially
       @param intervals the number of intervals per octave This increase the filter linearly
       @param threshold the minimal threshold of the hessian. The lower, the more features (but less robust) will be detected
       */
      SpeededUpRobustFeatures( ui32 octaves = 5, ui32 intervals = 4, ui32 init_step = 2, value_type threshold = 0.00012 ) : _threshold( threshold )
      {
         ui32 step = init_step;
         for ( ui32 o = 1; o <= octaves; ++o )
         {
            for ( ui32 i = 1; i <= intervals; ++i )
            {
               const ui32 filterSize = core::round( 3 * ( std::pow( 2.0, (int)o ) * i + 1 ) );
               
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
         FastHessianDetPyramid2D pyramid;
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

         FastHessianDetPyramid2D pyramid;
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
       */
      template <class T, class Mapper, class Alloc>
      Points computesFeatures( const core::Image<T, Mapper, Alloc>& i, const Points& points )
      {
         IntegralImage integral;
         integral.process( i );

         _computeAngle( integral, points );
         _computeFeatures( pyramid.getIntegralImage(), points );
         return points;
      }

   private:
      template <class T, class Mapper, class Alloc>
      Points _computesPoints( const core::Image<T, Mapper, Alloc>& i, FastHessianDetPyramid2D& pyramid )
      {
         ui32 nbPoints = 0;

         // each thread can work independently, so allocate an array og points that are not shared
         // between threads
         #ifndef NLL_NOT_MULTITHREADED
            const ui32 maxNumberOfThread = omp_get_max_threads();
         #else
            const ui32 maxNumberOfThread = 1;
         #endif
         std::vector< std::vector<Point> > bins( maxNumberOfThread );


         core::Timer timePyramid;
         pyramid.construct( i, _filterSizes, _filterSteps );

         {
            std::stringstream ss;
            ss << "Pyramid construction time=" << timePyramid.getCurrentTime();
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         for ( ui32 filter = 1; filter < _filterSizes.size() - 1 ; ++filter )
         {
            if ( pyramid.getPyramidDetHessian().size() <= filter )
               break; // the filter was not used in the pyramid...
            const Matrix& f = pyramid.getPyramidDetHessian()[ filter ];
            const int sizex = static_cast<int>( f.sizex() );
            const int sizey = static_cast<int>( f.sizey() );

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
                        core::vector3d interpolatedPoint = core::mat3Mulv( hessianHessian, hessianGradient );
                        if ( inverted && interpolatedPoint[ 0 ] < 0.5 &&
                                         interpolatedPoint[ 1 ] < 0.5 &&
                                         interpolatedPoint[ 2 ] < 0.5 )
                        {
                           const int size = _filterSizes[ filter ];
                           const int half = size / 2;
                           // here we need to compute the step between the two scales (i.e., their difference in size and not the step as for the position)
                           const int filterStep = static_cast<int>( _filterSizes[ filter + 1 ] - _filterSizes[ filter ] );

                           int px    = core::round( ( x    - interpolatedPoint[ 0 ] ) * _filterSteps[ filter ] + half );
                           int py    = core::round( ( y    - interpolatedPoint[ 1 ] ) * _filterSteps[ filter ] + half );
                           int scale = core::round( size   - interpolatedPoint[ 2 ]   * filterStep );

                           ui32 threadId = omp_get_thread_num();
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
         ui32 cur = 0;
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
         int nbPoints = static_cast<ui32>( points.size() );

         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int n = 0; n < nbPoints; ++n )
         {
            int y, x, sample_x, sample_y, count=0;
            int i = 0, ix = 0, j = 0, jx = 0, xs = 0, ys = 0;
            value_type dx, dy, mdx, mdy, co, si;
            value_type gauss_s1 = 0, gauss_s2 = 0;
            value_type rrx = 0, rry = 0, len = 0;
            value_type cx = -0.5, cy = 0; //Subregion centers for the 4x4 gaussian weighting
            Point& point = points[ n ];

            // this constant is to find the gaussian's sigma
            // we know that a filter 9*9 corresponds to a gaussian's sigma = 1.2
            // so for a filter of size X, sigma = 1.2 / 9 * X
            static const value_type scaleFactor = 1.2 / 9;
            float scale = (float)core::round( scaleFactor * point.scale );

            x = core::round( point.position[ 0 ] );
            y = core::round( point.position[ 1 ] );

            co = (float)cos( point.orientation);
            si = (float)sin( point.orientation);

            i = -8;

            //Calculate descriptor for this interest point
            while( i < 12 )
            {
               j = -8;
               i = i-4;

               cx += 1.f;
               cy = -0.5f;

               while( j < 12 ) 
               {
                  dx = dy = mdx = mdy = 0;
                  cy += 1;

                  j = j - 4;

                  ix = i + 5;
                  jx = j + 5;

                  xs = core::round( x + ( -jx * scale * si + ix * scale * co ) );
                  ys = core::round( y + (  jx * scale * co + ix * scale * si ) );

                  for ( int k = i; k < i + 9; ++k ) 
                  {
                     for ( int l = j; l < j + 9; ++l ) 
                     {
                        //Get coords of sample point on the rotated axis
                        sample_x = core::round( x + ( -l * scale * si + k * scale * co ) );
                        sample_y = core::round( y + (  l * scale * co + k * scale * si ) );

                        //Get the gaussian weighted x and y responses
                        gauss_s1 = gaussian( xs - sample_x, ys - sample_y, 2.5 * scale );

                        core::vector2ui bl( core::round( sample_x - scale ),
                                            core::round( sample_y - scale ) );
                        core::vector2ui tr( core::round( sample_x + scale ),
                                            core::round( sample_y + scale ) );
                        int size = ( tr[ 0 ] - bl[ 0 ] ) * ( tr[ 1 ] - bl[ 1 ] );

                        if ( bl[ 0 ] >= 0 && bl[ 1 ] >= 0 && tr[ 0 ] < image.sizex() && tr[ 1 ] < image.sizey() )
                        {
                           const value_type ry = HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::VERTICAL,
                                                                                    image,
                                                                                    bl,
                                                                                    tr ) / size;
                           const value_type rx = HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::HORIZONTAL,
                                                                                    image,
                                                                                    bl,
                                                                                    tr ) / size;

                           //Get the gaussian weighted x and y responses on rotated axis
                           rrx = gauss_s1 * ( -rx * si + ry * co );
                           rry = gauss_s1 * (  rx * co + ry * si );

                           dx += rrx;
                           dy += rry;
                           mdx += fabs( rrx );
                           mdy += fabs( rry );
                        }
                     }
                  }

                  //Add the values to the descriptor vector
                  gauss_s2 = gaussian( (int)cx - 2, (int)cy - 2, 1.5 );

                  point.features[ count++ ] = dx * gauss_s2;
                  point.features[ count++ ] = dy * gauss_s2;
                  point.features[ count++ ] = mdx * gauss_s2;
                  point.features[ count++ ] = mdy * gauss_s2;

                  len += ( dx * dx + dy * dy + mdx * mdx + mdy * mdy ) * gauss_s2 * gauss_s2;

                  j += 9;
               }
               i += 9;
            }

            //Convert to Unit Vector
            len = sqrt( len ) + 1e-7;
            for( ui32 i = 0; i < point.features.size(); ++i )
               point.features[ i ] /= len;
         }
      }

      static value_type gaussian(int x, int y, value_type sig)
      {
         return ( 1.0 / ( 2.0 * core::PI * sig * sig ) ) * std::exp( -( x * x + y * y ) / ( 2.0 * sig * sig ) );
      }

      /**
       @brief assign a repeable orientation for each point       

         The dominant orientation is determined for each SURF 
         key point in order to guarantee the invariance of feature 
         description to image rotatio
       */
      static void _computeAngle( const IntegralImage& i, Points& points )
      {
         // preprocessed gaussian of size 2.5
         static const value_type gauss25 [7][7] = {
           0.02350693969273, 0.01849121369071, 0.01239503121241, 0.00708015417522, 0.00344628101733, 0.00142945847484, 0.00050524879060,
           0.02169964028389, 0.01706954162243, 0.01144205592615, 0.00653580605408, 0.00318131834134, 0.00131955648461, 0.00046640341759,
           0.01706954162243, 0.01342737701584, 0.00900063997939, 0.00514124713667, 0.00250251364222, 0.00103799989504, 0.00036688592278,
           0.01144205592615, 0.00900063997939, 0.00603330940534, 0.00344628101733, 0.00167748505986, 0.00069579213743, 0.00024593098864,
           0.00653580605408, 0.00514124713667, 0.00344628101733, 0.00196854695367, 0.00095819467066, 0.00039744277546, 0.00014047800980,
           0.00318131834134, 0.00250251364222, 0.00167748505986, 0.00095819467066, 0.00046640341759, 0.00019345616757, 0.00006837798818,
           0.00131955648461, 0.00103799989504, 0.00069579213743, 0.00039744277546, 0.00019345616757, 0.00008024231247, 0.00002836202103
         };
         static const int id[] = { 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6 };

         // this constant is to find the gaussian's sigma
         // we know that a filter 9*9 corresponds to a gaussian's sigma = 1.2
         // so for a filter of size X, sigma = 1.2 / 9 * X
         static const value_type scaleFactor = 1.2 / 9;
         const int nbPoints = static_cast<int>( points.size() );

         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int n = 0; n < nbPoints; ++n )
         {
            const Point& point = points[ n ];
            const int scale = core::round( scaleFactor * point.scale );

            std::vector<LocalPoint> localPoints;
            localPoints.reserve( 109 );

            for ( int v = -6; v <= 6; ++v )
            {
               for ( int u = -6; u <= 6; ++u )
               {
                  if ( u * u + v * v < 36 )
                  {
                     const value_type gauss = gauss25[ id[ u + 6 ] ][ id[ v + 6 ] ];
                     const int x = point.position[ 0 ] + u * scale;
                     const int y = point.position[ 1 ] + v * scale;
                     const core::vector2ui bl( x - 2 * scale, y - 2 * scale );
                     const core::vector2ui tr( x + 2 * scale, y + 2 * scale );
                     if ( bl[ 0 ] >= 0 && bl[ 1 ] >= 0 && tr[ 0 ] < i.sizex() && tr[ 1 ] < i.sizey() )
                     {
                        const value_type dy = - gauss * HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::VERTICAL,
                                                                                           i,
                                                                                           bl,
                                                                                           tr );
                        const value_type dx = - gauss * HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::HORIZONTAL,
                                                                                           i,
                                                                                           bl,
                                                                                           tr );
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

   private:
      std::vector<ui32> _filterSizes;
      std::vector<ui32> _filterSteps;
      value_type _threshold;
   };
}
}

#endif