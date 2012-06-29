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
 
#ifndef NLL_UTILITY_STATISTICS_H_
# define NLL_UTILITY_STATISTICS_H_

namespace nll
{
namespace utility
{
   /**
    @brief This class will compute basic statistics on a set of attributs

    First the attributs must be defined using the <add> methos. Then each value for each attribut
    are added one by one. Finally, <computeStatistics> will compute the statistics

    Attributs can be defined as "error" measure i.e., they were produced by an error functions.
    This will affect the min value and worst values.

    Typical usecase is to report different kind of error measures given a problem.
    */
   class Statistics
   {
      struct Value
      {
         Value()
         {}

         Value( double v, const std::string& i ) : value( v ), id( i )
         {}

         double         value;      // value for this attribut
         std::string    id;         // id identifying this value
      };

      typedef std::vector<Value>             Values;
      typedef std::map<std::string, Values>  Lists;

   public:
      struct Attribut
      {
         Attribut( const std::string& pdescriptor, double pbinSize, bool pisError ) : descriptor( pdescriptor ), binSize( pbinSize ), isError( pisError )
         {}

         std::string    descriptor;    // parameter name
         double         binSize;       // the size to be used in the histogram
         bool           isError;       // indicates this this parameter is an error measure, used to select the worst examples

         bool operator<( const Attribut& p ) const
         {
            return descriptor < p.descriptor;
         }
      };
      typedef std::set<Attribut>   Attributs;

      struct Statistic
      {
         typedef std::vector< std::pair< double, std::string > > WorstValues;
         Statistic()
         {}

         double               mean;
         double               stddev;
         std::vector<size_t>    histogram;
         WorstValues          worstValues;
         double               minValue;
         double               maxValue;
         double               binSize;
      };
      typedef std::map<std::string, Statistic>  AttributsStatistics;

      void add( const Attribut& param )
      {
         std::pair<Attributs::iterator, bool> r = _attributs.insert( param );
         ensure( r.second, "the attribut has already been inserted" );
      }

      void add( const std::string& type, double value, const std::string id )
      {
         Attributs::iterator it = _attributs.find( Attribut( type, 1, true ) ); // we don't care about the other values, we just want to check this is a valid attribut
         ensure( it != _attributs.end(), "can't find attribut" );

         _vals[ type ].push_back( Value( value, id ) );
      }

      static void print( const AttributsStatistics& statistics, std::ostream& o = std::cout )
      {
         for ( AttributsStatistics::const_iterator attribut = statistics.begin(); attribut != statistics.end(); ++attribut )
         {
            o << "attribut=" << attribut->first << std::endl;
            o << " mean = " << attribut->second.mean << std::endl;
            o << " stddev = " << attribut->second.stddev << std::endl;
            o << " minValue = " << attribut->second.minValue << std::endl;
            o << " maxValue = " << attribut->second.maxValue << std::endl;
            o << " histogram = " << std::endl;

            const std::vector<size_t>& histogram = attribut->second.histogram;
            const double binSize = attribut->second.binSize;
            for ( size_t n = 0; n < histogram.size(); ++n )
            {
               if ( !histogram[ n ] )
                  continue;
               const double v1 = n * binSize + attribut->second.minValue;
               const double v2 = v1 + binSize;
               o << "  [" << v1 << "-" << v2 << "]=" << histogram[ n ] << std::endl;
            }

            o << " worst cases = " << std::endl;
            const Statistic::WorstValues& worstCases = attribut->second.worstValues;
            for ( size_t n = 0; n < worstCases.size(); ++n )
            {
               const size_t index = n;
               o << "  case=" << worstCases[ index ].second << " error=" << worstCases[ index ].first << std::endl;
            }
         }
      }

      AttributsStatistics computeStatistics( size_t nbWorstValuesPerAttribut = 300 ) const
      {
         AttributsStatistics attributsStatistics;
         for ( Attributs::const_iterator attribut = _attributs.begin(); attribut != _attributs.end(); ++attribut )
         {
            const std::string attributName = attribut->descriptor;
            Statistic& statistic = attributsStatistics[ attributName ];

            // compute the mean
            double mean = 0;
            Lists::const_iterator it = _vals.find( attributName );
            if ( it == _vals.end() )
               continue; // skip, no values
            const Values& values = it->second;
            if ( values.size() == 0 )
               continue;

            double maxValue = std::numeric_limits<double>::min();
            double minValue = std::numeric_limits<double>::max();
            for ( size_t n = 0; n < values.size(); ++n )
            {
               mean += values[ n ].value;
               if ( values[ n ].value > maxValue )
                  maxValue = values[ n ].value;
               if ( values[ n ].value < minValue )
                  minValue = values[ n ].value;
            }
            mean /= values.size();
            statistic.mean = mean;
            if ( attribut->isError )
            {
               statistic.minValue = 0; // if it is an "error" attribut, the min value is 0!
               minValue = 0;
            } else {
               statistic.minValue = minValue;
            }
            statistic.maxValue = maxValue;

            // compute stddev
            double stddev = 0;
            for ( size_t n = 0; n < values.size(); ++n )
            {
               const double val = values[ n ].value - mean;
               stddev += core::sqr( val );
            }
            stddev = std::sqrt( stddev / values.size() );
            statistic.stddev = stddev;

            // compute the histogram
            const double binSize = attribut->binSize;
            const size_t nbBins = 1 + (size_t)std::ceil( ( maxValue - minValue ) / binSize );
            std::vector<size_t> histogram( nbBins );
            for ( size_t n = 0; n < values.size(); ++n )
            {
               size_t bin = (size_t)( ( values[ n ].value - minValue ) / binSize );
               ++histogram[ bin ];
            }
            statistic.histogram = histogram;
            statistic.binSize = binSize;

            // worst cases
            std::vector< std::pair< double, std::string > > sorted;
            if ( attribut->isError )
            {
               for ( size_t n = 0; n < values.size(); ++n )
               {
                  // if this is an error, then take the biggest |value|
                  sorted.push_back( std::make_pair( fabs( values[ n ].value ), values[ n ].id ) );
               }
            } else {
               for ( size_t n = 0; n < values.size(); ++n )
               {
                  // take the value the furthest from the mean
                  sorted.push_back( std::make_pair( fabs( mean - values[ n ].value ), values[ n ].id ) );
               }
            }
            std::sort( sorted.begin(), sorted.end() );
            statistic.worstValues = Statistic::WorstValues( sorted.rbegin(), sorted.rbegin() + std::min<size_t>( nbWorstValuesPerAttribut, sorted.size() ) );
         }

         return attributsStatistics;
      }

      void clear()
      {
         _vals.clear();
         _attributs.clear();
      }

   private:
      Lists               _vals;            // store the statistics on different attributs
      Attributs           _attributs;      // descriptor of the different attributs to measure
   };
}
}

#endif
