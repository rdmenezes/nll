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

#ifndef NLL_FEATURE_SELECTION_RELIEFF_H_
# define NLL_FEATURE_SELECTION_RELIEFF_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Feature selection using the Relieff algorithm. Suitable for multi-class, high number of features and fast.
           It belongs to the filter category. It takes into account the relationships
           between variables.
    */
   template <class Point>
   class FeatureSelectionFilterRelieff : public FeatureSelectionFilter<Point>
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
      FeatureSelectionFilterRelieff( ui32 nbOfFeatures, ui32 steps = 1000, ui32 k = 10 ) : _nbOfFeatures( nbOfFeatures ), _steps( steps ), _k( k )
      {}

   protected:
      core::Buffer1D<bool> _compute( const Database& dat )
      {
         typedef std::pair<double, ui32> Pair;
         typedef Relieff<Point>     Relieff;
         Relieff relief;

         typename Relieff::FeatureRank rank = relief.process( dat, _steps, _k );
         std::vector<Pair> vecs( rank.size() );
         for ( ui32 n = 0; n < vecs.size(); ++n )
            vecs[ n ] = std::make_pair( rank[ n ], n );
         std::sort( vecs.rbegin(), vecs.rend() );

         // score are sorted, the highest ones are selected
         core::Buffer1D<bool> selection( static_cast<ui32>( vecs.size() ) ); // by default no features are selected
         for ( ui32 n = 0; n < _nbOfFeatures; ++n )
            selection[ vecs[ n ].second ] = true;
         return selection;
      }

   private:
      ui32     _nbOfFeatures;
      ui32     _steps;
      ui32     _k;
   };
}
}

#endif
