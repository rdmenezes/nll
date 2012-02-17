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

#ifndef NLL_FEATURE_TRANSFORMATION_H_
# define NLL_FEATURE_TRANSFORMATION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Define a feature transformation algorithm. From a set of features, another set is computed.
    */
   template <class Point>
   class FeatureTransformation
   {
   public:
      /**
       @brief Process a point according to the transformation.
       */
      virtual Point process( const Point& p ) const = 0;

      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i ) = 0;

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const = 0;

      /**
       @brief Virtual destructor
       */
      virtual ~FeatureTransformation(){}

   public:
      /**
       @brief Read from a file the status of the algorithm
       */
      void read( const std::string& f )
      {
         std::ifstream i( f.c_str(), std::ios::binary );
         ensure( i.is_open(), "file not found" );
         read( i );
      }

      /**
       @brief Write to a file the status of the algorithm
       */
      void write( const std::string& f )
      {
         std::ofstream o( f.c_str(), std::ios::binary );
         ensure( o.is_open(), "file not found" );
         write( o );
      }

      /**
       @brief Process a full database according to the transformation defined by <code>process</code>
       */
      template <class TDatabase>
      TDatabase transform( const TDatabase& dat ) const
      {
         TDatabase newDat;
         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            typename TDatabase::Sample  sample;
            sample.input   = process( dat[ n ].input );
            sample.output  = dat[ n ].output;
            sample.type    = dat[ n ].type;
            sample.debug   = dat[ n ].debug;
            newDat.add( sample );
         }
         return newDat;
      }
   };
}
}

#endif
