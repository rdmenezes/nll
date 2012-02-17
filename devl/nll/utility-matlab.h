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

#ifndef NLL_CORE_UTILITY_MATLAB_H_
# define NLL_CORE_UTILITY_MATLAB_H_

namespace nll
{
namespace core
{
   /**
    @brief Utility to export a vector of vectors as a matlab Matrix file
    */
   template <class Vectors>
   void exportVectorToMatlabAsRow( const Vectors& v, const std::string& file )
   {
      std::ofstream f( file.c_str() );
      if ( !f.good() )
         throw std::runtime_error( "Problem!" );

      f << "[";
      for ( ui32 n = 0; n < v.size(); ++n )
      {
         for ( ui32 nn = 0; nn < v[ n ].size(); ++nn )
            f << v[ n ][ nn ] << " ";
         f << "; ";
      }

      f << "]" << std::endl;
   }

   /**
    @brief Utility to import a matlab Matrix file (exported with dlmwrite) as a vector of vectors
    */
   template <class Vectors>
   Vectors readVectorFromMatlabAsColumn( const std::string& file )
   {
      Vectors v;

      std::vector< std::vector< double > > vectors;
      std::ifstream f( file.c_str() );
      if ( !f.good() )
         throw std::runtime_error( "Problem!" );

      while ( !f.eof() )
      {
         std::string line;
         std::getline( f, line );
         std::vector<const char*> splits = core::split( line, ',' );
         
         std::vector<double> record( splits.size() );
         for ( ui32 n = 0; n < splits.size(); ++n )
            record[ n ] = core::str2val<double>( splits[ n ] );
         if ( record.size() )
            vectors.push_back( record );
      }

      typedef typename Vectors::value_type Vector;

      v = Vectors( vectors.size() );
      for ( ui32 n = 0; n < vectors.size(); ++n )
      {
         Vector vv( vectors[ n ].size() );
         for ( ui32 nn = 0; nn < vectors[ n ].size(); ++nn )
         {
            vv[ nn ] = vectors[ n ][ nn ];
         }
         v[ n ] = vv;
      }

      return v;
   }
}
}

#endif
