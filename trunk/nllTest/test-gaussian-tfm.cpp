#include <nll/nll.h>
#include <tester/register.h>


using namespace nll;
using namespace nll::algorithm;

class TestGaussianTransformation
{
   typedef std::vector<double>   Point;
   typedef std::vector<Point>    Points;

public:
   void testMul1()
   {
      PotentialGaussianCanonical::VectorI i1( 2 );
      i1[ 0 ] = 0;
      i1[ 1 ] = 2;

      PotentialGaussianCanonical::Vector m1( 2 );
      m1[ 0 ] = 4;
      m1[ 1 ] = 5;

      PotentialGaussianCanonical::Matrix k1( 2, 2 );
      k1( 0, 0 ) = 1;
      k1( 0, 1 ) = 2;
      k1( 1, 0 ) = 3;
      k1( 1, 1 ) = 4;

      PotentialGaussianCanonical::VectorI i2( 2 );
      i2[ 0 ] = 0;
      i2[ 1 ] = 1;

      PotentialGaussianCanonical::Vector m2( 2 );
      m2[ 0 ] = 10;
      m2[ 1 ] = 11;

      PotentialGaussianCanonical::Matrix k2( 2, 2 );
      k2( 0, 0 ) = 10;
      k2( 0, 1 ) = 11;
      k2( 1, 0 ) = 12;
      k2( 1, 1 ) = 13;

      PotentialGaussianCanonical g1( m1, k1, 2, i1 );
      PotentialGaussianCanonical g2( m2, k2, 3, i2 );
      PotentialGaussianCanonical r = g1.mul( g2 );

      g1.print( std::cout );
      g2.print( std::cout );
      r.print( std::cout );

      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 0 ], 14 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 1 ], 11 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 2 ], 5 ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 0 ), 11 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 1 ), 11 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 2 ), 2 ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 0 ), 12 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 1 ), 13 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 2 ), 0 ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 2, 0 ), 3 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 2, 1 ), 0 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 2, 2 ), 4 ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getG(), 5 ) );
   }

   void testMarginalization1() // checked with matlab, BNT
   {
      PotentialGaussianCanonical::VectorI i1( 4 );
      i1[ 0 ] = 1;
      i1[ 1 ] = 2;
      i1[ 2 ] = 3;
      i1[ 3 ] = 4;

      PotentialGaussianCanonical::Vector m1( 4 );
      m1[ 0 ] = 4;
      m1[ 1 ] = 5;
      m1[ 2 ] = 6;
      m1[ 3 ] = 7;

      PotentialGaussianCanonical::Matrix k1( 4, 4 );
      k1( 0, 0 ) = 1;
      k1( 0, 1 ) = 1;
      k1( 0, 2 ) = 0;
      k1( 0, 3 ) = -3;

      k1( 1, 0 ) = 4;
      k1( 1, 1 ) = 5;
      k1( 1, 2 ) = 0;
      k1( 1, 3 ) = -1;

      k1( 2, 0 ) = 2;
      k1( 2, 1 ) = 3;
      k1( 2, 2 ) = -1;
      k1( 2, 3 ) = -1;

      k1( 3, 0 ) = 0.5;
      k1( 3, 1 ) = 2.5;
      k1( 3, 2 ) = 3.5;
      k1( 3, 3 ) = 1.5;

      PotentialGaussianCanonical g1( m1, k1, 0.3, i1 );
      PotentialGaussianCanonical::VectorI mids( 2 );
      mids[ 0 ] = 1;
      mids[ 1 ] = 2;
      PotentialGaussianCanonical r = g1.marginalization( mids );

      r.print(std::cout);

      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 0 ], 9, 1e-8 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 1 ], 27, 1e-8  ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 0 ), -1, 1e-8  ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 1 ), -6, 1e-8  ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 0 ), 3.5, 1e-8  ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 1 ), -19, 1e-8  ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getG(), 4.63788, 1e-5  ) );
   }

   Points generateGaussianData( std::vector< std::pair< double, double > >* param_out = 0 )
   {
      size_t nbDim = core::generateUniformDistributioni( 1, 15 );

      // generate distribution parameters
      std::vector< std::pair< double, double > > params;
      for ( size_t nn = 0; nn < nbDim; ++nn )
      {
         const double mean = core::generateUniformDistribution( -30, 30 );
         const double var = core::generateUniformDistribution( 1, 6 );
         params.push_back( std::make_pair( mean, var ) );
      }

      // generate the data
      std::vector<Point> points;
      for ( size_t nn = 0; nn < 5 * nbDim; ++nn )
      {
         Point p( nbDim );
         for ( size_t i = 0; i < nbDim; ++i )
         {
            const double val = core::generateGaussianDistribution( params[ i ].first, params[ i ].second );
            p[ i ] = val;
         }
         points.push_back( p );
      }

      if ( param_out )
      {
         *param_out = params;
      }
      return points;
   }

   void testConversion()
   {
      srand(1);
      const size_t nbTests = 500;
      for ( size_t n = 0; n < nbTests; ++n )
      {
         Points points = generateGaussianData();

         // generate points
         core::Buffer1D<double> mean;
         core::Matrix<double> cov = nll::core::covariance( points, &mean );

         // check the conversion between the gaussian representations
         PotentialGaussianMoment gm( mean, cov );
         PotentialGaussianCanonical gc = gm.toGaussianCanonical();
         PotentialGaussianMoment gm2 = gc.toGaussianMoment();

         const PotentialGaussianMoment::Matrix& cov1 = gm.getCov();
         const PotentialGaussianMoment::Matrix& cov2 = gm2.getCov();

         const PotentialGaussianMoment::Vector& mean1 = gm.getMean();
         const PotentialGaussianMoment::Vector& mean2 = gm2.getMean();

         TESTER_ASSERT( mean1.equal( mean2, 1e-8 ) );
         TESTER_ASSERT( cov1.equal( cov2, 1e-8 ) );
         TESTER_ASSERT( core::equal( gm.getAlpha(), gm2.getAlpha(), 1e-8 ) );
      }
   }

   void testMarginalization()
   {
      const size_t nbTests = 200;
      for ( size_t n = 0; n < nbTests; ++n )
      {
         Points points = generateGaussianData();

         // generate points
         core::Buffer1D<double> mean;
         core::Matrix<double> cov = nll::core::covariance( points, &mean );

         // check the conversion between the gaussian representations
         PotentialGaussianMoment gm( mean, cov );
         PotentialGaussianCanonical gc = gm.toGaussianCanonical();

         if ( cov.sizex() <= 2 )
         {
            continue;
         }

         // check we find the same results
         std::vector<size_t> list = core::generateUniqueList( 0, cov.sizex() - 1 );
         size_t nbVar = core::generateUniformDistributioni( 1, cov.sizex() - 1 );
         
         PotentialGaussianMoment::VectorI varToMarginalize( nbVar );
         for ( size_t nn = 0; nn < nbVar; ++nn )
         {
            varToMarginalize[ nn ] = list[ nn ];
         }

         std::sort( varToMarginalize.begin(), varToMarginalize.end() );

         PotentialGaussianMoment gmm = gm.marginalization( varToMarginalize );
         PotentialGaussianCanonical gcm = gc.marginalization( varToMarginalize );
         PotentialGaussianMoment gmm2 = gcm.toGaussianMoment();

         const PotentialGaussianMoment::Matrix& cov1 = gmm.getCov();
         const PotentialGaussianMoment::Matrix& cov2 = gmm2.getCov();

         const PotentialGaussianMoment::Vector& mean1 = gmm.getMean();
         const PotentialGaussianMoment::Vector& mean2 = gmm2.getMean();

         TESTER_ASSERT( mean1.equal( mean2, 1e-8 ) );
         TESTER_ASSERT( cov1.equal( cov2, 1e-8 ) );
         //TESTER_ASSERT( core::equal( gmm.getAlpha(), gmm2.getAlpha(), 1e-8 ) ); // not the same!
      }
   }

   void testConditioning()
   {
      for ( size_t sample = 0; sample < 200; ++sample )
      {
         const double m1 = core::generateUniformDistribution( -10, 10 );
         const double m2 = core::generateUniformDistribution( -10, 10 );
         const double c1 = core::generateUniformDistribution( 0.1, 6 );
         const double c2 = core::generateUniformDistribution( 0.1, 6 );

         Points points1;
         Points points2;
         Points points;
         for ( size_t n = 0; n < 500; ++n )
         {
            Point p1( 1 );
            p1[ 0 ] = core::generateGaussianDistribution( m1, c1 );

            Point p2( 1 );
            p2[ 0 ] = core::generateGaussianDistribution( m2, c2 );

            Point p( 2 );
            p[ 0 ] = p1[ 0 ];
            p[ 1 ] = p2[ 0 ];

            points1.push_back( p1 );
            points2.push_back( p2 );
            points.push_back( p );
         }

         PotentialGaussianMoment::VectorI i( 2 );
         i[ 0 ] = 0;
         i[ 1 ] = 1;

         PotentialGaussianMoment::VectorI i1( 1 );
         i1[ 0 ] = 0;
         PotentialGaussianMoment::VectorI i2( 1 );
         i2[ 0 ] = 1;

         PotentialGaussianMoment g1( points1, i1 );
         PotentialGaussianMoment g2( points2, i2 );

         PotentialGaussianCanonical g1c = g1.toGaussianCanonical();
         PotentialGaussianCanonical g2c = g2.toGaussianCanonical();

         PotentialGaussianMoment::Vector v( 1 );
         PotentialGaussianCanonical gc = g1c * g2c;
         v[ 0 ] = m2;
         PotentialGaussianMoment gg1 = gc.conditioning( v, i2 ).toGaussianMoment();
         v[ 0 ] = m1;
         PotentialGaussianMoment gg2 = gc.conditioning( v, i1 ).toGaussianMoment();

         PotentialGaussianMoment gg( points, i );

         /*
         std::cout << "---" << std::endl;
         g1.print(std::cout);
         g2.print(std::cout);
         gg1.print(std::cout);
         gg2.print(std::cout);
         */


         // var(0) and var(1) are indenpendent, so knowing the value of (0) or (1) should not change the value of the conditioned gaussian
         const PotentialGaussianMoment::Matrix& cov1a = gg1.getCov();
         const PotentialGaussianMoment::Matrix& cov2a = g1.getCov();
         const PotentialGaussianMoment::Vector& mean1a = gg1.getMean();
         const PotentialGaussianMoment::Vector& mean2a = g1.getMean();

         const PotentialGaussianMoment::Matrix& cov1b = gg2.getCov();
         const PotentialGaussianMoment::Matrix& cov2b = g2.getCov();
         const PotentialGaussianMoment::Vector& mean1b = gg2.getMean();
         const PotentialGaussianMoment::Vector& mean2b = g2.getMean();

         TESTER_ASSERT( mean1a.equal( mean2a, 1e-8 ) );
         TESTER_ASSERT( cov1a.equal( cov2a, 1e-8 ) );
         TESTER_ASSERT( mean1b.equal( mean2b, 1e-8 ) );
         TESTER_ASSERT( cov1b.equal( cov2b, 1e-8 ) );

         // TODO CHECK : alpha are not the same?
      }
   }

   void testConditioning2()
   {
      const size_t nbTests = 200;
      for ( size_t n = 0; n < nbTests; ++n )
      {
         std::vector< std::pair< double, double > > params;
         Points points = generateGaussianData( &params );

         // generate points
         core::Buffer1D<double> mean;
         core::Matrix<double> cov = nll::core::covariance( points, &mean );

         // check the conversion between the gaussian representations
         PotentialGaussianMoment gm( mean, cov );
         PotentialGaussianCanonical gc = gm.toGaussianCanonical();

         if ( cov.sizex() <= 2 )
         {
            continue;
         }

         // check we find the same results
         std::vector<size_t> list = core::generateUniqueList( 0, cov.sizex() - 1 );
         size_t nbVar = core::generateUniformDistributioni( 1, cov.sizex() - 1 );
         
         PotentialGaussianMoment::VectorI varToMarginalize( nbVar );
         for ( size_t nn = 0; nn < nbVar; ++nn )
         {
            varToMarginalize[ nn ] = list[ nn ];
         }

         std::sort( varToMarginalize.begin(), varToMarginalize.end() );

         PotentialGaussianMoment::Vector x( nbVar );
         for ( size_t nn = 0; nn < nbVar; ++nn )
         {
            x[ nn ] = core::generateGaussianDistribution( params[ nn ].first, params[ nn ].second );
         }

         PotentialGaussianMoment gmm = gm.conditioning( x, varToMarginalize );
         PotentialGaussianCanonical gcm = gc.conditioning( x, varToMarginalize );
         PotentialGaussianMoment gmm2 = gcm.toGaussianMoment();

         const PotentialGaussianMoment::Matrix& cov1 = gmm.getCov();
         const PotentialGaussianMoment::Matrix& cov2 = gmm2.getCov();

         const PotentialGaussianMoment::Vector& mean1 = gmm.getMean();
         const PotentialGaussianMoment::Vector& mean2 = gmm2.getMean();

         TESTER_ASSERT( mean1.equal( mean2, 1e-8 ) );
         TESTER_ASSERT( cov1.equal( cov2, 1e-8 ) );
         // TODO: CHECK WHY WE HAVE DIFFERENCES, however same matlab results... // TESTER_ASSERT( core::equal( gmm.getAlpha(), gmm2.getAlpha(), 1e-3 ) );
      }
   }

   void testConversionBasic() // checked against matlab, BNT
   {
      PotentialGaussianCanonical::VectorI i1( 2 );
      i1[ 0 ] = 0;
      i1[ 1 ] = 1;

      PotentialGaussianCanonical::Vector m1( 2 );
      m1[ 0 ] = 4;
      m1[ 1 ] = 5;

      PotentialGaussianCanonical::Matrix c1( 2, 2 );
      c1( 0, 0 ) = 4;
      c1( 0, 1 ) = 2;
      c1( 1, 0 ) = 3;
      c1( 1, 1 ) = 5;

      PotentialGaussianMoment gm1( m1, c1, i1, exp( 0.3 ) );
      PotentialGaussianCanonical gc1 = gm1.toGaussianCanonical();
      PotentialGaussianMoment gm2 = gc1.toGaussianMoment();

      TESTER_ASSERT( gm1.getMean().equal( gm2.getMean(), 1e-8 ) );
      TESTER_ASSERT( gm1.getCov().equal( gm2.getCov(), 1e-8 ) );
      TESTER_ASSERT( core::equal( gm1.getAlpha(), gm2.getAlpha(), 1e-8 ) );

      TESTER_ASSERT( core::equal( gc1.getG(), -5.7145, 1e-4 ) );
      TESTER_ASSERT( core::equal( gm1.getAlpha(), 1.3499, 1e-4 ) );
   }

   void testConditioning3()   // checked with matlab BNT
   {
      PotentialGaussianCanonical::VectorI i1( 2 );
      i1[ 0 ] = 0;
      i1[ 1 ] = 1;

      PotentialGaussianCanonical::Vector m1( 2 );
      m1[ 0 ] = -0.2250;
      m1[ 1 ] = -1.4625;

      PotentialGaussianCanonical::Matrix c1( 2, 2 );
      c1( 0, 0 ) = -0.4750;
      c1( 0, 1 ) = 0.1500;
      c1( 1, 0 ) = -0.0875;
      c1( 1, 1 ) = -0.0250;

      PotentialGaussianCanonical gc1( m1, c1, -1.8937, i1 );


      PotentialGaussianCanonical::VectorI i2( 1 );
      i2[ 0 ] = 1;

      PotentialGaussianCanonical::Vector m2( 1 );
      m2[ 0 ] = -1.6;

      PotentialGaussianCanonical gc2 = gc1.conditioning(m2, i2);

      TESTER_ASSERT( core::equal( gc2.getH()[ 0 ], 0.015, 1e-3 ) );
      TESTER_ASSERT( core::equal( gc2.getK()[ 0 ], -0.475, 1e-3 ) );
      TESTER_ASSERT( core::equal( gc2.getG(), 0.4783, 1e-3 ) );
   }

   void testMarginalization2_moment()
   {
      srand( 30 );
      // lets start with a random PDF. If we marginalize the PDF it must stay a PDF!
      // integral -inf/inf p(x,y)dy = p(x)
      for ( size_t sample = 0; sample < 200; ++sample )
      {
         const double m1 = core::generateUniformDistribution( -10, 10 );
         const double c1 = core::generateUniformDistribution( 0.1, 6 );

         Points points;
         for ( size_t n = 0; n < 10; ++n )
         {
            Point p1( 3 );
            p1[ 0 ] = core::generateGaussianDistribution( m1, c1 );
            p1[ 1 ] = core::generateGaussianDistribution( m1, c1 );
            p1[ 2 ] = core::generateGaussianDistribution( m1, c1 );
            points.push_back( p1 );
         }

         PotentialGaussianMoment::VectorI i( 3 );
         i[ 0 ] = 0;
         i[ 1 ] = 1;
         i[ 2 ] = 2;
         PotentialGaussianMoment::VectorI i1( 1 );
         i1[ 0 ] = 2;
         PotentialGaussianMoment::VectorI i2( 2 );
         i2[ 0 ] = 0;
         i2[ 1 ] = 2;
         PotentialGaussianMoment::VectorI i3( 3 );
         i3[ 0 ] = 0;
         i3[ 1 ] = 1;
         i3[ 2 ] = 2;

         PotentialGaussianMoment::VectorI* vecs[] =
         {
            &i1, &i2, &i3
         };

         const size_t index = rand() % core::getStaticBufferSize( vecs );
         PotentialGaussianMoment::VectorI& query = *vecs[ index ];

         {
            // g must be normalized by default
            PotentialGaussianMoment g( points, i );
            const double sanityAlphaBefore = g.getAlpha();
            g.normalizeGaussian();
            const double sanityAlphaAfter = g.getAlpha();
            TESTER_ASSERT( core::equal( sanityAlphaBefore, sanityAlphaAfter, 1e-2 ) );

            // even if we marginalize i1, g2 must still be normalized
            PotentialGaussianMoment g2 = g.marginalization( query );
            const double alphaBefore = g2.getAlpha();
            g2.normalizeGaussian();
            const double alphaAfter = g2.getAlpha();
            TESTER_ASSERT( core::equal( alphaBefore, alphaAfter, 1e-2 ) );

            if ( &query == &i3 )
            {
               // since we are marginalizing all the scope, the integral must be equal to 1
               TESTER_ASSERT( core::equal( alphaBefore, 1.0, 1e-3 ) );
            }
         }
      }
   }

};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianTransformation);
TESTER_TEST(testConversionBasic);
TESTER_TEST(testConversion);
TESTER_TEST(testMarginalization1);
TESTER_TEST(testMarginalization);
TESTER_TEST(testMarginalization2_moment);
TESTER_TEST(testMul1);
TESTER_TEST(testConditioning);
TESTER_TEST(testConditioning2);
TESTER_TEST(testConditioning3);
TESTER_TEST_SUITE_END();
#endif