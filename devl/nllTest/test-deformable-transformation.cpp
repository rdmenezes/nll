#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace algorithm
{
   template <class StorageT, class Adaptor>
   class DeformableTransformationDenseDisplacementField
   {
   public:
      typedef RbfGaussian::value_type     value_type;  // link all the value_type to avoid confusion...
      typedef core::Matrix<value_type>    Matrix;
      typedef core::Buffer1D<value_type>  Vector;
      typedef StorageT                    Storage;

      DeformableTransformationDenseDisplacementField()
      {}

      DeformableTransformationDenseDisplacementField( const Matrix& affine, const Storage& storage ) : _affine( affine ), _storage( storage )
      {}

   private:
      Matrix      _affine;
      Storage     _storage;
   };

   /**
    @brief Defines a Gaussian RBF parametrized by a value, variance and mean.

    val = exp( - sum( || x_i - mean_i ||^2 / ( 2 * var_i ) ) )

    The RBF is parametrized by (mean, var). This will give a "weight" to any point representing the
    influence of the RBF.
    */
   struct RbfGaussian
   {
      typedef f64                         value_type;
      typedef core::Buffer1D<value_type>  Vector;

   public:
      RbfGaussian( const Vector& value,
                   const Vector& mean,
                   const Vector& variance ) : _value( value ), _mean( mean ), _variance( variance )
      {}

      template <class Vector>
      core::Buffer1D<value_type> eval( const Vector& pos ) const
      {
         ensure( pos.size() == _mean.size(), "vector size mismatch" );
         value_type accum = 0;
         for ( size_t n = 0; n < _mean.size(); ++n )
         {
            const type diff = _mean[ n ] - x[ n ];
            accum += diff * diff / ( 2.0 * _var[ n ] );
         }

         return exp( - accum );
      }

      const Vector& getValue() const
      {
         return _value;
      }

      Vector& getValue()
      {
         return _value;
      }

      const Vector& getMean() const
      {
         return _mean;
      }

      Vector& getMean()
      {
         return _mean;
      }

      const Vector& getVariance() const
      {
         return _variance;
      }

      Vector& getVariance()
      {
         return _variance;
      }

   private:
      Vector    _value;
      Vector    _mean;
      Vector    _variance;
   };

   /**
    @brief model a deformable transformation using RBF functions as parameters.

    There are two components:
    - affine transformation, which basically move the DDF grid to the target position
    - Rbfs, specifying a deformable transformation

    The value of the displacement at point <x> is computed as follow:
    displacement = sum_i( Rbf::compute( A^-1 * x - mean_i ) * RbfValue_i  // we use A^-1 to reduce computational load and is equivalent to x - A * mean_i
    */
   template <class RbfT = RbfGaussian>
   class DeformableTransformationRadialBasis
   {
   public:
      typedef RbfT                        Rbf;
      typedef typename Rbf::value_type    value_type;
      typedef core::Matrix<value_type>    Matrix;
      typedef core::Buffer1D<value_type>  Vector;

   public:
      typedef std::vector<Rbf> Rbfs;

   public:
      
      DeformableTransformationRadialBasis()
      {}

      DeformableTransformationRadialBasis( const Matrix& affineTfm, const Rbfs& g ) : _affine( affineTfm ), _rbfs( g )
      {
         ensure( _affine.sizex() == _affine.sizey(), "must be square matrix" );
         ensure( g.size() > 0 && _affine.sizex() == g[ 0 ].getMean().size(), "size doesn't match" );
         _affineUpdated();
      }

      void clear()
      {
         _rbfs.clear();
      }

      void add( const Rbf& g )
      {
         _rbfs.push_back( g );
      }

      Rbf& operator[]( size_t i )
      {
         return _rbfs[ i ];
      }

      const Rbf& operator[]( size_t i ) const
      {
         return _rbfs[ i ];
      }

      size_t size() const
      {
         return _rbfs.size();
      }

      template <class VectorT>
      Vector getDisplacement( const VectorT& pInMm ) const
      {
         // compute pInMm * affine^-1
         Vector p( pInMm.size() + 1 );
         for ( ui32 n = 0; n < pInMm.size(); ++n )
         {
            p[ n ] = static_cast<value_type>( pInMm );
         }
         p[ pInMm.size() ] = 1;
         p = _invAffine * Matrix( p, p.size(), 1 );

         return _getDisplacement( p );
      }

      template <class T, class Mapper, class Allocator, class Storage, class Adaptor>
      void createDdf( DeformableTransformationDenseDisplacementField<Storage, Adaptor>& ddf, const core::Buffer1D<ui32>& size ) const
      {
         
      }



   private:
      // here pinv is in the space of the RBF
      template <class VectorT>
      Vector _getDisplacement( const VectorT& pinv ) const
      {
         // finally computes the displacement
         const ui32 nbDim = _affine.sizex();
         Vector accum( nbDim );
         for ( ui32 n = 0; n < mixture.size(); ++n )
         {
            const value_type weight = _rbfs[ n ].compute( pinv );
            for ( ui32 nn = 0; nn < nbDim; ++nn )
            {
               accum[ nn ] += weight * _rbfs[ n ].getValue()[ nn ];
            }
         }

         return accum;
      }

      void _affineUpdated()
      {
         _invAffine.clone( _affine );
         bool r = core::inverse( _invAffine );
         ensure( r, "not an affine transformation" );
      }

   private:
      Matrix   _affine;
      Matrix   _invAffine;
      Rbfs     _rbfs;
   };


   /**
    @brief Export a mixture of gaussians as a dense displacement field
    @param mixture the gaussian mixture. The intensity at a given point is computed from the weighted gaussian sum
    @param outDdf the allocated output DDF. Note that the <outDdf.getNbComponents()> must be 2

    The transformation represented by the <DeformableTransformationRadialBasis> will be discretized by a DDF for fast manipulation/visualization
    */
   template <class Rbf, class T, class Mapper, class Allocator>
   void exportAsDenseDensityField( const DeformableTransformationRadialBasis<Rbf>& mixture, core::Image<T, Mapper, Allocator>& outDdf )
   {
      typedef DeformableTransformationRadialBasis<Rbf> Tfm;
      typedef typename Tfm::value_type       value_type;

      ensure( outDdf.getNbComponents() == 2, "we must have exactly 2 components for a 2D DDF" );
      for ( ui32 y = 0; y < outDdf.sizey(); ++y )
      {
         for ( ui32 x = 0; x < outDdf.sizex(); ++x )
         {
            T* dst = outDdf.getPoint( x, y );
            core::Buffer1D<value_type> accum( 2 );
            core::Buffer1D<value_type> p( 2 );
            p[ 0 ] = static_cast<value_type>( x );
            p[ 1 ] = static_cast<value_type>( y );
            for ( ui32 n = 0; n < mixture.size(); ++n )
            {
               const value_type weight = mixture[ n ].compute( p );
               accum[ 0 ] += weight * mixture[ n ].getValue()[ 0 ];
               accum[ 1 ] += weight * mixture[ n ].getValue()[ 1 ];
            }

            dst[ 0 ] = static_cast<T>( p[ 0 ] );
            dst[ 1 ] = static_cast<T>( p[ 1 ] );
         }
      }
   }


}
}

