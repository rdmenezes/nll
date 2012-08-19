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

#ifndef NLL_ALGORITHM_REGISTRATION_PREPROCESSING_VOLUME_H_
# define NLL_ALGORITHM_REGISTRATION_PREPROCESSING_VOLUME_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Volume preprocessor, typically used in intensity based registrations to transform volume default
           values to bin ID
    */
   template <class TInput, class MapperI, class TOutput, class MapperO>
   class VolumePreprocessor
   {
   public:
      typedef imaging::VolumeSpatial<TInput, MapperI>     VolumeInput;
      typedef imaging::VolumeSpatial<TOutput, MapperO>    VolumeOutput;

      virtual VolumeOutput run( const VolumeInput& input ) const = 0;
      virtual ~VolumePreprocessor()
      {}
   };

   /**
    @ingroup algorithm
    @brief Preprocessing based on LUT index
    */
   template <class TInput, class MapperI, class MapperO>
   class VolumePreprocessorLut8bits : public VolumePreprocessor<TInput, MapperI, ui8, MapperO>, public core::NonCopyable
   {
   public:
      typedef VolumePreprocessor<TInput, MapperI, ui8, MapperO>   Base;
      typedef typename Base::VolumeInput                          VolumeInput;
      typedef typename Base::VolumeOutput                         VolumeOutput;

   public:
      VolumePreprocessorLut8bits( const imaging::LookUpTransformWindowingRGB& lut ) : _lut( lut )
      {
         ensure( lut.getNbComponents() == 1, "only one component is handled" );
      }

      virtual VolumeOutput run( const VolumeInput& input ) const
      {
         VolumeOutput output( input.getSize(), input.getPst() );
         typedef typename VolumeInput::const_iterator    InputIterator;
         typedef typename VolumeOutput::iterator         OutputIterator;

         InputIterator iit = input.begin();
         OutputIterator oit = output.begin();

         for ( ; iit != input.end(); ++iit, ++oit )
         {
            *oit = static_cast<typename VolumeOutput::value_type>( _lut.transform( *iit )[ 0 ] );
         }

         return output;
      }

   protected:
      const imaging::LookUpTransformWindowingRGB&     _lut;
   };
}
}

#endif
