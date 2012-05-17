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

#ifndef NLL_IMAGING_TRANSFORMATION_H_
# define NLL_IMAGING_TRANSFORMATION_H_

namespace nll
{
namespace imaging
{
   /**
    @ingroup imaging
    @brief This object defines a transformation, mapping a point in MM to another point also in MM
    @note Object representing a transformation. This is usually defined from source to target space (the target being the moved object)
    */
   class Transformation
   {
   public:
      typedef core::Matrix<f32>  Matrix;

   public:
      Transformation()
      {}

      virtual ~Transformation()
      {
         // nothing to do
      }

      /**
       @brief transform a point defined in MM to the new point in MM
       */
      virtual core::vector3f transform( const core::vector3f& p ) const = 0;

      /**
       @brief returns the deformable displacement in MM at this position in MM
       */
      virtual core::vector3f transformDeformableOnly( const core::vector3f& p ) const = 0;

      /**
       @brief get the affine part of the transformation
       */
      virtual const Matrix& getAffineMatrix() const = 0;

      /**
       @brief clone the transformation
       */
      virtual Transformation* clone() const = 0;

   protected:
      // copy disabled
      //Transformation& operator=( const Transformation );
      Transformation( const Transformation& );
   };


   /**
    @ingroup imaging
    @brief Defines an affine transformation
    */
   class TransformationAffine : public Transformation
   {
   public:
      /**
       @brief init with an affine matrix.
       @param init a 4x4 matrix.
                  
              If no shearing component, it defines the rotation, translation and spacing which transforms
              volume coordinate system into the patient coordinate system.
              <p>
              The matrix should be mapped like this:
              [ r1 r2 r3 ox ]
              [ r4 r5 r6 oy ]
              [ r7 r8 r9 oz ]
              [ 0  0  0  1  ]
              <p>
              With R the rotation matrix, and O the image origin in mm, S the image size
              <p>
              The origin is defined as the vector from the world origin to the voxel (0, 0, 0) of the volume,
              distance in minimeter
              <p>
              The spacing is extracted from the matrix: s_x = sqrt( r1^2 + r4^2 + r7^2 ) and so on
              for s_y and s_z spacing.
       */
      template <class T>
      TransformationAffine( const core::Matrix<T>& init ) : _affine( 4, 4, false )
      {
         ensure( init.sizex() == 4 && init.sizex() == 4, "only 4x4 matrices are handled" );
         for ( ui32 y = 0; y < 4; ++y )
            for ( ui32 x = 0; x < 4; ++x )
               _affine( y, x ) = static_cast<f32>( init( y, x ) );

         _affineInverted.clone( _affine );
         bool res = core::inverse( _affineInverted );
         ensure( res, "this is not an affine matrix!" );
      }

      /**
       @brief Default transformation (identity)
       */
      TransformationAffine()
      {
         _affine = core::identityMatrix<Matrix>( 4 );
         _affineInverted.clone( _affine );
      }

      /**
       @brief clone the transformation
       */
      virtual Transformation* clone() const
      {
         TransformationAffine* aff = new TransformationAffine( _affine );
         return aff;
      }

      /**
       @brief get the affine part of the transformation
       */
      virtual const Matrix& getAffineMatrix() const
      {
         return _affine;
      }

      /**
       @brief get the inverse of the affine transformation
       */
      virtual const Matrix& getInvertedAffineMatrix() const
      {
         return _affineInverted;
      }

      /**
       @brief returns the deformable displacement in MM at this position in MM
       */
      virtual core::vector3f transformDeformableOnly( const core::vector3f& ) const
      {
         return core::vector3f();
      }

      /**
       @brief transform a point defined in MM to a new point in MM
       */
      virtual core::vector3f transform( const core::vector3f& p ) const
      {
         return core::transf4( _affine, p );
      }

   protected:
      Matrix      _affine;
      Matrix      _affineInverted;
   };
}
}

#endif