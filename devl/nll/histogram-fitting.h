#ifndef NLL_ALGORITHM_HISTOGRAM_FITTING_H_
# define NLL_ALGORITHM_HISTOGRAM_FITTING_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Helper to fit gaussian functions on a histogram, assumed the original data being unavailable
    */
   class HistogramFitGaussian
   {
   public:
      /**
       @brief Generate gaussian data from this histogram. Baiscally, if the data were binned, we should find
              an histogram similar to the one given as input
       @param outData a vector<value, histogram index> is generated
       @param noise the gaussian noise added to the data
       */
      template <class Histogram>
      void generateData( const Histogram& h, std::vector< std::vector<double> >& outData, ui32 nbPoints = 2000, double noise = 0.1, double* scaleOut = 0 )
      {
         // compute the probability that a particular bin generates a point
         std::vector<double> pb( h.size() );
         double sum = 0;
         double max = std::numeric_limits<double>::min();
         for ( ui32 n = 0; n < static_cast<ui32>( h.size() ); ++n )
         {
            sum += h[ n ];
            if ( h[ n ] > max )
            {
               max = h[ n ];
            }
         }

         // the two dimensions can be so different that we need to normalize it so that the histogram size and content
         // can be compared with minimal bias...
         const double scale = 0.1 * static_cast<double>( h.size() ) / max;
         for ( ui32 n = 0; n < static_cast<ui32>( h.size() ); ++n )
         {
            pb[ n ] = h[ n ] / sum;
         }

         if ( scaleOut )
         {
            *scaleOut = scale;
         }

         // generate the points
         outData.reserve( nbPoints );
         core::Buffer1D<ui32> sampling = core::sampling( pb, nbPoints );
         for ( ui32 n = 0; n < nbPoints; ++n )
         {
            const ui32 index = sampling[ n ];
            const double mean = scale * h[ index ];
            const double x = core::generateGaussianDistribution( mean, noise * mean );
            const double y = index + core::generateGaussianDistribution( 0, 0.5 );

            outData.push_back( core::make_vector<double>( x, y ) );
         }
      }

      /**
       @brief Fits the histogram using a mixture of gaussians, returns the likelyhood the histogram
              has been generated by the fitted gaussians
       */
      template <class Histogram>
      double fit( const Histogram& h, ui32 nbGaussians, ui32 nbIter = 0, ui32 nbPoints = 1000, double noise = 0.01 )
      {
         double scale;
         std::vector< std::vector<double> > points;
         generateData( h, points, nbPoints, noise, &scale );
         
         Gmm gmm;
         gmm.em( points, 2, nbGaussians, nbIter, 1e-3 );

         _gaussians.clear();
         ensure( gmm.getGaussians().size() == nbGaussians, "wrong number of gaussians, something went wrong!" );
         for ( ui32 n = 0; n < nbGaussians; ++n )
         {
            Gmm::Gaussian g = gmm.getGaussians()[ n ];
            g.mean[ 0 ] /= scale;
            _gaussians.push_back( g );
         }

         return gmm.likelihood( points );
      }

      /**
       @brief Returns the computed gaussians. Note that we are using two variables:
              [ 0 ] : the histogram count
              [ 1 ] : the position in the histogram
       */
      const std::vector<Gmm::Gaussian>& getGaussians() const
      {
         return _gaussians;
      }

   private:
      std::vector<Gmm::Gaussian>  _gaussians;
   };
}
}

#endif