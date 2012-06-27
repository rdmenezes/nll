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

#ifndef NLL_ALGORITHM_TREE_CONTINUOUS_SPLITTING_CRITERIA_H_
# define NLL_ALGORITHM_TREE_CONTINUOUS_SPLITTING_CRITERIA_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Implementation of split points in decision tree
    */
   template <class DatabaseT>
   class ContinuousSplittingCriteria
   {
   public:
      typedef DatabaseT Database;
      typedef typename Database::Sample::Input::value_type     value_type;

   public:
      virtual ~ContinuousSplittingCriteria()
      {}

      /**
       @brief Compute the splitting points for the considered feature
       @param dat the database to operate on
       @param featureId the feature considered
       @param outSplits the computed splits
       */
      virtual void computeSplits( const Database& dat, ui32 featureId, std::vector<value_type>& outSplits ) = 0;
   };

   /**
    @ingroup algorithm
    @brief Implementation of split points in decision tree
    @see http://research.microsoft.com/pubs/65569/splits.pdf
          "Efficient Determination of Dynamic Split Points in a Decision Tree", D. Chickering, C. Meek, R. Rounthwaite
    */
   template <class DatabaseT>
   class SplittingCriteriaGaussianApproximation : public ContinuousSplittingCriteria<DatabaseT>
   {
   public:
      typedef ContinuousSplittingCriteria<DatabaseT>  Base;
      typedef typename Base::Database                 Database;
      typedef typename Base::value_type               value_type;

   public:
      SplittingCriteriaGaussianApproximation( ui32 nbSplits = 10 ) : _nbSplits( nbSplits )
      {
         ensure( _nbSplits > 2, "not enough splits" );
      }

      /**
       @brief Compute the splitting points for the considered feature
       @param dat the database to operate on
       @param featureId the feature considered
       @param outSplits the computed splits
       */
      virtual void computeSplits( const Database& dat, ui32 featureId, std::vector<value_type>& outSplits )
      {
         outSplits.clear();

         // here we fit a gaussian on the considered data
         value_type accumMean = 0;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            accumMean += dat[ n ].input[ featureId ];
         }
         accumMean /= dat.size();

         value_type accumStddev = 0;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            accumStddev += core::sqr( dat[ n ].input[ featureId ] - accumMean );
         }
         accumStddev = sqrt( accumStddev / dat.size() );

         // now get all the splits
         outSplits.reserve( _nbSplits );
         for ( ui32 n = 0; n < _nbSplits; ++n )
         {
            // split ratio from [-1, 1] domain
            const value_type splitRatio = 2 * static_cast<value_type>( n ) / ( _nbSplits - 1 ) - 1;
            const value_type ratio = static_cast<value_type>( core::CumulativeGaussianFunction::erfinv_lut( splitRatio ) );
            const value_type split = accumMean + accumStddev * ratio;
            outSplits.push_back( split );
         }
      }

   private:
      ui32  _nbSplits;
   };

   /**
    @ingroup algorithm
    @brief Implementation of split points in decision tree
    @see http://research.microsoft.com/pubs/65569/splits.pdf
          "Effcient Determination of Dynamic Split Points in a Decision Tree", D. Chickering, C. Meek, R. Rounthwaite
    */
   template <class DatabaseT>
   class SplittingCriteriaUniformApproximation : public ContinuousSplittingCriteria<DatabaseT>
   {
   public:
      typedef ContinuousSplittingCriteria<DatabaseT>  Base;
      typedef typename Base::Database                 Database;
      typedef typename Base::value_type               value_type;

      SplittingCriteriaUniformApproximation( ui32 nbSplits = 10 ) : _nbSplits( nbSplits )
      {
      }

      /**
       @brief Compute the splitting points for the considered feature
       @param dat the database to operate on
       @param featureId the feature considered
       @param outSplits the computed splits
       */
      virtual void computeSplits( const Database& dat, ui32 featureId, std::vector<value_type>& outSplits )
      {
         outSplits.clear();

         // here we fit a gaussian on the considered data
         value_type max = std::numeric_limits<value_type>::min();
         value_type min = std::numeric_limits<value_type>::max();
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            min = std::min( min, dat[ n ].input[ featureId ] );
            max = std::max( max, dat[ n ].input[ featureId ] );
         }

         const value_type interval = ( max - min ) / _nbSplits;

         // now get all the splits
         outSplits.reserve( _nbSplits );
         for ( ui32 n = 0; n < _nbSplits; ++n )
         {
            const value_type split = min + n * ( max - min ) / ( _nbSplits ) - 0.5 * interval;
            outSplits.push_back( split );
         }
      }

   private:
      ui32  _nbSplits;
   };
}
}

#endif
