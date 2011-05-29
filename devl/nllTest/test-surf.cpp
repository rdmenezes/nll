#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

//#define NLL_ALGORITHM_SURF_NO_OPENMP

namespace nll
{
namespace algorithm
{
   namespace impl
   {
      /**
       @brief Returns the angle
       */
      template <class T>
      T getAngle( T x, T y )
      {
        if( x > 0 && y >= 0 )
          return atan( y / x );

        if( x < 0 && y >= 0)
           return core::PI - atan( - y / x );

        if( x < 0 && y < 0)
           return core::PI + atan( y / x );

        if( x > 0 && y < 0 )
           return 2 * core::PI - atan( -y / x );

        return 0;
      }

      /**
       @brief Returns the direction of the closest (left or right) to reach the other angle
       */
      int getDirectionToClosest( double angle1, double angle2 )
      {
         if ( fabs( angle1 - angle2 ) > core::PI )
         {
            return angle1 > angle2 ? 1 : -1;
         } else {
            return angle1 > angle2 ? -1 : 1;
         }
      }
   }

   /**
    @brief Hold a stack of the hessian determinant using a crude approximation of a gaussian in 2D
           for each point, H(x, o) =| Lxx Lxy |
                                    | Lxy Lyy |
                           det(H)= Lxx*Lyy-(0.9 * Lxy)^2
           note the normalization factor 0.9 comes from the gaussian approximation
    */
   class FastHessianDetPyramid2D
   {
   public:
      typedef double                   value_type;
      typedef core::Matrix<value_type> Matrix;

   public:
      /**
       @brief Construct and computes the hessian determinant pyramid
       @param i the image,
       @param scales the size of each level of the pyramid (in pixel), must be in increasing order
       @param displacements the step between two filter evaluation for this particular level
       */
      template <class T, class Mapper, class Alloc>
      void construct( const core::Image<T, Mapper, Alloc>& i, const std::vector<ui32>& scales, const std::vector<ui32>& displacements )
      {
         ensure( displacements.size() == scales.size(), "must be the same size" );

         _pyramidDetHessian.clear();
         _pyramidLaplacian.clear();
         _scales = scales;
         _displacements = displacements;

         const T max = (T)std::max( abs( *std::max_element( i.begin(), i.end() ) ),
                                    abs( *std::min_element( i.begin(), i.end() ) ) );

         // construct an integral image
         IntegralImage image;
         image.process( i );

         _integralImage = image;

          
         // build each level by redimensioning the approximated gaussian derivatives
         ui32 lastScale = 0;

         for ( size_t n = 0; n < scales.size(); ++n )
         {
            ensure( scales[ n ] % 2 == 1, "scales must be odd numbers" );
            ensure( scales[ n ] >= 9, "minimal size" );
            //ensure( lastScale < scales[ n ], "scales must be in increasing order" );

            lastScale = scales[ n ];
            const ui32 step = displacements[ n ];

            const int sizeFilterx = scales[ n ];
            const int sizeFiltery = scales[ n ];
            const double sizeFilter = sizeFilterx * sizeFiltery * max; // we normalize by the filter size and maximum value

            const int halfx = sizeFilterx / 2;
            const int halfy = sizeFilterx / 2;

            // the total size must take into account the step size and filter size (it must be fully inside the image to be computed)
            const int resx = ( (int)i.sizex() - 2 * halfx + 1 ) / (int)step;
            const int resy = ( (int)i.sizey() - 2 * halfy + 1 ) / (int)step;

            if ( resx <= 0 || resy <= 0 )
               break;   // the scale is too big!
            Matrix detHessian( resy, resx );
            Matrix laplacian( resy, resx );

            // compute the hessian
            #ifndef NLL_ALGORITHM_SURF_NO_OPENMP
            # pragma omp parallel for
            #endif
            for ( int y = 0; y < resy; ++y )
            {
               for ( int x = 0; x < resx; ++x )
               {
                  core::vector2ui bl( x * step, y * step );
                  core::vector2ui tr( bl[ 0 ] + sizeFilterx - 1, bl[ 1 ] + sizeFiltery - 1 );
                  if ( tr[ 0 ] < image.sizex() && tr[ 1 ] < image.sizey() )
                  {
                     const double dxx = HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::VERTICAL_TRIPLE,
                                                                           image,
                                                                           bl,
                                                                           tr ) / sizeFilter;
                     const double dyy = HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::HORIZONTAL_TRIPLE,
                                                                           image,
                                                                           bl,
                                                                           tr ) / sizeFilter;
                     const double dxy = HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::CHECKER,
                                                                           image,
                                                                           bl,
                                                                           tr ) / sizeFilter;
                     detHessian( y, x ) = dxx * dyy - core::sqr( 0.9 * dxy );
                     laplacian( y, x ) = dxx + dyy;
                  } else {
                     detHessian( y, x ) = 0;
                     laplacian( y, x ) = 0;
                  }
               }
            }

