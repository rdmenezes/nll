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

#ifndef NLL_ALGORITHM_TRANSFORMATION_EVALUATOR_SIMILARITY_H_
# define NLL_ALGORITHM_TRANSFORMATION_EVALUATOR_SIMILARITY_H_

namespace nll
{
namespace algorithm
{
   /**
    @brief Implementation of the registration evaluator using a similatity function criteria to maximize
    */
   template <class T, class Storage>
   class RegistrationEvaluatorSimilarity : public RegistrationEvaluator<T, Storage>
   {
   public:
      typedef RegistrationEvaluator<T, Storage>          Base;
      typedef typename Base::Volume                      Volume;
      typedef std::vector< std::pair<double, double> >   SimilarityPlot;
      typedef HistogramMaker<T, Storage>                 HistogramMakerAlgorithm;

   public:
      RegistrationEvaluatorSimilarity( const Volume& source, const Volume& target, 
                                       const SimilarityFunction& similarity,
                                       const TransformationCreator& creator,
                                       const HistogramMakerAlgorithm& histogramMaker,
                                       size_t jointHistogramNbBins = 256 ) : Base( source, target, creator ), _similarity( similarity ), _histogramMaker( histogramMaker ), _jointHistogramNbBins( jointHistogramNbBins )
      {}

      RegistrationEvaluatorSimilarity( const SimilarityFunction& similarity,
                                       const HistogramMakerAlgorithm& histogramMaker,
                                       size_t jointHistogramNbBins = 256 ) : _similarity( similarity ), _histogramMaker( histogramMaker ), _jointHistogramNbBins( jointHistogramNbBins )
      {}

      /**
       @brief Evaluate the transformation parameters
       @note the (0, 0) bin of the histogram is removed as we don't want to count the background
             (a lot of background matching doesn't mean the registration is good!)
       */
      virtual double evaluate( const imaging::Transformation& transformation ) const
      {
         // then construct a joint histogram
         JointHistogram jointHistogram( _jointHistogramNbBins );
         _histogramMaker.compute( getSource(), transformation, getTarget(), jointHistogram );

         // Remove the background intensity from the histogram. Reason: a lot of background match does not mean the registration is good.
         // It will biase the joint histogram measure
         const JointHistogram::value_type nbBackground = jointHistogram( 0, 0 );
         jointHistogram( 0, 0 ) = 0;
         jointHistogram.setNbSamples( static_cast<JointHistogram::value_type>( jointHistogram.getNbSamples() - nbBackground ) );

         // then run the similarity measure
         const double val = _similarity.evaluate( jointHistogram );
         _lastRunHistogram = jointHistogram;
         return val;
      }

      /**
       @brief Returns the last histogram computed
       */
      const JointHistogram& getLastRunHistogram() const
      {
         return _lastRunHistogram;
      }

      /**
       @brief Helper to print the similarity while varying only one parameter
       @return a list of pair containaing <parameter to vary, similarity>
       @note this is helpful to understand why a minimization algorithm might fail
       */
      SimilarityPlot returnSimilarityAlongParameter( const core::Buffer1D<f64>& startingParameters, size_t varyingParameter, double increment, size_t nbIterations ) const
      {
         SimilarityPlot result;
         result.reserve( nbIterations );

         const double start = startingParameters[ varyingParameter ];
         for ( size_t iter = 0; iter < nbIterations; ++iter )
         {
            core::Buffer1D<f64> p;
            p.clone( startingParameters );

            const double value = iter * increment + start;
            p[ varyingParameter ] = value;
            const double similarity = evaluate( *getTransformationCreator().create( p ) );
            result.push_back( std::make_pair( value, similarity ) );
         }

         return result;
      }

   protected:
      const SimilarityFunction&        _similarity;
      const HistogramMakerAlgorithm&   _histogramMaker;
      size_t                           _jointHistogramNbBins;

      // keep it for debugging purpose
      mutable JointHistogram           _lastRunHistogram;
   };


   /**
    @brief Helper class contructing a RegistrationEvaluator type given a spatial volume
    */
   template <class Volume>
   class RegistrationEvaluatorHelper
   {
   private:
      // not constructible!
      RegistrationEvaluatorHelper()
      {};

   public:
      typedef typename Volume::value_type    value_type;
      typedef typename Volume::VoxelBuffer   VoxelBuffer;

      typedef RegistrationEvaluatorSimilarity<value_type, VoxelBuffer>   EvaluatorSimilarity;
   };
}
}

#endif