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

#ifndef NLL_ALGORITHM_ICA_H_
# define NLL_ALGORITHM_ICA_H_

namespace nll
{
namespace algorithm
{
   /**
    @ingroup algorithm
    @brief Good general purpose constrast function
    @see IndependentComponentAnalysis
    */
   class TraitConstrastFunctionG1
   {
   public:
      /**
       @param a1 it must be 1 <= a1 <= 2
       */
      TraitConstrastFunctionG1( double a1 = 1.0 ) : _a1( a1 )
      {}

      inline double evaluate( double val ) const
      {
         return log ( cosh( _a1 * val ) ) / _a1;
      }

      inline double evaluateDerivative( double val ) const
      {
         return tanh( _a1 * val );
      }

   private:
      double _a1;
   };

   /**
    @ingroup algorithm
    @brief To use when the independent components are highly super-Gaussian, or when robustness is very important
    @see IndependentComponentAnalysis
    */
   class TraitConstrastFunctionG2
   {
   public:
      /**
       @param a2 must be close to 1
       */
      TraitConstrastFunctionG2()
      {}

      inline double evaluate( double val ) const
      {
         return - exp( - val * val / 2 );
      }

      inline double evaluateDerivative( double val ) const
      {
         return val * exp( - val * val / 2 );
      }
   };

   class TraitConstrastFunctionG5
   {
   public:
      /**
       @param a1 it must be 1 <= a1 <= 2
       */
      TraitConstrastFunctionG5( double a1 = 1.0 ) : _a1( a1 )
      {}

      inline double evaluate( double val ) const
      {
         return tanh( _a1 * val );
      }

      inline double evaluateDerivative( double val ) const
      {
         const double v = tanh( _a1 * val );
         return _a1 * ( 1 - v * v );
      }

   private:
      double _a1;
   };

   class TraitConstrastFunctionG6
   {
   public:
      /**
       @param a2 must be close to 1
       */
      TraitConstrastFunctionG6()
      {}

      inline double evaluate( double val ) const
      {
         return val * exp( - val * val / 2 );
      }

      inline double evaluateDerivative( double val ) const
      {
         const double v = val * val;
         return ( 1 - v ) * exp( - v / 2 );
      }
   };

   /**
    @ingroup algorithm
    @brief kurtosis, is justified on statistical grounds only for estimating sub-Gaussian independent
           components when there are no outliers
    */
   class TraitConstrastFunctionG3
   {
   public:
      /**
       @param a2 must be close to 1
       */
      TraitConstrastFunctionG3()
      {}

      inline double evaluate( double val ) const
      {
         const double v = val * val;
         return v * v / 4;
      }

      inline double evaluateDerivative( double val ) const
      {
         return val * val * val;
      }
   };

   /**
    @ingroup algorithm
    */
   class TraitConstrastFunctionG4
   {
   public:
      /**
       @param a2 must be close to 1
       */
      TraitConstrastFunctionG4()
      {}

      inline double evaluate( double val ) const
      {
         const double v = val * val * val;
         return v;
      }

      inline double evaluateDerivative( double val ) const
      {
         return 3 * val * val;
      }
   };


   /**
    @ingroup algorithm
    @brief Independent Component Analysis, implementaing the FastICA algorithm

    Independent component analysis (ICA) is a statistical method for transforming an observed multidimensional
    random vector into components that are statistically as independent from each other as possible. For example,
    there are several emitters and recepters, the recepters receive a mixed signal of sources
    and ICA is trying to unmix this mixed signal.
    
    It is assumed that the emitters are not following a gaussian distribution (or at maximum one can)

    it is implementing this paper: http://www.cs.helsinki.fi/u/ahyvarin/papers/TNN99new.pdf
	Implementing the stabilized version of the FastICA algorithm, optimizing the nongaussianity criterion.
	
	Nongaussianity is here measured by the approximation of negentropy
	
	Each unmixing component is computed one by one by reorthogonalizing the bases with the previously computed
	unmixing components with gram schmidt

    The learning process is:
    - PCA transform of the input data
    - normalization (0 mean, 1 variance)
    - run FastICA algorthm

    @note G4-G6 seem to work better
    */
   template <class TraitConstrastFunction = TraitConstrastFunctionG5>
   class IndependentComponentAnalysis
   {
   public:
      typedef core::Matrix<double>     Matrix;
      typedef core::Buffer1D<double>   Vector;
      typedef std::vector<Vector>      Vectors;

