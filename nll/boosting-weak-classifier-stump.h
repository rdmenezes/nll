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

#ifndef NLL_ALGORITHM_BOOSTING_STUMP_H_
# define NLL_ALGORITHM_BOOSTING_STUMP_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Weak classifier using simple thresholding attribut to compute the class

    It will scan each feature and each value of the feature to compute the best feature and threshold.
    
    The time complexity of algorithm is linear in the number of samples. To compute the best threshold, an histogram of
    the data is created, then each bin is linearly checked.

    The classifier handles only binary decision problems

    */
   template <class DatabaseT>
   class WeakClassifierStump : public WeakClassifier<DatabaseT>
   {
   public:
      typedef WeakClassifier<DatabaseT>   Base;
      typedef typename Base::value_type   value_type;
      typedef typename Base::Database     Database;

      /**
       @pram nbBinsRatio the number of bins used to compute the data's histogram, in proportion of the number of samples
       @note Beware of the <nbBinsRatio> as this is depending on the number of samples in the database. If the number of samples
             is very low, the histogram will be rough and not distinguish between two cases close to each other...
       */
      WeakClassifierStump( value_type nbBinsRatio ) : _nbBinsRatio( nbBinsRatio )
      {}

      /**
       @brief Learn the weighted database.
       @param database it is assumed all data in <code>database</code> are training data
       @param weights the weights associated to each data sample
       */
      virtual value_type learn( const Database& databaseTraining, const core::Buffer1D<value_type> weights )
      {
         const ui32 nbSamples = databaseTraining.size();
         if ( nbSamples == 0 )
         {
            return static_cast<value_type>( 1 );
         }

         // compute first the min/max for each feature
         const ui32 nbFeatures = static_cast<ui32>( databaseTraining[ 0 ].input.size() );
         std::vector<value_type> minFeatureValue( nbFeatures, std::numeric_limits<value_type>::max() );
         std::vector<value_type> maxFeatureValue( nbFeatures, std::numeric_limits<value_type>::min() );
         for ( ui32 n = 0; n < nbSamples; ++n )
         {
            const typename Database::Sample& sample = databaseTraining[ n ];
            for ( ui32 f = 0; f < nbFeatures; ++f )
            {
               ensure( sample.input.size() == nbFeatures, "all inputs must have exactly the same size" );
               minFeatureValue[ f ] = std::min<value_type>( minFeatureValue[ f ], static_cast<value_type>( sample.input[ f ] ) );
               maxFeatureValue[ f ] = std::max<value_type>( maxFeatureValue[ f ], static_cast<value_type>( sample.input[ f ] ) );
            }
         }

         value_type bestError = std::numeric_limits<value_type>::max();
         for ( ui32 f = 0; f < nbFeatures; ++f )
         {
            value_type error;
            value_type threshold;
            bool isInfReturningZeroClass;
            _computeBestThreshold( databaseTraining, weights, f, minFeatureValue[ f ], maxFeatureValue[ f ], error, threshold, isInfReturningZeroClass );

            if ( error < bestError )
            {
               bestError = error;
               _isInfReturningZeroClass = isInfReturningZeroClass;
               _featureId = f;
               _threshold = threshold;
            }
         }

         return bestError;
      }

      virtual ui32 test( const typename Database::Sample::Input& input ) const
      {
         if ( _isInfReturningZeroClass )
         {
            return input[ _featureId ] >= _threshold;
         } else {
            return input[ _featureId ] < _threshold;
         }
      }

      value_type getThreshold() const
      {
         return _threshold;
      }

      ui32 getFeatureId() const
      {
         return _featureId;
      }

      /**
       @brief Returns true if the considered feature is less than the threshold, the class is 0, else 1
       */
      bool isInfReturningZeroClass() const
      {
         return _isInfReturningZeroClass;
      }

   private:
      // compute the best stump given a feature ID and a database and min/max value for this feature
      void _computeBestThreshold( const Database& database,
                                  const core::Buffer1D<value_type> weights,
                                  ui32 feature,
                                  value_type minFeatureValue,
                                  value_type maxFeatureValue,
                                  value_type& outError,
                                  value_type& outThreshold,
                                  bool&       outIsInfReturningZeroClass ) const
      {
         // first put the data into bin: this way we avoid to sort the data reducing the algorithmic complexity to o(n)
         const ui32 nbSamples = database.size();
         const ui32 nbBins = static_cast<ui32>( database.size() * _nbBinsRatio );
         value_type factor = ( nbBins - 1 ) / ( maxFeatureValue - minFeatureValue );   // factor mapping the 

         std::vector<value_type> binsZero( nbBins + 1 ); // +1 to help with the bound computation in <_computeBestBin>
         std::vector<value_type> binsOne( nbBins + 1 );
         for ( ui32 n = 0; n < nbSamples; ++n )
         {
            ui32 bin = static_cast<ui32>( ( database[ n ].input[ feature ] - minFeatureValue ) * factor );
            if ( database[ n ].output == 0 )
            {
               binsZero[ bin ] += weights[ n ];
            } else {
               binsOne[ bin ] += weights[ n ];
            }
         }

         // then compute the cumulative distribution
         ui32 bestBin;                
         _computeBestBin( binsZero, binsOne, bestBin, outError, outIsInfReturningZeroClass );
         outThreshold = minFeatureValue + bestBin / factor;
      }

      // given weighted bins, compute the optimal threshold
      static void _computeBestBin( const std::vector<value_type>& binsZero, const std::vector<value_type>& binsOne,
                                   ui32& outBestBin, value_type& outMaxError, bool& isInfClassZero )
      {
         outMaxError = 10;
         const value_type pbOne = std::accumulate( binsOne.begin(), binsOne.end(), 0.0f );

         // then compute the cumulative distribution
         value_type accumulateClassZero = 0;
         value_type accumulateClassOne = 0;
         const ui32 nbBins = static_cast<ui32>( binsZero.size() );
         for ( ui32 bin = 0; bin < nbBins; ++bin )
         {
            // now compute the probabilities
            const value_type pbClassRightInf = accumulateClassZero + ( pbOne - accumulateClassOne );
            const value_type pbInfError = 1 - pbClassRightInf;
            const value_type pbSupError = pbClassRightInf;

            if ( pbInfError < outMaxError )
            {
               isInfClassZero = true;
               outBestBin = bin;
               outMaxError = pbInfError;
            }

            if ( pbSupError < outMaxError )
            {
               isInfClassZero = false;
               outBestBin = bin;
               outMaxError = pbSupError;
            }

            accumulateClassZero += binsZero[ bin ];
            accumulateClassOne += binsOne[ bin ];
         }
      }

   private:
      ui32           _featureId;
      value_type     _threshold;
      bool           _isInfReturningZeroClass;     // means that if feature < threshold, the class is 0, else 1
      value_type     _nbBinsRatio;
   };

   /**
    @ingroup algorithm
    @brief Stump factory
    */
   template <class DatabaseT>
   class StumpFactory
   {
   public:
      typedef typename WeakClassifierStump<DatabaseT>::value_type value_type;

      StumpFactory( value_type nbBinsRatio = (value_type)1.0 ) : _nbBinsRatio( nbBinsRatio )
      {}

      std::shared_ptr<WeakClassifierStump<DatabaseT>> create() const
      {
         return std::shared_ptr<WeakClassifierStump<DatabaseT>>( new WeakClassifierStump<DatabaseT>( _nbBinsRatio ) );
      }

   private:
      value_type _nbBinsRatio;
   };
}
}

#endif