            _pyramidDetHessian.push_back( detHessian );
            _pyramidLaplacian.push_back( laplacian );
         }
      }

      /**
       @brief Computes the gradient of the hessian at position (x, y, map)
              using finite difference
       */
      core::vector3d getHessianGradient( ui32 x, ui32 y, ui32 map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < _pyramidDetHessian[ map ].sizex() - 1 &&
                 y < _pyramidDetHessian[ map ].sizey() - 1 );

         int xminus, yminus;
         indexInMap( x, y, map, map - 1, xminus, yminus );  // we need to look up the closed index in a map that has different dimensions

         int xplus, yplus;
         indexInMap( x, y, map, map + 1, xplus, yplus ); // we need to look up the closed index in a map that has different dimensions

         const Matrix& current = _pyramidDetHessian[ map ];
         return core::vector3d( ( current( y, x + 1 ) - current( y, x - 1 ) ) / 2,
                                ( current( y + 1, x ) - current( y - 1, x ) ) / 2,
                                ( _pyramidDetHessian[ map + 1 ]( yplus, xplus ) -
                                  _pyramidDetHessian[ map - 1 ]( yminus, xminus ) ) / 2 );
      }

      /**
       @brief Computes the hessian of the hessian at position (x, y, map)
              using finite difference
       */
      Matrix getHessianHessian( ui32 x, ui32 y, ui32 map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < _pyramidDetHessian[ map ].sizex() &&
                 y < _pyramidDetHessian[ map ].sizey() );

         const Matrix& mc = _pyramidDetHessian[ map ];
         const Matrix& mm = _pyramidDetHessian[ map - 1 ];
         const Matrix& mp = _pyramidDetHessian[ map + 1 ];
         const value_type val = mc( y, x );

         int xm, ym;
         indexInMap( x, y, map, map - 1, xm, ym );  // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xm > 0 && ym > 0 &&
                 xm < (int)_pyramidDetHessian[ map - 1 ].sizex() - 1 &&
                 ym < (int)_pyramidDetHessian[ map - 1 ].sizey() - 1 );

         int xp, yp;
         indexInMap( x, y, map, map + 1, xp, yp ); // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xp > 0 && ym > 0 &&
                 xp < (int)_pyramidDetHessian[ map + 1 ].sizex() - 1 &&
                 yp < (int)_pyramidDetHessian[ map + 1 ].sizey() - 1 );

         const value_type dxx = mc( y, x + 1 ) + mc( y, x - 1 ) - 2 * val;
         const value_type dyy = mc( y + 1, x ) + mc( y - 1, x ) - 2 * val;
         const value_type dss = mp( yp, xp )   + mm( ym, xm )   - 2 * val;
         const value_type dxy = ( mc( y + 1, x + 1 ) + mc( y - 1, x - 1 ) -
                              mc( y - 1, x + 1 ) - mc( y + 1, x - 1 ) ) / 4;
         const value_type dxs = ( mp( yp, xp + 1 ) + mm( ym, xm - 1 ) -
                              mm( ym, xm + 1 ) - mp( yp, xp - 1 ) ) / 4;
         const value_type dys = ( mp( yp + 1, xp ) + mm( ym - 1, xm ) -
                              mm( ym + 1, xm ) - mp( yp - 1, xp ) ) / 4;

         Matrix hs( 3, 3 );
         hs( 0, 0 ) = dxx;
         hs( 0, 1 ) = dxy;
         hs( 0, 2 ) = dxs;

         hs( 1, 0 ) = dxy;
         hs( 1, 1 ) = dyy;
         hs( 1, 2 ) = dys;

         hs( 2, 0 ) = dxs;
         hs( 2, 1 ) = dys;
         hs( 2, 2 ) = dss;

         return hs;
      }

      // computes the index in mapDest the closest from (xRef, yRef, mapDest)
      void indexInMap( ui32 xRef, ui32 yRef, ui32 mapRef, ui32 mapDest, int& outx, int& outy ) const
      {
         if ( mapRef == mapDest )
         {
            outx = xRef;
            outy = yRef;
         } else {
            // map a point at a given scale to the image space
            const int half = _scales[ mapRef ] / 2;
            const int x = xRef * _displacements[ mapRef ] + half;
            const int y = yRef * _displacements[ mapRef ] + half;

            // convert the image space coordinate to the other scale space
            const int halfd = _scales[ mapDest ] / 2;
            outx = ( x - halfd ) / (int)_displacements[ mapDest ];
            outy = ( y - halfd ) / (int)_displacements[ mapDest ];
         }
      }

      /**
       @brief returns true if all value around the projection (xRef, yRef, mapRef) on mapDest are smaller
       */
      bool isDetHessianMax( value_type val, ui32 xRef, ui32 yRef, ui32 mapRef, ui32 mapDest ) const
      {
         int x, y;

         // if it is outside, then skip it
         indexInMap( xRef, yRef, mapRef, mapDest, x, y );
         const Matrix& m = _pyramidDetHessian[ mapDest ];
         if ( x < 1 || y < 1 || x + 1 >= (int)m.sizex() || y + 1 >= (int)m.sizey() )
            return false;

         return val >= m( y + 0, x + 0 ) &&
                val >= m( y + 1, x + 0 ) &&
                val >= m( y - 1, x + 0 ) &&
                val >= m( y + 0, x + 1 ) &&
                val >= m( y + 1, x + 1 ) &&
                val >= m( y - 1, x + 1 ) &&
                val >= m( y + 0, x - 1 ) &&
                val >= m( y + 1, x - 1 ) &&
                val >= m( y - 1, x - 1 );
      }

      const std::vector<Matrix>& getPyramidDetHessian() const
      {
         return _pyramidDetHessian;
      }

      const std::vector<Matrix>& getPyramidLaplacianSign() const
      {
         return _pyramidLaplacian;
      }

      const IntegralImage& getIntegralImage() const
      {
         return _integralImage;
      }

   private:
      std::vector<Matrix>  _pyramidDetHessian;
      std::vector<Matrix>  _pyramidLaplacian;
      std::vector<ui32>    _scales;
      std::vector<ui32>    _displacements;
      IntegralImage        _integralImage;
   };

   /**
    @brief Implementation of the Speeded Up Robust Features or SURF algorithm

    @see http://sites.google.com/site/chrisevansdev/files/opensurf.pdf for good notes on the algorithm
         http://www.vision.ee.ethz.ch/~surf/eccv06.pdf reference paper
    */
   class SpeededUpRobustFeatures
   {
      typedef double                     value_type;
      typedef core::Matrix<value_type>   Matrix;

   public:
      struct Point
      {
         Point( core::vector2i p, ui32 s ) : position( p ), scale( s ), features( 4 )
         {}

         core::Buffer1D<value_type> features;
         value_type                 orientation;
         core::vector2i             position;
         ui32                       scale;
      };

      typedef core::Buffer1D<Point> Points;

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
         ui32 nbPoints = 0;

         // each thread can work independently, so allocate an array og points that are not shared
         // between threads
         #ifndef NLL_ALGORITHM_SURF_NO_OPENMP
            const ui32 maxNumberOfThread = omp_get_max_threads();
         #else
            const ui32 maxNumberOfThread = 1;
         #endif
         std::vector< std::vector<Point> > bins( maxNumberOfThread );


         core::Timer timePyramid;
         FastHessianDetPyramid2D pyramid;
         pyramid.construct( i, _filterSizes, _filterSteps );
         std::cout << "Pyramid=" << timePyramid.getCurrentTime() << std::endl;

         for ( ui32 filter = 1; filter < _filterSizes.size() - 1 ; ++filter )
         {
            if ( pyramid.getPyramidDetHessian().size() <= filter )
               break; // the filter was not used in the pyramid...
            const Matrix& f = pyramid.getPyramidDetHessian()[ filter ];
            const int sizex = static_cast<int>( f.sizex() );
            const int sizey = static_cast<int>( f.sizey() );

            #ifndef NLL_ALGORITHM_SURF_NO_OPENMP
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

         Points points( nbPoints );
         ui32 cur = 0;
         for ( size_t bin = 0; bin < bins.size(); ++bin )
         {
            for ( size_t p = 0; p < bins[ bin ].size(); ++p )
            {
               points[ cur++ ] = bins[ bin ][ p ];
            }
         }

         core::Timer timeOrientation;
         _computeAngle( pyramid.getIntegralImage(), points );
         std::cout << "Orientation time=" << timeOrientation.getCurrentTime() << std::endl;

         core::Timer timeFeeatures;
         _computeFeatures( pyramid.getIntegralImage(), points );
         std::cout << "Features time=" << timeFeeatures.getCurrentTime() << std::endl;

         return points;
      }

      template <class T, class Mapper, class Alloc>
      Points computesFeatures( const core::Image<T, Mapper, Alloc>& i )
      {
         Points points = computesPoints( i );
         return points;
      }

   private:
      void _computeFeatures( const IntegralImage& i, Points& points ) const
      {
         int nbPoints = static_cast<ui32>( points.size() );

         #ifndef NLL_ALGORITHM_SURF_NO_OPENMP
         # pragma omp parallel for
         #endif
         for ( int n = 0; n < nbPoints; ++n )
         {
            Point& point = points[ n ];

            // this constant is to find the gaussian's sigma
            // we know that a filter 9*9 corresponds to a gaussian's sigma = 1.2
            // so for a filter of size X, sigma = 1.2 / 9 * X
            static const value_type scaleFactor = 1.2 / 9;
            /*const*/ float scale = static_cast<float>( scaleFactor * point.scale );

            // features for this point : mean(dx) mean(dy) mean(|dx|) and mean(|dy|)
            value_type dxm = 0;
            value_type dym = 0;
            value_type adxm = 0;
            value_type adym = 0;

            //point.orientation = 0; // TODO REMOVE
            //scale  = 2; // TODO REMOVE

            //std::cout << "pos=" << point.position << std::endl;
            //std::cout << "scale=" << scale << std::endl;
            //std::cout << "angle=" << point.orientation << std::endl;

            // computes the rotation factor according to the feature orientation
            const core::TransformationRotation rotation( (float)point.orientation, core::vector2f( 0, 0 ) );
            const core::TransformationRotation rotationInv( (float)-point.orientation, core::vector2f( 0, 0 ) );

            // the haar feature must be of size 2 * scale, but we the point (x, y) is not centered
            const float haarHalfDistMin = scale / 2;
            for ( int r = -10; r < 10; r += 5 )
            {
               for ( int c = -10; c < 10; c += 5 )
               {
                  for ( int y = r; y < r + 5; ++y )
                  {
                     for ( int x = c; x < c + 5; ++x )
                     {
                        // rotated point according to the descriptor
                        const value_type gauss = gaussian( x, y, 3.3 ); // TODO check this variance
                        core::vector2f pr = rotation( core::vector2f( x * scale, y * scale ) );

                        //std::cout << "x,y=" << x << y << std::endl;
                        //std::cout << "rotated scaled x, y=" << pr << std::endl;

                        pr[ 0 ] += point.position[ 0 ];
                        pr[ 1 ] += point.position[ 1 ];

                        // computes the coordinates
                        core::vector2ui bl( core::round( pr[ 0 ] - haarHalfDistMin ), core::round( pr[ 1 ] - haarHalfDistMin ) );
                        core::vector2ui tr( bl[ 0 ] + 2 * scale, bl[ 1 ] + 2 * scale);

                        //std::cout << "corner=" << std::endl << bl << tr << std::endl;
                        if ( bl[ 0 ] < i.sizex() && bl[ 1 ] < i.sizey() &&
                             tr[ 0 ] < i.sizex() && tr[ 1 ] < i.sizey() )
                        {
                           // compute the derivatives
                           const value_type dy = - gauss * HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::VERTICAL,
                                                                                              i,
                                                                                              bl,
                                                                                              tr );
                           const value_type dx = - gauss * HaarFeatures2d::Feature::getValue( HaarFeatures2d::Feature::HORIZONTAL,
                                                                                              i,
                                                                                              bl,
                                                                                              tr );

                           // rotate the features to have the correct alignment
                           core::vector2f vect = rotationInv( core::vector2f( (float)dx, (float)dy ) ); // TODO check inv or not?

                           dxm  += vect[ 0 ];
                           dym  += vect[ 1 ];
                           adxm += fabs( vect[ 0 ] );
                           adym += fabs( vect[ 1 ] );
                        }
                     }
                  }
               }
            }

            /*
            if ( nbPoints )
            {
               dxm /= nbPoints;
               dym /= nbPoints;
            }*/

            value_type norm = sqrt( dxm  * dxm  +
                                    dym  * dym  +
                                    adxm * adxm +
                                    adym * adym ) + 1e-7;

            point.features[ 0 ] = dxm  / norm;
            point.features[ 1 ] = dym  / norm;
            point.features[ 2 ] = adxm / norm;
            point.features[ 3 ] = adym / norm;
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
         struct LocalPoint
         {
            LocalPoint( value_type a, value_type x, value_type y ) : angle( a ), dx( x ), dy( y )
            {}

            value_type angle;
            value_type dx;
            value_type dy;
         };

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

         #ifndef NLL_ALGORITHM_SURF_NO_OPENMP
         # pragma omp parallel for
         #endif
         for ( int n = 0; n < nbPoints; ++n )
         {
            const Point& point = points[ n ];
            const int scale = core::round( scaleFactor * point.scale );
            //std::cout << "scale=" << scale << std::endl;

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
                        const value_type angle = impl::getAngle( dx, dy );
                        localPoints.push_back( LocalPoint( angle, dx, dy ) );
                     }
                  }
               }
            }
           
            const int nbLocalPoints = static_cast<int>( localPoints.size() );

            
            //
            // TODO check: simple averaging seems better...
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
            points[ n ].orientation = impl::getAngle( dx, dy );
            
            /*
            //
            // TODO check: implementation following the paper... but seems not as good as the simple one!!
            //
            const double step = 0.15;
            const double window = 0.3 * core::PI;
            double best_angle = 0;
            double best_norm = 0;
            for ( double angleMin = 0 ; angleMin < core::PI * 2; angleMin += step )
            {
               double dx = 0;
               double dy = 0;
               ui32 nbPoints = 0;
            

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
                  best_angle = impl::getAngle( dx, dy );
               }
            }

            // assign the angle
            if ( best_norm > 0 )
            {
               points[ n ].orientation = best_angle;
            }*/
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

