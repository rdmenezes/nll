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

#ifndef NLL_FEATURE_SELECTION_PEARSON_H_
# define NLL_FEATURE_SELECTION_PEARSON_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Compute the pearson coefficient for each feature and each class. This feature selection algorithm belongs
           to the filter class, 1 feature at a time is tested. Can be used on datasets with a lots of features.

           It is known that 2 features assessed independantly could be useless, but together can achieve perfect
           discrimination (for example xor problem with x and y axis). This algorithm can't see this. Also it will select
           redundand features (each feature gives discriminativ information, but the same info). But the algorithm is fastest
           and can still achieves very good results in some case.

    @todo evaluate the method for multiclass database.
    */
   template <class Point>
   class FeatureSelectionFilterPearson : public FeatureSelectionFilter<Point>
   {
   public:
      typedef FeatureSelectionFilter<Point> Base;
      typedef typename Base::Database       Database;
      typedef Classifier<Point>             ClassifierType;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      FeatureSelectionFilterPearson( ui32 nbFeatures ) : _nbFeatures( nbFeatures )
      {}

   protected:
      /**
       @brief compute the selected features : compute the max for each class of the pearson coefficient,
              return the set of features with the highest number of features.
       */
      virtual core::Buffer1D<bool> _compute( const Database& dat )
      {
         assert( dat.size() );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "FeatureSelectionFilterPearson search started" );
         ui32 nbFeatures = dat[ 0 ].input.size();
         ui32 nbclass = core::getNumberOfClass( dat );

         std::vector<std::pair<f64, ui32> > score( nbFeatures );
         for ( ui32 n = 0; n < nbFeatures; ++n )
         {
            core::Buffer1D<f64> f( dat.size() );
            for ( ui32 nn = 0; nn < dat.size(); ++nn )
               f[ nn ] = dat[ nn ].input[ n ];
            core::Buffer1D<f64> c( dat.size() );

            f64 max = -1;
            for ( ui32 nn = 0; nn < nbclass; ++nn )
            {
               for ( ui32 i = 0; i < dat.size(); ++i )
                  c[ i ] = ( dat[ i ].output == nn );
               f64 pearson = nll::core::absolute( core::correlation( core::Matrix<f64>( f ), core::Matrix<f64>( c ) ) );
               // TODO : for a multiclass database is that correct (or add the correlation)?
               // Can't find any reference on this. Needs to be tested.
               max = std::max( max, pearson );
            }
            score[ n ] = std::make_pair<f64, ui32>( max, n );
         }

         for ( ui32 n = 0; n < nbFeatures; ++n )
         {
            std::stringstream log;
            log << "Feature[" << n << "]=" << score[ n ].first;
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, log.str() );
         }

         core::Buffer1D<bool> result( nbFeatures );
         std::sort( score.rbegin(), score.rend() );
         for ( ui32 n = 0; n < _nbFeatures; ++n )
            result[ score[ n ].second ] = true;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "FeatureSelectionFilterPearson search ended" );
         return result;
      }

   protected:
      ui32 _nbFeatures;
   };
}
}

#endif
