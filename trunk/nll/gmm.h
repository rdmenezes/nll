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

         MetricEuclidian( ui32 size ) : _size( size )
         {}

         double distance( const Point& p1, const Point& p2 )
         {
            return nll::core::generic_norm2<Point, nll::f64>( p1, p2, _size );
         }

      private:
         ui32 _size;
      };

      /**
       It is the allocator used by the kmeans initialization step
       */
      struct AllocatorKPoint
      {
         typedef f64          value_type;
         typedef value_type*  Point;

         static Point allocate( ui32 size )
         {
            return new value_type[ size ];
         }

         static void deallocate( const Point p )
         {
            delete [] p;
         }
      };

      typedef f64                                                             ComputingType;
      typedef core::Buffer1D<ComputingType>                                   Vector;
      typedef core::Matrix<ComputingType, core::IndexMapperRowMajorFlat2D>    Matrix;

      struct            Gaussian
      {
         Matrix         covariance;
         Vector         mean;
         ComputingType  weight;

         Gaussian( const Matrix& m, const Vector& c, const ComputingType w ) :
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
      Gmm() : _pointSize( 0 )
      {}

      /**
       @brief return the likelihood of a point from the computed model
              Points requires operator[], size()
       */
      template <class Points>
      ComputingType likelihood( const Points& points ) const
      {
         ensure( _pointSize, "error: bad init?" );
         Matrix p( static_cast<ui32> ( points.size() ), _pointSize );
         for ( ui32 y = 0; y < points.size(); ++y )
            for ( ui32 x = 0; x < _pointSize; ++x )
               p( y, x ) = points[ y ][ x ];
         Matrix density, norm_density;
         _expectation( p, density, norm_density );

         ComputingType l = 0;
		   for ( ui32 i = 0; i < p.sizey(); ++i )
		   {
			   ComputingType l2 = 0;
			   for ( ui32 k = 0; k < _gaussians.size(); ++k )
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
         for ( ui32 n = 0; n < gmm._gaussians.size(); ++n )
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
      void em( const Points& points, ui32 pointSize, ui32 nbGaussians, ui32 nbIter, ComputingType minDiffStop = 0.1 )
      {
         _pointSize = pointSize;
         Matrix p( static_cast<ui32> ( points.size() ), _pointSize );
         for ( ui32 y = 0; y < points.size(); ++y )
            for ( ui32 x = 0; x < _pointSize; ++x )
               p( y, x ) = points[ y ][ x ];
         _gaussians = Gaussians( nbGaussians );
         _init( p, nbGaussians );

         Matrix density, norm_density;
         ComputingType likelihood = INT_MIN;
         ComputingType likelihood_old = likelihood;

         do
         {
            _expectation( p, density, norm_density );
            _maximization( p, norm_density );
            likelihood_old = likelihood;
			   likelihood = _likelihood( p, density );
            assert( likelihood_old - likelihood <= 0.01 );
            if ( core::absolute( likelihood_old - likelihood ) < minDiffStop )
               break;   // stop if the diff between old and new likelihood is les than the threshold
# ifdef NLL_GMM_DEBUG
            std::cout << "likelihood=" << likelihood <<std::endl;
# endif
         } while ( nbIter-- );
      }

      /**
       @brief write the gmm to a stream
       */
      void write( std::ostream& o ) const
      {
         core::write<ui32>( _pointSize, o );
         core::write<Gaussians>( _gaussians, o );
      }
      
      /**
       @brief read the gmm from a stream
       */
      void read( std::istream& i )
      {
         core::read<ui32>( _pointSize, i );
         core::read<Gaussians>( _gaussians, i );
      }

   private:
      void _init( Matrix& points, ui32 nbGaussians )
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
         for ( ui32 n = 0; n < points.sizey(); ++n )
            kpoints[ n ] = &points( n, 0 );
         KMeans::KMeansResult kres = kmeans( kpoints, nbGaussians );

         std::vector<ui32> count( nbGaussians );
         std::vector<ui32> rang( points.sizey() );
         for ( ui32 n = 0; n < points.sizey(); ++n )
         {
            rang[ n ] = kres.first[ n ];
            ++count[ rang[ n ] ];
         }
         core::sortRow( points, rang );
         ui32 index = 0;
         _gaussians = Gaussians( nbGaussians );
         for ( ui32 n = 0; n < nbGaussians; ++n )
         {
            _gaussians[ n ].covariance = core::covariance( points, index, index + count[ n ] - 1, &_gaussians[ n ].mean );
            _gaussians[ n ].weight = static_cast<ComputingType>( count[ n ] ) / points.sizey();
            index += count[ n ];
         }

         // free the memory allocated for the kmean centroids
         for ( ui32 n = 0; n < nbGaussians; ++n )
            delete [] kres.second[ n ];
      }

      // covariance is approximated to its diagonal
      void _expectation( const Matrix& points, Matrix& out_density, Matrix& out_norm_density ) const
      {
         Matrix expectation( points.sizey(), _gaussians.size(), false );
         for ( ui32 g = 0; g < _gaussians.size(); ++g )
         {
            Matrix covInv( _pointSize, _pointSize );
            ComputingType det = 1.0;
            for ( ui32 n = 0; n < _pointSize; ++n )
            {
               ComputingType val = _gaussians[ g ].covariance( n, n );
               assert( val );  // singular covariance
               det *= val;
               covInv( n, n ) = 1.0 / val;
            }
            ComputingType normFactor =  1 / ( pow( 2 * core::PI, static_cast<f64>( _pointSize ) / 2 ) * sqrt( core::absolute( det ) ) );

            for ( ui32 n = 0; n < points.sizey(); ++n )
            {
               ComputingType sum = 0;
               for ( ui32 f = 0; f < _pointSize; ++f )
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
         for ( ui32 g = 0; g < _gaussians.size(); ++g )
            for ( ui32 i = 0; i < points.sizey(); ++i )
            {
               ComputingType accum = 0;   
               for ( ui32 k = 0; k < _gaussians.size(); ++k )
                  accum += _gaussians[ k ].weight * expectation( i, k );
               assert( accum );
               norm_density( i, g ) = _gaussians[ g ].weight * expectation( i, g ) / accum;
            }

         out_density = expectation;
         out_norm_density = norm_density;
      }

      void _maximization( const Matrix& points, const Matrix& expectation )
      {
         for ( ui32 g = 0; g < _gaussians.size(); ++g )
         {
            // weight       
            ComputingType sum_p = 0;
            for ( ui32 i = 0; i < points.sizey(); ++i )
               sum_p += expectation( i, g );
            _gaussians[ g ].weight = sum_p / points.sizey();
            
            // mean 
            assert( sum_p );
            Vector mean( _pointSize );
            for ( ui32 i = 0; i < points.sizey(); ++i )
               for ( ui32 k = 0; k < _pointSize; ++k )
                  mean[ k ] += expectation( i, g ) * points( i, k );
            core::generic_div_cte<ComputingType*> ( mean.getBuf(), sum_p, _pointSize );
            _gaussians[ g ].mean = mean;
            
            // cov // TODO : OPTIMIZE
            Matrix cov( _pointSize, _pointSize );
            Matrix v( _pointSize, 1, false );
			   for ( ui32 i = 0; i < points.sizey(); ++i )
            {
               for ( ui32 k2 = 0; k2 < _pointSize; ++k2 )
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
		   for ( ui32 i = 0; i < points.sizey(); ++i )
		   {
			   ComputingType l2 = 0;
			   for ( ui32 k = 0; k < _gaussians.size(); ++k )
				   l2 += _gaussians[ k ].weight * expectation( i, k );
			   l += log( l2 );
		   }
		   return l;
      }


   private:
      ui32                 _pointSize;
      Gaussians            _gaussians;
   };
}
}

#endif
