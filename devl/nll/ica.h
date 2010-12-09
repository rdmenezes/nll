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
      TraitConstrastFunctionG1( double a1 = 2.0 ) : _a1( a1 )
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
      TraitConstrastFunctionG2( double a2 = 1 ) : _a2( a2 )
      {}

      inline double evaluate( double val ) const
      {
         return - 1 / _a2 * exp( -_a2 * val * val / 2 );
      }

      inline double evaluateDerivative( double val ) const
      {
         return val * exp( -_a2 * val * val / 2 );
      }

   private:
      double _a2;
   };

   /**
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
         return v * v;
      }

      inline double evaluateDerivative( double val ) const
      {
         return val * val * val;
      }
   };


   /**
    @brief Independent Component Analysis, implementaing the FastICA algorithm

    Independent component analysis (ICA) is a statisticalmethod for transforming an observed multidimensional
    random vector into components that are statistically as independent from each other as possible. For example,
    there are several emitters and recepters, the recepters receive a mixed signal of recepters
    and ICA is trying to unmix this mixed signal.
    
    It is assumed that the emitters are not following a gaussian distribution (at maximum one can)

    it is implementing this paper: http://www.cs.helsinki.fi/u/ahyvarin/papers/TNN99new.pdf
    */
   template <class TraitConstrastFunction = TraitConstrastFunctionG1>
   class IndependentComponentAnalysis
   {
   public:
      typedef core::Matrix<double>     Matrix;
      typedef core::Buffer1D<double>   Vector;
      typedef std::vector<Vector>      Vectors;

   public:
      IndependentComponentAnalysis( TraitConstrastFunction contrast = TraitConstrastFunction() ) : _contrast( contrast ), _pca( 1 )
      {}

      /**
       @brief This will automatically normalize the data (0 mean, 1 variance) and decorelate the input data by PCA
              Finally, compute the FastICA algorithm
       @note if dataRatioToUsePerCycle != 1 the algorithm doesn't converge...
       */
      template <class Points>
      void compute( const Points& points, ui32 nbSource, ui32 runReorthogonalizationEveryXCycle = 10, double epsilon = 1e-6, double dataRatioToUsePerCycle = 1.0, ui32 maxNumOfCycles = 1000 )
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

         std::vector<ui32> selector( normalizedPoints.size() );
         for ( ui32 n = 0; n < normalizedPoints.size(); ++n )
            selector[ n ] = n;

         const ui32 nbSamples = static_cast<ui32>( dataRatioToUsePerCycle * normalizedPoints.size() );
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

               // randomize the selection vector
               core::randomize( selector, 0.8f );

               // select the dataRatioToUsePerCycle% of example from the selector vector
               std::vector<double> meanA( nbDim );
               double meanB = 0;
               for ( ui32 sample = 0; sample < nbSamples; ++sample )
               {
                  const ui32 idx = selector[ sample ];
                  double accum = 0;
                  for ( ui32 nn = 0; nn < nbDim; ++nn )
                     accum += unmixing[ source ][ nn ] * normalizedPoints[ idx ][ nn ];

                  const double gwx = _contrast.evaluate( accum );
                  const double gderivwx = _contrast.evaluateDerivative( accum );
                  for ( ui32 nn = 0; nn < nbDim; ++nn )
                  {
                     meanA[ nn ] += normalizedPoints[ idx ][ nn ] * gwx;
                     meanB += gderivwx;
                  }
               }

               // update the mixing matrix
               double norm = 0;
               for ( ui32 nn = 0; nn < nbDim; ++nn )
               {
                  const double val = ( meanA[ nn ] - meanB * unmixing[ source ][ nn ] ) / nbSamples;
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

         _unmixingSignal = unmixing;
      }

      /**
       @brief Transform a point to point with feature as independent as possible
       */
      template <class Point>
      Point transform( const Point& p ) const
      {
         assert( _unmixingSignal.size() && _unmixingSignal[ 0 ].size() == p.size() );

         const ui32 nbSources = static_cast<ui32>( _unmixingSignal.size() );
         const ui32 nbCmp = static_cast<ui32>( _unmixingSignal[ 0 ].size() );
         Vector input( nbSources );
         for ( ui32 src = 0; src < nbSources; ++src )
         {
            input[ src ] = p[ src ];
         }
         input = _pca.process( input );
         input = _normalize.process( input );

         Point out( nbSources );
         for ( ui32 src = 0; src < nbSources; ++src )
         {
            for ( ui32 cmp = 0; cmp < nbCmp; ++cmp )
            {
               out[ src ] += p[ cmp ] * _unmixingSignal[ src ][ cmp ];
            }
         }
         return out;
      }

      const Vectors& getUnmixingMatrix() const
      {
         return _unmixingSignal;
      }

      /**
       @brief Read the transformation from an input stream
       */
      virtual void read( std::istream& i )
      {
         core::read<Vectors>( _unmixingSignal, i );
         _pca.read( i );
         _normalize.read( i );
      }

      /**
       @brief Write the transformation to an output stream
       */
      virtual void write( std::ostream& o ) const
      {
         core::write<Vectors>( _unmixingSignal, o );
         _pca.write( o );
         _normalize.write( o );
      }

   private:
      // whiten the data: 0 mean, 1 variance, 0 correlation
      template <class Points, class Points2>
      void _whitening( const Points& points, Points2& out )
      {
         out = Points2( points.size() );

         // rotate data
         PrincipalComponentAnalysis<Points> pca( static_cast<ui32>( points[ 0 ].size() ) );
         pca.compute( points );
         for ( ui32 n = 0; n < points.size(); ++n )
         {
            out[ n ] = pca.process( points[ n ] );
         }

         Normalize<Points2::value_type> normalize;
         normalize.compute( out );
         for ( ui32 n = 0; n < points.size(); ++n )
         {
            out[ n ] = normalize.process( out[ n ] );
         }

         _normalize = Normalize<Vector>( normalize );
         _pca = PrincipalComponentAnalysis<Vectors>( pca );
      }

      Vector _generateRandomVector( ui32 nbDim) const
      {
         Vector randomVector( nbDim );
         double accumRandom = 0;
         for ( ui32 nn = 0; nn < nbDim; ++nn )
         {
            const double val = core::generateUniformDistribution( -0.001, 0.001 );
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
      Normalize<Vector>                   _normalize;
   };
}
}

#endif
