#ifndef NLL_CORE_MATH_KURTOSIS_H_
# define NLL_CORE_MATH_KURTOSIS_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief returns the kurtosis of a random variable
    @see http://en.wikipedia.org/wiki/Kurtosis

    Kurtosis of known distribution:
      - Laplace distribution, a.k.a. double exponential distribution, excess kurtosis = 3
      - hyperbolic secant distribution, excess kurtosis = 2
      - logistic distribution, excess kurtosis = 1.2
      - normal distribution, excess kurtosis = 0
      - raised cosine distribution, excess kurtosis = -0.593762...
      - Wigner semicircle distribution, excess kurtosis = -1
      - uniform distribution, excess kurtosis = -1.2. 
    */
   template <class Points>
   double kurtosis( const Points& p, double* outMean = 0, double* outVar = 0 )
   {
      ensure( p.size(), "empty size!" );

      double mean = 0;
      for ( ui32 n = 0; n < p.size(); ++n )
      {
         mean += p[ n ];
      }
      mean /= p.size();
      
      double var = 0;
      double e = 0;
      for ( ui32 n = 0; n < p.size(); ++n )
      {
         const double val = core::sqr( p[ n ] - mean );
         var += val;
         e += core::sqr( val );
      }
      var /= p.size();

      if ( outMean )
      {
         *outMean = mean;
      }

      if ( outVar )
      {
         *outVar = var;
      }

      return e / ( core::sqr( var ) * p.size() ) - 3;
   }
}
}

#endif
