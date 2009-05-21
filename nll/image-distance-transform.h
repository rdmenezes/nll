#ifndef NLL_DISTANCE_TRANSFORM_H_
# define NLL_DISTANCE_TRANSFORM_H_

namespace nll
{
namespace core
{
   namespace impl
   {
      /**
       @brief 1D distance transform. It has to be run several times for multidimentionnal distance transform.
              We assume that in the pass of this algorithm (i.e. the first time this algorithm is run on a volume)
              buf[ n ] == 0 (meaning we have the voxel n on) or buf[ n ] == -inf (meaning voxel n is off).
              We also assume that <code>buf</code> and <code>outBuf</code> DON'T point to the same buffer, else
              expect wrong results!
              See "Distance Transforms of Sampled Functions" from Pedro F. Felzenszwalb Daniel P. Huttenlocher for
              reference paper.

       We are using 'abstract' buffer so that we can give customized accessor to a buffer allowing all the
       computations to be 'in place' (in several dimension we don't have to copy each result in the final volume)
       saving a lot of time.

       Generically written since it is potentially usable in a lot of problems (i.e. HMM)

       @param buf the input abstract buffer. Needs to provide double operator[]( unsigned ) const
       @param size the size of the input and output buffers.
       @param outBuf the output abstract buffer. Needs toprovide double &operator[]( unsigned ). It should be
              allocated beforehand.
       */
      template <class AbstractBufferIn, class AbstractBufferOut>
      void dt1d( const AbstractBufferIn& buf, unsigned size, AbstractBufferOut& outBuf )
      {
         const double infinity = std::numeric_limits<double>::max();
         const double ninfinity = std::numeric_limits<double>::min();

         std::vector<double> z( size + 1 );
         std::vector<unsigned> v( size + 1 );

         int k = 0;
         v[ 0 ] = 0;
         z[ 0 ] = ninfinity;
         z[ 1 ] = infinity;

         for ( unsigned q = 1; q < size; ++q )
         {
            double s = ( buf[ q ] + q * q - ( buf[ static_cast<unsigned>( v[ k ] ) ] + v[ k ] * v [ k ] ) )
                     / ( 2 * q - 2 * v[ k ] );

            // difference with the paper "&& k" so that we are sure the first parabolla is never destroyed
            // Reasons why we should never destroy it is that anyway the initial parabolla is the worst
            // lower enveloppe
            while ( s <= z[ k ] && k  )
            {
               --k;
               s = ( buf[ q ] + q * q - ( buf[ static_cast<unsigned>( v[ k ] ) ] + v[ k ] * v [ k ] ) )
                   / ( 2 * q - 2 * v[ k ] );
            }

            ++k;
            v[ k ] = q;
            z[ k ] = s;
            z[ k + 1 ] = infinity;
         }

         k = 0;
         for ( unsigned q = 0; q < size; ++q )
         {
            while ( z[ k + 1 ] < q )
               ++k;
            outBuf[ q ] = ( q - v[ k ] ) * ( q - v[ k ] ) + buf[ static_cast<unsigned>( v[ k ] ) ];
         }
      }

      template <class T, class Mapper>
      class ImageWrapperRow
      {
      public:
         ImageWrapperRow( Image<T, Mapper>& i, ui32 col ) :
            _i( i ), _col( col )
            {}
         T operator[]( ui32 n ) const
         {
            return _i( n, _col, 0 );
         }

         T& operator[]( ui32 n )
         {
            return _i( n, _col, 0 );
         }
      private:
         ImageWrapperRow& operator=( const ImageWrapperRow& );

      private:
         Image<T, Mapper>&    _i;
         ui32                 _col;
      };

      template <class T, class Mapper>
      class ImageWrapperCol
      {
      public:
         ImageWrapperCol( Image<T, Mapper>& i, ui32 row ) :
            _i( i ), _row( row )
            {}
         T operator[]( ui32 n ) const
         {
            return _i( _row, n, 0 );
         }
         T& operator[]( ui32 n )
         {
            return _i( _row, n, 0 );
         }
      private:
         ImageWrapperCol& operator=( const ImageWrapperCol& );

      private:
         Image<T, Mapper>&    _i;
         ui32                 _row;
      };
   }

   /**
    return the distance transform of a grayscale image using a norm-2 metric. The complexity ofthe algorithm
           is O(n*2), n the number of pixels.
    */
   template <class T, class Mapper>
   Image<double, Mapper> distanceTransform( class Image<T, Mapper>& i )
   {
      ensure( i.getNbComponents() == 1, "only greyscale image handled" );
      Image<double, Mapper> buf1( i.sizex(), i.sizey(), 1, false );
      Image<double, Mapper> buf2( i.sizex(), i.sizey(), 1, false );
      for ( ui32 y = 0; y < i.sizey(); ++y )
         for ( ui32 x = 0; x < i.sizex(); ++x )
            buf1( x, y, 0 ) = i( x, y, 0 );

      for ( ui32 y = 0; y < i.sizey(); ++y )
      {
         impl::ImageWrapperRow<double, Mapper> mappedBuffer( buf1, y );
         impl::ImageWrapperRow<double, Mapper> mappedBufferOut( buf2, y );
         impl::dt1d( mappedBuffer, i.sizex(), mappedBufferOut );
      }

      for ( ui32 x = 0; x < i.sizex(); ++x )
      {
         impl::ImageWrapperCol<double, Mapper> mappedBuffer( buf2, x );
         impl::ImageWrapperCol<double, Mapper> mappedBufferOut( buf1, x );
         impl::dt1d( mappedBuffer, i.sizey(), mappedBufferOut );
      }
      return buf1;
   }
}
}

#endif