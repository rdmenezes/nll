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

#ifndef NLL_GMM_H_
# define NLL_GMM_H_

# include <iostream>
# include <cmath>
# include "kmeans.h"
# include "io.h"

//# define NLL_GMM_DEBUG

//
// define a IS_NAN macro
//
# ifdef IS_NAN
#  undef IS_NAN
# endif
# ifdef _MSC_VER
#  define IS_NAN( x )    ( !_finite( x ) )
# else
// TODO: portability issue
#  define IS_NAN( x )    false
#endif



namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Gaussian mixture model algorithm, using expectation maximization for optimizing GMM parameters
           As an init step, a kmeans algorithm (using euclidian distance) is used to determine the k gaussians.
    */
   class Gmm
   {
      /**
       It is the metric used by the kmeans initialization step
       */
      struct MetricEuclidian
      {
         typedef f64*   Point;

         MetricEuclidian( size_t size ) : _size( size )
         {}

         double distance( const Point& p1, const Point& p2 )
         {
            return nll::core::generic_norm2<Point, nll::f64>( p1, p2, _size );
         }

      private:
         size_t _size;
      };

      /**
       It is the allocator used by the kmeans initialization step
       */
      struct AllocatorKPoint
      {
         typedef f64          value_type;
         typedef value_type*  Point;

         static Point allocate( size_t size )
         {
            return new value_type[ size ];
         }

         static void deallocate( const Point p )
         {
            delete [] p;
         }
      };

   public:
      typedef f64                                                             ComputingType;
      typedef core::Buffer1D<ComputingType>                                   Vector;
      typedef core::Matrix<ComputingType, core::IndexMapperRowMajorFlat2D>    Matrix;

      struct            Gaussian
      {
         Matrix         covariance;
         Vector         mean;
         ComputingType  weight;  // sum(weight) must be equal to 1!!

         Gaussian( const Vector& m, const Matrix& c, const ComputingType w ) :
            mean( m ), covariance( c ), weight( w ) {}

         Gaussian() : weight( 0 ){}

         void write( std::ostream& o ) const
         {
            core::write<Matrix>( covariance, o );
            core::write<Vector>( mean, o );
            core::write<ComputingType>( weight, o );
         }
         void clone( const Gaussian& g )
         {
            covariance.clone( g.covariance );
            mean.clone( g.mean );
            weight = g.weight;
         }
         void read( std::istream& i )
         {
            core::read<Matrix>( covariance, i );
            core::read<Vector>( mean, i );
            core::read<ComputingType>( weight, i );
         }
      };
      typedef core::Buffer1D<Gaussian>       Gaussians;

   public:
      /**
       @brief Create an empty GMM
       */
      Gmm() : _pointSize( 0 )
      {}

      /**
       @brief Create a GMM that is already precomputed, typically used if we want to generate
              points from a known distribution
       */
      Gmm( const std::vector<Gaussian>& params )
      {
         ensure( params.size(), "empty parameters" );
         _pointSize = params[ 0 ].mean.size();
         _gaussians = Gaussians( (size_t)params.size() );
         for ( size_t n = 0; n < params.size(); ++n )
            _gaussians[ n ] = params[ n ];
      }

      /**
       @brief Generate a random point generated by the model
       @note out need not be allocated
       @note there is a serious overhead so it is advised to generate a lot of points from this call...
       */
      void generate( size_t nbPoints, std::vector< core::Buffer1D<double> >& out ) const
      {
         core::Buffer1D<double> pbs( _gaussians.size() );
         std::vector<core::NormalMultiVariateDistribution> normals;
         for ( size_t n = 0; n < pbs.size(); ++n )
         {
            pbs[ n] = _gaussians[ n ].weight;
            normals.push_back( core::NormalMultiVariateDistribution( _gaussians[ n ].mean, _gaussians[ n ].covariance ) );
         }
         core::Buffer1D<size_t> sampled = core::sampling( pbs, nbPoints );
         
         out = std::vector< core::Buffer1D<double> >( nbPoints );
         for ( size_t n = 0; n < nbPoints; ++n )
         {
            out[ n ] = normals[ sampled[ n ] ].generate();
         }
      }

      /**
       @brief return the log likelihood of a point from the computed model
              Points requires operator[], size()
       */
      template <class Points>
      ComputingType likelihood( const Points& points ) const
      {
         ensure( _pointSize, "error: bad init?" );
         Matrix p( static_cast<size_t> ( points.size() ), _pointSize );
         for ( size_t y = 0; y < points.size(); ++y )
            for ( size_t x = 0; x < _pointSize; ++x )
               p( y, x ) = points[ y ][ x ];
         Matrix density, norm_density;
         _expectation( p, density, norm_density );

         ComputingType l = 0;
		   for ( size_t i = 0; i < p.sizey(); ++i )
		   {
			   ComputingType l2 = 0;
			   for ( size_t k = 0; k < _gaussians.size(); ++k )
				   l2 += _gaussians[ k ].weight * density( i, k );
			   l += log( l2 );
		   }
		   return l;
      }

      /**
       @brief clone the Gmm
       */
      void clone( const Gmm& gmm )
      {
         _gaussians = Gaussians( gmm._gaussians.size() );
         for ( size_t n = 0; n < gmm._gaussians.size(); ++n )
            _gaussians[ n ].clone( gmm._gaussians[ n ] );
      }

      /**
       @brief apply expectation maximization to optimize the model
       @param minDiffStop if the likelihood of 2 iterations is below this number, the algorithm is stopped
       @param pointSize the dimensionality of a point. It must be the same accross all the points.
       @param nbGaussians the number of gaussians to be used
       @param the number of times the EM algorithm is applied

       Points requires operator[], size()
       */
      template <class Points>
      void em( const Points& points, size_t pointSize, size_t nbGaussians, size_t nbIter, ComputingType minDiffStop = 0.1 )
      {
         {
            std::stringstream ss;
            ss << "Mixture of gaussian fitting using EM," <<
                  " nbGaussian=" << nbGaussians <<
                  " nbIter=" << nbIter <<
                  " minDiffStop=" << minDiffStop <<
                  " nbPoints=" << points.size();
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         _pointSize = pointSize;
         Matrix p( static_cast<size_t> ( points.size() ), _pointSize );
         for ( size_t y = 0; y < points.size(); ++y )
            for ( size_t x = 0; x < _pointSize; ++x )
               p( y, x ) = points[ y ][ x ];
         _gaussians = Gaussians( nbGaussians );
         _init( p, nbGaussians );

         {
            std::stringstream ss;
            ss << " KMeans initialization=" << std::endl;
            for ( size_t n = 0; n < nbGaussians; ++n )
            {
               ss << " gaussian weight=" << _gaussians[ n ].weight << std::endl
                  << " mean:" << std::endl;
               _gaussians[ n ].mean.print( ss );
               ss << " cov:" << std::endl;
               _gaussians[ n ].covariance.print( ss );
            }
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
         }

         Matrix density, norm_density;
         ComputingType likelihood = INT_MIN;
         ComputingType likelihood_old = likelihood;

         while ( nbIter-- > 0 )
         {
            _expectation( p, density, norm_density );
            _maximization( p, norm_density );
            likelihood_old = likelihood;
			   likelihood = _likelihood( p, density );
            //assert( likelihood_old - likelihood <= 0.01 );
            if ( core::absolute( likelihood_old - likelihood ) < minDiffStop )
               break;   // stop if the diff between old and new likelihood is les than the threshold

            {
               std::stringstream ss;
               ss << "likelihood=" << likelihood;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
            }

# ifdef NLL_GMM_DEBUG
            std::cout << "likelihood=" << likelihood <<std::endl;
# endif
         }

         // log the parameter's model
         std::stringstream ss;
         ss << "Gmm.em:" << nbGaussians << " gaussians" << std::endl;
         for ( size_t n = 0; n < nbGaussians; ++n )
         {
            ss << "gaussian weight=" << _gaussians[ n ].weight << std::endl
               << "mean:" << std::endl;
            _gaussians[ n ].mean.print( ss );
            ss << "cov:" << std::endl;
            _gaussians[ n ].covariance.print( ss );
         }
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
      }

      /**
       @brief write the gmm to a stream
       */
      void write( std::ostream& o ) const
      {
         core::write<size_t>( _pointSize, o );
         core::write<Gaussians>( _gaussians, o );
      }
      
      /**
       @brief read the gmm from a stream
       */
      void read( std::istream& i )
      {
         core::read<size_t>( _pointSize, i );
         core::read<Gaussians>( _gaussians, i );
      }

      /**
       @return the gaussians found
       */
      const Gaussians& getGaussians() const
      {
         return _gaussians;
      }

   private:
      void _init( Matrix& points, size_t nbGaussians )
      {
         typedef Matrix::value_type*      KPoint;
         typedef MetricEuclidian          KMetric;
         typedef std::vector<KPoint>      KPoints;

         typedef BuildKMeansUtility< KPoint,
                                     KMetric,
                                     KPoints,
                                     AllocatorKPoint > KMeansUtility;
         typedef KMeans<KMeansUtility> KMeans;

         KMetric metric( _pointSize );
         KMeansUtility utility( _pointSize, metric );
         KMeans kmeans( utility );

          KMeansUtility::Points kpoints( points.sizey() );
         for ( size_t n = 0; n < points.sizey(); ++n )
            kpoints[ n ] = &points( n, 0 );
         KMeans::KMeansResult kres = kmeans( kpoints, nbGaussians );

         std::vector<size_t> count( nbGaussians );
         std::vector<size_t> rang( points.sizey() );
         for ( size_t n = 0; n < points.sizey(); ++n )
         {
            rang[ n ] = kres.first[ n ];
            ++count[ rang[ n ] ];
         }
         core::sortRow( points, rang );
         size_t index = 0;
         _gaussians = Gaussians( nbGaussians );
         for ( size_t n = 0; n < nbGaussians; ++n )
         {
            _gaussians[ n ].covariance = core::covariance( points, index, index + count[ n ] - 1, &_gaussians[ n ].mean );
            _gaussians[ n ].weight = static_cast<ComputingType>( count[ n ] ) / points.sizey();
            index += count[ n ];
         }

         // free the memory allocated for the kmean centroids
         for ( size_t n = 0; n < nbGaussians; ++n )
            delete [] kres.second[ n ];
      }

      // covariance is approximated to its diagonal
      void _expectation( const Matrix& points, Matrix& out_density, Matrix& out_norm_density ) const
      {
         Matrix expectation( points.sizey(), _gaussians.size(), false );
         for ( size_t g = 0; g < _gaussians.size(); ++g )
         {
            Matrix covInv( _pointSize, _pointSize );
            ComputingType det = 1.0;
            for ( size_t n = 0; n < _pointSize; ++n )
            {
               ComputingType val = _gaussians[ g ].covariance( n, n );
               assert( val );  // singular covariance
               det *= val;
               covInv( n, n ) = 1.0 / val;
            }
            ComputingType normFactor =  1 / ( pow( 2 * core::PI, static_cast<f64>( _pointSize ) / 2 ) * sqrt( core::absolute( det ) ) );

            for ( size_t n = 0; n < points.sizey(); ++n )
            {
               ComputingType sum = 0;
               for ( size_t f = 0; f < _pointSize; ++f )
               {
                  ComputingType pmean = points( n, f ) - _gaussians[ g ].mean[ f ];
                  sum += ( pmean * covInv( f, f ) * pmean );
               }
               expectation( n, g ) = normFactor * exp( -0.5 * sum );
               if ( IS_NAN( expectation( n, g ) ) )
					   std::cout << "error: expectation couldn't be calculated acurately, you should restart it!" << std::endl;
            }
         }

         Matrix norm_density( points.sizey(), _gaussians.size() );
         for ( size_t g = 0; g < _gaussians.size(); ++g )
            for ( size_t i = 0; i < points.sizey(); ++i )
            {
               ComputingType accum = 0;   
               for ( size_t k = 0; k < _gaussians.size(); ++k )
                  accum += _gaussians[ k ].weight * expectation( i, k );
               //assert( accum );
               if ( fabs( accum ) <= 1e-8 )
                  norm_density( i, g ) = 1e-8;
               else
                  norm_density( i, g ) = _gaussians[ g ].weight * expectation( i, g ) / accum;
            }

         out_density = expectation;
         out_norm_density = norm_density;
      }

      void _maximization( const Matrix& points, const Matrix& expectation )
      {
         for ( size_t g = 0; g < _gaussians.size(); ++g )
         {
            // weight       
            ComputingType sum_p = 0;
            for ( size_t i = 0; i < points.sizey(); ++i )
               sum_p += expectation( i, g );
            _gaussians[ g ].weight = sum_p / points.sizey();
            
            // mean 
            assert( sum_p );
            Vector mean( _pointSize );
            for ( size_t i = 0; i < points.sizey(); ++i )
               for ( size_t k = 0; k < _pointSize; ++k )
                  mean[ k ] += expectation( i, g ) * points( i, k );
            core::generic_div_cte<ComputingType*> ( mean.getBuf(), sum_p, _pointSize );
            _gaussians[ g ].mean = mean;
            
            // cov // TODO : OPTIMIZE
            Matrix cov( _pointSize, _pointSize );
            Matrix v( _pointSize, 1, false );
			   for ( size_t i = 0; i < points.sizey(); ++i )
            {
               for ( size_t k2 = 0; k2 < _pointSize; ++k2 )
                  v( k2, 0 ) =  points( i, k2 ) - mean( k2 );
               Matrix v_trans( v.getBuf(), 1, _pointSize, false);
               Matrix mul = core::mul( v, v_trans );
               core::mul( mul, expectation( i, g ) );
               core::add( cov, mul );
            }
            core::div( cov, sum_p );
            _gaussians[ g ].covariance = cov;
         }
      }

      ComputingType _likelihood( const Matrix& points, const Matrix& expectation )
      {
         ComputingType l = 0;
		   for ( size_t i = 0; i < points.sizey(); ++i )
		   {
			   ComputingType l2 = 0;
			   for ( size_t k = 0; k < _gaussians.size(); ++k )
				   l2 += _gaussians[ k ].weight * expectation( i, k );
			   l += log( l2 );
		   }
		   return l;
      }


   private:
      size_t                 _pointSize;
      Gaussians            _gaussians;
   };
}
}

#endif