   public:
      IndependentComponentAnalysis( TraitConstrastFunction contrast = TraitConstrastFunction() ) : _contrast( contrast )
      {}

      /**
       @brief This will automatically normalize the data (0 mean, 1 variance) and decorelate the input data by PCA
              Finally, compute the FastICA algorithm
       @note if dataRatioToUsePerCycle != 1 the algorithm doesn't converge...
       */
      template <class Points>
      void compute( const Points& points, ui32 nbSource, double step = 0.1, ui32 runReorthogonalizationEveryXCycle = 1, double epsilon = 1e-6, ui32 maxNumOfCycles = 1000 )
      {
         if ( points.size() == 0 )
            return;
         const ui32 nbDim = static_cast<ui32>( points[ 0 ].size() );
         ensure( nbSource <= nbDim, "you can't have more source than signals" );
         _unmixingSignal.clear();

         std::stringstream ss;
         ss << "independent component analysis, nb selected independent sources=" << nbSource;
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );

         std::vector<typename Points::value_type> normalizedPoints;
         _whitening( points, normalizedPoints );

         const ui32 nbSamples = static_cast<ui32>( normalizedPoints.size() );
         if ( nbSamples == 0 )
            throw std::runtime_error( "no sample selected!" );

         Vectors unmixing;
         unmixing.reserve( nbSource );
         for ( ui32 source = 0; source < nbSource; ++source )
         {
            // init of the source, initialize a vector with random small values
            unmixing.push_back( _generateRandomVector( nbDim ) );

            // run the cycle
            for ( ui32 cycle = 0; cycle < maxNumOfCycles; ++cycle )
            {
               Vector oldMixing;
               oldMixing.clone( unmixing[ source ] );

               std::vector<double> meanA( nbDim );
               double meanB = 0;
               double beta = 0;
               for ( ui32 sample = 0; sample < nbSamples; ++sample )
               {
                  // accum = w^t * x
                  double accum = 0;
                  for ( ui32 nn = 0; nn < nbDim; ++nn )
                     accum += unmixing[ source ][ nn ] * normalizedPoints[ sample ][ nn ];

                  const double gwx = _contrast.evaluate( accum );
                  const double gderivwx = _contrast.evaluateDerivative( accum );
                  for ( ui32 nn = 0; nn < nbDim; ++nn )
                  {
                     meanA[ nn ] += normalizedPoints[ sample ][ nn ] * gwx;
                  }
                  meanB += gderivwx;
                  beta += accum * gwx;
               }

               // compute the mean value
               beta /= nbSamples;
               for ( ui32 nn = 0; nn < nbDim; ++nn )
               {
                  meanA[ nn ] /= nbSamples;
               }
               meanB /= nbSamples;

               // update the mixing matrix
               double norm = 0;
               for ( ui32 nn = 0; nn < nbDim; ++nn )
               {
                  const double val = unmixing[ source ][ nn ] - step * ( meanA[ nn ] - beta * unmixing[ source ][ nn ] ) / ( meanB - beta );
                  //const double val = meanA[ nn ] - meanB * unmixing[ source ][ nn ];

                  norm += val * val;
                  unmixing[ source ][ nn ] = val;
               }

               // normalize the source
               norm = sqrt( norm );
               ensure( norm, "null norm!" );
               for ( ui32 nn = 0; nn < nbDim; ++nn )
               {
                  unmixing[ source ][ nn ] /= norm;
               }

               if ( ( cycle % runReorthogonalizationEveryXCycle ) == 0 )
               {
                  _reorthogonalize( unmixing );
               }

               const double diff = core::norm2( oldMixing, unmixing[ source ] );

               std::stringstream sss;
               sss << " source=" << source << " cycle=" << cycle << " diffError=" << diff;
               core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, sss.str() );

               if ( fabs( diff ) < epsilon )
                  break;
            }

            _reorthogonalize( unmixing );
         }

         // rotate the unmixing matrix
         Vectors unmixingR;
         unmixingR.reserve( nbSource );
         for ( ui32 source = 0; source < nbSource; ++source )
         {
            Vector w = unmixing[ source ];
            w = Matrix( w, 1, w.size() ) * _pca.getProjection();

            unmixingR.push_back( w );
         }

         for ( ui32 source = 0; source < nbSource; ++source )
         {
            std::stringstream sss;
            sss << " umixing source tfm=" << source << " cycle=";
            for ( ui32 dim = 0; dim < nbDim; ++dim )
            {
               sss << unmixing[ source ][ dim ] << " ";
            }
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, sss.str() );
         }