class TestSurf
{
public:

   void printPoints( core::Image<ui8>& output, algorithm::SpeededUpRobustFeatures::Points& points )
   {
      for ( ui32 n = 0; n < points.size(); ++n )
      {
         ui32 px = points[ n ].position[ 0 ];
         ui32 py = points[ n ].position[ 1 ];
         ui32 scale = points[ n ].scale;
         ui32 half = scale / 2;

         int dx = (int)(cos( points[ n ].orientation ) * half);
         int dy = (int)(sin( points[ n ].orientation ) * half);
         if ( px > 5 &&
              py > 5 &&
              px + dx < output.sizex() - 1 &&
              py + dy < output.sizey() - 1 &&
              px + dx > 0 &&
              py + dy > 0 )
         {
            core::bresham( output, core::vector2i( px + 5, py ), core::vector2i( px - 5, py ),    core::vector3uc(255, 255, 255) );
            core::bresham( output, core::vector2i( px, py - 5 ), core::vector2i( px, py + 5 ),    core::vector3uc(255, 255, 255) );
            core::bresham( output, core::vector2i( px, py ),     core::vector2i( px + dx, py + dy), core::vector3uc(0, 0, 255) );
         }
      }
   }

   /**
    @brief the similarity can be greater than 1 if the points are clustered with a distance < tolPixel
    */
   template <class Transformation>
   std::vector< std::pair<ui32, ui32> > getRepeatablePointPosition( const algorithm::SpeededUpRobustFeatures::Points& p1,
                                                                    const algorithm::SpeededUpRobustFeatures::Points& p2,
                                                                    const Transformation& tfm,
                                                                    double tolPixel )
   {
      std::vector< std::pair<ui32, ui32> > index;
      const double tolPixel2 = tolPixel * tolPixel;

      for ( ui32 n1 = 0; n1 < (ui32)p1.size(); ++n1 )
      {
         for ( ui32 n2 = 0; n2 < (ui32)p2.size(); ++n2 )
         {
            const algorithm::SpeededUpRobustFeatures::Point& t1 = p1[ n1 ];
            const algorithm::SpeededUpRobustFeatures::Point& t2 = p2[ n2 ];

            core::vector2f p = tfm( core::vector2f( (f32)t2.position[ 0 ], (f32)t2.position[ 1 ] ) );

            float dx = p[ 0 ] - t1.position[ 0 ];
            float dy = p[ 1 ] - t1.position[ 1 ];
            double d = dx * dx + dy * dy;
            if ( d < tolPixel2 )
            {
               index.push_back( std::make_pair( n1, n2 ) );
               break;
            }
         }
      }

      return index;
   }

