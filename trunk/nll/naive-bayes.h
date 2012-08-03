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

#ifndef NLL_ALGORITHM_NAIVE_BAYES_H_
# define NLL_ALGORITHM_NAIVE_BAYES_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief NaiveBayes classifier implementation

    See "http://en.wikipedia.org/wiki/Naive_Bayes_classifier"
    */
   class NaiveBayes
   {
      typedef core::Buffer1D<double>   Vector;

      struct Class
      {
         Vector      mean;
         Vector      var;

         template <class Point>
         double computeProbability( const Point& p ) const
         {
            double pp = 0;
            for ( size_t n = 0; n < mean.size(); ++n )
            {
               // if we have a log of features, better do a log for numerical stability
               const double pf = _sqrdiv[ n ] * exp( - core::sqr( p[ n ] - mean[ n ] ) / _norm[ n ] );
               pp += log( pf );
            }
            return exp( pp );
         }

         void precompute()
         {
            _sqrdiv = Vector( mean.size() );
            _norm = Vector( mean.size() );
            for ( size_t n = 0; n < mean.size(); ++n )
            {
               _sqrdiv[ n ] = 1.0 / ( sqrt( 2 * nll::core::PI * core::sqr( var[ n ] ) ) + 1e-8 );
               _norm[ n ] = 2.0 * core::sqr( var[ n ] ) + 1e-8;
            }
         }

      private:
         Vector _sqrdiv;
         Vector _norm;
      };
      typedef std::vector<Class> Classes;

   public:
      template <class Database>
      void compute( const Database& database )
      {
         if ( database.size() == 0 )
            return;
         const size_t nbFeatures = database[ 0 ].input.size();

         // compute some constants
         const size_t nbClasses = getNumberOfClass( database );
         _classes = Classes( nbClasses );

         // first filter the training samples
         Database training = core::filterDatabase( database, core::make_vector<size_t>( Database::Sample::LEARNING ), Database::Sample::LEARNING );

         for ( size_t classid = 0; classid < nbClasses; ++classid )
         {
            _classes[ classid ].mean = Vector( nbFeatures );
            _classes[ classid ].var  = Vector( nbFeatures );

            Vector& mean = _classes[ classid ].mean;
            Vector& var  = _classes[ classid ].var;

            // filter the samples by class
            typedef core::DatabaseInputAdapterClass<Database> Adapter;
            Adapter adapter( training, classid );

            // compute the mean
            for ( size_t n = 0; n < adapter.size(); ++n )
            {
               typename Adapter::Point& s = adapter[ n ];
               for ( size_t f = 0; f < nbFeatures; ++f )
               {
                  mean[ f ] += s[ f ];
               }
            }

            for ( size_t f = 0; f < nbFeatures; ++f )
            {
               mean[ f ] /= adapter.size();
            }

            // compute the variance
            for ( size_t n = 0; n < adapter.size(); ++n )
            {
               typename Adapter::Point& s = adapter[ n ];
               for ( size_t f = 0; f < nbFeatures; ++f )
               {
                  double val = s[ f ] - mean[ f ];
                  var[ f ] += core::sqr( val );
               }
            }

            for ( size_t f = 0; f < nbFeatures; ++f )
            {
               var[ f ] /= adapter.size();
            }

            _classes[ classid ].precompute();
         }
      }

      void write( std::ostream& o ) const
      {
         if ( !o.good() )
            throw std::runtime_error( "cannot read from stream" );

         size_t nbC = (size_t)_classes.size();
         core::write<size_t>( nbC, o );
         for ( size_t n = 0; n < nbC; ++n )
         {
            _classes[ n ].mean.write( o );
            _classes[ n ].var.write( o );
         }
      }

      void read( std::istream& i )
      {
         if ( !i.good() )
            throw std::runtime_error( "cannot read from stream" );
         
         // read
         size_t nbC = 0;
         core::read<size_t>( nbC, i );
         _classes = Classes( nbC );
         for ( size_t classid = 0; classid < nbC; ++classid )
         {
            _classes[ classid ].mean.read( i );
            _classes[ classid ].var.read( i );
            _classes[ classid ].precompute();
         }
      }

      template <class Point>
      size_t test( const Point& p, core::Buffer1D<double>* probability = 0 ) const
      {
         double max = (double)INT_MIN;
         double sum = 0;
         size_t index = (size_t)_classes.size();

         if ( probability )
         {
            *probability = core::Buffer1D<double>( (size_t)_classes.size() );
         }

         for ( size_t n = 0; n < (size_t)_classes.size(); ++n )
         {
            double v = _classes[ n ].computeProbability( p );
            if ( v > max )
            {
               max = v;
               index = n;
            }

            if ( probability )
            {
               sum += v;
               ( *probability )[ n ] = v;
            }
         }

         if ( probability )
         {
            for ( size_t n = 0; n < (size_t)_classes.size(); ++n )
               ( *probability )[ n ] /= sum;
         }

         assert( index < _classes.size() );
         return index;
      }

      const Classes& getParameters() const
      {
         return _classes;
      }

   private:
      Classes  _classes;
   };
}
}

#endif