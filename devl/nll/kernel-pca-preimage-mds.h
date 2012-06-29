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

#ifndef NLL_ALGORITHM_KERNEL_PCA_PREIMAGE_MDS_H_
# define NLL_ALGORITHM_KERNEL_PCA_PREIMAGE_MDS_H_

namespace nll
{
namespace algorithm
{
   namespace impl
   {
      template <class Point, class Kernel>
      std::vector< std::pair<double, size_t> > computeDistanceInputFromFeature( const core::Buffer1D<double>& buf, const Kernel& kernel )
      {
         throw std::runtime_error( "this must be implemented for each type of kernel" );
      }

      template <class Point>
      std::vector< std::pair<double, size_t> > computeDistanceInputFromFeature( const core::Buffer1D<double>& df, const KernelRbf<Point>& kernel )
      {
         std::vector< std::pair<double, size_t> > d;
         for ( size_t n = 0; n < df.size(); ++n )
         {
            // compute the distance in input space
            double di = - kernel.getVar() * log( 1 - 0.5 * df[ n ] );
            if ( di < 0 || IS_NAN( di ) )
               di = 1e30;
            d.push_back( std::make_pair( di, n ) );
         }

         return d;
      }
   }

   /**
    @brief Generic pre-image implementing the Multi-Dimensional Scaling (MDS) by James T. Kwok and Ivor W. Tsang
           in "The Pre-Image Problem in Kernel Methods", 2003
    @note this is typically used for denoising problems: we project the noisy example on the feature space and try 
          to reconstruct a denoised example using a trained KernelPca

    @brief for computation see http://opus.kobv.de/tuberlin/volltexte/2006/1256/pdf/bakir_goekhan.pdf
                           and http://www.hpl.hp.com/conferences/icml2003/papers/345.pdf
                               http://cmp.felk.cvut.cz/cmp/software/stprtool/manual/kernels/preimage/list/rbfpreimg3.html

          Given a point in feature space z, we are looking for the closest preimage x so that x = arg min_x ||theta(x) - z ||
    */
   template <class Point, class Kernel>
   class KernelPreImageMDS
   {
   public:
      //typedef KernelRbf<Point>         Kernel;
      typedef core::Buffer1D<double>   Vector;
      typedef core::Matrix<double>     Matrix;

   public:
      KernelPreImageMDS( const KernelPca<Point, Kernel>& kernelPca ) : _kernelPca( kernelPca )
      {
         const std::vector<Point>& supports = kernelPca.getSupports();
         const Kernel& kernel = kernelPca.getKernel();

         _bias = _computeBias( kernelPca );
         _kernel = Matrix( (size_t)supports.size(), (size_t)supports.size() );
         for ( size_t nx = 0; nx < supports.size(); ++nx )
         {
            for ( size_t ny = 0; ny < supports.size(); ++ny )
            {
               _kernel( ny, nx ) = kernel( supports[ nx ], supports[ ny ] );
            }
         }
      }