   template <class Transformation>
   std::vector< ui32 > getRepeatablePointOrientation( const std::vector< std::pair<ui32, ui32> >& match,
                                                      const algorithm::SpeededUpRobustFeatures::Points& p1,
                                                      const algorithm::SpeededUpRobustFeatures::Points& p2,
                                                      const Transformation& tfm,
                                                      double tol )
   {

      std::vector< ui32 > matchOrientation;
      for ( ui32 n = 0; n < (ui32)match.size(); ++n )
      {
         const algorithm::SpeededUpRobustFeatures::Point& t1 = p1[ match[ n ].first ];
         const algorithm::SpeededUpRobustFeatures::Point& t2 = p2[ match[ n ].second ];

         const ui32 id2 = match[ n ].second;
         core::vector2f p( (f32)( 1000 * cos( t2.orientation ) ), 
                           (f32)( 1000 * sin( t2.orientation ) ) );
         core::vector2f pTfm = tfm( p );
         const double a1 = algorithm::impl::getAngle(  pTfm[ 0 ], pTfm[ 1 ] );
         if ( fabs( a1 - t1.orientation ) < tol )
         {
            matchOrientation.push_back( n );
         }
      }

      return matchOrientation;
   }

   void testBasic()
   {
      // init
      core::Image<ui8> image( NLL_TEST_PATH "data/feature/sf.bmp" );

      core::Image<ui8> output;
      output.clone( image );

      TESTER_ASSERT( image.sizex() );
      core::decolor( image );

      algorithm::SpeededUpRobustFeatures surf( 5, 4, 2, 0.001 );

      nll::core::Timer timer;
      algorithm::SpeededUpRobustFeatures::Points points1 = surf.computesFeatures( image );
      std::cout << "done=" << timer.getCurrentTime() << std::endl;

      std::cout << "nbPOints=" << points1.size() << std::endl;

      printPoints( output, points1 );
      core::writeBmp( output, "c:/tmp/o.bmp" );
   }

