#include <nll/nll.h>
#include <tester/register.h>
#include <vector>
#include <sstream>
#include "config.h"

namespace nll
{
namespace imaging
{
   /**
    @brief Rescale slope and intercept values
    */
   struct Rsi
   {
      Rsi( double s, double i ) : slope( s ), intercept( i )
      {}

      Rsi()
      {}

      virtual void write( std::ostream& f ) const
      {
         core::write<double>( slope, f );
         core::write<double>( intercept, f );
      }

      virtual void read( std::istream& f )
      {
         core::read<double>( slope, f );
         core::read<double>( intercept, f );
      }

      double   slope;
      double   intercept;
   };

   /**
    @brief This class will discretize a volume, typically of floating values to a <integer, RSI> representation

    volume( x, y, z ) is almost equal to volumeOut( x, y, z ) * rsi[ z ].slope + rsi[ z ].intercept
    */
   class VolumeDiscretizer
   {
   public:
      template <class Volume, class DiscreteType>
      void discretize( const Volume& volume, VolumeMemoryBuffer<DiscreteType>& volumeOut, std::vector<Rsi>& rsiOut )
      {
         volumeOut = VolumeMemoryBuffer<DiscreteType>( volume.getSize()[ 0 ], volume.getSize()[ 1 ], volume.getSize()[ 2 ], false );
         rsiOut.clear();
         rsiOut.reserve( volume.getSize()[ 2 ] );

         typedef typename Volume::ConstDirectionalIterator                      ConstIterator;
         typedef typename VolumeMemoryBuffer<DiscreteType>::DirectionalIterator Iterator;
         const double range = std::numeric_limits<DiscreteType>::max() - std::numeric_limits<DiscreteType>::min() + 1;
         for ( ui32 z = 0; z < volume.getSize()[ 2 ]; ++z )
         {
            // get the min/max value
            double min = std::numeric_limits<double>::max();
            double max = std::numeric_limits<double>::min();
            for ( ui32 y = 0; y < volume.getSize()[ 1 ]; ++y )
            {
               ConstIterator it = volume.getIterator( 0, y, z );
               for ( ui32 x = 0; x < volume.getSize()[ 0 ]; ++x )
               {
                  min = std::min<double>( min, *it );
                  max = std::max<double>( max, *it );
                  it.addx();
               }
            }

            // compute the RSI having the maximum spread to minimize the discretization error
            const double diff = max - min;
            const double intercept = min;
            const double slope = ( diff < 1e-8 ) ? 1.0 : diff / ( range - 1 );   // here we want max (-min as we care only about the range) value to be mapped to the last discrete value of <DiscreteType>

            // finally discretize them
            for ( ui32 y = 0; y < volume.getSize()[ 1 ]; ++y )
            {
               Iterator outIt = volumeOut.getIterator( 0, y, z );
               ConstIterator it = volume.getIterator( 0, y, z );
               for ( ui32 x = 0; x < volume.getSize()[ 0 ]; ++x )
               {
                  const typename Volume::value_type valOrig = *it;
                  const double valComputed = ( static_cast<double>( valOrig ) - intercept ) / slope;
                  *outIt = static_cast<DiscreteType>( valComputed );
                  it.addx();
                  outIt.addx();
               }
            }
            rsiOut.push_back( Rsi( slope, intercept ) );
         }
      }
   };

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
      void _mergeChunks( Chunks<Type>& chunks ) const
      {
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
                  chunks_out.push_back( new ChunkSimilar<T>( *it, nbElements ) );
                  if ( nbElements <= maxElements )
                     break;
                  nbSimilarValues -= nbElements;
               }

