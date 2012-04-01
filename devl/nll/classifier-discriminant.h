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

#ifndef CLASSIFIER_DISCRIMINANT_H_
# define CLASSIFIER_DISCRIMINANT_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief quadratic discriminant analysis for classification
    */
   template <class Point>
   class ClassifierDiscriminant : public Classifier<Point>
   {
      typedef Classifier<Point>  Base;

   public:
      // don't override these
      using Base::read;
      using Base::write;
      using Base::createOptimizer;
      using Base::test;
      using Base::learnTrainingDatabase;

      // for gcc...
      typedef typename Base::Result                   Result;
      typedef typename Base::Database                 Database;
      typedef typename Base::Class                    Class;

   public:
      /**
        Create the parameter specification
             - no parameters
       */
      static ParameterOptimizers buildParameters()
      {
         ParameterOptimizers parameters;
         return parameters;
      }

   public:
      /**
       @param defines the weight decay to be used
       */
      ClassifierDiscriminant() : Base( buildParameters() )
      {}

      virtual ClassifierDiscriminant* deepCopy() const
      {
         ClassifierDiscriminant* c = new ClassifierDiscriminant();
         c->_qda = _qda;
         c->_crossValidationBin = this->_crossValidationBin;
         return c;
      }

      virtual void read( std::istream& i )
      {
         _qda.read( i );
      }

      virtual void write( std::ostream& o ) const
      {
         _qda.write( o );
      }

      virtual Class test( const Point& p ) const
      {
         return _qda.test( p );
      }

      virtual Class test( const Point& p, core::Buffer1D<double>& probability ) const
      {
         return _qda.test( p, &probability );
      }

      /**
       param parameters:
             - no parameters
       */
      virtual void learn( const Database& dat, const core::Buffer1D<f64>& )
      {
         _qda.compute( dat );
      }

   private:
      QuadraticDiscriminantAnalysis _qda;
   };
}
}

#endif
