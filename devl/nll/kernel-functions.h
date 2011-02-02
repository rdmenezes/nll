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

#ifndef NLL_ALGORITHM_KERNEL_FUNCTION_H_
# define NLL_ALGORITHM_KERNEL_FUNCTION_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Defines A Kernel function prototype
    */
   template <class Point>
   class KernelFunction
   {
   public:
      
      virtual double operator()( const Point& p1, const Point& p2 ) const = 0;
      virtual KernelFunction* clone() const = 0;
      virtual bool read( std::istream& i ) = 0;
      virtual bool write( std::ostream& o ) const = 0;
   };

   /**
    @ingroup algorithm
    @brief Defines a polynomial kernel function K(x, y)=(x.y)^degree

    Point must define double operator[](unsigned) const and size()
    */
   template <class Point>
   class KernelPolynomial : public KernelFunction<Point>
   {
   public:
      KernelPolynomial( double degree ) : _degree( degree )
      {
         ensure( degree >= 1, "kernel degree error" );
      }

      KernelPolynomial( std::istream& i )
      {
         read( i );
      }

      virtual double operator()( const Point& p1, const Point& p2 ) const
      {
         assert( p1.size() == p2.size() );
         double sum = 0;
         for ( ui32 n = 0; n < p1.size(); ++n )
            sum += p1[ n ] * p2[ n ];
         return pow( sum, _degree );
      }

      virtual KernelPolynomial* clone() const
      {
         return new KernelPolynomial( _degree );
      }

      virtual bool read( std::istream& i )
      {
         core::read<double>( _degree, i );
         return true;
      }

      virtual bool write( std::ostream& o ) const
      {
         core::write<double>( _degree, o );
         return true;
      }

   private:
      double _degree;
   };

   /**
    @ingroup algorithm
    @brief Defines a RBF kernel exp(-norm( x1, x2 )/var)
    */
   template <class Point>
   class KernelRbf  : public KernelFunction<Point>
   {
   public:
      KernelRbf( double var ) : _var( var )
      {
      }

      KernelRbf( std::istream& i )
      {
         read( i );
      }

      double getVar() const
      {
         return _var;
      }

      virtual double operator()( const Point& p1, const Point& p2 ) const
      {
         assert( p1.size() == p2.size() );
         double sum = 0;
         for ( ui32 n = 0; n < p1.size(); ++n )
         {
            double val = p1[ n ] - p2[ n ];
            sum += val * val;
         }
         return exp( - sum / ( _var ) );
      }

      virtual KernelRbf* clone() const
      {
         return new KernelRbf( _var );
      }

      virtual bool read( std::istream& i )
      {
         core::read<double>( _var, i );
         return true;
      }

      virtual bool write( std::ostream& o ) const
      {
         core::write<double>( _var, o );
         return true;
      }

   private:
      double _var;
   };
}
}

#endif
