#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace algorithm
{
   /**
    @brief Defines a Gaussian RBF parametrized by a value, variance and mean.

    val = exp( - sum( || x_i - mean_i ||^2 / ( 2 * var_i ) ) )

    The RBF is parametrized by (mean, var). This will give a "weight" to any point representing the
    influence of the RBF.
    */
   struct RbfGaussian
   {
      typedef f32                         value_type;
      typedef core::Buffer1D<value_type>  Vector;

   public:
      RbfGaussian( const Vector& value,
                   const Vector& mean,
                   const Vector& variance ) : _value( value ), _mean( mean ), _variance( variance )
      {}

      template <class Vector>
      value_type eval( const Vector& pos ) const
      {
         ensure( pos.size() == _mean.size(), "vector size mismatch" );
         value_type accum = 0;
         for ( ui32 n = 0; n < _mean.size(); ++n )
         {
            const value_type diff = _mean[ n ] - pos[ n ];
            accum += diff * diff / ( 2 * _variance[ n ] );
         }

         return static_cast<value_type>( exp( - accum ) );
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
    - affine transformation, which basically move the DDF grid in source space to the target space
    - Rbfs, specifying a deformable transformation. They are specified in source space.

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

      /**
       @brief add a RBF to our set of RBFs.

       Note that the RBF must be specified in source space (i.e., the affine transformation will be applied
       on the RBF to get the displacement in target space)
       */
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

      const Matrix& getAffineTfm() const
      {
         _affine;
      }

      /**
       @brief return the displacement given a point in target space (after all transformation applied and in MM)
              This is usually the normal way of computing the displacement.
       */
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

      /**
       @brief return the displacement given a point in source space (i.e., by not applying the affine transformation)
       */
      template <class VectorT>
      Vector getDisplacementInSourceSpace( const VectorT& pInSrc ) const
      {
         return _getDisplacement( pInSrc );
      }

   private:
      // here pinv is in the space of the RBF
      template <class VectorT>
      Vector _getDisplacement( const VectorT& pinv ) const
      {
         // finally computes the displacement
         const ui32 nbDim = _affine.sizex();
         Vector accum( nbDim );
         for ( ui32 n = 0; n < _rbfs.size(); ++n )
         {
            const value_type weight = _rbfs[ n ].eval( pinv );
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
      Matrix   _affine;       // the transform source->target
      Matrix   _invAffine;
      Rbfs     _rbfs;         // specified in source space
   };

   class DeformableTransformationDenseDisplacementField2d
   {
   public:
      typedef f32                                  value_type;
      typedef core::Matrix<value_type>             Matrix;
      typedef core::Buffer1D<value_type>           Vector;
      typedef core::Image<value_type>              Storage;

      DeformableTransformationDenseDisplacementField2d() // empty but not valid DDF
      {}

      // create an empty DDF with a specified size
      DeformableTransformationDenseDisplacementField2d( const core::vector2ui& size, const Matrix tfm ) : _affine( tfm ), _storage( size[ 0 ], size[ 1 ], 2 )
      {
         _invaffine.clone( _affine );
         const bool r = core::inverse( _invaffine );
         ensure( r, "matrix is not affine" );
      }

      /**
       @brief Create a DDF from a RBF transformation

       The position of the RBF is important as we do not say what portion of the target space we are mapping! RBFs means
       should be inside a grid [0..X][0..Y], with (X,Y) < size

       For example, assume the RBF are in the range [0..5][0..3], and the size = [10, 6]. If we import the DDF with this
       method, it will basically create a grid of size [10, 6], with position (0, 0) the first voxel of the
       grid corresponding in the DDF mean (0, 0). It will then evaluate the RBF at each position of the grid.
       */
      template <class Rbf>
      DeformableTransformationDenseDisplacementField2d( const core::vector2ui& size, const DeformableTransformationRadialBasis<Rbf>& rbfTfm )
      {
         importFromRbfTfm( size, rbfTfm );
      }

      /**
       @note create a DDF form a RBF transformation.
       
       Note that the affine matrix is updated to the one contained by the rbfTfm transform
       */
      template <class Rbf>
      void importFromRbfTfm( const core::vector2ui& size, const DeformableTransformationRadialBasis<Rbf>& rbfTfm )
      {
         // set the initial parameters
         _storage = Storage( size[ 0 ], size[ 1 ], 2 );
         _affine = rbfTfm.getAffineTfm();
         _invaffine.clone( _affine );
         const bool r = core::inverse( _invaffine );
         ensure( r, "matrix is not affine" );


         // then compute the DDF. Now, we are using the same affine transformation matrix between the DDF and RBF
         // based transform, so we can work directly in the non transformed space
         for ( ui32 y = 0; y < _storage.sizey(); ++y )
         {
            for ( ui32 x = 0; x < _storage.sizex(); ++x )
            {
               value_type* p = _storage.point( x, y );
               Vector d = rbfTfm.getDisplacementInSourceSpace( core::make_buffer1D<value_type>( static_cast<value_type>( x ),
                                                                                                static_cast<value_type>( y ) ) );
               p[ 0 ] = d[ 0 ];
               p[ 1 ] = d[ 1 ];
            }
         }
      }

      /**
       @brief Return the displacement at a specified point expressed in the target space
       */
      template <class VectorT>
      Vector getDisplacement( const VectorT& ptarget ) const
      {
         // compute p in target space
         Vector v( ptarget.size() + 1 );
         for ( ui32 n = 0; n < ptarget.size(); ++n )
         {
            v[ n ] = ptarget[ n ];
         }

         Vector vsrc = _invaffine * Matrix( v, v.size(), 1 );

         // now interpolate
         return getDisplacementSource( vsrc );
      }

      template <class VectorT>
      Vector getDisplacementSource( const VectorT& psource ) const
      {
         // now interpolate
         Vector out( 2 );
         core::InterpolatorLinear2D<value_type, Storage::IndexMapper, Storage::Allocator> interpolator( _storage );
         interpolator.interpolateValues( vsrc[ 0 ], vsrc[ 1 ], out.begin() );
         return out;
      }

      ui32 sizex() const
      {
         return _storage.sizex();
      }

      ui32 sizey() const
      {
         return _storage.sizey();
      }

   private:
      

   private:
      Matrix      _affine;
      Matrix      _invaffine;
      Storage     _storage;
   };

   /**

   TODO rename TARGET=>SOURCE
    @brief Resample a target Image defined by (target, affine) given a deformable transformation
    @param target the target image which used the <affine> tranformation to be transformed
    @param affine the affine transformation by which the target image was transformed
    @param targetOut the output image defined in the same space than the target image

    The <target> image and <affine> will define a target space.
    The DDF is also defining another target space
    Now, using the image target, find the corresponding displacement in the DDF target space. Using this displacement,
    deform the target image
    */
   template <class Image, class Matrix>
   void resample( const Image& source, const Matrix& affine, const DeformableTransformationDenseDisplacementField2d& ddf, Image& targetOut )
   {
      // compute the rotation and spacing of the deformation
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

   void testDdfResampling()
   {
      algorithm::DeformableTransformationRadialBasis<> rbfTfm;

      algorithm::DeformableTransformationDenseDisplacementField2d ddfTfm( core::vector2ui( 15, 30 ), rbfTfm );
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