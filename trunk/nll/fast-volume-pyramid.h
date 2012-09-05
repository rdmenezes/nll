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

#ifndef NLL_ALGORITHM_FAST_VOLUME_PYRAMID_H_
# define NLL_ALGORITHM_FAST_VOLUME_PYRAMID_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Construct a pyramid of volumes

    Each new level of the pyramid has its size divided by two, with the level 0 begin the original volume.

    To subsample a voxel, its 2^n x 2^n x 2^n neighboorhood is averaged, always starting from the original volume
    */
   template <class T, class StorageBuffer = imaging::VolumeMemoryBuffer<T> >
   class VolumePyramid
   {
   public:
      typedef imaging::VolumeSpatial<T, StorageBuffer>   Volume;
      typedef std::vector<Volume>                        Volumes;
      typedef typename Volume::Matrix                    Matrix;
      typedef typename Matrix::value_type                MatrixType;

   public:
      VolumePyramid()
      {}

      VolumePyramid( const Volume& volume, size_t nbLevels )
      {
         construct( volume, nbLevels );
      }

      /**
       @brief Construct the pyramid
       @param volume the volume to use. Note that the level 0 is taking a reference on this volume
       @param nbLevels the number of levels. Each level has size / 2 compared to the previous one
       */
      void construct( const Volume& volume, size_t nbLevels )
      {
         IntegralImage3d volumeIntegral;
         volumeIntegral.process( volume );

         _volumes.clear();
         _volumes.reserve( nbLevels );
         _volumes.push_back( volume ); // level 0 is the original volume

         core::vector3i factors( 1, 1, 1 );
         for ( size_t n = 1; n < nbLevels; ++n )
         {
            // compute the next increments
            const core::vector3i sizeNext( volume.getSize()[ 0 ] / ( 2 * factors[ 0 ] ),
                                           volume.getSize()[ 1 ] / ( 2 * factors[ 1 ] ),
                                           volume.getSize()[ 2 ] / ( 2 * factors[ 2 ] ) );
            int sizeMin = *std::max_element( sizeNext.getBuf(), sizeNext.getBuf() + 3 );
            for ( int nn = 0; nn < 3; ++nn )
            {
               if ( sizeMin <= sizeNext[ nn ] )
                  factors[ nn ] *= 2;
            }

            Matrix pst;
            pst.clone( volume.getPst() );
            const int area = factors[ 0 ] * factors[ 1 ] * factors[ 2 ];
            const int areaSmooth = ( factors[ 0 ] + 2 * ( factors[ 0 ] / 2 ) ) *
                                   ( factors[ 1 ] + 2 * ( factors[ 1 ] / 2 ) ) *
                                   ( factors[ 2 ] + 2 * ( factors[ 2 ] / 2 ) );

            // set the correct spacing
            for ( size_t x = 0; x < 3; ++x )
            {
               for ( size_t y = 0; y < 3; ++y )
               {
                  pst( y, x ) *= factors[ x ];
               }
            }

            // now build the volume
            Volume v( core::vector3ui( volume.sizex() / factors[ 0 ],
                                       volume.sizey() / factors[ 1 ],
                                       volume.sizez() / factors[ 2 ] ),
                      pst,
                      volume.getBackgroundValue() );
            ensure( v.sizex() > 0, "too many pyramid levels" );
            ensure( v.sizey() > 0, "too many pyramid levels" );
            ensure( v.sizez() > 0, "too many pyramid levels" );

            const int sizez = static_cast<int>( volume.sizez() ) / factors[ 2 ];
            const int sizey = static_cast<int>( volume.sizey() ) / factors[ 1 ];
            const int sizex = static_cast<int>( volume.sizex() ) / factors[ 0 ];

            core::vector3i halfFactors( factors[ 0 ] / 2,
                                        factors[ 1 ] / 2,
                                        factors[ 2 ] / 2 );

            #if !defined(NLL_NOT_MULTITHREADED)
            # pragma omp parallel for
            #endif
            for ( int z = 0; z < sizez; ++z )
            {
               const int zRef = z * factors[ 2 ];
               typename Volume::DirectionalIterator  lineIt = v.getIterator( 0, 0, z );
               for ( int y = 0; y < sizey; ++y )
               {
                  const int yRef = y * factors[ 1 ];
                  typename Volume::DirectionalIterator  voxelIt = lineIt;
                  for ( int x = 0; x < sizex; ++x )
                  {
                     const int xRef = x * factors[ 0 ];
                     IntegralImage3d::value_type val;

                     if ( x >= halfFactors[ 0 ] && y >= halfFactors[ 1 ] && z >= halfFactors[ 2 ] &&
                          x + halfFactors[ 0 ] < v.sizex() &&
                          y + halfFactors[ 1 ] < v.sizey() &&
                          z + halfFactors[ 2 ] < v.sizez() )
                     {
                        val = volumeIntegral.getSum( core::vector3i( xRef - halfFactors[ 0 ], yRef - halfFactors[ 1 ], zRef - halfFactors[ 2 ] ),
                                                     core::vector3i( xRef + factors[ 0 ] - 1 + halfFactors[ 0 ],
                                                                     yRef + factors[ 1 ] - 1 + halfFactors[ 1 ],
                                                                     zRef + factors[ 2 ] - 1 + halfFactors[ 2 ] ) ) / areaSmooth;
                     } else {
                        val = volumeIntegral.getSum( core::vector3i( xRef, yRef, zRef ),
                                                     core::vector3i( xRef + factors[ 0 ] - 1,
                                                                     yRef + factors[ 1 ] - 1,
                                                                     zRef + factors[ 2 ] - 1 ) ) / area;
                     }
                     *voxelIt = static_cast<T>( val );
                     voxelIt.addx();
                  }
                  lineIt.addy();
               }
            }

            _volumes.push_back( v );
         }
      }

      const Volume& operator[]( const size_t level ) const
      {
         return _volumes[ level ];
      }

      Volume& operator[]( const size_t level )
      {
         return _volumes[ level ];
      }

      size_t size() const
      {
         return _volumes.size();
      }

   private:
      Volumes  _volumes;
   };
}
}

#endif