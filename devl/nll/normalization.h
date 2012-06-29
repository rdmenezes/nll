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

#ifndef NLL_ALGORITM_NORMALIZATION_H_
# define NLL_ALGORITM_NORMALIZATION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Normalize features of a sequence of points. Each feature is independantly normalized
           for (mean,variance)= (0,1)

           A point must define operator[](size_t) and size_t size() and constructible(size)
    */
   template <class Point>
   class Normalize
   {
   public:
      typedef core::Buffer1D<double>   Vector;

   public:
      virtual ~Normalize()
      {}

      Normalize()
      {}

      Normalize( const Vector& mean, const Vector& var )
      {
         _mean.clone( mean );
         _var.clone( var );
      }

      template <class PPoint>
      Normalize( const Normalize<PPoint>& normalize )
      {
         _mean.clone( normalize.getMean() );
         _var.clone( normalize.getVariance() );
      }

      /**
       Compute the mean and variance of every features.

       Points must define const Point& operator[](size_t) and size_t size() const
       */
      template <class Points>
      bool compute( const Points& points )
      {
         if ( !points.size() )
            return false;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, "start compute normalization..." );

         const size_t nbFeatures = (size_t)points[ 0 ].size();
         _mean = Vector( nbFeatures );
         _var = Vector( nbFeatures );
         size_t nbSamples = static_cast<size_t>( points.size() );
         for ( size_t n = 0; n < points.size(); ++n )
         {
            for ( size_t nn = 0; nn < nbFeatures; ++nn )
               _mean[ nn ] += points[ n ][ nn ];
         }

         for ( size_t nn = 0; nn < nbFeatures; ++nn )
            _mean[ nn ] /= nbSamples;

         for ( size_t n = 0; n < points.size(); ++n )
         {
            for ( size_t nn = 0; nn < nbFeatures; ++nn )
            {
               double val = points[ n ][ nn ] - _mean[ nn ];
               _var[ nn ] += val * val;
            }
         }

         for ( size_t nn = 0; nn < nbFeatures; ++nn )
         {
            if ( fabs( _var[ nn ] ) < 1e-10 )
            {
               nllWarning( "null variance, this attribut should be discarded instead" );
               _var[ nn ] = 1;
            } else {
               _var[ nn ] /= nbSamples;
            }
         }

         std::stringstream ss1;
         ss1 << " mean vector=";
         std::stringstream ss2;
         ss2 << " variance vector=";

         for ( size_t nn = 0; nn < nbFeatures; ++nn )
         {
            ss1 << _mean[ nn ] << " ";
            ss2 << _var[ nn ] << " ";
         }

         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss1.str() );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss2.str() );
         return true;
      }

      /**
       @brief write the state of the class to a stream
       */
      bool write( std::ostream& o ) const
      {
         _mean.write( o );
         _var.write( o );
         return true;
      }

      /**
       @brief restore the state of the class from a stream
       */
      bool read( std::istream& i )
      {
         _mean.read( i );
         _var.read( i );
         return true;
      }

      /**
       @brief normalize a point
       */
      Point process( const Point& p ) const
      {
         ensure( p.size() == _var.size(), "error size" );

         Point res( p.size() );
         for ( size_t nn = 0; nn < p.size(); ++nn )
            res[ nn ] = ( p[ nn ] - _mean[ nn ] ) / _var[ nn ];
         return res;
      }

      /**
       @brief take a processed point and reconstruct it (unnormalize it!)
       */
      Point reconstruct( const Point& p ) const
      {
         assert( p.size() == _mean.size() );
         Point r( _mean.size() );
         for ( size_t nn = 0; nn < p.size(); ++nn )
            r[ nn ] = p[ nn ] * _var[ nn ] + _mean[ nn ];
         return r;
      }

      /**
       @return the mean for each feature
       */
      const Vector& getMean() const
      {
         return _mean;
      }

      /**
       @return the variance
       */
      const Vector& getVariance() const
      {
         return _var;
      }

   protected:
      Vector   _mean;
      Vector   _var;
   };
}
}

#endif
