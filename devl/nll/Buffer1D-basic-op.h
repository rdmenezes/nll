#ifndef NLL_CORE_BUFFER1D_BASIC_OP_H_
# define NLL_CORE_BUFFER1D_BASIC_OP_H_

namespace nll
{
namespace core
{
   template <class T, class Mapper, class Allocator>
   Buffer1D<T, Mapper, Allocator>& operator+=( Buffer1D<T, Mapper, Allocator>& dst, const Buffer1D<T, Mapper, Allocator>& src )
   {
      assert( dst.size() == src.size() );
      generic_add<T*, const T*>( dst.getBuf(), src.getBuf(), dst.size() );
      return dst;
   }

   template <class T, class Mapper, class Allocator>
   Buffer1D<T, Mapper, Allocator> operator+( const Buffer1D<T, Mapper, Allocator>& src1, const Buffer1D<T, Mapper, Allocator>& src2 )
   {
      assert( src1.size() == src2.size() );
      Buffer1D<T, Mapper, Allocator> res( src.size(), false, src1.getAllocator() );

      T* bufDst = res.getBuf();
      const T* bufSrc1 = src1.getBuf();
      const T* bufSrc2 = src2.getBuf();

      for ( ui32 n = 0; n < src.size(); ++n )
      {
         bufDst[ n ] = bufSrc1[ n ] + bufSrc2[ n ];
      }

      return res;
   }

   template <class T, class Mapper, class Allocator>
   double norm2( const Buffer1D<T, Mapper, Allocator>& src1 )
   {
      return generic_norm2<const T*, double> ( src1.getBuf(), src1.size() );
   }

   template <class T, class Mapper, class Allocator>
   double dot( const Buffer1D<T, Mapper, Allocator>& src1, const Buffer1D<T, Mapper, Allocator>& src2 )
   {
      assert( src1.size() == src2.size() );
      const T* bufSrc1 = src1.getBuf();
      const T* bufSrc2 = src2.getBuf();

      double accum = 0;
      for ( ui32 n = 0; n < src1.size(); ++n )
      {
         accum += bufSrc1[ n ] * bufSrc2[ n ];
      }

      return accum;
   }

   template <class T>
   double dot( const std::vector<T>& src1, const std::vector<T>& src2 )
   {
      assert( src1.size() == src2.size() );

      double accum = 0;
      for ( ui32 n = 0; n < src1.size(); ++n )
      {
         accum += src1[ n ] * src2[ n ];
      }

      return accum;
   }

   template <class T, class Mapper, class Allocator>
   double norm2( const Buffer1D<T, Mapper, Allocator>& src1, const Buffer1D<T, Mapper, Allocator>& src2 )
   {
      assert( src1.size() == src2.size() );
      return generic_norm2<const T*, double> ( src1.getBuf(), src2.getBuf(), src1.size() );
   }

   template <class T, class Mapper, class Allocator>
   Buffer1D<T, Mapper, Allocator> operator-( const Buffer1D<T, Mapper, Allocator>& src1, const Buffer1D<T, Mapper, Allocator>& src2 )
   {
      assert( src1.size() == src2.size() );
      Buffer1D<T, Mapper, Allocator> res( src.size(), false, src1.getAllocator() );

      T* bufDst = res.getBuf();
      const T* bufSrc1 = src1.getBuf();
      const T* bufSrc2 = src2.getBuf();

      for ( ui32 n = 0; n < src.size(); ++n )
      {
         bufDst[ n ] = bufSrc1[ n ] - bufSrc2[ n ];
      }

      return res;
   }

   template <class T, class Mapper, class Allocator>
   Buffer1D<T, Mapper, Allocator>& operator-=( Buffer1D<T, Mapper, Allocator>& dst, const Buffer1D<T, Mapper, Allocator>& src )
   {
      assert( dst.size() == src.size() );
      generic_sub<T*, const T*>( dst.getBuf(), src.getBuf(), dst.size() );
      return dst;
   }

   template <class T, class Mapper, class Allocator>
   Buffer1D<T, Mapper, Allocator>& operator*=( Buffer1D<T, Mapper, Allocator>& dst, T val )
   {
      generic_mul_cte<T*>( dst.getBuf(), val, dst.size() );
      return dst;
   }

   template <class T, class Mapper, class Allocator>
   Buffer1D<T, Mapper, Allocator>& operator/=( Buffer1D<T, Mapper, Allocator>& dst, T val )
   {
      assert( val != 0 );
      generic_div_cte<T*>( dst.getBuf(), val, dst.size() );
      return dst;
   }
}
}

#endif