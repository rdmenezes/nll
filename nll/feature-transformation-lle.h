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

#ifndef NLL_ALGORITHM_FEATURE_TRANSFORMATION_LLE_H_
# define NLL_ALGORITHM_FEATURE_TRANSFORMATION_LLE_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Compute a locally linear embedding of the features (unsupervised learning) and train a neural
           network mapping the input coordinates to LLE based coordinates

           This method is mainly aimed at displaying high dimentional data, but a wrapper for generic dimentional
           reduction is provided nonetheless.

    @see http://jmlr.csail.mit.edu/papers/volume4/saul03a/saul03a.pdf

    @note the training must be quite exhaustive to map appropriately all the coordinate space!
    */
   template <class Point>
   class FeatureSelectionLle : public FeatureTransformation<Point>
   {
      typedef Mlp<FunctionSimpleDifferenciableSigmoid> Mapper;

   public:
      typedef FeatureTransformation<Point>  Base;
      typedef Classifier<Point>             ClassifierType;
      typedef FeatureTransformationNormalization<Point> Normalizer;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

      virtual Point process( const Point& p ) const
      {
         Point normalized = _normalizer.process( p );
         core::Buffer1D<double> mapper = _mapper.propagate( normalized );

         Point final( mapper.size() );
         for ( size_t n = 0; n < final.size(); ++n )
            final[ n ] = mapper[ n ];
         return final;
      }

      template <class Database>
      bool compute( const Database& _dat, size_t nbComponents, size_t nbNeurons = 10, double learningRate = 0.5, double time = 10, size_t nbNeightbours = 12 )
      {
         // take only the training & validation datasets
         typedef typename Database::Sample::Input  PointT;
         Database learningDat = core::filterDatabase( _dat,
                                                      core::make_vector<size_t>( Database::Sample::LEARNING,
                                                                                 Database::Sample::VALIDATION ),
                                                      Database::Sample::LEARNING );
         
         typedef core::ClassificationSample< PointT, PointT > Sample;
         typedef core::Database<Sample> TfmDatabase;

         TfmDatabase tfmDat;
         for ( size_t n = 0; n < learningDat.size(); ++n )
         {
            tfmDat.add( Sample( learningDat[ n ].input, PointT(), Sample::LEARNING ) );
         }

         // normalize the database
         FeatureTransformationNormalization<PointT> normalization;
         normalization.compute( tfmDat );
         tfmDat = normalization.transform( tfmDat );

         typedef core::DatabaseInputAdapter< TfmDatabase >   Adapter;
         Adapter points( tfmDat );

         // compute the LLE coordinates
         LocallyLinearEmbedding lle;
         std::vector<PointT> pointsTfm = lle.transform( points, nbComponents, nbNeightbours );

         // generate a mapping normalized points -> LLE coordinates
         for ( size_t n = 0; n < tfmDat.size(); ++n )
         {
            tfmDat[ n ].output = pointsTfm[ n ];
         }

         StopConditionMlpThreshold stop( time );
         Mapper mapper( core::make_vector<size_t>( _dat[ 0 ].input.size(), nbNeurons, nbComponents ) );
         mapper.learn( tfmDat, stop, learningRate );

         _mapper = mapper;
         _normalizer = Normalizer( normalization.getMean(), normalization.getStddev() );
         return true;
      }

      virtual void read( std::istream& i ) 
      {
         _normalizer.read( i );
         _mapper.read( i );
      }

      virtual void write( std::ostream& o ) const
      {
         _normalizer.write( o );
         _mapper.write( o );
      }

   private:
      Normalizer  _normalizer;
      Mapper      _mapper;
   };
}
}

#endif
