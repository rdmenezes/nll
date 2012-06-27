#include <nll/nll.h>
#include <sstream>
#include <tester/register.h>

using namespace nll;

/**
 Dummy distance transform
 */
std::auto_ptr< imaging::VolumeSpatial<double> > computeDummyDistanceTransform( const imaging::VolumeSpatial<double>& img )
{
   imaging::VolumeSpatial<double>* dm = new imaging::VolumeSpatial<double>(img.getSize(), img.getPst(), 0);
   const core::vector3ui size = img.getSize();

   for ( int nz1 = 0; nz1 < (int)size[ 2 ]; ++nz1 )
   {
      for ( int ny1 = 0; ny1 < (int)size[ 1 ]; ++ny1 )
      {
         for ( int nx1 = 0; nx1 < (int)size[ 0 ]; ++nx1 )
         {

            double min = 1e20;
            for ( int nz2 = 0; nz2 < (int)size[ 2 ]; ++nz2 )
            {
               for ( int ny2 = 0; ny2 < (int)size[ 1 ]; ++ny2 )
               {
                  for ( int nx2 = 0; nx2 < (int)size[ 0 ]; ++nx2 )
                  {
                     if ( ( img( nx2, ny2, nz2 ) ) )
                     {
                        double d = core::sqr( img.getSpacing()[ 0 ] * ( (double)nx1 - nx2 ) ) +
                                   core::sqr( img.getSpacing()[ 1 ] * ( (double)ny1 - ny2 ) ) +
                                   core::sqr( img.getSpacing()[ 2 ] * ( (double)nz1 - nz2 ) );
                        if ( d < min )
                           min = d;
                     }
                  }
               }
            }
            (*dm)( nx1, ny1, nz1 ) = min;
         }
      }
   }
   return std::auto_ptr< imaging::VolumeSpatial<double> >( dm );
}


class TestDistanceTransform
{
public:
   void testAgainstDummyDistanceTransform()
   {
      std::cout << "Test distance transform..." << std::endl;
      const int nb_iteration = 10;

      const int seed = 0;
      srand( seed );
      const core::vector3ui size( 20, 15, 35 );
      double voxel_rate = 0.02;

      double meanTime = 0;
      double meanTimeDummy = 0;

      int snb = size[ 0 ] * size[ 1 ] * size[ 2 ];
      int nb_voxels = static_cast<int>( snb * voxel_rate );

      for ( int iteration = 0; iteration < nb_iteration; ++iteration )
      {
         std::cout << "iteration:" << iteration << std::endl;

         core::Matrix<float> pst = core::identityMatrix< core::Matrix<float> >( 4 );
         pst( 0, 0 ) = 1;
         pst( 1, 1 ) = 2;
         pst( 2, 2 ) = 3;

         std::auto_ptr< imaging::VolumeSpatial<double> > img( new imaging::VolumeSpatial<double>( size, pst ) );
         for ( int n = 0; n < nb_voxels; ++n )
         {
            (*img)( rand() % (size[ 0 ] ),
                    rand() % (size[ 1 ] ),
                    rand() % (size[ 2 ] ) ) = 1;
         }

         imaging::VolumeSpatial<double>* doublevolume = new imaging::VolumeSpatial<double>( size, pst );
         for (unsigned k = 0; k < size[ 2 ]; ++k)
         {
            for (unsigned j = 0; j < size[ 1 ]; ++j)
            {
               for (unsigned i = 0; i < size[ 0 ]; ++i)
               {
                  const double val = (*img)(i,j,k);
                  (*doublevolume)(i,j,k) = val;
               }
            }
         }

         time_t start = clock();
         std::auto_ptr<imaging::VolumeSpatial<float>> dt = imaging::distanceTransform( *doublevolume );

         time_t enddt = clock();
         std::auto_ptr<imaging::VolumeSpatial<double>> dtref = computeDummyDistanceTransform( *img );
         time_t enddtref = clock();

         for ( unsigned nz1 = 0; nz1 < size[ 2 ]; ++nz1 )
         {
            for ( unsigned ny1 = 0; ny1 < size[ 1 ]; ++ny1 )
            {
               for ( unsigned nx1 = 0; nx1 < size[ 0 ]; ++nx1 )
               {
                  const double vref = (*dtref)( nx1, ny1, nz1 );
                  const double v = (*dt)( nx1, ny1, nz1 );
                  TESTER_ASSERT( fabs( v - vref ) < 1e-6 );
               }
            }
         }

         double timedt = ( enddt - start ) / ( (double)CLOCKS_PER_SEC );
         double timedtref = ( enddtref - start ) / ( (double)CLOCKS_PER_SEC ) - timedt;
         meanTime += timedt;
         meanTimeDummy += timedtref;
      }

      std::cout << "mean time distance transform 11x15x10=" << meanTime / nb_iteration << std::endl;
      std::cout << "mean time dummy distance transform 11x15x10=" << meanTimeDummy / nb_iteration << std::endl;
      TESTER_ASSERT( meanTime < meanTimeDummy );
   }
};


#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestDistanceTransform);
TESTER_TEST(testAgainstDummyDistanceTransform); 
TESTER_TEST_SUITE_END();
#endif