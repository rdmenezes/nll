#include <nll/nll.h>
#include <sstream>
#include <tester/register.h>
#include "config.h"

//#define NLL_NOT_MULTITHREADED

using namespace nll;

#define NLL_SURF_3D_NB_AREA_PER_FEATURE         4.0
#define NLL_SURF_3D_NB_AREA_PER_FEATURE_SIZE    20.0
#define NLL_SURF_3D_NB_SUBAREA_PER_AREA         5.0

namespace nll
{
namespace core
{
   /**
    @brief represent a 3D rotation
    */
   class Rotation3D
   {
   public:
      Rotation3D( double phi, double theta, double psi )
      {
         const double ch = std::cos(phi);
         const double sh = std::sin(phi);
         const double ca = std::cos(theta);
         const double sa = std::sin(theta);
         const double cb = std::cos(psi);
         const double sb = std::sin(psi);

         _rotation = core::Matrix<double>( 4, 4 );

         _rotation( 0, 0 ) = ch * ca;
         _rotation( 0, 1 ) = sh*sb - ch*sa*cb;
         _rotation( 0, 2 ) = ch*sa*sb + sh*cb;
         _rotation( 1, 0 ) = sa;
         _rotation( 1, 1 ) = ca*cb;
         _rotation( 1, 2 ) = -ca*sb;
         _rotation( 2, 0 ) = -sh*ca;
         _rotation( 2, 1 ) = sh*sa*cb + ch*sb;
         _rotation( 2, 2 ) = -sh*sa*sb + ch*cb;

         _rotation( 3, 3 ) = 1;
      }

      core::vector3d transform( const core::vector3d& p ) const
      {
         return core::transf4( _rotation, p );
      }

   private:
      static core::Matrix<double> _phiAngle( double phi )
      {
         const double ci = std::cos( phi );
         const double si = std::sin( phi );
         core::Matrix<double> m( 4, 4 );
         m( 0, 0 ) = ci;
         m( 1, 0 ) = -si;
         m( 0, 1 ) = si;
         m( 1, 1 ) = ci;
         m( 2, 2 ) = 1;
         m( 3, 3 ) = 1;
         return m;
      }

      static core::Matrix<double> _thetaAngle( double theta )
      {
         const double ci = std::cos( theta );
         const double si = std::sin( theta );
         core::Matrix<double> m( 4, 4 );
         m( 1, 1 ) = ci;
         m( 2, 1 ) = -si;
         m( 1, 2 ) = si;
         m( 2, 2 ) = ci;
         m( 0, 0 ) = 1;
         m( 3, 3 ) = 1;
         return m;
      }

      static core::Matrix<double> _psiAngle( double psi )
      {
         const double ci = std::cos( psi );
         const double si = std::sin( psi );
         core::Matrix<double> m( 4, 4 );
         m( 0, 0 ) = ci;
         m( 1, 0 ) = -si;
         m( 0, 1 ) = si;
         m( 1, 1 ) = ci;
         m( 2, 2 ) = 1;
         m( 3, 3 ) = 1;
         return m;
      }

   private:
      core::Matrix<double> _rotation;
   };
}
namespace algorithm
{
   /**
    @brief Hold a stack of the hessian determinant using a crude approximation of a gaussian in 2D
           for each point,           | Lxx Lxy Lxz |   | a b c |
                           H(x, o) = | Lyx Lyy Lyz | = | d e f |
                                     | Lzx Lzy Lzz |   | g h i |

                           det(H)= Lxx * | Lyy Lyx | - Lyy * | Lxx Lxz | + Lzz * | Lxx Lxy |
                                         | Lzy Lzz |         | Lzx Lzz |         | Lyx Lyy |
                                 = a(ei - 0.81hf) - e(bi - 0.81hc) + i(ae - 0.81bd)
           note the normalization factor 0.9^2 comes from the gaussian approximation of the gaussian derivatives
    */
   class FastHessianDetPyramid3d
   {
   public:
      typedef double                      value_type; // float as we don't need that much accuracy, we will save a lot of space like this...
      typedef imaging::Volume<value_type> Volume;
      typedef core::Matrix<value_type>    Matrix;

