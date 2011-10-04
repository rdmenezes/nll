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
      GaussianMultivariateCanonical::VectorI i1( 2 );
      i1[ 0 ] = 0;
      i1[ 1 ] = 2;

      GaussianMultivariateCanonical::Vector m1( 2 );
      m1[ 0 ] = 4;
      m1[ 1 ] = 5;

      GaussianMultivariateCanonical::Matrix k1( 2, 2 );
      k1( 0, 0 ) = 1;
      k1( 0, 1 ) = 2;
      k1( 1, 0 ) = 3;
      k1( 1, 1 ) = 4;

      GaussianMultivariateCanonical::VectorI i2( 2 );
      i2[ 0 ] = 0;
      i2[ 1 ] = 1;

      GaussianMultivariateCanonical::Vector m2( 2 );
      m2[ 0 ] = 10;
      m2[ 1 ] = 11;

      GaussianMultivariateCanonical::Matrix k2( 2, 2 );
      k2( 0, 0 ) = 10;
      k2( 0, 1 ) = 11;
      k2( 1, 0 ) = 12;
      k2( 1, 1 ) = 13;

      GaussianMultivariateCanonical g1( m1, k1, 2, i1 );
      GaussianMultivariateCanonical g2( m2, k2, 3, i2 );
      GaussianMultivariateCanonical r = g1.mul( g2 );

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

   void testMarginalization1()
   {
      GaussianMultivariateCanonical::VectorI i1( 4 );
      i1[ 0 ] = 0;
      i1[ 1 ] = 1;
      i1[ 2 ] = 2;
      i1[ 3 ] = 3;

      GaussianMultivariateCanonical::Vector m1( 4 );
      m1[ 0 ] = 4;
      m1[ 1 ] = 5;
      m1[ 2 ] = 6;
      m1[ 3 ] = 7;

      GaussianMultivariateCanonical::Matrix k1( 4, 4 );
      unsigned accum = 0;
      for ( unsigned y = 0; y < 4; ++y )
      {
         for ( unsigned x = 0; x < 4; ++x )
         {
            ++accum;
            k1( y, x ) = accum * accum;
         }
      }
      GaussianMultivariateCanonical g1( m1, k1, 2, i1 );
      GaussianMultivariateCanonical::VectorI mids( 2 );
      mids[ 0 ] = 1;
      mids[ 1 ] = 2;
      GaussianMultivariateCanonical r = g1.marginalization( mids );

      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 0 ], 1.5882352941176499, 1e-8 ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getH()[ 1 ], 1.5882352941176485, 1e-8  ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 0 ), 0.94117647058823906, 1e-8  ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 0, 1 ), 0.94117647058823906, 1e-8  ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 0 ), 0.94117647058823906, 1e-8  ) );
      TESTER_ASSERT( nll::core::equal<double>( r.getK()( 1, 1 ), 0.94117647058823906, 1e-8  ) );

      TESTER_ASSERT( nll::core::equal<double>( r.getG(), -0.093587560811534409, 1e-8  ) );
   }

   Points generateGaussianData( std::vector< std::pair< double, double > >* param_out = 0 )
   {
      ui32 nbDim = core::generateUniformDistributioni( 1, 15 );

      // generate distribution parameters
      std::vector< std::pair< double, double > > params;
      for ( ui32 nn = 0; nn < nbDim; ++nn )
      {
         const double mean = core::generateUniformDistribution( -30, 30 );
         const double var = core::generateUniformDistribution( 1, 6 );
         params.push_back( std::make_pair( mean, var ) );
      }

      // generate the data
      std::vector<Point> points;
      for ( ui32 nn = 0; nn < 5 * nbDim; ++nn )
      {
         Point p( nbDim );
         for ( ui32 i = 0; i < nbDim; ++i )
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
      const ui32 nbTests = 500;
      for ( ui32 n = 0; n < nbTests; ++n )
      {
         Points points = generateGaussianData();

         // generate points
         core::Buffer1D<double> mean;
         core::Matrix<double> cov = nll::core::covariance( points, &mean );

         // check the conversion between the gaussian representations
         GaussianMultivariateMoment gm( mean, cov );
         GaussianMultivariateCanonical gc = gm.toGaussianCanonical();
         GaussianMultivariateMoment gm2 = gc.toGaussianMoment();

         const GaussianMultivariateMoment::Matrix& cov1 = gm.getCov();
         const GaussianMultivariateMoment::Matrix& cov2 = gm2.getCov();

         const GaussianMultivariateMoment::Vector& mean1 = gm.getMean();
         const GaussianMultivariateMoment::Vector& mean2 = gm2.getMean();

         TESTER_ASSERT( mean1.equal( mean2, 1e-8 ) );
         TESTER_ASSERT( cov1.equal( cov2, 1e-8 ) );
         TESTER_ASSERT( core::equal( gm.getAlpha(), gm2.getAlpha(), 1e-8 ) );
      }
   }

   void testMarginalization()
   {
      const ui32 nbTests = 200;
      for ( ui32 n = 0; n < nbTests; ++n )
      {
         Points points = generateGaussianData();

         // generate points
         core::Buffer1D<double> mean;
         core::Matrix<double> cov = nll::core::covariance( points, &mean );

         // check the conversion between the gaussian representations
         GaussianMultivariateMoment gm( mean, cov );
         GaussianMultivariateCanonical gc = gm.toGaussianCanonical();

         if ( cov.sizex() <= 2 )
         {
            continue;
         }

         // check we find the same results
         std::vector<ui32> list = core::generateUniqueList( 0, cov.sizex() - 1 );
         ui32 nbVar = core::generateUniformDistributioni( 1, cov.sizex() - 1 );
         
         GaussianMultivariateMoment::VectorI varToMarginalize( nbVar );
         for ( ui32 nn = 0; nn < nbVar; ++nn )
         {
            varToMarginalize[ nn ] = list[ nn ];
         }

         std::sort( varToMarginalize.begin(), varToMarginalize.end() );

         GaussianMultivariateMoment gmm = gm.marginalization( varToMarginalize );
         GaussianMultivariateCanonical gcm = gc.marginalization( varToMarginalize );
         GaussianMultivariateMoment gmm2 = gcm.toGaussianMoment();

         const GaussianMultivariateMoment::Matrix& cov1 = gmm.getCov();
         const GaussianMultivariateMoment::Matrix& cov2 = gmm2.getCov();

         const GaussianMultivariateMoment::Vector& mean1 = gmm.getMean();
         const GaussianMultivariateMoment::Vector& mean2 = gmm2.getMean();

         TESTER_ASSERT( mean1.equal( mean2, 1e-8 ) );
         TESTER_ASSERT( cov1.equal( cov2, 1e-8 ) );
      }
   }

   void testConditioning()
   {
      for ( ui32 sample = 0; sample < 200; ++sample )
      {
         const double m1 = core::generateUniformDistribution( -10, 10 );
         const double m2 = core::generateUniformDistribution( -10, 10 );
         const double c1 = core::generateUniformDistribution( 0.1, 6 );
         const double c2 = core::generateUniformDistribution( 0.1, 6 );

         Points points1;
         Points points2;
         Points points;
         for ( ui32 n = 0; n < 500; ++n )
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

         GaussianMultivariateMoment::VectorI i1( 1 );
         i1[ 0 ] = 0;
         GaussianMultivariateMoment::VectorI i2( 1 );
         i2[ 0 ] = 1;

         GaussianMultivariateMoment g1( points1, i1 );
         GaussianMultivariateMoment g2( points2, i2 );

         GaussianMultivariateCanonical g1c = g1.toGaussianCanonical();
         GaussianMultivariateCanonical g2c = g2.toGaussianCanonical();

         GaussianMultivariateMoment::Vector v( 1 );
         GaussianMultivariateCanonical gc = g1c * g2c;
         v[ 0 ] = m2;
         GaussianMultivariateMoment gg1 = gc.conditioning( v, i2 ).toGaussianMoment();
         v[ 0 ] = m1;
         GaussianMultivariateMoment gg2 = gc.conditioning( v, i1 ).toGaussianMoment();

         GaussianMultivariateMoment gg( points, i2 );

         /*
         std::cout << "---" << std::endl;
         g1.print(std::cout);
         g2.print(std::cout);
         gg1.print(std::cout);
         gg2.print(std::cout);
         */


         // var(0) and var(1) are indenpendent, so knowing the value of (0) or (1) should not change the value of the conditioned gaussian
         const GaussianMultivariateMoment::Matrix& cov1a = gg1.getCov();
         const GaussianMultivariateMoment::Matrix& cov2a = g1.getCov();
         const GaussianMultivariateMoment::Vector& mean1a = gg1.getMean();
         const GaussianMultivariateMoment::Vector& mean2a = g1.getMean();

         const GaussianMultivariateMoment::Matrix& cov1b = gg2.getCov();
         const GaussianMultivariateMoment::Matrix& cov2b = g2.getCov();
         const GaussianMultivariateMoment::Vector& mean1b = gg2.getMean();
         const GaussianMultivariateMoment::Vector& mean2b = g2.getMean();

         TESTER_ASSERT( mean1a.equal( mean2a, 1e-8 ) );
         TESTER_ASSERT( cov1a.equal( cov2a, 1e-8 ) );
         TESTER_ASSERT( mean1b.equal( mean2b, 1e-8 ) );
         TESTER_ASSERT( cov1b.equal( cov2b, 1e-8 ) );

         // TODO CHECK : alpha are not the same?
      }
   }

   void testConditioning2()
   {
      const ui32 nbTests = 200;
      for ( ui32 n = 0; n < nbTests; ++n )
      {
         std::vector< std::pair< double, double > > params;
         Points points = generateGaussianData( &params );

         // generate points
         core::Buffer1D<double> mean;
         core::Matrix<double> cov = nll::core::covariance( points, &mean );

         // check the conversion between the gaussian representations
         GaussianMultivariateMoment gm( mean, cov );
         GaussianMultivariateCanonical gc = gm.toGaussianCanonical();

         if ( cov.sizex() <= 2 )
         {
            continue;
         }

         // check we find the same results
         std::vector<ui32> list = core::generateUniqueList( 0, cov.sizex() - 1 );
         ui32 nbVar = core::generateUniformDistributioni( 1, cov.sizex() - 1 );
         
         GaussianMultivariateMoment::VectorI varToMarginalize( nbVar );
         for ( ui32 nn = 0; nn < nbVar; ++nn )
         {
            varToMarginalize[ nn ] = list[ nn ];
         }

         std::sort( varToMarginalize.begin(), varToMarginalize.end() );

         GaussianMultivariateMoment::Vector x( nbVar );
         for ( ui32 nn = 0; nn < nbVar; ++nn )
         {
            x[ nn ] = core::generateGaussianDistribution( params[ nn ].first, params[ nn ].second );
         }

         GaussianMultivariateMoment gmm = gm.conditioning( x, varToMarginalize );
         GaussianMultivariateCanonical gcm = gc.conditioning( x, varToMarginalize );
         GaussianMultivariateMoment gmm2 = gcm.toGaussianMoment();

         const GaussianMultivariateMoment::Matrix& cov1 = gmm.getCov();
         const GaussianMultivariateMoment::Matrix& cov2 = gmm2.getCov();

         const GaussianMultivariateMoment::Vector& mean1 = gmm.getMean();
         const GaussianMultivariateMoment::Vector& mean2 = gmm2.getMean();

         TESTER_ASSERT( mean1.equal( mean2, 1e-8 ) );
         TESTER_ASSERT( cov1.equal( cov2, 1e-8 ) );
         // TODO: CHECK WHY WE HAVE DIFFERENCES // TESTER_ASSERT( core::equal( gmm.getAlpha(), gmm2.getAlpha(), 1e-3 ) );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestGaussianTransformation);
TESTER_TEST(testMarginalization1);
TESTER_TEST(testMarginalization);
TESTER_TEST(testMul1);
TESTER_TEST(testConditioning);
TESTER_TEST(testConversion);
TESTER_TEST(testConditioning2);
TESTER_TEST_SUITE_END();
#endif