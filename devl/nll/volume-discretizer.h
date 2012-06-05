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

#ifndef NLL_IMAGING_VOLUME_DISCRETIZER_H_
# define NLL_IMAGING_VOLUME_DISCRETIZER_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
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
    @ingroup imaging
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
}
}

#endif
