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

#ifndef NLL_FEATURE_TRANSFORMATION_PCA_H_
# define NLL_FEATURE_TRANSFORMATION_PCA_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Define a feature transformation algorithm usin PCA. It is suitable for very high
           dimentional dataset.

    The PCA is decorrelating the features, it is assumed the variance of the feature
    is the important information. Internally, it is selecting the highest (egigen vector, eigen value)
    which "rotates" the data so that they are not correlated anymore.

    If the features have not a normal distribution or the infomation is not in the variance,
    this preprocessing should not be used.

    @see Pca
    */
   template <class Point>
   class FeatureTransformationPca : public FeatureTransformation<Point>
   {
   public:
      typedef FeatureTransformation<Point>          Base;

   public:
      // don't override these
      using Base::process;
      using Base::read;
      using Base::write;

   private:
      typedef typename Classifier<Point>::Database          PcaDatabase;
      typedef core::DatabaseInputAdapterRead<PcaDatabase>   Points;
      typedef PrincipalComponentAnalysis<Points>            Pca;

   public:
      FeatureTransformationPca() : _pca( 0 )
      {
      }

      ~FeatureTransformationPca()
      {
         delete _pca;
      }

      /**
       @brief Compute the principal components of the database. Only the <code>LEARNING|VALIDATION</code> are
              used.
       @return true if successful
       */
      template <class Database>
      bool compute( const Database& _dat, size_t nbComponents )
      {
         if ( _pca )
            delete _pca;
         _pca = new Pca();
         Database learningDat = core::filterDatabase( _dat,
                                                      core::make_vector<size_t>( Database::Sample::LEARNING,
                                                                                 Database::Sample::VALIDATION ),
                                                      Database::Sample::LEARNING );
         core::DatabaseInputAdapterRead<Database> databaseAdapter( learningDat );
         return _pca->compute( databaseAdapter, nbComponents );
      }

      /**
       @brief Returns the projection found.
       */
      const core::Matrix<double>& getProjection() const
      {
         ensure( _pca, "no PCA computed" );
         return _pca->getProjection();
      }

      /**
       @brief Process a point according to the transformation.
       */
      virtual Point process( const Point& p ) const
      {
         assert( _pca );
         return _pca->process( p );
      }
   
      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i )
      {
         if ( !_pca )
            _pca = new Pca();
         _pca->read( i );
      }

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const
      {
         ensure( _pca, "no PCA computed" );
         _pca->write( o );
      }

   private:
      // copy disabled
      FeatureTransformationPca& operator=( const FeatureTransformationPca& );
      FeatureTransformationPca( const FeatureTransformationPca& );

   protected:
      Pca*     _pca;
   };
}
}

#endif
