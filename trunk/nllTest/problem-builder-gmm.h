#ifndef PROBLEM_BUILDER_GMM_H_
# define PROBLEM_BUILDER_GMM_H_

namespace nll
{
   extern const ui8 colors[ 7 ][ 3 ];


   /**
    @brief This will generate a gmm samples for different class and displays results...
    */
   class ProblemBuilderGmm
   {
      typedef algorithm::Gmm::Gaussian Gaussian;
      typedef std::vector<Gaussian>    Gaussians;
      typedef algorithm::Gmm::Vector   Vector;
      typedef algorithm::Gmm::Matrix   Matrix;

   public:
      typedef core::Database< core::ClassificationSample<core::Buffer1D<double>, ui32> > Database;

   public:
      ProblemBuilderGmm()
      {
         _minBound = 0.0;
         _maxBound = 100.0;
      }

      /**
       @brief specify a problem
       */
      void generate( ui32 nbClass, ui32 nbGaussiansPerClass, ui32 nbDimensions, double gaussianScale = 1.0 )
      {
         //
         // generate the GMM
         //
         std::vector<Gaussians> pb( nbClass );
         for ( ui32 n = 0; n < nbClass; ++n )
         {
            // compute the prior
            double sum = 0;
            Vector prior( nbGaussiansPerClass );
            for ( ui32 nn = 0; nn < nbGaussiansPerClass; ++nn )
            {
               prior[ nn ] = core::generateUniformDistribution( 2, 10 );
               sum += prior[ nn ];
            }
            for ( ui32 nn = 0; nn < nbGaussiansPerClass; ++nn )
            {
               prior[ nn ] /= sum;
            }

            // compute mean and cov
            for ( ui32 nn = 0; nn < nbGaussiansPerClass; ++nn )
            {
               Vector mean( nbDimensions );
               Matrix cov( nbDimensions, nbDimensions );
               for ( ui32 dim = 0; dim < nbDimensions; ++dim )
               {
                  cov( dim, dim ) = core::generateUniformDistribution( 0.5, 2 ) * gaussianScale;
                  mean[ dim ] = core::generateUniformDistribution( _minBound, _maxBound );
               }

               pb[ n ].push_back( Gaussian(mean, cov, prior[ nn ] ) );
            }
         }

         // check the gmm are not overlapping too much
         bool isModified = true;
         while ( isModified )
         {
            isModified = false;
            for ( ui32 n = 0; n < nbClass; ++n )
            {
               for ( ui32 nn = 0; nn < nbGaussiansPerClass; ++nn )
               {
                  for ( ui32 n2 = n + 1; n2 < nbClass; ++n2 )
                  {
                     for ( ui32 nn2 = 0; nn2 < nbGaussiansPerClass; ++nn2 )
                     {
                        double dist = core::norm2( pb[ n ][ nn ].mean, pb[ n2 ][ nn2 ].mean );
                        double det1 = sqrt( core::det( pb[ n ][ nn ].covariance ) );
                        double det2 = sqrt( core::det( pb[ n2 ][ nn2 ].covariance ) );
                        /*
                        for ( ui32 i = 0; i < nbDimensions; ++i )
                        {
                           det1 = std::max( pb[ n ][ nn ].covariance( i, i ), det1 );
                           det2 = std::max( pb[ n2 ][ nn2 ].covariance( i, i ), det2 );
                        }*/

                        while ( dist < ( fabs( det1 ) + fabs( det2 ) ) / 2 )
                        {
                           std::cout << "dist=" << dist << " - " << ( fabs( det1 ) + fabs( det2 ) ) << std::endl;
                           const double var = core::generateUniformDistribution( 0, 5 );
                           const ui32 dim = rand() % nbDimensions;
                           const double diff = core::sign( pb[ n ][ nn ].mean[ dim ] - pb[ n2 ][ nn2 ].mean[ dim ] );
                           if ( rand() %2 )
                           {
                              pb[ n ][ nn ].mean[ dim ] += diff * var;
                           } else {
                              pb[ n2 ][ nn2 ].mean[ dim ] -= diff * var;
                           }

                           dist = core::norm2( pb[ n ][ nn ].mean, pb[ n2 ][ nn2 ].mean );
                           det1 = sqrt( core::det( pb[ n ][ nn ].covariance ) );
                           det2 = sqrt( core::det( pb[ n2 ][ nn2 ].covariance ) );
                        }
                     }
                  }
               }
            }
         }

         for ( ui32 n = 0; n < nbClass; ++n )
         {
            _gmms.push_back( algorithm::Gmm( pb[ n ] ) );
         }
      }

      /**
       @brief Generate iid samples from the Gmms
       */
      Database generateSamples( ui32 nb ) const
      {
         typedef Database::Sample Sample;
         core::Database< Sample > dat;
         for ( ui32 gmm = 0; gmm < _gmms.size(); ++gmm )
         {
            std::vector< core::Buffer1D<double> > points;
            _gmms[ gmm ].generate( nb, points );
            for ( ui32 n = 0; n < nb; ++n )
               dat.add( Sample( points[ n ], gmm, (Sample::Type) ( n % 2 ) ) );
         }
         return dat;
      }

