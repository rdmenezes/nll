#ifndef NLL_MATH_CORRELATION_H_
# define NLL_MATH_CORRELATION_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Compute the correlation of 2 vectors.
    */
   template <class T, class Mapper>
   double correlation( const Matrix<T, Mapper>& v1, const Matrix<T, Mapper>& v2 )
   {
      // assert if it is really a vector, with the same dimention
      assert( std::min( v1.sizex(), v1.sizey() ) == 1 );
      assert( std::min( v2.sizex(), v2.sizey() ) == 1 );
      assert( v1.size() == v2.size() );
      assert( v1.size() );

      double mean1 = 0, mean2 = 0;
      for ( ui32 n = 0; n < v1.size(); ++n )
      {
         mean1 += v1[ n ];
         mean2 += v2[ n ];
      }
      mean1 /= v1.size();
      mean2 /= v2.size();

      double cov = 0, var1 = 0, var2 = 0;
      for ( ui32 n = 0; n < v1.size(); ++n )
      {
         const double t1 = v1[ n ] - mean1;
         const double t2 = v2[ n ] - mean2;
         cov += t1 * t2;
         var1 += t1 * t1;
         var2 += t2 * t2;
      }
      return cov / sqrt( var1 * var2 );
   }
}
}

#endif