      /**
       @brief compute the preimage
       @param feature the point in feature space we want to find the pre-image
       @param nbNeighbours the number of neighbours to be used to compute the preimage

       direct implementation of http://cmp.felk.cvut.cz/cmp/software/stprtool/manual/kernels/preimage/list/rbfpreimg3.html
       */
      template <class Point2>
      Point preimage( const Point2& feature, size_t nbNeighbours = 7 ) const
      {
         const Kernel& kernel = _kernelPca.getKernel();
         const std::vector<Point>& supports = _kernelPca.getSupports();
         ensure( _kernelPca.getSupports().size() > 0, "kernel PCA not trained!" );


         // compute the distance in input space by method 1
         Vector df = _computeDistance( _kernelPca, feature );  // ||feature, support||_2^2 distance in feature space
         std::vector< std::pair<double, size_t> > d = impl::computeDistanceInputFromFeature( df, kernel );

         // compute the neighbours
         std::sort( d.begin(), d.end() );
         nbNeighbours = std::min<size_t>( nbNeighbours, (size_t)d.size() ); // if there is less than <nbNeighbours> then use only these ones

         // center the neighbours
         const size_t pointDim = static_cast<size_t>( supports[ 0 ].size() );
         Matrix centered( pointDim, nbNeighbours );
         Vector mean( pointDim );
         for ( size_t dim = 0; dim < pointDim; ++dim )
         {
            // center for each axis
            double sum = 0;
            for ( size_t n = 0; n < nbNeighbours; ++n )
            {
               const size_t id = d[ n ].second;
               sum += supports[ id ][ dim ];
            }
            mean[ dim ] = sum / nbNeighbours;
            for ( size_t n = 0; n < nbNeighbours; ++n )
            {
               const size_t id = d[ n ].second;
               for ( size_t dim = 0; dim < pointDim; ++dim )
               {
                  centered( dim, n ) = supports[ id ][ dim ] - mean[ dim ];
               }
            }
         }

         // compute the projection
         Vector eiv;
         Matrix r;
         bool result = core::svd( centered, centered, eiv, r );
         ensure( result, "cannot compute SVD" );

         // compute the second distance estimate
         Matrix z = _getProjection( eiv, r );

         Vector d02( nbNeighbours );
         for ( size_t n = 0; n < z.sizex(); ++n )
         {
            double sum = 0;
            for ( size_t y = 0; y < z.sizey(); ++y )
            {
               sum += core::sqr( z( y, n ) );
            }
            d02[ n ] = sum;
         }

         // compute
         // [U,L,V] = svd(X*H);
         // Z = L*V';
         // d02 = sum(Z.^2)';
         // z = -0.5*pinv(Z')*(d2-d02);
         // x = U*z + sum(X,2)/nn;
         for ( size_t n = 0; n < d02.size(); ++n )
         {
            d02[ n ] = - 0.5 * ( d[ n ].first - d02[ n ] );
         }

         core::transpose( z );
         Matrix pinvz = core::pseudoInverse( z );
         z = pinvz * Matrix( d02, d02.size(), 1 );
         Matrix sx = centered * z;

         Point p( sx.sizey() );
         for ( size_t y = 0; y < sx.sizey(); ++y )
         {
            double sum = 0;
            for ( size_t x = 0; x < nbNeighbours; ++x )
            {
               const size_t id = d[ x ].second;
               sum += supports[ id ][ y ];
            }
            p[ y ] += sx( y, 0 ) + sum / nbNeighbours;
         }

         return p;
      }

   private:
      // non copiable
      KernelPreImageMDS( const KernelPreImageMDS& );
      KernelPreImageMDS operator=( const KernelPreImageMDS& );

   private:
      // compute Z
      // [U,L,V] = svd(X*H);
      // r = rank(L);
      // return Z = L*V';
      static Matrix _getProjection( const Vector& eiv, const Matrix& eig )
      {
         Matrix proj( eiv.size(), eig.sizey() );
         for ( size_t y = 0; y < proj.sizey(); ++y )
         {
            for ( size_t x = 0; x < proj.sizex(); ++x )
            {
               proj( y, x ) = eiv[ y ] * eig( x, y );
            }
         }
         return proj;
      }

      // compute the distance in feature space between the test point <feature> and the support
      template <class Point2>
      Vector _computeDistance( const KernelPca<Point, Kernel>& kernelPca, const Point2& feature ) const
      {  
         const std::vector<Point>& supports = kernelPca.getSupports();

         // first project the <feature> on the alphas
         Vector kx( feature.size() );
         for ( size_t n = 0; n < kx.size(); ++n )
         {
           kx[ n ] = feature[ n ] - _bias[ n ];
         }
         Matrix projection = kernelPca.getEigenVectors() * Matrix( kx, kx.size(), 1 );

         // here we want to compute the distance by using the kernel matrix
         // different from the denoising only case...
         Matrix kalpha = _kernel * projection;
         core::transpose( projection );
         Matrix const2 = projection * kalpha;

         // compute the distance in feature space between projection and all supports
         Vector dist( (size_t)supports.size() );
         for ( size_t n = 0; n < dist.size(); ++n )
         {
            dist[ n ] = 1 + const2[ 0 ] - 2 * kalpha[ n ];
         }

         return dist;
      }

      // compute (-_sumA+mean(_sumA)' * eiv
      static Vector _computeBias( const KernelPca<Point, Kernel>& kernelPca )
      {
         Vector bias;
         bias.clone( kernelPca.getBias() );

         double sum = 0;
         for ( size_t n = 0; n < bias.size(); ++n )
            sum += bias[ n ];
         sum /= bias.size();

         for ( size_t n = 0; n < bias.size(); ++n )
            bias[ n ] = sum - bias[ n ];
         Matrix b = Matrix( bias, 1, bias.size() ) * kernelPca.getEigenVectors();
         return b;
      }

   private:
      const KernelPca<Point, Kernel>&  _kernelPca;
      Vector                           _bias;
      Matrix                           _kernel;
   };
}
}

#endif