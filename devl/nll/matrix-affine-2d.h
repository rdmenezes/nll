/*
 * Numerical learning library
 * http://nll.googlecode.com/
 *
 * Copyright (c) 2009-2011, Ludovic Sibille
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

#ifndef NLL_TRANSFORMATION_AFFINE_2D_H_
# define NLL_TRANSFORMATION_AFFINE_2D_H_

namespace nll
{
namespace core
{
   /**
    @brief Create a 2D transformation matrix, given a rotation, scaling and translation parameters
    @param roation the rotation angle from the x-axis in radian

    the matrix has this format:
                       | scalingx * rx0 ; scalingy * ry0 ; tx |
                   M = | scalingx * rx1 ; scalingy * ry1 ; ty |
                       |     0          ;     0          ; 1  |
    */
   inline core::Matrix<double>
   createTransformationAffine2D( double rotation,
                                 const core::vector2d& scaling,
                                 const core::vector2d& translation )
   {
      const double co = cos( rotation );
      const double si = sin( rotation );
      core::Matrix<double> tfm( 3, 3, false );
      tfm( 0, 0 ) =  co * scaling[ 0 ];
      tfm( 0, 1 ) = -si * scaling[ 1 ];
      tfm( 0, 2 ) =  translation[ 0 ];
      tfm( 1, 0 ) =  si * scaling[ 0 ];
      tfm( 1, 1 ) =  co * scaling[ 1 ];
      tfm( 1, 2 ) =  translation[ 1 ];
      tfm( 2, 0 ) =  0;
      tfm( 2, 1 ) =  0;
      tfm( 2, 2 ) =  1;

      return tfm;
   }
}
}

#endif