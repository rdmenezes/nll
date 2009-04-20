#ifndef NLL_SEQUENCE_CONVERTER_H_
# define NLL_SEQUENCE_CONVERTER_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief convert a sequence type to another. T2 needs to provide value_type, operator[] and T2(unsigned int) which allocate memory for N elements
   */
   template <class T1, class T2>
   inline T2 convert( const T1& arg, ui32 size )
   {
      T2 buf( size );
      for ( ui32 n = 0; n < size; ++n )
         buf[ n ] = static_cast<typename T2::value_type>( arg[ n ] );
      return buf;
   }

   /**
    @ingroup core
    @brief convert a sequence type to another. T2 needs to provide value_type, operator[] and T2(unsigned int) which allocate memory for N elements
   */
   template <class T1, class T2>
   inline void convert( const T1& arg, T2& out, ui32 size )
   {
      out = convert<T1, T2>( arg, size );
   }
}
}

#endif
