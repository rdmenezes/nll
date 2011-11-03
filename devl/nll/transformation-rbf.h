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

#ifndef NLL_CORE_TRANSFORMATION_RBF_H_
# define NLL_CORE_TRANSFORMATION_RBF_H_

namespace nll
{
namespace core
{
   /**
    @brief Defines a Gaussian RBF parametrized by a value, variance and mean.

    val = exp( - sum( || x_i - mean_i ||^2 / ( 2 * var_i ) ) )

    The RBF is parametrized by (mean, var). This will give a "weight" to any point representing the
    influence of the RBF.

    In the transformation context, they are defined in the source space in MM (so there is no PST associated
    with the RBFs)
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
    - Rbfs, specifying a deformable transformation. They are specified in MM and are spread on the
      source image (so there is no PST associated with the RBFs).

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

       Typically the RBFs will be spread on the source image. Each RBF will have its position in MM
       */
      DeformableTransformationRadialBasis( const Matrix& affineTfm, const Rbfs& g ) : _affine( affineTfm ), _rbfs( g )
      {
         ensure( _affine.sizex() == _affine.sizey(), "must be square matrix" );
         ensure( g.size() > 0 && _affine.sizex() == ( g[ 0 ].getMean().size() + 1 ), "size doesn't match" );
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
         return _affine;
      }

      const Matrix& getAffineInvTfm() const
      {
         return _invAffine;
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
         const ui32 nbDim = _affine.sizex() - 1;
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
}
}

#endif
