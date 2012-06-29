#include <nll/nll.h>
#include <sstream>
#include <tester/register.h>

using namespace nll;

class TestIntegral3D
{
public:
   void testPos()
   {
      srand(0);

      imaging::Volume<double> volume( 16, 32, 22 );
      volume.fill( 1 );
      for ( size_t z = 0; z < volume.size()[ 2 ]; ++z )
      {
         for ( size_t y = 0; y < volume.size()[ 1 ]; ++y )
         {
            for ( size_t x = 0; x < volume.size()[ 0 ]; ++x )
            {
               volume( x, y, z ) = rand() % 100;
            }
         }
      }

      algorithm::IntegralImage3d integral;
      core::Timer t1;
      integral.process( volume );

      for ( size_t z = 0; z < volume.size()[ 2 ]; ++z )
      {
         for ( size_t y = 0; y < volume.size()[ 1 ]; ++y )
         {
            for ( size_t x = 0; x < volume.size()[ 0 ]; ++x )
            {
               double val = 0;
               for ( size_t za = 0; za <= z; ++za )
               {
                  for ( size_t ya = 0; ya <= y; ++ya )
                  {
                     for ( size_t xa = 0; xa <= x; ++xa )
                        val += volume( xa, ya, za );
                  }
               }

               const double valfound = integral( x, y, z );
               TESTER_ASSERT( fabs( val - valfound ) < 1e-2 );

            }
         }
      }
   }

   void testSum()
   {
      srand(0);

      imaging::Volume<double> volume( 16, 32, 22 );
      
      for ( size_t z = 0; z < volume.size()[ 2 ]; ++z )
      {
         for ( size_t y = 0; y < volume.size()[ 1 ]; ++y )
         {
            for ( size_t x = 0; x < volume.size()[ 0 ]; ++x )
            {
               volume( x, y, z ) = rand() % 100;
            }
         }
      }

      algorithm::IntegralImage3d integral;
      core::Timer t1;
      integral.process( volume );
      for ( size_t n = 0; n < 5000; ++n )
      {
         
         core::vector3ui bl( rand() % volume.size()[ 0 ],
                             rand() % volume.size()[ 1 ],
                             rand() % volume.size()[ 2 ] );
         core::vector3ui tr( rand() % volume.size()[ 0 ],
                             rand() % volume.size()[ 1 ],
                             rand() % volume.size()[ 2 ] );
                             
         for ( size_t c = 0; c < 3; ++c )
         {
            if ( bl[ c ] > tr[ c ] )
               std::swap( bl[ c ], tr[ c ] );
         }

         double sum = 0;
         for ( size_t z = bl[ 2 ]; z <= tr[ 2 ]; ++z )
         {
            for ( size_t y = bl[ 1 ]; y <= tr[ 1 ]; ++y )
            {
               for ( size_t x = bl[ 0 ]; x <= tr[ 0 ]; ++x )
               {
                  sum += volume( x, y, z );
               }
            }
         }

         const double sumFound = integral.getSum( bl, tr );
         TESTER_ASSERT( fabs( sumFound - sum ) < 1e-5 );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestIntegral3D);
TESTER_TEST(testPos);
TESTER_TEST(testSum);
TESTER_TEST_SUITE_END();
#endif
