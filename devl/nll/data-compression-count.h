/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2012, Ludovic Sibille
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ludovic Sibille nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY LUDOVIC SIBILLE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NLL_ALGORITHM_DATA_COMPRESSION_COUNT_H_
# define NLL_ALGORITHM_DATA_COMPRESSION_COUNT_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Data compressor based on counting similar value, efficient for data having long contiguous sequence of the same value

    1 byte for the chunk ID (Similar/Enumerate)
    2 bytes for the size int number of <T> represented by this chunk
         sizeof(T) bytes for storing the Similar chunk
      OR sizeof(T) * number of (T) for enumerate chunk
    */
   class DataCompressorCount
   {
   public:
      typedef ui16    ChunkSizeType;

      template <class T>
      struct Chunk
      {
         enum ChunkId
         {
            CHUNK_SIMILAR     = 0,
            CHUNK_ENUMERATE   = 1
         };

         // write to a stream the chunk representation
         virtual void write( std::ostream& f ) const = 0;

         // read from a stream the chunk representation. It is assumed we already know the chunkId
         virtual void read( std::istream& f ) = 0;

         // return the number of items the chunk is representing
         virtual size_t numberOfValuesRepresented() const = 0;

         // decode the chunks to a raw buffer. It is must be preallocated with the correct size (i.e. at least <numberOfValuesRepresented>)!
         virtual void decode( T* buf ) const = 0;

         virtual ~Chunk()
         {}
      };

      /**
       @brief encode a chunk composed of the same values
       */
      template <class T>
      struct ChunkSimilar : public Chunk<T>
      {
         ChunkSimilar( const T v, ChunkSizeType s ) : value( v ), size( s )
         {
            STATIC_ASSERT( core::IsPOD<T>::value );  // this is only working for POD type
         }

         ChunkSimilar()
         {}

         virtual void write( std::ostream& f ) const
         {
            static const ui8 chunkId = 0;
            core::write<ui8>( chunkId, f );

            core::write<ChunkSizeType>( size, f );
            core::write<T>( value, f );
         }

         virtual void read( std::istream& f )
         {
            core::read<ChunkSizeType>( size, f );
            core::read<T>( value, f );
         }

         virtual size_t numberOfValuesRepresented() const 
         {
            return size;
         }

         virtual void decode( T* buf ) const
         {
            for ( size_t n = 0; n < size; ++n )
            {
               buf[ n ] = value;
            }
         }

         T              value;
         ChunkSizeType  size;
      };

      /**
       @brief Enumerate on the values
       */
      template <class T>
      struct ChunkEnumerate : public Chunk<T>
      {
         ChunkEnumerate()
         {}

         ChunkEnumerate( std::auto_ptr<T> vs, ChunkSizeType s ) : values( vs ), size( s )
         {
            STATIC_ASSERT( core::IsPOD<T>::value );  // this is only working for POD type
         }

         virtual void write( std::ostream& f ) const
         {
            static const ui8 chunkId = 1;
            core::write<ui8>( chunkId, f );

            core::write<ChunkSizeType>( size, f );
            for ( ui32 n = 0; n < size; ++n )
            {
               core::write<T>( values.get()[ n ], f );
            }
         }

         virtual void read( std::istream& f )
         {
            core::read<ChunkSizeType>( size, f );
            std::auto_ptr<T> ptr( new T[ size ] );
            for ( ui32 n = 0; n < size; ++n )
            {
               core::read<T>( ptr.get()[ n ], f );
            }
            values = ptr;
         }

         virtual size_t numberOfValuesRepresented() const 
         {
            return size;
         }

         virtual void decode( T* buf ) const
         {
            for ( size_t n = 0; n < size; ++n )
            {
               buf[ n ] = values.get()[ n ];
            }
         }

         std::auto_ptr<T>     values;
         ChunkSizeType        size;
      };

      template <class T>
      class Chunks
      {
         typedef std::vector< Chunk<T>* > Storage;
      public:
         void push_back( Chunk<T>* c )
         {
            _chunks.push_back( c );
         }

         ~Chunks()
         {
            clear();
         }

         size_t size() const
         {
            return _chunks.size();
         }

         const Chunk<T>& operator[]( size_t index ) const
         {
            return _chunks[ index ];
         }

         void clear()
         {
            for ( Storage::iterator it = _chunks.begin(); it != _chunks.end(); ++it )
            {
               delete *it;
            }
            _chunks.clear();
         }

         void write( std::ostream& stream ) const
         {
            const size_t nbChunks = _chunks.size();
            core::write<size_t>( nbChunks, stream );
            for ( Storage::const_iterator it = _chunks.begin(); it != _chunks.end(); ++it )
            {
               (*it)->write( stream );
            }
         }

         void read( std::istream& stream )
         {
            if ( !stream.good() )
               return;
            _chunks.clear();
            size_t nbChunks = 0;
            core::read<size_t>( nbChunks, stream );
            for ( size_t n = 0; n < nbChunks; ++n )
            {
               bool isEof = stream.eof();
               ensure( !isEof, "eof before all chunks were read" );
               ui8 type = 255;
               core::read<ui8>( type, stream );
               ensure( type < 2, "invalid chunk id. Data corrupted." );

               Chunk<T>* c = 0;
               if ( type == (int)Chunk<T>::CHUNK_SIMILAR )
               {
                  c = new ChunkSimilar<T>();
                  c->read( stream );
               } else if ( type == (int)Chunk<T>::CHUNK_ENUMERATE )
               {
                  c = new ChunkEnumerate<T>();
                  c->read( stream );
               }
               _chunks.push_back( c );
            }
         }

         void decode( T* buf ) const
         {
            for ( Storage::const_iterator it = _chunks.begin(); it != _chunks.end(); ++it )
            {
               (*it)->decode( buf );
               buf += (*it)->numberOfValuesRepresented();
            }
         }

         size_t dataSize() const
         {
            size_t s = 0;
            for ( Storage::const_iterator it = _chunks.begin(); it != _chunks.end(); ++it )
            {
               s += (*it)->numberOfValuesRepresented();
            }
            return s;
         }

      private:
         Storage _chunks;
      };

      DataCompressorCount( size_t nbSimilarValueToCreateSimilarChunk = 10 ) : _nbSimilarValueToCreateSimilarChunk( nbSimilarValueToCreateSimilarChunk )
      {}

   private:
      template <class Type>
      void _mergeChunks( Chunks<Type>& ) const
      {
         // now merge all the small chunks as they are not efficient enough

         // TODO
      }

   public:
      template <class const_iterator>
      void compress( const_iterator begin, const_iterator end, Chunks<typename std::iterator_traits<const_iterator>::value_type>& chunks_out ) const
      {
         typedef typename std::iterator_traits<const_iterator>::value_type  T;
         chunks_out.clear();

         for ( const_iterator it = begin; it != end; )
         {
            const_iterator similar = it;
            for ( ; similar != end && *it == *similar; ++similar )
            {}

            size_t nbSimilarValues = similar - it;
            if ( nbSimilarValues >= 2 )
            {
               const size_t maxElements = std::numeric_limits<ChunkSizeType>::max();
               while ( 1 )
               {
                  const size_t nbElements = std::min( maxElements, nbSimilarValues );
                  chunks_out.push_back( new ChunkSimilar<T>( *it, (ui16)nbElements ) );
                  if ( nbElements <= maxElements )
                     break;
                  nbSimilarValues -= nbElements;
               }

               it = similar;
            } else {
               if ( similar == end )
               {
                  // handle special case: the last value
                  size_t nbPossibleEnumeration = end - it;
                  std::auto_ptr<T> ptr( new T[ nbPossibleEnumeration ] );
                  for ( size_t index = 0; index < nbPossibleEnumeration; ++index, ++it )
                  {
                     const T val = *it;
                     ptr.get()[ index ] = val;
                  }
                  chunks_out.push_back( new ChunkEnumerate<T>( ptr, (ui16)nbPossibleEnumeration ) );
                  break;
               } else {
                  // general case: we have values at the end...
                  const_iterator dissimilar = similar + 1;
                  T last = *similar;
                  for ( ; dissimilar != end && last != *dissimilar; ++dissimilar )
                  {
                     last = *dissimilar;
                  }
                  const size_t nbPossibleEnumeration = dissimilar - similar;
                  std::auto_ptr<T> ptr( new T[ nbPossibleEnumeration ] );
                  const_iterator copy = it;
                  for ( size_t index = 0; index < nbPossibleEnumeration; ++index, ++copy )
                  {
                     const T val = *copy;
                     ptr.get()[ index ] = val;
                  }
                  chunks_out.push_back( new ChunkEnumerate<T>( ptr, (ui16)nbPossibleEnumeration ) );
                  it = copy;
               }
            }

            _mergeChunks( chunks_out );
         }
      }

   private:
      size_t  _nbSimilarValueToCreateSimilarChunk;
   };
}
}

#endif
