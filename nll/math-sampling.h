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
      std::vector<double> sd( p.size() + 1 );
      double prob = 0;
      for ( ui32 n = 0; n < p.size(); ++n )
      {
         prob += p[ n ];
         sd[ n + 1 ] = prob;
      }
      sd[ p.size() ] = 10; // we set an impossible "probability" so we are sure we won't miss the last one
      ensure( fabs( prob - 1 ) <= 0.01, "probability must sum to 1" );
      
      Buffer1D<ui32> points( nbSampledPoints );
      for ( ui32 n = 0; n < nbSampledPoints; ++n )
      {
         double point = static_cast<double>( rand() ) / RAND_MAX;
         size_t nn = 0;
         // we don't need to test the end of the buffer as it is not possible to reach it!
         for ( ; point >= sd[ nn ]; ++nn )
            ;
         points[ n ] = static_cast<ui32>( nn - 1 );
         assert( points[ n ] < p.size() );
      }
      return points;
   }
}
}

#endif
