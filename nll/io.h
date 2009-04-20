#ifndef NLL_IO_H_
# define NLL_IO_H_

# include <iostream>
# include <vector>
# include "type-traits.h"

namespace nll
{
namespace core
{
   template <class T> void write( typename BestConstArgType<T>::type val, std::ostream& f );
   template <class T> void read( T& val, std::istream& f );

   /**
    @ingroup core
    @brief write plain data [native types only!!] to a stream
    */
   template <class T>
   void writePlainData( const T& data, std::ostream& f )
   {
      f.write( ( i8* )&data, sizeof( T ) );
   }

   /**
    @ingroup core
    @brief read plain data [native types only!!] to a stream
    */
   template <class T>
   void readPlainData( T& out_data, std::istream& f )
   {
      f.read( ( i8* )&out_data, sizeof( T ) );
   }

   template <class T, bool isNative>
   struct _write
   {
      _write( typename BestConstArgType<T>::type val, std::ostream& f )
      {
         val.write( f );
      }
   };

   template <class T>
   struct _write<T, true>
   {
      _write( typename BestConstArgType<T>::type val, std::ostream& f )
      {
         f.write( (i8*)( &val ), sizeof ( val ) );
      }
   };

   template <class T>
   struct _write<std::vector<T>, false>
   {
      _write( const std::vector<T>& val, std::ostream& f )
      {
         ui32 size = static_cast<ui32>( val.size() );
         write<ui32>( size, f );
         for ( ui32 n = 0; n < val.size(); ++n )
            write<T>( val[ n ], f );
      }
   };

   /**
    @ingroup core
    @brief write data to a stream. If native type, write it using stream functions, else the type needs to provide write()
    */
   template <class T> void write( typename BestConstArgType<T>::type val, std::ostream& f )
   {
      _write<T, IsNativeType<T>::value>(val, f);
   }

   template <class T, bool isNative>
   struct _read
   {
      _read( T& val, std::istream& f)
      {
         val.read( f );
      }
   };

   template <class T>
   struct _read<T, true>
   {
      _read( T& val, std::istream& f)
      {
         f.read( (i8*)( &val ), sizeof ( val ) );
      }
   };

   template <class T>
   struct _read<std::vector<T>, false>
   {
      _read( std::vector<T>& val, std::istream& i )
      {
         ui32 size = 0;
         read<ui32>( size, i );
         assert( size );
         val = std::vector<T>( size );
         for ( ui32 n = 0; n < size; ++n )
            read<T>( val[ n ], i );
      }
   };

   /**
    @ingroup core
    @brief write data to a stream. If native type, write it using stream functions, else the type needs to provide write()
    */
   template <class T> void read( T& val, std::istream& f )
   {
      _read<T, IsNativeType<T>::value>(val, f);
   }
}
}

#endif
