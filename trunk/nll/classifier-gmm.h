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

#ifndef NLL_CLASSIFIER_GMM_H_
# define NLL_CLASSIFIER_GMM_H_

# include "gmm.h"
# include "classifier.h"
# include "io.h"

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Gaussian mixture model algorithm with expectation-maximization for learning

     Points should be viewed as 1 point for the classifier: 1 point is actually composed of a sequence points
     */
   template <class Points>
   class ClassifierGmm : public Classifier<Points>
   {
   public:
      typedef Gmm                                           TGmm;
      typedef std::vector<TGmm>                             Gmms;
      typedef Classifier<Points>                            Base;

      // for gcc...
      typedef typename Base::Point                    Point;
      typedef typename Base::Result                   Result;
      typedef typename Base::Database                 Database;
      typedef typename Base::Class                    Class;

   public:
      // don't override these
      using Base::read;
      using Base::write;
      using Base::createOptimizer;
      using Base::test;
      using Base::learnTrainingDatabase;

   public:
      static ParameterOptimizers buildParameters()
      {
         ParameterOptimizers parameters;
         parameters.push_back( new ParameterOptimizerGaussianInteger( 1, 10, 6, 4, 1 ) );
         parameters.push_back( new ParameterOptimizerGaussianInteger( 3, 15, 6, 4, 1 ) );
         return parameters;
      }

   public:
      ClassifierGmm() : Base( buildParameters() )
      {}
      virtual ClassifierGmm* deepCopy() const
      {
         ClassifierGmm* cgmm = new ClassifierGmm();
         cgmm->_gmms = Gmms( _gmms.size() );
         for ( size_t n = 0; n < _gmms.size(); ++n )
            cgmm->_gmms[ n ].clone( _gmms[ n ] );
         cgmm->_crossValidationBin = this->_crossValidationBin;
         return cgmm;
      }

      virtual void read( std::istream& i )
      {
         size_t size = 0;
         core::read<size_t>( size, i );
         if ( size == 0)
            return;
         for ( size_t n = 0; n < size; ++n )
            core::read<TGmm>( _gmms[ n ], i );
      }

      virtual void write( std::ostream& o ) const
      {
         size_t size = static_cast<size_t>( _gmms.size() );
         core::write<size_t>( size, o );
         for ( size_t n = 0; n < size; ++n )
            core::write<TGmm>( _gmms[ n ], o );
      }


      virtual Class test( const Points& p ) const
      {
         double likelihood_max = INT_MIN;
         size_t class_max = INT_MAX;
         for ( size_t n = 0; n < _gmms.size(); ++n )
         {
            double l = _gmms[ n ].likelihood( p );
            if ( l > likelihood_max )
            {
               class_max = n;
               likelihood_max = l;
            }
         }
         assert( ! core::equal<double>( likelihood_max, INT_MIN ) );
         return class_max;
      }

      virtual Class test( const Point& p, core::Buffer1D<double>& probability ) const
      {
         probability = core::Buffer1D<double>( (size_t)_gmms.size() );

         double likelihood_max = INT_MIN;
         double sum = 0;
         size_t class_max = INT_MAX;
         for ( size_t n = 0; n < _gmms.size(); ++n )
         {
            double l = _gmms[ n ].likelihood( p );
            if ( l > likelihood_max )
            {
               class_max = n;
               likelihood_max = l;
            }

            probability[ n ] = exp( l );
            sum += probability[ n ];
         }

         ensure( sum > 0, "probability error" );
         for ( size_t n = 0; n < _gmms.size(); ++n )
            probability[ n ] /= sum;


         assert( ! core::equal<double>( likelihood_max, INT_MIN ) );
         return class_max;
      }

      /**
       @param parameters parameters of the learning phase 
              - parameters[ 0 ] = nbGaussians
              - parameters[ 1 ] = nbIter
       */
      virtual void learn( const typename Base::Database& dat, const core::Buffer1D<f64>& parameters )
      {
         std::map<size_t, size_t> sizeDat;
         for ( size_t n = 0; n < dat.size(); ++n )
            if ( dat[ n ].type == Base::Sample::LEARNING )
               sizeDat[ dat[ n ].output ] += static_cast<size_t>( dat[ n ].input.size() );

         if ( !sizeDat.size() )
            return;

         for ( size_t n = 0; n < sizeDat.size(); ++n )
         {
            std::map<size_t, size_t>::const_iterator it = sizeDat.find( n );
            assert( it != sizeDat.end() ); // a class is missing : database class must be continuous and starts at 0
            Points points( it->second );
            size_t index = 0;
            for ( size_t n2 = 0; n2 < dat.size(); ++n2 )
            {
               if ( dat[ n2 ].type == Base::Sample::LEARNING && dat[ n2 ].output == n )
                  for ( size_t nn = 0; nn < dat[ n2 ].input.size(); ++nn )
                     points[ index++ ] = dat[ n2 ].input[ nn ];
            }

            TGmm gmm;
            ensure( points.size(), "empty points, error!" );
            size_t pointSize = static_cast<size_t>( points[ 0 ].size() );
            ensure( pointSize, "point dimension is 0, error!" );
            assert( parameters.size() == this->_parametersPrototype.size() );
            gmm.em( points, pointSize, static_cast<size_t>( parameters[ 0 ] ), static_cast<size_t>( parameters[ 1 ] ) );
            _gmms.push_back( gmm );
         }
      }

      const Gmms& getModel() const { return _gmms; }

   private:
      Gmms     _gmms;
   };
}
}

#endif
