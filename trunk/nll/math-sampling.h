#ifndef NLL_MATH_SAMPLING_H_
# define NLL_MATH_SAMPLING_H_

# include <algorithm>

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Sampling of a set of points according to a probability for being sampled.
    @param p the probabilities to be used. It <b>must</b> sums to 1
    @param nbSampledPoints the number of points to generate.
    @return the index of the selected points

    Complexity is n1 * log n2 (n2 the number of points to be selected, n1 in the probabilities)
    <code>Probabilities</code> types needs to define:
    - <code>floatingType operator[]( ui32 n ) const</code>
    - <code>ui32 size() const</code>
    */
   template <class Probabilities>
   Buffer1D<ui32> sampling( const Probabilities& p, ui32 nbSampledPoints )
   {
      if ( !nbSampledPoints )
         return Buffer1D<ui32>();
      std::vector<double> sd( p.size() );
      double prob = 0;
      sd[ 0 ] = 0;
      const int PRECISION = 10000;
      for ( ui32 n = 0; n < p.size(); ++n )
      {
         prob += p[ n ] * PRECISION;
         sd[ n ] = prob;
      }
      ensure( prob <= PRECISION + 0.01, "probability must sum to 1" );
      
      Buffer1D<ui32> points( nbSampledPoints );
      for ( ui32 n = 0; n < nbSampledPoints; ++n )
      {
         int point = rand() % PRECISION;
         std::vector<double>::const_iterator it = std::lower_bound( sd.begin(), sd.end(), point );
         std::vector<double>::difference_type diff = it - sd.begin();
         points[ n ] = static_cast<ui32>( diff );
      }
      return points;
   }
}
}

#endif
