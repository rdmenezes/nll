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

#ifndef FEATURE_TRANSFORMATION_NORMALIZATION_H_
# define FEATURE_TRANSFORMATION_NORMALIZATION_H_

#pragma warning( push )
#pragma warning( disable:4244 ) // conversion from 'const double' to XXX, possible loss of data

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief This preprocessing class will normalize a database so that the mean( input ) = 0
    and variance( input ) = 1

    For each feature, apply f_x = ( f_x - mean( f_x ) ) / var( f_x )
    */
   template <class Point>
   class FeatureTransformationNormalization : public FeatureTransformation<Point>
   {
      typedef Normalize<Point>               Impl;
   public:
      typedef core::Buffer1D<double>         Vector;
      typedef FeatureTransformation<Point>   Base;

      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   public:
      FeatureTransformationNormalization()
      {}

      FeatureTransformationNormalization( const Vector& mean, const Vector& var ) : _impl( mean, var )
      {
      }

      /**
       @brief Computes the mean and variance independantly for each feature. Only the
              LEARNING|VALIDATION data are used to compute the parameters.
       */
      template <class Database>
      bool compute( const Database& points )
      {
         Database learningDat = core::filterDatabase( points,
                                                      core::make_vector<size_t>( Database::Sample::LEARNING,
                                                                                 Database::Sample::VALIDATION ),
                                                      Database::Sample::LEARNING );
         core::DatabaseInputAdapterRead<Database> databaseAdapter( learningDat );
         return _impl.compute( databaseAdapter );
      }

      /**
       @brief Process a point according to the transformation.
       */
      virtual Point process( const Point& p ) const
      {
         return _impl.process( p );
      }

      /**
       @return the mean for each feature
       */
      const Vector& getMean() const
      {
         return _impl.getMean();
      }

      /**
       @return the variance
       */
      const Vector& getVariance() const
      {
         return _impl.getVariance();
      }

      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i )
      {
         _impl.read( i );
      }

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const
      {
         _impl.write( o );
      }

   protected:
      Impl     _impl;
   };
}
}

#pragma warning( pop )

#endif