   void testRepeatability()
   {
      // init
      core::Image<ui8> image( NLL_TEST_PATH "data/feature/sf.bmp" );

      core::Image<ui8> output;
      output.clone( image );

      core::Image<ui8> image2;
      image2.clone( image );

      core::TransformationRotation tfm( 0.1, core::vector2f( 0, -10 ) );
      core::transformUnaryFast( image2, tfm );

      core::Image<ui8> output2;
      output2.clone( image2 );

      TESTER_ASSERT( image.sizex() );
      core::decolor( image );
      core::decolor( image2 );

      algorithm::SpeededUpRobustFeatures surf( 5, 4, 2, 0.00071 );

      nll::core::Timer timer;
      algorithm::SpeededUpRobustFeatures::Points points1 = surf.computesFeatures( image );
      std::cout << "done=" << timer.getCurrentTime() << std::endl;

      nll::core::Timer timer2;
      algorithm::SpeededUpRobustFeatures::Points points2 = surf.computesFeatures( image2 );
      std::cout << "done2=" << timer2.getCurrentTime() << std::endl;

      std::cout << "nbPOints=" << points1.size() << std::endl;
      std::cout << "nbPOints=" << points2.size() << std::endl;

      std::vector< std::pair<ui32, ui32> > repeatablePointIndex = getRepeatablePointPosition( points1, points2, tfm, 4);
      std::cout << "similarity=" << (double)repeatablePointIndex.size() / std::max( points1.size(), points2.size() ) << std::endl;

      std::vector< ui32 > orientation = getRepeatablePointOrientation( repeatablePointIndex, points1, points2, tfm, 0.3 );
      std::cout << "repeatable orientation=" << (double)orientation.size() / repeatablePointIndex.size() << std::endl;

      

      printPoints( output, points1 );
      core::writeBmp( output, "c:/tmp/o.bmp" );

      printPoints( output2, points2 );
      core::writeBmp( output2, "c:/tmp/o2.bmp" );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSurf);
TESTER_TEST(testBasic);
//TESTER_TEST(testRepeatability);
TESTER_TEST_SUITE_END();
#endif