      /**
       @brief Print the samples on the image
       */
      void printMap( core::Image<ui8>& im, const Database& dat ) const
      {
         if ( dat[ 0 ].input.size() != 2 )
            throw std::runtime_error( "only dim = 2 !!" );

         for ( ui32 n = 0; n < dat.size(); ++n )
         {
            i32 x = (i32)( dat[ n ].input[ 0 ] - _minBound );
            i32 y = (i32)( dat[ n ].input[ 1 ] - _minBound );
            if ( x >= 0 && x < (int)im.sizex() &&
                 y >= 0 && y < (int)im.sizey() )
            {
               im( x, y, 0 ) = colors[ dat[ n ].output ][ 0 ];
               im( x, y, 1 ) = colors[ dat[ n ].output ][ 1 ];
               im( x, y, 2 ) = colors[ dat[ n ].output ][ 2 ];
            }
         }
      }

      /**
       @brief Print the decision boundary of a Gmm
       */
      void printMap( core::Image<ui8>& im, const algorithm::Gmm& gmm, ui8* color ) const
      {
         const ui32 dx = (ui32)( _maxBound - _minBound );
         const ui32 dy = (ui32)( _maxBound - _minBound );
         for ( ui32 y = 0; y < dy; ++y )
         {
            for ( ui32 x = 0; x < dx; ++x )
            {
               std::vector<double> point( 2 );
               point[ 0 ] = x + _minBound;
               point[ 1 ] = y + _minBound;
               std::vector< std::vector< double > > points;
               points.push_back( point );


               double pb = exp( gmm.likelihood( points ) );
               const double coef = pb * 256;
               im( x, y, 0 ) = (ui8)std::min<double>( im( x, y, 0 ) + color[ 0 ] * coef, 200.0 );
               im( x, y, 1 ) = (ui8)std::min<double>( im( x, y, 1 ) + color[ 1 ] * coef, 200.0 );
               im( x, y, 2 ) = (ui8)std::min<double>( im( x, y, 2 ) + color[ 2 ] * coef, 200.0 );
            }
         }
      }

      /**
       @brief Print the decision boundary of a classifier
       */
      template <class Point>
      core::Image<ui8> printClassifier( const algorithm::Classifier<Point>& c ) const
      {
         const ui32 dx = (ui32)( _maxBound - _minBound );
         const ui32 dy = (ui32)( _maxBound - _minBound );
         core::Image<ui8> im( dx, dy, 3 );
         for ( ui32 y = 0; y < dy; ++y )
         {
            for ( ui32 x = 0; x < dx; ++x )
            {
               Point p( 2 );
               p[ 0 ] = x + _minBound;
               p[ 1 ] = y + _minBound;
               ui32 id = c.test( p );
               im( x, y, 0 ) = (ui8)std::min<double>( 200, colors[ id ][ 0 ] );
               im( x, y, 1 ) = (ui8)std::min<double>( 200, colors[ id ][ 1 ] );
               im( x, y, 2 ) = (ui8)std::min<double>( 200, colors[ id ][ 2 ] );
            }
         }
         return im;
      }

      /**
       @brief Return the probability represented by the generated Gmms
       */
      core::Image<ui8> generateMap() const
      {
         if ( _gmms.size() == 0 || _gmms[ 0 ].getGaussians().size() == 0 )
            throw std::runtime_error( "error!" );
         const ui32 nbDim = _gmms[ 0 ].getGaussians()[ 0 ].mean.size();
         if ( nbDim != 2 )
            throw std::runtime_error( "not handled, ensure dim == 2" );


         const ui32 dx = (ui32)( _maxBound - _minBound );
         const ui32 dy = (ui32)( _maxBound - _minBound );
         core::Image<ui8> im( dx, dy, 3 );

         for ( ui32 y = 0; y < dy; ++y )
         {
            for ( ui32 x = 0; x < dx; ++x )
            {
               std::vector<double> point( 2 );
               point[ 0 ] = x + _minBound;
               point[ 1 ] = y + _minBound;
               std::vector< std::vector< double > > points;
               points.push_back( point );

               double max = -10;
               ui32 maxIndex = 0;
               for ( ui32 n = 0; n < _gmms.size(); ++n )
               {
                  double pb = exp( _gmms[ n ].likelihood( points ) );
                  if ( pb > max )
                  {
                     maxIndex = n;
                     max = pb;
                  }
               }

               const double coef = max * 256 * _gmms[ 0 ].getGaussians().size();
               im( x, y, 0 ) = (ui8)std::min<double>( colors[ maxIndex ][ 0 ] * coef, 200.0 );
               im( x, y, 1 ) = (ui8)std::min<double>( colors[ maxIndex ][ 1 ] * coef, 200.0 );
               im( x, y, 2 ) = (ui8)std::min<double>( colors[ maxIndex ][ 2 ] * coef, 200.0 );
            }
         }

         return im;
      }

      const std::vector<algorithm::Gmm>& getGmms() const
      {
         return _gmms;
      }

   private:
      std::vector<algorithm::Gmm> _gmms;
      double                      _minBound;
      double                      _maxBound;
   };
}

#endif