               it = similar;
            } else {
               const_iterator dissimilar;
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
                  chunks_out.push_back( new ChunkEnumerate<T>( ptr, nbPossibleEnumeration ) );
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
                  chunks_out.push_back( new ChunkEnumerate<T>( ptr, nbPossibleEnumeration ) );
                  it = copy;
               }
            }

            _mergeChunks( chunks_out );
         }
      }

   private:
      size_t  _nbSimilarValueToCreateSimilarChunk;
   };

   /**
    @brief Format using a very simple compression technique
    
    It is simply taking advange that most of the voxels are identical when linearly read
    */
   class VolumeMf3Format
   {
      typedef ui16   DiscretizationType;
      enum Mf3ID
      {
         formatVersionNumber = 3
      };

   public:
      template <class T, class VolumeMemoryBufferType>
      void write( const imaging::VolumeSpatial<T, VolumeMemoryBufferType>& vol, std::ostream& file ) const
      {
         
         if ( !file.good() )
            throw std::runtime_error( "file error" );

         // first discretize the volume with to the desired level
         VolumeDiscretizer discretizer;

         std::vector<Rsi> rsi;
         imaging::VolumeMemoryBuffer<DiscretizationType> buffer;
         discretizer.discretize( vol, buffer, rsi );

         // encode the volume's data
         DataCompressorCount compressor( 15 );
         DataCompressorCount::Chunks<DiscretizationType> chunks;
         compressor.compress<const DiscretizationType*>( buffer.begin(), buffer.end(), chunks );

         // export the volume's data
         // 0 : the format used
         unsigned int firstWord = (unsigned int)formatVersionNumber;
         file.write( (char*)&firstWord, sizeof( unsigned int ) );

         // Dimensions
         const unsigned int width  = vol.getSize()[ 0 ];
         const unsigned int height = vol.getSize()[ 1 ];
         const unsigned int depth  = vol.getSize()[ 2 ];

         file.write( (char*)&width, sizeof( unsigned int ) );
         file.write( (char*)&height, sizeof( unsigned int ) );
         file.write( (char*)&depth, sizeof( unsigned int ) );

         core::write< std::vector<Rsi> >( rsi, file );

         // PST
         vol.getPst().write( file );
         
         // now the chunks
         chunks.write( file );
      }

      template <class T, class VolumeMemoryBufferType>
      void read( imaging::VolumeSpatial<T, VolumeMemoryBufferType>& vol, std::istream& file ) const
      {
         typedef imaging::VolumeSpatial<T, VolumeMemoryBufferType> Volume;

         if ( !file.good() )
            throw std::runtime_error( "file error" );

         // export the volume's data
         // 0 : the format used
         unsigned int firstWord = 0;
         file.read( (char*)&firstWord, sizeof( unsigned int ) );
         ensure( firstWord == formatVersionNumber, "wrong file: version number doesn't match" );

         // Dimensions
         unsigned int width;
         unsigned int height;
         unsigned int depth;

         file.read( (char*)&width,  sizeof( unsigned int ) );
         file.read( (char*)&height, sizeof( unsigned int ) );
         file.read( (char*)&depth,  sizeof( unsigned int ) );

         std::vector<Rsi> rsi;
         core::read< std::vector<Rsi> >( rsi, file );
         
         Volume::Matrix pst;
         pst.read( file );

         
         // now the chunks
         DataCompressorCount::Chunks<DiscretizationType> chunks;
         chunks.read( file );

         std::auto_ptr<DiscretizationType> vals( new DiscretizationType[ chunks.dataSize() ] );
         chunks.decode( vals.get() );


         size_t index = 0;
         vol = imaging::VolumeSpatial<T, VolumeMemoryBufferType>( core::vector3ui( width, height, depth ), pst );
         for ( unsigned int k = 0; k < depth; ++k )
         {
            for ( unsigned int j = 0; j < height; ++j )
            {
               for ( unsigned int i = 0; i < width; ++i )
               {
                  vol( i, j, k ) = rsi[ k ].slope * vals.get()[ index++ ] + rsi[ k ].intercept;
               }
            }
         }
      }
   };
}
}

using namespace nll;
using namespace nll::core;
using namespace nll::algorithm;
using namespace nll::imaging;

class TestVolumeCompressor
{
public:
   void testBasic()
   {
      const double minValueSlice0 = -10;
      const double maxValueSlice0 = 20;
      VolumeMemoryBuffer<double> input( 3, 6, 70 );
      input( 0, 0, 0 ) = 3;
      input( 1, 1, 0 ) = minValueSlice0;
      input( 2, 0, 0 ) = maxValueSlice0;

      for ( ui32 z = 1; z < input.getSize()[ 2 ] - 1; ++z ) // we want specif cases for empty values & maximal range test
      {
         for ( ui32 y = 0; y < input.getSize()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < input.getSize()[ 0 ]; ++x )
            {
               const double val = core::generateUniformDistribution( -10000, 50000 );
               input( x, y, z ) = val;
            }
         }
      }

