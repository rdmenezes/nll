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

# ifndef NLL_DEBUG_DECOMPOSE_H_
# define NLL_DEBUG_DECOMPOSE_H_

namespace nll
{
namespace debug
{
   /**
    @ingroup debug
    @brief decompose a vector into a collection of images ( like from Gabor features )
    */
   template <class ImageType, class Mapper, class VectorType>
   std::vector< core::Image<ImageType, Mapper> > decomposeVector( const core::Buffer1D<VectorType>& vec, ui32 sizex, ui32 sizey, ui32 nbcomp )
   {
      typedef core::Image<ImageType, Mapper> Image;
      typedef std::vector<Image>             Images;
      typedef core::Buffer1D<VectorType>     Vector;
      
      ui32 imageSize = sizex * sizey * nbcomp;
      assert( vec.size() % ( imageSize ) == 0 ); // else it is likely that image size doesn't match
      ui32 nbI = vec.size() / imageSize;
      Images is( nbI );
      for ( ui32 n = 0; n < nbI; ++n )
      {
         Image i( sizex, sizey, nbcomp, false );
         for ( ui32 nn = 0; nn < imageSize; ++nn ) 
            i[ nn ] = static_cast<ImageType> ( vec[ imageSize * n + nn ] );
         is[ n ] = i;
      }
      return is;
   }
}
}

#endif
