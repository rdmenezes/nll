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

#ifndef NLL_ALGORITHM_REGISTRATION_ALGORITHM_H_
# define NLL_ALGORITHM_REGISTRATION_ALGORITHM_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Registration algorithm interface
    */
   template <class T, class Storage>
   class RegistrationAlgorithm
   {
   public:
      typedef imaging::VolumeSpatial<T, Storage>   Volume;
      typedef typename Volume::Matrix              Matrix;

      /**
       @brief Register a target (or moving) volume to a source (i.e., the target will be the volume to be moved to align source/target)
       @param source the fixed volume
       @param target the moving volume
       @param source2TargetInitTransformation the initial transformation aligning the target to the source. It is defined
              from the source to the target (i.e., in an affine transform it is usally inversed)
       @return the transformation aligning the target on the source but defined from source->target
       */
      virtual std::shared_ptr<imaging::Transformation> evaluate( const Volume& source, const Volume& target, const imaging::Transformation& source2TargetInitTransformation ) const = 0;

      virtual ~RegistrationAlgorithm()
      {}
   };
}
}

#endif