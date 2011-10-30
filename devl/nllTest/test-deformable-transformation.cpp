#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;

namespace nll
{
namespace core
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

      /**
       @brief Construct a RBF transform based on RBFs and PST
       @param affineTfm acts as a PST, i.e., localize the transformation in MM
       */
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

      const Matrix& getAffineInvTfm() const
      {
         _invAffine;
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

   /**
    @brief Map a source image to a target image using a DDF and affine transform

    Internally, we are using a <Storage> image to store the displacement field. This field
    has its own affine matrix. This storage affine models the mapping of the DDF index to
    position in MM and is not the same as the affine tfm passed to initialize.
    */
   class DeformableTransformationDenseDisplacementField2d
   {
   public:
      typedef f32                                  value_type;
      typedef core::Matrix<value_type>             Matrix;
      typedef core::Buffer1D<value_type>           Vector;
      typedef core::ImageSpatial<value_type>       Storage;

      DeformableTransformationDenseDisplacementField2d() // empty but not valid DDF
      {}

      /**
       @brief create an empty DDF with a specified size and PST.

       The size and PST will define a 2D area in MM to map from
       */
      DeformableTransformationDenseDisplacementField2d( const core::vector2ui& size, const Matrix tfm ) : _affine( tfm )
      {
         _affineUpdated();

         // TODO get the right tfm for storage
         _storage = Storage( size[ 0 ], size[ 1 ], 2, tfm );
      }

      /**
       @brief create a DDF form a RBF transformation.
       @param size the new size of the DDF
       @param tfm the PST of the DDF, which map index coordinates to MM coordinates
       @param rbfTfm a deformable transformation based on RBFs
       */
      template <class Rbf>
      DeformableTransformationDenseDisplacementField2d( const core::vector2ui& size, const Matrix tfm, const DeformableTransformationRadialBasis<Rbf>& rbfTfm )
      {
         importFromRbfTfm( size, tfm, rbfTfm );
      }

      /**
       @brief create a DDF form a RBF transformation.
       @param size the new size of the DDF
       @param tfm the PST of the DDF, which map index coordinates to MM coordinates
       @param rbfTfm a deformable transformation based on RBFs
       */
      template <class Rbf>
      void importFromRbfTfm( const core::vector2ui& size, const Matrix tfm, const DeformableTransformationRadialBasis<Rbf>& rbfTfm )
      {
         // set the initial parameters
         ensure( rbfTfm.getAffineTfm().size() == 9, "must be a 2D RBF tfm" );
         Matrix tfmStorage = tfm; // TODO COMPUTE CORRECT TFM
         _storage = Storage( size[ 0 ], size[ 1 ], 2, tfmStorage );
         _affine = tfm;
         _affineUpdated();

         // compute the transformation DDF->RBF
         const Matrix tfmDdfToRbf = getStorageAffineTfm() * rbfTfm.getAffineInvTfm();
         const core::vector2f dx( tfmDdfToRbf( 0, 0 ), tfmDdfToRbf( 1, 0 ) );
         const core::vector2f dy( tfmDdfToRbf( 0, 1 ), tfmDdfToRbf( 1, 1 ) );

         // get the origin of the DDF in the RBF to initilize the mapping position
         Vector ddfOriginInRbf = getStorageAffineInvTfm() * Matrix( core::make_buffer1D<value_type>( rbfTfm.getAffineTfm()( 0, 3 ), rbfTfm.getAffineTfm()( 1, 3 ), 1 ), 3, 1 );

         // then compute the DDF. Now, we are using the same affine transformation matrix between the DDF and RBF
         // based transform, so we can work directly in the non transformed space
         Vector linePos = core::make_buffer1D<value_type>( ddfOriginInRbf[ 0 ], ddfOriginInRbf[ 1 ] );
         for ( ui32 y = 0; y < _storage.sizey(); ++y )
         {
            Vector startLine;
            startLine.clone( linePos );
            for ( ui32 x = 0; x < _storage.sizex(); ++x )
            {
               value_type* p = _storage.point( x, y );
               Vector d = rbfTfm.getDisplacementInSourceSpace( startLine );
               p[ 0 ] = d[ 0 ];
               p[ 1 ] = d[ 1 ];

               startLine[ 0 ] += dx[ 0 ];
               startLine[ 1 ] += dx[ 1 ];
            }

            linePos[ 0 ] += dy[ 0 ];
            linePos[ 1 ] += dy[ 1 ];
         }
      }

      /**
       @brief Return the displacement at a specified point expressed in the target space in MM
       */
      template <class VectorT>
      Vector getDisplacement( const VectorT& ptarget ) const
      {
         assert( ptarget.size() == 2 ); // "wrong dimensions!"

         // compute p in target space
         Vector v( ptarget.size() + 1 );
         for ( ui32 n = 0; n < ptarget.size(); ++n )
         {
            v[ n ] = ptarget[ n ];
         }
         v[ ptarget.size() ] = 1;

         Vector vsrc = getStorageAffineInvTfm() * Matrix( v, v.size(), 1 );

         // now interpolate
         return getDisplacementSource( vsrc );
      }

      /**
       @brief Compute the displacement in index space directly
       */
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

      const Matrix& getAffineTfm() const
      {
         _affine;
      }

      const Matrix& getStorageAffineTfm() const
      {
         _affine;
      }

      const Matrix& getAffineInvTfm() const
      {
         _invAffine;
      }

      const Matrix& getStorageAffineInvTfm() const
      {
         _invAffine;
      }

      const Storage& getStorage() const
      {
         return _storage;
      }

   private:
      void _affineUpdated()
      {
         _invAffine.clone( _affine );
         const bool r = core::inverse( _invAffine );
         ensure( r, "matrix is not affine" );
      }

   private:
      Matrix      _affine;
      Matrix      _invAffine;
      Storage     _storage;
   };

   /**
    @brief Efficient mapping of the target pixels to source voxels using a DDF as transformation

    For each pixel of the target, apply the tfm on the source volume and find the corresponding source pixel

    CAUTION: Internally, the mapper will use different threads. So for maximal efficiency, new processors
    will be instanciated for each line from the initial processor. Typically the start/end methods will be called
    on the original processor (this is to allow reduction if necessary), while the process method will be called
    on the replicated processors.

    Processors MUST be threadsafe
    */
   class ImageTransformationMapperDeformable
   {
   public:
      typedef core::Matrix<float>   Matrix;

      /**
       @brief Map a transformed target coordinate system to a <resampled> coordinate system
       @param target the <target> volume
       @param tfm an affine transformattion defined as <source> to <target>, inv(tfm) will be applied on the target volume
       @param resampled the volume to map the coordinate from

       Basically, for each voxel of the resampled, it finds the corresponding pixels in the transformed target volume.

       A typical use case is:
       - compute registration between source and target. It will be returned the a source->target matrix
       - resample the moving volume (target) in the source geometry with correct registraton

       The deformable case is slighly more complex than the affine case as we need to retrieve a displacement
       in the DDF to add to the lookup target index
       */
      template <class Processor, class T, class Mapper, class Alloc>
      void run( Processor& procOrig, const ImageSpatial<T, Mapper, Alloc>& target,
               const DeformableTransformationDenseDisplacementField2d& ddfTfm,
               ImageSpatial<T, Mapper, Alloc>& resampled )
      {
         const Matrix tfm = ddfTfm.getAffineTfm();

         typedef ImageSpatial<T, Mapper, Alloc>   ImageType;

         if ( !target.size() || !resampled.size() )
         {
            return; // nothing to do
         }

         ensure( target.getNbComponents() == resampled.getNbComponents(), "must have the same number of dimensions" );

         // compute the transformation target voxel -> resampled voxel
         Matrix transformation = target.getInvertedPst() * tfm * resampled.getPst();
         core::vector2f dx( transformation( 0, 0 ),
                            transformation( 1, 0 ) );
         core::vector2f dy( transformation( 0, 1 ),
                            transformation( 1, 1 ) );

         // compute the target origin with the tfm applied
         core::Matrix<float> targetOriginTfm;
         targetOriginTfm.clone( tfm );
         core::inverse( targetOriginTfm );
         targetOriginTfm = targetOriginTfm * target.getPst();
         core::vector2f targetOrigin2 = transf2d4( targetOriginTfm, core::vector2f( 0, 0 ) );

         // create the transformation representing this displacement and compute the resampled origin in this
         // coordinate system
         Matrix g( 3, 3 );
         for ( ui32 y = 0; y < 2; ++y )
            for ( ui32 x = 0; x < 2; ++x )
               g( y, x ) = targetOriginTfm(y, x);
         g( 2, 2 ) = 1;
         g( 0, 2 ) = targetOrigin2[ 0 ];
         g( 1, 2 ) = targetOrigin2[ 1 ];

         core::VolumeGeometry2d geom2( g );
         core::vector2f originInTarget = geom2.positionToIndex( resampled.getOrigin() );
         core::vector2f slicePosSrc = originInTarget;

         procOrig.start();
       
         #ifndef NLL_NOT_MULTITHREADED
         # pragma omp parallel for
         #endif
         for ( int y = 0; y < (int)resampled.sizey(); ++y )
         {
            Processor proc = procOrig;
            typename ImageType::DirectionalIterator  lineIt = resampled.getIterator( 0, y, 0 );
            core::vector3f linePosSrc = core::vector3f( originInTarget[ 0 ] + y * dy[ 0 ],
                                                        originInTarget[ 1 ] + y * dy[ 1 ],
                                                        0 );
            typename ImageType::DirectionalIterator  voxelIt = lineIt;
            for ( ui32 x = 0; x < resampled.sizex(); ++x )
            {
               proc.process( voxelIt, &linePosSrc[0] );
               linePosSrc[ 0 ] += dx[ 0 ];
               linePosSrc[ 1 ] += dx[ 1 ];
               voxelIt.addx();
            }
            lineIt.addy();
         }
         procOrig.end();
      }

   private:
      template <class T, class Mapper, class Allocator, class Vector>
      static Vector transf2d4( const core::Matrix<T, Mapper, Allocator>& m, const Vector& v )
      {
         assert( m.sizex() == 3 && m.sizey() == 3 );
         return Vector( v[ 0 ] * m( 0, 0 ) + v[ 1 ] * m( 0, 1 ) + m( 0, 2 ),
                        v[ 0 ] * m( 1, 0 ) + v[ 1 ] * m( 1, 1 ) + m( 1, 2 ) );
      }
   };

   /**
    @brief Resample a 2D spatial image using a 2 DDF deformable transformation
    @param target the target image to map from
    @param ddf a transformation defined source->target applied to the target image (so internally the affine TFM will be inverted before applying it to the target)
    @param source the image to map to
    */
   template <class T, class Mapper, class Allocator>
   void resample( const ImageSpatial<T, Mapper, Allocator>& target, const DeformableTransformationDenseDisplacementField2d& ddf, ImageSpatial<T, Mapper, Allocator>& source )
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
      typedef core::DeformableTransformationRadialBasis<>            RbfTransform;
      typedef core::DeformableTransformationDenseDisplacementField2d DdfTransform;
      RbfTransform rbfTfm;

      DdfTransform::Matrix pst = core::identityMatrix<DdfTransform::Matrix>( 3 );
      DdfTransform ddfTfm( core::vector2ui( 15, 30 ), pst, rbfTfm );
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