struct TestDeformable2D
{
   typedef core::Image<double>   Image;
   typedef core::Image<ui8>      Imagec;

   Imagec toImagec( const Image& i )
   {
      Imagec iout( i.sizex(), i.sizey(), i.getNbComponents() );
      for ( ui32 x = 0; x < i.sizex(); ++x )
      {
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            for ( ui32 c = 0; c < i.getNbComponents(); ++c )
            {
               iout( x, y, c ) = (ui8)NLL_BOUND( i(x, y, c ), 0, 255 );
            }
         }
      }
      return iout;
   }

   static Image createCheckerboard0()
   {
      Image i( 3, 3, 2 );
      i( 0, 0, 0 ) = 1;
      i( 1, 0, 0 ) = 0;
      i( 2, 0, 0 ) = 1;

      i( 0, 1, 0 ) = 0;
      i( 1, 1, 0 ) = 1;
      i( 2, 1, 0 ) = 0;

      i( 0, 2, 0 ) = 1;
      i( 1, 2, 0 ) = 0;
      i( 2, 2, 0 ) = 1;

      i( 0, 0, 1 ) = 0;
      i( 1, 0, 1 ) = 1;
      i( 2, 0, 1 ) = 0;

      i( 0, 1, 1 ) = 1;
      i( 1, 1, 1 ) = 0;
      i( 2, 1, 1 ) = 1;

      i( 0, 2, 1 ) = 0;
      i( 1, 2, 1 ) = 1;
      i( 2, 2, 1 ) = 0;

      return i;
   }

   static Image createCheckerboard()
   {
      Image i = createCheckerboard0();
      Image iout( i.sizex(), i.sizey(), 3 );
      for ( ui32 x = 0; x < i.sizex(); ++x )
      {
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            for ( ui32 c = 0; c < i.getNbComponents(); ++c )
            {
               iout( x, y, c ) = i(x, y, 0 ) * 255;
            }
         }
      }
      return iout;
   }

   void testResamplerNearest2Dvals()
   {
      typedef core::InterpolatorNearestNeighbor2D<Image::value_type, Image::IndexMapper, Image::Allocator> Interpolator;

      Image i = createCheckerboard0();
      Interpolator interpolator( i );
      
      Image::value_type out[ 2 ];
      interpolator.interpolateValues( 0, 0, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0.49, 0, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( -0.49, -0.49, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0.50, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 1 );

      interpolator.interpolateValues( 0.50, 0.50, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( -0.51, -0.51, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 1, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 1 );

      interpolator.interpolateValues( 2, 0, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 2.49, 0, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0, 2.49, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0, 2.51, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 2.51, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );
   }

   void testResamplerNearest2D()
   {
      typedef core::InterpolatorNearestNeighbor2D<Image::value_type, Image::IndexMapper, Image::Allocator> Interpolator;

      Image i = createCheckerboard0();
      Interpolator interpolator( i );
      
      Image::value_type out[ 1 ];
      out[ 0 ] = interpolator.interpolate( 0, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( 0.49, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( -0.49, -0.49, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( 0.50, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 0.50, 0.50, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( -0.51, -0.51, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 1, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 2, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( 2.49, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( 0, 2.49, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( 0, 2.51, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 2.51, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );
   }

   void testResamplerLinear2Dvals()
   {
      typedef core::InterpolatorLinear2D<Image::value_type, Image::IndexMapper, Image::Allocator> Interpolator;

      Image i = createCheckerboard0();
      Interpolator interpolator( i );

      Image::value_type out[ 2 ];
      interpolator.interpolateValues( 0, 0, out );
      TESTER_ASSERT( out[ 0 ] == 1 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0.49, 0, out );
      TESTER_ASSERT( core::equal<double>( out[ 0 ], 0.51, 1e-3 ) && core::equal<double>( out[ 1 ], 0.49, 1e-3 ) );

      interpolator.interpolateValues( -0.49, -0.49, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0.50, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0.5 && out[ 1 ] == 0.5 );

      interpolator.interpolateValues( 0.50, 0.50, out );
      TESTER_ASSERT( out[ 0 ] == 0.5 && out[ 1 ] == 0.5 );

      interpolator.interpolateValues( -0.51, -0.51, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 1, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 1 );

      interpolator.interpolateValues( 2, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 ); // we are just one the border...

      interpolator.interpolateValues( 1.999999, 0, out );
      TESTER_ASSERT( core::equal<double>( out[ 0 ], 1, 1e-3 ) && core::equal<double>( out[ 1 ], 0, 1e-3 ) ); // we are just one the border...

      interpolator.interpolateValues( 2.49, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0, 2.49, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 0, 2.51, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );

      interpolator.interpolateValues( 2.51, 0, out );
      TESTER_ASSERT( out[ 0 ] == 0 && out[ 1 ] == 0 );
   }

   void testResamplerLinear2D()
   {
      typedef core::InterpolatorLinear2D<Image::value_type, Image::IndexMapper, Image::Allocator> Interpolator;

      Image i = createCheckerboard0();
      Interpolator interpolator( i );

      Image::value_type out[ 1 ];
      out[ 0 ] = interpolator.interpolate( 0, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 1 );

      out[ 0 ] = interpolator.interpolate( 0.49, 0, 0 );
      TESTER_ASSERT( core::equal<double>( out[ 0 ], 0.51, 1e-3 ) );

      out[ 0 ] = interpolator.interpolate( -0.49, -0.49, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 0.50, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0.5 );

      out[ 0 ] = interpolator.interpolate( 0.50, 0.50, 0 );
      TESTER_ASSERT( out[ 0 ] == 0.5 );

      out[ 0 ] = interpolator.interpolate( -0.51, -0.51, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 1, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 2, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 ); // we are just one the border...

      out[ 0 ] = interpolator.interpolate( 1.999999, 0, 0 );
      TESTER_ASSERT( core::equal<double>( out[ 0 ], 1, 1e-3 ) ); // we are just one the border...

      out[ 0 ] = interpolator.interpolate( 2.49, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 0, 2.49, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 0, 2.51, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );

      out[ 0 ] = interpolator.interpolate( 2.51, 0, 0 );
      TESTER_ASSERT( out[ 0 ] == 0 );
   }

   void testResizeNn()
   {
      Image oi = createCheckerboard();
      Image i = createCheckerboard();
      core::rescaleNearestNeighbor( i, 256, 256 );
      for ( ui32 x = 0; x < i.sizex(); ++x )
      {
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            ui32 ratiox = i.sizex() / oi.sizex();
            ui32 ratioy = i.sizey() / oi.sizey();
            const ui32 xi = x / ratiox;
            const ui32 yi = y / ratioy;
            if ( ( x % ratiox ) != 0 && ( y % ratioy ) != 0 )
            {
               TESTER_ASSERT( i( x, y, 0 ) == oi( xi, yi, 0 ) );
            }
         }
      }
      //core::writeBmp( toImagec(i), "c:/tmp/reampled.bmp" );
   }

   // check the resampling with a zoom factor, check we find the same result than resizing
   void testResampleNn()
   {
      Image oi = createCheckerboard();
      Image i = createCheckerboard();
      core::rescaleNearestNeighbor( i, 256, 256 );

      Image resampled(256, 256, 3 );
      double col[ 3 ] = {0, 0, 0};

      const ui32 ratiox = i.sizex() / oi.sizex();
      const ui32 ratioy = i.sizey() / oi.sizey();

      core::Matrix<double> tfm = core::identityMatrix< core::Matrix<double> >( 3 );
      tfm( 0, 0 ) = ratiox;
      tfm( 1, 1 ) = ratioy;

      core::resampleNearestNeighbour( oi, resampled, tfm, col );
      for ( ui32 x = 0; x < i.sizex(); ++x )
      {
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            const ui32 xi = x / ratiox;
            const ui32 yi = y / ratioy;
            if ( ( x % ratiox ) != 0 && ( y % ratioy ) != 0 )
            {
               TESTER_ASSERT( i( x, y, 0 ) == oi( xi, yi, 0 ) );
               TESTER_ASSERT( i( x, y, 1 ) == oi( xi, yi, 1 ) );
               TESTER_ASSERT( i( x, y, 2 ) == oi( xi, yi, 2 ) );
            }
         }
      }

      for ( ui32 x = 0; x < i.sizex(); ++x )
      {
         for ( ui32 y = 0; y < i.sizey(); ++y )
         {
            TESTER_ASSERT( i( x, y, 0 ) == resampled( x, y, 0 ) );
            TESTER_ASSERT( i( x, y, 1 ) == resampled( x, y, 1 ) );
            TESTER_ASSERT( i( x, y, 2 ) == resampled( x, y, 2 ) );
         }
      }

      //core::writeBmp( toImagec(i), "c:/tmp/reampled.bmp" );
      //core::writeBmp( toImagec(resampled), "c:/tmp/reampled2.bmp" );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestDeformable2D);
 TESTER_TEST(testResamplerNearest2Dvals);
 TESTER_TEST(testResamplerNearest2D);
 TESTER_TEST(testResamplerLinear2Dvals);
 TESTER_TEST(testResamplerLinear2D);
 TESTER_TEST(testResizeNn);
 TESTER_TEST(testResampleNn);
TESTER_TEST_SUITE_END();
#endif