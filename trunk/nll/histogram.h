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

#ifndef NLL_CORE_HISTOGRAM_H_
# define NLL_CORE_HISTOGRAM_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Class allowing histogramming of any data, provided std::min/max, std::numeric_limits<>::max/min and access operators are provided
    */
   template < class CountsStore = std::vector<ui32> >
   class Histogram
   {
   public:
      typedef CountsStore  Counts;

   public:
      Histogram() : _min( std::numeric_limits<double>::min() ), _max( std::numeric_limits<double>::max() )
      {
      }

      /**
       @brief Returns the min bound of the histogram, i.e., _counts[ 0 ] map the _min value
       */
      double getMin() const
      {
         return _min;
      }

      /**
       @brief Returns the max bound of the histogram, i.e., _counts[ nbBins - 1 ] map the _max value
       */
      double getMax() const
      {
         return _max;
      }

      /**
       @brief Returns the distance between two bins of _counts
       */
      double getBinInterval() const
      {
         return _binInterval;
      }

      const Counts& getCounts() const
      {
         return _counts;
      }

      /**
       @brief Compute the histogram of the data given the number of bins.

       @note the min and max of the histogram will be automatically computed
       */
      template <class Data>
      void compute( const Data& data, ui32 nbBins )
      {
         typedef typename Data::value_type      value_type;
         typedef typename Data::const_iterator  const_iterator;

         value_type min = std::numeric_limits<value_type>::max();
         value_type max = std::numeric_limits<value_type>::min();

         // compute min/max
         for ( const_iterator it = data.begin(); it != data.end(); ++it )
         {
            min = std::min( *it, min );
            max = std::max( *it, max );
         }

         return compute( data, nbBins, min, max );
      }

      /**
       @brief Compute the histogram of the data given the number of bins and the histogram bounds
       */
      template <class Data>
      void compute( const Data& data, ui32 nbBins, const typename Data::value_type min, const typename Data::value_type max )
      {
         typedef typename Data::value_type      value_type;
         typedef typename Data::const_iterator  const_iterator;

         ensure( min <= max, "error ordering min/max" );
         ensure( nbBins >= 1, "we need at least one bin!" );

         _min = static_cast<double>( min );
         _max = static_cast<double>( max );
         _binInterval = ( static_cast<double>( max - min ) ) / ( nbBins - 1 );
         _counts.resize( nbBins );
         const int indexMax = static_cast<int>( nbBins ) - 1;

         for ( const_iterator it = data.begin(); it != data.end(); ++it )
         {
            int val = static_cast<ui32>( ( static_cast<double>( *it ) - _min ) / _binInterval );
            if ( val < 0 )
            {
               val = 0;
            }
            if ( val > indexMax )
            {
               val = indexMax;
            }
            ++_counts[ val ];
         }
      }

   private:
      double   _min;
      double   _max;
      double   _binInterval;
      Counts   _counts;
   };
}
}

#endif