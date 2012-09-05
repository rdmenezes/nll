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

#ifndef NLL_ALGORITHM_REGISTRATION_JOINT_HISTOGRAM_SIMILARITY_H_
# define NLL_ALGORITHM_REGISTRATION_JOINT_HISTOGRAM_SIMILARITY_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Function measuring the similarity represented as a joint histogram
    */
   class SimilarityFunction
   {
   public:
      /**
       @brief evalaute a joint histogram
       @return the lower the value the more similar it represents
       */
      virtual double evaluate( const JointHistogram& jh ) const = 0;

      ~SimilarityFunction()
      {}
   };

   /**
    @brief Sum of square differences similarity function
    @note there is a normalization factor (which must be taken into account if we want the derivative!)
    */
   class SimilarityFunctionSumOfSquareDifferences : public SimilarityFunction
   {
   public:
      virtual double evaluate( const JointHistogram& jh ) const
      {
         double sum = 0;
         for ( size_t y = 0; y < jh.getNbBins(); ++y )
         {
            for ( size_t x = 0; x < jh.getNbBins(); ++x )
            {
               const double val = jh( x, y ) * core::sqr( double( x ) - double( y ) );
               sum += val;
            }
         }

         if ( jh.getNbSamples() == 0 )
         {
            core::LoggerNll::write( core::LoggerNll::ERROR, "joint histogram is empty!" );
            return std::numeric_limits<double>::max();
         }

         // - normalize the function by the count and bins so that they give similar intensities
         // - remove the diagonal counts
         return sum / ( jh.getNbSamples() * jh.getNbBins() * jh.getNbBins() ) * 33;
      }
   };


   /**
    @brief Mutual information similarity function
    */
   class SimilarityFunctionMutualInformation : public SimilarityFunction
   {
   public:
      virtual double evaluate( const JointHistogram& jh ) const
      {
         if ( jh.getNbSamples() == 0 )
         {
            core::LoggerNll::write( core::LoggerNll::ERROR, "joint histogram is empty!" );
            return std::numeric_limits<double>::max();
         }

         core::Buffer1D<JointHistogram::value_type> psource( jh.getNbBins() );
         core::Buffer1D<JointHistogram::value_type> ptarget( jh.getNbBins() );

         for ( size_t n = 0; n < jh.getNbBins(); ++n )
         {
            JointHistogram::value_type psrc = 0;
            JointHistogram::value_type ptgt = 0;

            for ( size_t nn = 0; nn < jh.getNbBins(); ++nn )
            {
               psrc += jh( n, nn );
               ptgt += jh( nn, n );
            }

            psource[ n ] = psrc / jh.getNbSamples();
            ptarget[ n ] = ptgt / jh.getNbSamples();
         }

         double sum = 0;
         for ( size_t source = 0; source < jh.getNbBins(); ++source )
         {
            for ( size_t target = 0; target < jh.getNbBins(); ++target )
            {
               const JointHistogram::value_type pxy = jh( source, target ) / jh.getNbSamples();
               if ( core::equal<JointHistogram::value_type>( pxy, 0, (JointHistogram::value_type)1e-6 ) )
                  continue;
               sum += pxy * std::log( pxy / ( psource[ source ] * ptarget[ target ] + 0.1 ) );
            }
         }

         // normalize the function...
         return - sum;
      }
   };
}
}

#endif