      VolumeMemoryBuffer<ui8> output;
      std::vector<Rsi> rsi;
      VolumeDiscretizer discretizer;
      discretizer.discretize( input, output, rsi );

      for ( ui32 z = 0; z < input.getSize()[ 2 ]; ++z )
      {
         for ( ui32 y = 0; y < input.getSize()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < input.getSize()[ 0 ]; ++x )
            {
               const double expectedVal = input( x, y, z );
               const double val = output( x, y, z );
               const double computedVal = val * rsi[ z ].slope + rsi[ z ].intercept;
               TESTER_ASSERT( fabs( expectedVal - computedVal ) < rsi[ z ].slope );
            }
         }
      }

      TESTER_ASSERT( fabs( rsi[ 0 ].intercept - minValueSlice0 ) < 1e-5 );
      const double expectedSlopeSlice0 = ( maxValueSlice0 - minValueSlice0 ) / 255.0;
      TESTER_ASSERT( fabs( rsi[ 0 ].slope - expectedSlopeSlice0 ) < 1e-5 );
   }

   void testCompression()
   {
      for ( ui32 n = 0; n < 500; ++n )
      {
         const ui32 size = ( rand() % 500 ) + 50;
         std::vector<ui32> test;
         for ( ui32 nn = 0; nn < size; ++nn )
         {
            const ui32 v = rand() % 3;
            test.push_back( v );
         }

         DataCompressorCount compressor( 2 );

         DataCompressorCount::Chunks<ui32> chunks;
         compressor.compress( test.begin(), test.end(), chunks );
      
         const size_t nbData = chunks.dataSize();
         TESTER_ASSERT( nbData == size );

         std::vector<ui32> res( size );
         chunks.decode( &res[ 0 ] );

         TESTER_ASSERT( res == test );

         std::stringstream ss;
         chunks.write( ss );

         std::vector<ui32> resRead( size );
         DataCompressorCount::Chunks<ui32> chunksRead;
         chunksRead.read( ss );
         chunks.decode( &resRead[ 0 ] );
         TESTER_ASSERT( chunksRead.dataSize() == size );
         TESTER_ASSERT( resRead == test );
      }
   }

   void testReadWriteMf3()
   {
      const std::string out = "c:/tmp/test.mf3";
      VolumeMf3Format mf3File;

      VolumeSpatial<double> volume;
      VolumeSpatial<double> volume2;

      {
         core::Timer importTime;
         imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/ct-long.mf2", volume );
         std::cout << "importTimeToBeat=" << importTime.getCurrentTime() << std::endl;

         std::ofstream f( out.c_str(), std::ios::binary );
         core::Timer exportTime;
         mf3File.write( volume, f );
         std::cout << "exportTime=" << exportTime.getCurrentTime() << std::endl;
      }

      {
         core::Timer importTime;
         std::ifstream f2( out.c_str(), std::ios::binary );
         mf3File.read( volume2, f2 );
         std::cout << "importTimeNewVersion=" << importTime.getCurrentTime() << std::endl;
      }

      TESTER_ASSERT( volume.getSize() == volume2.getSize() );
      for ( ui32 z = 0; z < volume.getSize()[ 2 ]; ++z )
      {
         for ( ui32 y = 0; y < volume.getSize()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < volume.getSize()[ 0 ]; ++x )
            {
               const double expectedVal = volume( x, y, z );
               const double expectedVal2 = volume2( x, y, z );
               TESTER_ASSERT( fabs( expectedVal - expectedVal2 ) < 1.0 );
            }
         }
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolumeCompressor);
TESTER_TEST(testBasic);
TESTER_TEST(testCompression);
TESTER_TEST(testReadWriteMf3);
TESTER_TEST_SUITE_END();
#endif