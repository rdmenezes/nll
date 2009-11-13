#ifndef NLL_CORE_ALLOCATOR_ALIGNED_H_
# define NLL_CORE_ALLOCATOR_ALIGNED_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Provide a 16 byte aligned memory allocator
    @note T must be a basic type only as malloc/free is used (thus object no constructed)
    */
   template <class T>
   class Allocator16ByteAligned
   {
   public:
      typedef T value_type;
      typedef value_type* pointer;
      typedef const value_type* const_pointer;
      typedef value_type& reference;
      typedef const value_type& const_reference;
      typedef std::size_t size_type;
      typedef std::ptrdiff_t difference_type;

   public:
      template<typename U>
      struct rebind {
         typedef Allocator16ByteAligned<U> other;
      };

   public:
      inline explicit Allocator16ByteAligned()
      {
         STATIC_ASSERT( IsNativeType<T>::value );
      }

      inline ~Allocator16ByteAligned()
      {}

      inline explicit Allocator16ByteAligned( Allocator16ByteAligned const& )
      {
      }

      template<typename U>
      inline explicit Allocator16ByteAligned( Allocator16ByteAligned<U> const& )
      {
         STATIC_ASSERT( IsNativeType<T>::value );
      }

      inline pointer address( reference r )
      {
         return &r;
      }

      inline const_pointer address( const_reference r )
      {
         return &r;
      }

      inline pointer allocate( size_type cnt, typename std::allocator<void>::const_pointer = 0)
      { 
         // we are allocating 15 bytes to align data + a word to store original buffer location
         // we also store at the begining of the buffer the non aligned pointer
         // so we can deallocate it easily
         void* buf = malloc( sizeof( T ) * cnt + 15 + sizeof( void* ) );
         assert( buf );
         pointer ptr = reinterpret_cast<pointer>( reinterpret_cast<size_t> ( reinterpret_cast<char*>( buf ) + sizeof( void* ) + 15 ) & ~ (size_t) 0x0F );
         reinterpret_cast<void**>( ptr)[ -1 ] = buf;
         return ptr;
      }

      inline void deallocate(pointer p, size_type)
      { 
         free( reinterpret_cast<void**>( p )[ -1 ] );
      }
   };
}
}

#endif