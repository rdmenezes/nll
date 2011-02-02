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

#ifndef FEATURE_TRANSFORMATION_KERNEL_PCA_H_
# define FEATURE_TRANSFORMATION_KERNEL_PCA_H_

namespace nll
{
namespace algorithm
{
   /**
    Non linear feature transformation using a kernel PCA algorithm.
    */
   template <class Point, class Kernel>
   class FeatureTransformationKernelPca : public FeatureTransformation<Point>
   {
   public:
      typedef  FeatureTransformation<Point>  Base;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      FeatureTransformationKernelPca( const Kernel& kernel )
      {
         _kernel = kernel.clone();
      }

      virtual ~FeatureTransformationKernelPca()
      {
         delete _kernel;
      }

      /**
       Compute the principal components using only the LEARNING|VALIDATION samples
       */
      template <class Database>
      bool compute( const Database& dat, ui32 nbFeatures )
      {
         typedef core::DatabaseInputAdapterRead<Database>   PointsAdapter;
         Database learningDat = core::filterDatabase( dat,
                                                      core::make_vector<nll::ui32>( Database::Sample::LEARNING,
                                                                                    Database::Sample::VALIDATION ),
                                                      Database::Sample::LEARNING );
         PointsAdapter adapter( learningDat );
         return _algorithm.compute( adapter, nbFeatures, *_kernel );
      }

      /**
       @brief Process a point according to the transformation.
       */
      virtual Point process( const Point& p ) const
      {
         return _algorithm.transform( p );
      }

      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i )
      {
         _algorithm.read( i );
      }

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const
      {
         _algorithm.write( o );
      }

   protected:
      Kernel*                    _kernel;
      KernelPca<Point, Kernel>   _algorithm;
   };
}
}

#endif