         for ( ui32 source = 0; source < nbSource; ++source )
         {
            std::stringstream sss;
            sss << " umixing source tfm rot=" << source << " cycle=";
            for ( ui32 dim = 0; dim < nbDim; ++dim )
            {
               sss << unmixingR[ source ][ dim ] << " ";
            }
            core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, sss.str() );
         }

         _unmixingSignal = unmixingR;
      }

      /**
       @brief Transform a point to point with feature as independent as possible
       */
      template <class Point>
      Point transform( const Point& p ) const
      {
         assert( _unmixingSignal.size() /*&& _unmixingSignal[ 0 ].size() == p.size()*/ );

         const ui32 nbSources = static_cast<ui32>( _unmixingSignal.size() );
         Vector input( static_cast<ui32>( p.size() ) );
         for ( ui32 n = 0; n < p.size(); ++n )
         {
            input[ n ] = p[ n ] - _pca.getMean()[ n ];
         }

         Point out( nbSources );
         for ( ui32 src = 0; src < nbSources; ++src )
         {
            Matrix proj( _unmixingSignal[ src ], 1, _unmixingSignal[ src ].size() );
            Vector inputProj = proj * Matrix( input, (ui32)input.size(), 1 ) + proj * Matrix( _pca.getMean(), (ui32)p.size(), 1 );
            assert( inputProj.size() == 1 );
            out[ src ] = inputProj[ 0 ];
         }
         return out;
      }

      const TraitConstrastFunction& getConstrastFunction() const
      {
         return _contrast;
      }

      const Vectors& getUnmixingMatrix() const
      {
         return _unmixingSignal;
      }

      const PrincipalComponentAnalysis<Vectors>& getPcaTransform() const
      {
         return _pca;
      }

      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i )
      {
         core::read<Vectors>( _unmixingSignal, i );
         _pca.read( i );
      }

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const
      {
         core::write<Vectors>( _unmixingSignal, o );
         _pca.write( o );
      }

   private:
      // whiten the data: 0 mean, 1 variance, 0 correlation
      template <class Points, class Points2>
      void _whitening( const Points& points, Points2& out )
      {
         out = Points2( points.size() );

         // rotate data
         PrincipalComponentAnalysis<Points> pca;
         pca.compute( points, static_cast<ui32>( points[ 0 ].size() ) );

         Matrix mean = pca.getMean();
         Matrix transform( pca.getEigenValues().size(), mean.size() );
         for ( ui32 y = 0; y < transform.sizex(); ++y )
         {
            ensure( fabs( pca.getEigenValues()[ pca.getPairs()[ y ].second ] ) >= 1e-12, "an eigen value is null! reduce the dimension" ); // TODO reduce the dimension automatically instead
            const double scaling = 1 / sqrt( pca.getEigenValues()[ pca.getPairs()[ y ].second ] );
            for ( ui32 x = 0; x < transform.sizex(); ++x )
            {
               // we redimension the projection
               // each row in <transform> is a projection, from the higest EIV to the lowest
               transform( y, x ) = scaling * pca.getEigenVectors()( x, pca.getPairs()[ y ].second );
            }
         }
         
         _pca = PrincipalComponentAnalysis<Vectors>( mean, transform );
         PrincipalComponentAnalysis<Points> pcaTemp( mean, transform );
         for ( ui32 n = 0; n < points.size(); ++n )
         {
            out[ n ] = pcaTemp.process( points[ n ] );
         }

         std::stringstream ss;
         ss << "whitening matrix=" << std::endl;
         _pca.getProjection().print( ss );
         core::LoggerNll::write( core::LoggerNll::IMPLEMENTATION, ss.str() );
      }

      Vector _generateRandomVector( ui32 nbDim) const
      {
         Vector randomVector( nbDim );
         double accumRandom = 0;
         for ( ui32 nn = 0; nn < nbDim; ++nn )
         {
            const double val = core::generateUniformDistribution( -0.1, 0.1 );
            accumRandom += val * val;
            randomVector[ nn ] = val;
         }

         return randomVector;
      }

      // assume m[0..source][...] are computed and m[0..source-1] are orthonormals, orthonormalize the source^th source
      void _reorthogonalize( Vectors& vectors )
      {
         typedef core::GramSchmidt<> GramSchmidt;
         GramSchmidt::orthogonalizeLast( vectors );
      }

   private:
      TraitConstrastFunction  _contrast;
      Vectors                 _unmixingSignal;

      PrincipalComponentAnalysis<Vectors> _pca;
   };
}
}

#endif