   public:
      /**
       @brief Construct and computes the hessian determinant pyramid
       @param i the image,
       @param scales the size of each level of the pyramid (in pixel), must be in increasing order
       @param displacements the step between two filter evaluation for this particular level
       */
      template <class VolumeT>
      void construct( const VolumeT& i, const std::vector<ui32>& scales, const std::vector<ui32>& displacements )
      {
         ensure( displacements.size() == scales.size(), "must be the same size" );

         _pyramidDetHessian.clear();
         _scales = scales;
         _displacements = displacements;

         const ui32 sizex = i.size()[ 0 ];
         const ui32 sizey = i.size()[ 1 ];
         const ui32 sizez = i.size()[ 2 ];

         const value_type max = (value_type)std::max( abs( *std::max_element( i.begin(), i.end() ) ),
                                                      abs( *std::min_element( i.begin(), i.end() ) ) );

         // construct an integral image
         IntegralImage3d image;
         image.process( i );
         _integralImage = image;

         for ( size_t n = 0; n < scales.size(); ++n )
         {
            core::Timer scaleTimer;
            ensure( scales[ n ] % 2 == 1, "scales must be odd numbers" );
            ensure( scales[ n ] >= 9, "minimal size" );

            const ui32 step = displacements[ n ];

            const int sizeFilterz = scales[ n ];
            const int sizeFilterx = scales[ n ];
            const int sizeFiltery = scales[ n ];
            const double sizeFilter = sizeFilterx * sizeFiltery * sizeFilterz * max; // we normalize by the filter size and maximum value

            const int halfx = sizeFilterx / 2;
            const int halfy = sizeFiltery / 2;
            const int halfz = sizeFilterz / 2;

            // the total size must take into account the step size and filter size (it must be fully inside the image to be computed)
            const int resx = ( (int)i.size()[ 0 ] ) / (int)step;
            const int resy = ( (int)i.size()[ 1 ] ) / (int)step;
            const int resz = ( (int)i.size()[ 2 ] ) / (int)step;

            //std::cout << "scale=" << n << " size=" << resx << " " << resy << " " << resz << std::endl;

            if ( resx <= 0 || resy <= 0 || resz <= 0 )
               break;   // the scale is too big!
            Volume detHessian( resx, resy, resz );

            #ifndef NLL_NOT_MULTITHREADED
            # pragma omp parallel for
            #endif
            for ( int z = 0; z < resz; ++z )
            {
               for ( int y = 0; y < resy; ++y )
               {
                  for ( int x = 0; x < resx; ++x )
                  {
                     const core::vector3ui bl( x * step, y * step, z * step );
                     const core::vector3ui tr( bl[ 0 ] + sizeFilterx - 1, bl[ 1 ] + sizeFiltery - 1, bl[ 2 ] + sizeFilterz - 1 );
                     if ( tr[ 0 ] < sizex && tr[ 1 ] < sizey && tr[ 2 ] < sizez )
                     {
                        const double dxx = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DX,
                                                                              image,
                                                                              bl,
                                                                              tr ) / sizeFilter;
                        const double dyy = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DY,
                                                                              image,
                                                                              bl,
                                                                              tr ) / sizeFilter;
                        const double dzz = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DZ,
                                                                              image,
                                                                              bl,
                                                                              tr ) / sizeFilter;
                        const double dxy = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DXY,
                                                                              image,
                                                                              bl,
                                                                              tr ) / sizeFilter;
                        const double dxz = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DXZ,
                                                                              image,
                                                                              bl,
                                                                              tr ) / sizeFilter;
                        const double dyz = HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::DYZ,
                                                                              image,
                                                                              bl,
                                                                              tr ) / sizeFilter;
                        const double val = dxx * ( dyy * dzz - 0.81 * dyz * dyz ) -
                                           dyy * ( dxy * dzz - 0.81 * dyz * dxz ) +
                                           dzz * ( dxx * dyy - 0.81 * dxy * dxy );
                        detHessian( x, y, z ) = static_cast<value_type>( val );
                     }
                  }
               }
            }
            std::cout << "Time scale=" << scaleTimer.getCurrentTime() << std::endl;
            _pyramidDetHessian.push_back( detHessian );
         }
      }

      // computes the index in mapDest the closest from (xRef, yRef, mapDest)
      void indexInMap( ui32 xRef, ui32 yRef, ui32 zRef, ui32 mapRef, ui32 mapDest, int& outx, int& outy, int& outz ) const
      {
         if ( mapRef == mapDest )
         {
            outx = xRef;
            outy = yRef;
            outz = zRef;
         } else {
            // map a point at a given scale to the image space
            const int x = xRef * _displacements[ mapRef ];
            const int y = yRef * _displacements[ mapRef ];
            const int z = zRef * _displacements[ mapRef ];

            // convert the image space coordinate to the other scale space
            outx = ( x ) / (int)_displacements[ mapDest ];
            outy = ( y ) / (int)_displacements[ mapDest ];
            outz = ( z ) / (int)_displacements[ mapDest ];
         }
      }

      /**
       @brief Computes the gradient of the hessian at position (x, y, z, map)
              using finite difference
       */
      core::vector4d getHessianGradient( ui32 x, ui32 y, ui32 z, ui32 map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && z > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < _pyramidDetHessian[ map ].size()[ 0 ] - 1 &&
                 y < _pyramidDetHessian[ map ].size()[ 1 ] - 1 &&
                 z < _pyramidDetHessian[ map ].size()[ 2 ] - 1 );

         int xminus, yminus, zminus;
         indexInMap( x, y, z, map, map - 1, xminus, yminus, zminus );  // we need to look up the closed index in a map that has different dimensions

         int xplus, yplus, zplus;
         indexInMap( x, y, z, map, map + 1, xplus, yplus, zplus ); // we need to look up the closed index in a map that has different dimensions

         const Volume& current = _pyramidDetHessian[ map ];
         return core::vector4d( ( current( x + 1, y, z ) - current( x - 1, y, z ) ) / 2,
                                ( current( x, y + 1, z ) - current( x, y - 1, z ) ) / 2,
                                ( current( x, y, z + 1 ) - current( x, y, z - 1 ) ) / 2,
                                ( _pyramidDetHessian[ map + 1 ]( xplus,  yplus,  zplus ) -
                                  _pyramidDetHessian[ map - 1 ]( xminus, yminus, zplus ) ) / 2 );
      }

      /**
       @brief returns true if all value around the projection (xRef, yRef, mapRef) on mapDest are smaller
       */
      bool isDetHessianMax( value_type val, ui32 xRef, ui32 yRef, ui32 zRef, ui32 mapRef, ui32 mapDest ) const
      {
         int x, y, z;

         // if it is outside, then skip it
         indexInMap( xRef, yRef, zRef, mapRef, mapDest, x, y, z );
         if ( mapDest >= _pyramidDetHessian.size() )
            return false;
         const Volume& m = _pyramidDetHessian[ mapDest ];
         if ( x < 1 || y < 1 || z < 1 || x + 1 >= (int)m.size()[ 0 ] || y + 1 >= (int)m.size()[ 1 ] || z + 1 >= (int)m.size()[ 2 ] )
            return false;

         return 
                // current slice
                val >= m( x + 0, y + 0, z ) &&
                val >= m( x + 1, y + 0, z ) &&
                val >= m( x - 1, y + 0, z ) &&
                val >= m( x + 0, y + 1, z ) &&
                val >= m( x + 1, y + 1, z ) &&
                val >= m( x - 1, y + 1, z ) &&
                val >= m( x + 0, y - 1, z ) &&
                val >= m( x + 1, y - 1, z ) &&
                val >= m( x - 1, y - 1, z ) &&

                // direct z neighbours
                val >= m( x, y, z - 1 ) &&
                val >= m( x, y, z + 1 ) &&

                // upper and lower rest of the slices
                val >= m( x + 0, y + 0, z + 1 ) &&
                val >= m( x + 1, y + 0, z + 1 ) &&
                val >= m( x - 1, y + 0, z + 1 ) &&
                val >= m( x + 0, y + 1, z + 1 ) &&
                val >= m( x + 1, y + 1, z + 1 ) &&
                val >= m( x - 1, y + 1, z + 1 ) &&
                val >= m( x + 0, y - 1, z + 1 ) &&
                val >= m( x + 1, y - 1, z + 1 ) &&
                val >= m( x - 1, y - 1, z + 1 ) &&

                val >= m( x + 0, y + 0, z - 1 ) &&
                val >= m( x + 1, y + 0, z - 1 ) &&
                val >= m( x - 1, y + 0, z - 1 ) &&
                val >= m( x + 0, y + 1, z - 1 ) &&
                val >= m( x + 1, y + 1, z - 1 ) &&
                val >= m( x - 1, y + 1, z - 1 ) &&
                val >= m( x + 0, y - 1, z - 1 ) &&
                val >= m( x + 1, y - 1, z - 1 ) &&
                val >= m( x - 1, y - 1, z - 1 );
      }

      /**
       @brief Computes the hessian of the hessian at position (x, y, z, map)
              using finite difference
       */
      Matrix getHessianHessian( ui32 x, ui32 y, ui32 z, ui32 map ) const
      {
         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( x > 0 && y > 0 && z > 0 && map > 0 &&
                 map < _scales.size() - 1 &&
                 x < _pyramidDetHessian[ map ].size()[ 0 ] &&
                 y < _pyramidDetHessian[ map ].size()[ 1 ] &&
                 z < _pyramidDetHessian[ map ].size()[ 2 ] );

         const Volume& mc = _pyramidDetHessian[ map ];
         const Volume& mm = _pyramidDetHessian[ map - 1 ];
         const Volume& mp = _pyramidDetHessian[ map + 1 ];
         const value_type val = mc( x, y, z );

         int xm, ym, zm;
         indexInMap( x, y, z, map, map - 1, xm, ym, zm );  // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xm > 0 && ym > 0 && zm > 0 &&
                 xm < (int)_pyramidDetHessian[ map - 1 ].size()[ 0 ] - 1 &&
                 ym < (int)_pyramidDetHessian[ map - 1 ].size()[ 1 ] - 1 &&
                 zm < (int)_pyramidDetHessian[ map - 1 ].size()[ 2 ] - 1 );

         int xp, yp, zp;
         indexInMap( x, y, z, map, map + 1, xp, yp, zp ); // we need to look up the closed index in a map that has different dimensions

         // check the bounds, it cannot be on the border as the gradient is not
         // defined here
         assert( xp > 0 && yp > 0 && zp > 0 &&
                 xp < (int)_pyramidDetHessian[ map + 1 ].size()[ 0 ] - 1 &&
                 yp < (int)_pyramidDetHessian[ map + 1 ].size()[ 1 ] - 1 && 
                 zp < (int)_pyramidDetHessian[ map + 1 ].size()[ 2 ] - 1 );

         //
         // TODO CHECK
         //

         //std::cout << "check=" << x << " " << y << " " << z << " " << " toMP=" << xp << " " << yp << " " << zp << " sizeM=" << mp.size() << std::endl;
         const value_type dxx = mc( x + 1, y, z ) + mc( x - 1, y, z ) - 2 * val;
         const value_type dyy = mc( x, y + 1, z ) + mc( x, y - 1, z ) - 2 * val;
         const value_type dzz = mc( x, y, z + 1 ) + mc( x, y, z - 1 ) - 2 * val;
         const value_type dss = mp( xp, yp, zp )   + mm( xm, ym, zm ) - 2 * val;

         const value_type dxy = ( mc( x + 1, y + 1, z ) + mc( x - 1, y - 1, z ) -
                                  mc( x + 1, y - 1, z ) - mc( x - 1, y + 1, z ) ) / 4;
         const value_type dxz = ( mc( x + 1, y, z + 1 ) + mc( x - 1, y, z - 1 ) -
                                  mc( x + 1, y, z - 1 ) - mc( x - 1, y, z + 1 ) ) / 4;
         const value_type dyz = ( mc( x, y + 1, z + 1 ) + mc( x, y - 1, z + 1 ) -
                                  mc( x, y - 1, z + 1 ) - mc( x, y + 1, z + 1 ) ) / 4;

         const value_type dxs = ( mp( xp + 1, yp, zp ) + mm( xm - 1, ym, zm ) -
                                  mm( xm + 1, ym, zm ) - mp( xp - 1, yp, zp ) ) / 4;
         const value_type dys = ( mp( xp, yp + 1, zp ) + mm( xm, ym - 1, zm ) -
                                  mm( xm, ym + 1, zm ) - mp( xp, yp - 1, zp ) ) / 4;
         const value_type dzs = ( mp( xp, yp, zp + 1 ) + mm( xm, ym, zm - 1 ) -
                                  mm( xm, ym, zm + 1 ) - mp( xp, yp, zp - 1 ) ) / 4;

         // returns the hessian matrix defined as:
         //     | dxx dxy dxz dxs |
         // H = | dyx dyy dyz dys |
         //     | dzx dzy dzz dzs |
         //     | dsx dsy dsz dss |
         Matrix hs( 4, 4 );
         
         hs( 0, 0 ) = dxx;
         hs( 0, 1 ) = dxy;
         hs( 0, 2 ) = dxz;
         hs( 0, 3 ) = dxs;

         hs( 1, 0 ) = dxy;
         hs( 1, 1 ) = dyy;
         hs( 1, 2 ) = dyz;
         hs( 1, 3 ) = dys;

         hs( 2, 0 ) = dxz;
         hs( 2, 1 ) = dyz;
         hs( 2, 2 ) = dzz;
         hs( 2, 3 ) = dzs;

         hs( 3, 0 ) = dxs;
         hs( 3, 1 ) = dys;
         hs( 3, 2 ) = dzs;
         hs( 3, 3 ) = dss;

         return hs;
      }

      const std::vector<Volume>& getPyramidDetHessian() const
      {
         return _pyramidDetHessian;
      }

      const IntegralImage3d& getIntegralImage() const
      {
         return _integralImage;
      }

   private:
      std::vector<Volume>  _pyramidDetHessian;
      std::vector<ui32>    _scales;
      std::vector<ui32>    _displacements;
      IntegralImage3d      _integralImage;
   };

   /**
    @brief Implementation of the Speeded Up Robust Features or SURF algorithm for 3 dimentional data

           Feature detector which intend to be robust to scaling, illumination and rotation

    @see http://sites.google.com/site/chrisevansdev/files/opensurf.pdf for good notes on the algorithm
         http://www.vision.ee.ethz.ch/~surf/eccv06.pdf reference paper

    @note we are assuming the spacing is the same for all dimensions
    */
   class SpeededUpRobustFeatures3d
   {
      typedef double                            value_type;
      typedef FastHessianDetPyramid3d::Volume   Volume;
      typedef core::Matrix<value_type>          Matrix;

      struct LocalPoint
      {
         LocalPoint( value_type x, value_type y, value_type z ) : dx( x ), dy( y ), dz( z )
         {}

         value_type dx;
         value_type dy;
         value_type dz;
      };

   public:
      struct Point
      {
         typedef core::Buffer1D<value_type> Features;
         Point( core::vector3i p, ui32 s ) : position( p ), scale( s ), features( static_cast<int>( 4 * ( NLL_SURF_3D_NB_AREA_PER_FEATURE * NLL_SURF_3D_NB_AREA_PER_FEATURE * NLL_SURF_3D_NB_AREA_PER_FEATURE ) ) )
         {}

         Point()
         {}

         Features                   features;
         value_type                 orientation1;     // dxy
         value_type                 orientation2;     // drz
         core::vector3i             position;
         ui32                       scale;
      };

      typedef core::Buffer1D<Point> Points;

      /**
       @brief Expose the <point>'s <features> array as if it was stored as an array only
       */
      class PointsFeatureWrapper
      {
      public:
         typedef core::Buffer1D<SpeededUpRobustFeatures3d::value_type>  value_type;

      public:
         PointsFeatureWrapper( Points& points ) : _points( points )
         {}

         ui32 size() const
         {
            return _points.size();
         }

         core::Buffer1D<SpeededUpRobustFeatures3d::value_type>& operator[]( ui32 n )
         {
            return _points[ n ].features;
         }

         const core::Buffer1D<SpeededUpRobustFeatures3d::value_type>& operator[]( ui32 n ) const
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
      SpeededUpRobustFeatures3d( ui32 octaves = 5, ui32 intervals = 4, ui32 init_step = 2, value_type threshold = 0.0000012 ) : _threshold( threshold )
      {
         //std::cout << "intervals=" << intervals << " o=" << octaves << std::endl;
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
      template <class Volume>
      Points computesPoints( const Volume& i )
      {
         std::stringstream ss;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "SURF point detection started..." );
         FastHessianDetPyramid3d pyramid;
         Points points = _computesPoints( i, pyramid );

         core::Timer angle;
         _computeAngle( pyramid.getIntegralImage(), points );
         std::cout << "Angle computation=" << angle.getCurrentTime() << std::endl;

         ss << "Number of points detected=" << points.size();
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         return points;
      }

   private:
      // sig = standard deviation
      static value_type gaussian(value_type x, value_type y, value_type z, value_type sig)
      {
         return ( 1.0 / ( 2.0 * core::PI * sig * sig ) ) * std::exp( -( x * x + y * y + z * z ) / ( 2.0 * sig * sig ) );
      }



      void _computeFeatures( const IntegralImage& image, Points& points ) const
      {
         int nbPoints = static_cast<ui32>( points.size() );
         const f32 area_size = NLL_SURF_3D_NB_AREA_PER_FEATURE_SIZE / NLL_SURF_3D_NB_AREA_PER_FEATURE;
         const f32 area_pos_min = - NLL_SURF_3D_NB_AREA_PER_FEATURE / 2 * area_size;
         const f32 area_pos_max =   NLL_SURF_3D_NB_AREA_PER_FEATURE / 2 * area_size;
         const f32 area_pos_center =   area_pos_min + area_size / 2;
         const f32 dd5x5 = (float)(area_size / NLL_SURF_3D_NB_SUBAREA_PER_AREA);
         /*
         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int n = 0; n < nbPoints; ++n )
         {
            Point& point = points[ n ];

            // this constant is to find the gaussian's sigma
            // we know that a filter 9*9 corresponds to a gaussian's sigma = 1.2
            // so for a filter of size X, sigma = 1.2 / 9 * X
            static const value_type scaleFactor = 1.2 / 9;
            value_type scale = core::round( scaleFactor * point.scale );

            const value_type x = point.position[ 0 ];
            const value_type y = point.position[ 1 ];

            const value_type co = cos( point.orientation);
            const value_type si = sin( point.orientation);

            const int size = (int)core::sqr( 2 * scale );

            ui32 count = 0;
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
                        const int sample_x = core::round( x + ( di * scale * co - dj * scale * si ) );
                        const int sample_y = core::round( y + ( di * scale * si + dj * scale * co ) );

                        //Get the gaussian weighted x and y responses
                        const value_type gauss_s1 = gaussian( di - cx, dj - cy, 2.5 * scale );

                        core::vector2ui bl( core::round( sample_x - scale ),
                                            core::round( sample_y - scale ) );
                        core::vector2ui tr( core::round( sample_x + scale ),
                                            core::round( sample_y + scale ) );

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
                           const value_type rrx = gauss_s1 * ( -rx * si + ry * co );
                           const value_type rry = gauss_s1 * (  rx * co + ry * si );

                           dx += rrx;
                           dy += rry;
                           mdx += fabs( rrx );
                           mdy += fabs( rry );
                        }
                     }
                  }

                  //Add the values to the descriptor vector
                  const value_type gauss_s2 = gaussian( cx, cy, 3.3 * scale );
                  point.features[ count++ ] = dx * gauss_s2;
                  point.features[ count++ ] = dy * gauss_s2;
                  point.features[ count++ ] = mdx * gauss_s2;
                  point.features[ count++ ] = mdy * gauss_s2;

                  len += ( dx * dx + dy * dy + mdx * mdx + mdy * mdy ) * gauss_s2 * gauss_s2;
               }
            }

            //Convert to Unit Vector
            len = (sqrt( len ) + 1e-7);
            for( ui32 i = 0; i < point.features.size(); ++i )
               point.features[ i ] /= len;
         }*/
      }

      /**
       @brief assign a repeable orientation for each point       

         The dominant orientation is determined for each SURF 
         key point in order to guarantee the invariance of feature 
         description to image rotatio
       */
      
      static void _computeAngle( const IntegralImage3d& i, Points& points )
      {
         // preprocessed gaussian of size 2.5
         // we need to weight the response so that it is more tolerant to the noise. Indeed, the further
         // away from the centre, the more likely it is to be noisier
         static const value_type gauss25 [7][7][7]  = {
            {
               0.002666886202051,   0.002461846247067,   0.001936556847605,   0.001298112875237,   0.000741493840234, 0.000360923799514,   0.000149705024458,
               0.002461846247067,   0.002272570512958,   0.001787667281731,   0.001198309214586,   0.000684485084665, 0.000333174659132,   0.000138195155213,
               0.001936556847605,   0.001787667281731,   0.001406228890127,   0.000942623414366,   0.000538435038082, 0.000262084469474,   0.000108708159355,
               0.001298112875237,   0.001198309214586,   0.000942623414366,   0.000631859370512,   0.000360923799514, 0.000175680473643,   0.000072869258383,
               0.000741493840234,   0.000684485084665,   0.000538435038082,   0.000360923799514,   0.000206162945641, 0.000100350278886,   0.000041623580865,
               0.000360923799514,   0.000333174659132,   0.000262084469474,   0.000175680473643,   0.000100350278886, 0.000048845724634,   0.000020260371887,
               0.000149705024458,   0.000138195155213,   0.000108708159355,   0.000072869258383,   0.000041623580865, 0.000020260371887,   0.000008403656043
            },
            {
               0.002461846247067,   0.002272570512958,   0.001787667281731,   0.001198309214586,   0.000684485084665, 0.000333174659132,   0.000138195155213,
               0.002272570512958,   0.002097846988828,   0.001650224889666,   0.001106178824010,   0.000631859370512, 0.000307558974046,   0.000127570206769,
               0.001787667281731,   0.001650224889666,   0.001298112875237,   0.000870151082288,   0.000497038185121, 0.000241934457906,   0.000100350278886,
               0.001198309214586,   0.001106178824010,   0.000870151082288,   0.000583279713537,   0.000333174659132, 0.000162173516961,   0.000067266803563,
               0.000684485084665,   0.000631859370512,   0.000497038185121,   0.000333174659132,   0.000190312385141, 0.000092634982804,   0.000038423407891,
               0.000333174659132,   0.000307558974046,   0.000241934457906,   0.000162173516961,   0.000092634982804, 0.000045090286861,   0.000018702680473,
               0.000138195155213,   0.000127570206769,   0.000100350278886,   0.000067266803563,   0.000038423407891, 0.000018702680473,   0.000007757552263
            },
            {
               0.001936556847605,   0.001787667281731,   0.001406228890127,   0.000942623414366,   0.000538435038082, 0.000262084469474,   0.000108708159355,
               0.001787667281731,   0.001650224889666,   0.001298112875237,   0.000870151082288,   0.000497038185121, 0.000241934457906,   0.000100350278886,
               0.001406228890127,   0.001298112875237,   0.001021131754471,   0.000684485084665,   0.000390984084430, 0.000190312385141,   0.000078938325238,
               0.000942623414366,   0.000870151082288,   0.000684485084665,   0.000458824073463,   0.000262084469474, 0.000127570206769,   0.000052913941807,
               0.000538435038082,   0.000497038185121,   0.000390984084430,   0.000262084469474,   0.000149705024458, 0.000072869258383,   0.000030224923164,
               0.000262084469474,   0.000241934457906,   0.000190312385141,   0.000127570206769,   0.000072869258383, 0.000035469275908,   0.000014712049536,
               0.000108708159355,   0.000100350278886,   0.000078938325238,   0.000052913941807,   0.000030224923164, 0.000014712049536,   0.000006102306744
            },
            {
               0.001298112875237,   0.001198309214586,   0.000942623414366,   0.000631859370512,   0.000360923799514, 0.000175680473643,   0.000072869258383,
               0.001198309214586,   0.001106178824010,   0.000870151082288,   0.000583279713537,   0.000333174659132, 0.000162173516961,   0.000067266803563,
               0.000942623414366,   0.000870151082288,   0.000684485084665,   0.000458824073463,   0.000262084469474, 0.000127570206769,   0.000052913941807,
               0.000631859370512,   0.000583279713537,   0.000458824073463,   0.000307558974046,   0.000175680473643, 0.000085512866874,   0.000035469275908,
               0.000360923799514,   0.000333174659132,   0.000262084469474,   0.000175680473643,   0.000100350278886, 0.000048845724634,   0.000020260371887,
               0.000175680473643,   0.000162173516961,   0.000127570206769,   0.000085512866874,   0.000048845724634, 0.000023775766660,   0.000009861781723,
               0.000072869258383,   0.000067266803563,   0.000052913941807,   0.000035469275908,   0.000020260371887, 0.000009861781723,   0.000004090498537
            },
            {
               0.000741493840234220,   0.000684485084665209,   0.000538435038082152,   0.000360923799514429,   0.000206162945641395,   0.000100350278886302, 0.000041623580864519,
               0.000684485084665209,   0.000631859370512295,   0.000497038185120945,   0.000333174659131743,   0.000190312385140791,   0.000092634982804403, 0.000038423407891183,
               0.000538435038082152,   0.000497038185120945,   0.000390984084430091,   0.000262084469474381,   0.000149705024457774,   0.000072869258383364, 0.000030224923164329,
               0.000360923799514429,   0.000333174659131743,   0.000262084469474381,   0.000175680473643293,   0.000100350278886302,   0.000048845724634120, 0.000020260371886937,
               0.000206162945641395,   0.000190312385140791,   0.000149705024457774,   0.000100350278886302,   0.000057320988859612,   0.000027901120641273, 0.000011572908058766,
               0.000100350278886302,   0.000092634982804403,   0.000072869258383364,   0.000048845724634120,   0.000027901120641273,   0.000013580933415951, 0.000005633139105622,
               0.000041623580864519,   0.000038423407891183,   0.000030224923164329,   0.000020260371886937,   0.000011572908058766,   0.000005633139105622, 0.000002336529840137
            },
            {
               0.000360923799514429,   0.000333174659131743,   0.000262084469474381,   0.000175680473643293,   0.000100350278886302,   0.000048845724634120,  0.000020260371886937,
               0.000333174659131743,   0.000307558974046307,   0.000241934457905870,   0.000162173516961070,   0.000092634982804403,   0.000045090286860856,  0.000018702680472704,
               0.000262084469474381,   0.000241934457905870,   0.000190312385140791,   0.000127570206768727,   0.000072869258383364,   0.000035469275908233,  0.000014712049536454,
               0.000175680473643293,   0.000162173516961070,   0.000127570206768727,   0.000085512866873989,   0.000048845724634120,   0.000023775766659657,  0.000009861781722555,
               0.000100350278886302,   0.000092634982804403,   0.000072869258383364,   0.000048845724634120,   0.000027901120641273,   0.000013580933415951,  0.000005633139105622,
               0.000048845724634120,   0.000045090286860856,   0.000035469275908233,   0.000023775766659657,   0.000013580933415951,   0.000006610549978256,  0.000002741943167798,
               0.000020260371886937,   0.000018702680472704,   0.000014712049536454,   0.000009861781722555,   0.000005633139105622,   0.000002741943167798,  0.000001137311170805
            },
            {
               0.000149705024457774,   0.000138195155213183,   0.000108708159355106,   0.000072869258383364,   0.000041623580864519,   0.000020260371886937,  0.000008403656043015,
               0.000138195155213183,   0.000127570206768727,   0.000100350278886302,   0.000067266803562755,   0.000038423407891183,   0.000018702680472704,  0.000007757552262718,
               0.000108708159355106,   0.000100350278886302,   0.000078938325237764,   0.000052913941807354,   0.000030224923164329,   0.000014712049536454,  0.000006102306743534,
               0.000072869258383364,   0.000067266803562755,   0.000052913941807354,   0.000035469275908233,   0.000020260371886937,   0.000009861781722555,  0.000004090498537249,
               0.000041623580864519,   0.000038423407891183,   0.000030224923164329,   0.000020260371886937,   0.000011572908058766,   0.000005633139105622,  0.000002336529840137,
               0.000020260371886937,   0.000018702680472704,   0.000014712049536454,   0.000009861781722555,   0.000005633139105622,   0.000002741943167798,  0.000001137311170805,
               0.000008403656043015,   0.000007757552262718,   0.000006102306743534,   0.000004090498537249,   0.000002336529840137,   0.000001137311170805,  0.000000471737238914
            }
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
            localPoints.reserve( 1331 );

            for ( int w = -6; w <= 6; ++w )
            {
               for ( int v = -6; v <= 6; ++v )
               {
                  for ( int u = -6; u <= 6; ++u )
                  {
                     if ( u * u + v * v + w * w < 6 * 6 )
                     {
                        const value_type gauss = gauss25[ id[ u + 6 ] ][ id[ v + 6 ] ][ id[ w + 6 ] ];
                        const int x = point.position[ 0 ] + u * scale;
                        const int y = point.position[ 1 ] + v * scale;
                        const int z = point.position[ 2 ] + w * scale;
                        const core::vector3ui bl( x - 2 * scale, y - 2 * scale, z - 2 * scale );
                        const core::vector3ui tr( x + 2 * scale, y + 2 * scale, z + 2 * scale );
                        
                        if ( bl[ 0 ] >= 0 && bl[ 1 ] >= 0 && bl[ 2 ] >= 0 && tr[ 0 ] < i.size()[ 0 ] && tr[ 1 ] < i.size()[ 1 ] && tr[ 2 ] < i.size()[ 2 ] && z > 2 * scale && y > 2 * scale && x > 2 * scale )
                        {
                           const value_type dy = - gauss * HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::HALFY,
                                                                                              i,
                                                                                              bl,
                                                                                              tr );
                           
                           const value_type dx = - gauss * HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::HALFX,
                                                                                              i,
                                                                                              bl,
                                                                                              tr );
                           
                           const value_type dz = - gauss * HaarFeatures3d::Feature::getValue( HaarFeatures3d::Feature::HALFZ,
                                                                                              i,
                                                                                              bl,
                                                                                              tr );
                           localPoints.push_back( LocalPoint( dx, dy, dz ) );
                        }
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
            double dz = 0;
            for ( int nn = 0; nn < nbLocalPoints; ++nn )
            {
               dx += localPoints[ nn ].dx;
               dy += localPoints[ nn ].dy;
               dz += localPoints[ nn ].dz;
            }
            if ( nbLocalPoints )
            {
               dx /= nbLocalPoints;
               dy /= nbLocalPoints;
               dz /= nbLocalPoints;
            }
            double norm;
            core::carthesianToSphericalCoordinate( dx, dy, dz, norm, points[ n ].orientation1, points[ n ].orientation2 );
         }
      }

      template <class VolumeT>
      Points _computesPoints( const VolumeT& i, FastHessianDetPyramid3d& pyramid )
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
         timePyramid.start();

         for ( ui32 filter = 1; filter < _filterSizes.size() - 1 ; ++filter )
         {
            if ( pyramid.getPyramidDetHessian().size() <= filter )
               break; // the filter was not used in the pyramid...
            const Volume& f = pyramid.getPyramidDetHessian()[ filter ];
            const int sizex = static_cast<int>( f.size()[ 0 ] );
            const int sizey = static_cast<int>( f.size()[ 1 ] );
            const int sizez = static_cast<int>( f.size()[ 2 ] );
            const ui32 step = _filterSteps[ filter + 1 ];

            #ifndef NLL_NOT_MULTITHREADED
            # pragma omp parallel for reduction(+ : nbPoints)
            #endif
            for ( int z = 0; z < sizez; ++z )
            {
               for ( int y = 0; y < sizey; ++y )
               {
                  for ( int x = 0; x < sizex; ++x )
                  {
                     const value_type val = f( x, y, z );
                     if ( val > _threshold )
                     {
                        //std::cout << "val=" << val << "pos=" << x << " " << y << " " << z << " step=" << step << std::endl;
                        bool isMax = pyramid.isDetHessianMax( val, x, y, z, filter, filter )     &&
                                     pyramid.isDetHessianMax( val, x, y, z, filter, filter + 1 ) &&
                                     pyramid.isDetHessianMax( val, x, y, z, filter, filter - 1 );
                        if ( isMax )
                        {
                           
                           core::vector4d hessianGradient = pyramid.getHessianGradient( x, y, z, filter );
                           
                           Matrix hessianHessian = pyramid.getHessianHessian( x, y, z, filter );
                           
                           const bool inverted = core::inverse( hessianHessian );
                           core::vector4d interpolatedPoint = core::mat4Mulv( hessianHessian, hessianGradient );
                           if ( inverted && interpolatedPoint[ 0 ] < 0.5 &&
                                            interpolatedPoint[ 1 ] < 0.5 &&
                                            interpolatedPoint[ 2 ] < 0.5 )
                           {
                              
                              const int size = _filterSizes[ filter ];
                              const int half = size / 2;
                              // here we need to compute the step between the two scales (i.e., their difference in size and not the step as for the position)
                              const int filterStep = static_cast<int>( _filterSizes[ filter + 1 ] - _filterSizes[ filter ] );

                              int px    = core::round( ( x    - interpolatedPoint[ 0 ] ) * _filterSteps[ filter ] );
                              int py    = core::round( ( y    - interpolatedPoint[ 1 ] ) * _filterSteps[ filter ] );
                              int pz    = core::round( ( z    - interpolatedPoint[ 2 ] ) * _filterSteps[ filter ] );
                              int scale = core::round( size   - interpolatedPoint[ 3 ]   * filterStep );

                              #ifndef NLL_NOT_MULTITHREADED
                              ui32 threadId = omp_get_thread_num();
                              #else
                              ui32 threadId = 0;
                              #endif
                              bins[ threadId ].push_back( Point( core::vector3i( px, py, pz ), scale ) );

                              ++nbPoints;
                              
                           }
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

   private:
      std::vector<ui32> _filterSizes;
      std::vector<ui32> _filterSteps;
      value_type _threshold;
   };
}
}

class TestSurf3D
{
public:
   /**
    */
   void simpleTest()
   {
      std::vector<ui32> scales = core::make_vector<ui32>( 9, 15, 21 );
      std::vector<ui32> steps = core::make_vector<ui32>( 2, 2, 2 );
      imaging::Volume<float> volume( 512, 512, 300 );

      algorithm::FastHessianDetPyramid3d pyramid;
      pyramid.construct( volume, scales, steps );
   }

   void testSurf3d()
   {
      const std::string volname = NLL_TEST_PATH "data/medical/ct.mf2";

      imaging::VolumeSpatial<float> volume;
      
      std::cout << "loadind..." << std::endl;
      bool loaded = nll::imaging::loadSimpleFlatFile( volname, volume );
      TESTER_ASSERT( loaded );

      std::cout << "runing surf..." << std::endl;
      algorithm::SpeededUpRobustFeatures3d surf( 5, 4, 2, 0.00001 );

      algorithm::SpeededUpRobustFeatures3d::Points points = surf.computesPoints( volume );
      std::cout << "nbPoints=" << points.size() << std::endl;
      std::ofstream f( "c:/tmp/points.txt" );
      for ( ui32 n = 0; n < points.size(); ++n )
      {
         const core::vector3f point = volume.indexToPosition( core::vector3f( points[ n ].position[ 0 ],
                                                                              points[ n ].position[ 1 ],
                                                                              points[ n ].position[ 2 ] ) );
         f << point[ 0 ] << " " << point[ 1 ] << " " << point[ 2 ] << " " << ( points[ n ].scale * volume.getSpacing()[ 0 ] ) << std::endl;
      }
   }

   void testCarthesianToSphericalCoordinate()
   {
      // checked against matlab cart2sph(x,y,z)
      {
         core::vector3d pos( 3, 0, 0 );
         core::vector3d result = core::carthesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 3, 1e-8 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 0, 1e-8 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 0, 1e-8 ) );
      }

      {
         core::vector3d pos( 1, 1, 0 );
         core::vector3d result = core::carthesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 1.4142, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 0.7854, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0, 1e-4 ) );
      }

      {
         core::vector3d pos( 0, 0, 1 );
         core::vector3d result = core::carthesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 1, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 0, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 1.5708, 1e-4 ) );
      }

      {
         core::vector3d pos( 10, 20, 15 );
         core::vector3d result = core::carthesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 1.1071, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( -10, 20, 15 );
         core::vector3d result = core::carthesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 2.0344, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( -10, -20, 15 );
         core::vector3d result = core::carthesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], -2.0344, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( 10, -20, 15 );
         core::vector3d result = core::carthesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], -1.1071, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], 0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( 10, 20, -15 );
         core::vector3d result = core::carthesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 1.1071, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], -0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( -10, 20, -15 );
         core::vector3d result = core::carthesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], 2.0344, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], -0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( -10, -20, -15 );
         core::vector3d result = core::carthesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], -2.0344, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], -0.5909, 1e-4 ) );
      }

      {
         core::vector3d pos( 10, -20, -15 );
         core::vector3d result = core::carthesianToSphericalCoordinate( pos );
         TESTER_ASSERT( core::equal<double>( result[ 0 ], 26.9258 , 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 1 ], -1.1071, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( result[ 2 ], -0.5909, 1e-4 ) );
      }
   }

   void testRotation3d()
   {
      /*
      {
         core::Rotation3D rot( 0, 0 );
         core::vector3d p = rot.transform( core::vector3d( 1, 2, 3 ) );
         TESTER_ASSERT( core::equal<double>( p[ 0 ], 1, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( p[ 1 ], 2, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( p[ 2 ], 3, 1e-4 ) );
      }

      {
         core::Rotation3D rot( 0.4636, 0, 0 );
         core::vector3d p = rot.transform( core::vector3d( 2.2361, 0, 0 ) );
         TESTER_ASSERT( core::equal<double>( p[ 0 ], 2, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( p[ 1 ], 1, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( p[ 2 ], 0, 1e-4 ) );
      }*/

      {
         core::Rotation3D rot( -0.7854, 0, 0 );
         core::vector3d p = rot.transform( core::vector3d( 1, 0, 0 ) );
         TESTER_ASSERT( core::equal<double>( p[ 0 ], 2, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( p[ 1 ], 1, 1e-4 ) );
         TESTER_ASSERT( core::equal<double>( p[ 2 ], 0, 1e-4 ) );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestSurf3D);
//TESTER_TEST(simpleTest);
//TESTER_TEST(testSurf3d);
//TESTER_TEST(testCarthesianToSphericalCoordinate);
TESTER_TEST(testRotation3d);
TESTER_TEST_SUITE_END();
#endif