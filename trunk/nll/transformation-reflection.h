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

#ifndef NLL_ALGORITHM_TRANSFORMATION_REFLECTION_H_
# define NLL_ALGORITHM_TRANSFORMATION_REFLECTION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Compute the matrix representing a 3D planar reflection

    @note we can determine algebraically the transformation matrix, however it involves a lot of system solving.
          Instead we are going to use a transformation estimator that maps point of the plane on the plane passing by
          the origin belonging the plane XY. Solutions should be identical.
    */
   inline core::Matrix<double> comuteReflectionPlanar3dMatrix( const core::GeometryPlane& plane )
   {
      typedef EstimatorTransformSimilarityIsometric TransformationEstimator;
      typedef core::Matrix<double>                  Matrix;

      // first compute a transformation that maps the plane origin to the coordinate system's origin
      // additionally the transformation will map the input plane onto the plane XY
      // these conditions are enough to define a unique transformation
      std::vector<core::vector3f> pointsInOriginalPlane;
      pointsInOriginalPlane.reserve( 3 );
      pointsInOriginalPlane.push_back( plane.getOrigin() );
      pointsInOriginalPlane.push_back( plane.getOrigin() + plane.getAxisX() );
      pointsInOriginalPlane.push_back( plane.getOrigin() + plane.getAxisY() );

      std::vector<core::vector3f> pointsInDestinationPlane;
      pointsInDestinationPlane.reserve( 3 );
      pointsInDestinationPlane.push_back( core::vector3f( 0, 0, 0 ) );
      pointsInDestinationPlane.push_back( core::vector3f( 1, 0, 0 ) );
      pointsInDestinationPlane.push_back( core::vector3f( 0, 1, 0 ) );

      TransformationEstimator estimator( 0, 0, std::numeric_limits<double>::max() );   // we don't want any constraints on the transformation
      const Matrix tfmPlaneToDefault = estimator.compute( pointsInOriginalPlane, pointsInDestinationPlane );
      ensure( estimator.getLastResult() == TransformationEstimator::OK, "should be impossible, there must always be a unique solution if the plane is well defined" );

      // we know exactly the reflection matrix for the plane XY
      Matrix zDefaultReflection = core::identityMatrix<Matrix>( 4 );
      zDefaultReflection( 3, 3 ) = -1;

      // finally, compute the transformation default plane back to the original plane
      Matrix tfmDefaultToPlane = tfmPlaneToDefault.clone();
      const bool isOk = core::inverse( tfmDefaultToPlane );
      ensure( isOk, "should not happen: the matrix should be well defined" );

      // finally return the composition of these transformations
      // note: the homogeneous coefficien may not be equal to 1 anymore,
      // so normalize it so that tfm( 3, 3 ) = 1
      return core::resetHomogeneous4x4( tfmDefaultToPlane * zDefaultReflection * tfmPlaneToDefault );
   }
}
}

#endif