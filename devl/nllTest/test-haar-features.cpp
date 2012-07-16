#include <nll/nll.h>
#include <tester/register.h>

using namespace nll;
using namespace nll::core;
using namespace nll::algorithm;

namespace impl
{
   
}

class TestHaarFeature
{
public:
   double dummySum( const core::Image<ui8>& i, const core::vector2i& min, const core::vector2i& max )
   {
      double sum = 0;
      for ( size_t ny = min[ 1 ]; ny <= max[ 1 ]; ++ny )
      {
         for ( size_t nx = min[ 0 ]; nx <= max[ 0 ]; ++nx )
         {
            sum += i( nx, ny, 0 );
         }
      }

      return sum;
   }

   double dummySum( const imaging::Volume<ui8>& i, const core::vector3i& min, const core::vector3i& max )
   {
      double sum = 0;
      for ( size_t nz = min[ 2 ]; nz <= max[ 2 ]; ++nz )
      {
         for ( size_t ny = min[ 1 ]; ny <= max[ 1 ]; ++ny )
         {
            for ( size_t nx = min[ 0 ]; nx <= max[ 0 ]; ++nx )
            {
               sum += i( nx, ny, nz );
            }
         }
      }

      return sum;
   }

   void set( core::Image<ui8>& i, const core::vector2i& min, const core::vector2i& max, ui8 val )
   {
      for ( size_t ny = min[ 1 ]; ny <= max[ 1 ]; ++ny )
      {
         for ( size_t nx = min[ 0 ]; nx <= max[ 0 ]; ++nx )
         {
            i( nx, ny, 0 ) = val;
         }
      }
   }

   void set( imaging::Volume<ui8>& i, const core::vector3i& min, const core::vector3i& max, ui8 val )
   {
      for ( size_t nz = min[ 2 ]; nz <= max[ 2 ]; ++nz )
      {
         for ( size_t ny = min[ 1 ]; ny <= max[ 1 ]; ++ny )
         {
            for ( size_t nx = min[ 0 ]; nx <= max[ 0 ]; ++nx )
            {
               i( nx, ny, nz ) = val;
            }
         }
      }
   }

   void testBasic()
   {
      // create a basic image
      Image<ui8> i( 50, 35, 1 );
      for ( size_t m = 0; m < i.sizey(); ++m )
         for ( size_t n = 0; n < i.sizex(); ++n )
            i( n, m, 0 ) = static_cast<ui8>( n + m * i.sizex() + 1 );

      // computes the Integral image
      IntegralImage integral;
      integral.process( i );
      
      srand( 0 );
      for ( size_t n = 0; n < 500; ++n )
      {
         
         int x1 = rand() % i.sizex();
         int x2 = rand() % i.sizex();
         int y1 = rand() % i.sizey();
         int y2 = rand() % i.sizey();
         
         if ( x1 > x2 )
            std::swap( x1, x2 );
         if ( y1 > y2 )
            std::swap( y1, y2 );

         const double sum = dummySum( i, vector2i( x1, y1 ), vector2i( x2, y2 ) );
         TESTER_ASSERT( equal<double>( integral.getSum( vector2i( x1, y1 ), vector2i( x2, y2 ) ), sum ) );
      }
   }

   void testHaar2d_vertical()
   {
      // create a basic image
      Image<ui8> i( 9, 9, 1 );
      set( i, core::vector2i( 0, 0 ), core::vector2i( 3, 8 ), 1 );
      set( i, core::vector2i( 5, 0 ), core::vector2i( 8, 8 ), 2 );

      IntegralImage integral;
      integral.process( i );

      const double val = HaarFeatures2d::getValue( HaarFeatures2d::VERTICAL, integral, core::vector2i( 4, 4 ), 9 );
      const double expected = + dummySum( i, core::vector2i( 0, 0 ), core::vector2i( 3, 8 ) )
                              - dummySum( i, core::vector2i( 5, 0 ), core::vector2i( 8, 8 ) );
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar3d_dx()
   {
      // create a basic image
      imaging::Volume<ui8> i( 9, 9, 9 );
      set( i, core::vector3i( 0, 0, 0 ), core::vector3i( 3, 8, 8 ), 1 );
      set( i, core::vector3i( 5, 0, 0 ), core::vector3i( 8, 8, 8 ), 2 );

      IntegralImage3d integral;
      integral.process( i );

      const double val = HaarFeatures3d::getValue( HaarFeatures3d::DX, integral, core::vector3i( 4, 4, 4 ), 9 );
      const double expected = + dummySum( i, core::vector3i( 0, 0, 0 ), core::vector3i( 3, 8, 8 ) )
                              - dummySum( i, core::vector3i( 5, 0, 0 ), core::vector3i( 8, 8, 8 ) );
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar3d_dy()
   {
      // create a basic image
      imaging::Volume<ui8> i( 9, 9, 9 );
      set( i, core::vector3i( 0, 0, 0 ), core::vector3i( 8, 3, 8 ), 1 );
      set( i, core::vector3i( 0, 5, 0 ), core::vector3i( 8, 8, 8 ), 2 );

      IntegralImage3d integral;
      integral.process( i );

      const double val = HaarFeatures3d::getValue( HaarFeatures3d::DY, integral, core::vector3i( 4, 4, 4 ), 9 );
      const double expected = + dummySum( i, core::vector3i( 0, 0, 0 ), core::vector3i( 8, 3, 8 ) )
                              - dummySum( i, core::vector3i( 0, 5, 0 ), core::vector3i( 8, 8, 8 ) );
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar3d_dz()
   {
      // create a basic image
      imaging::Volume<ui8> i( 9, 9, 9 );
      set( i, core::vector3i( 0, 0, 0 ), core::vector3i( 8, 8, 3 ), 1 );
      set( i, core::vector3i( 0, 0, 5 ), core::vector3i( 8, 8, 8 ), 2 );

      IntegralImage3d integral;
      integral.process( i );

      const double val = HaarFeatures3d::getValue( HaarFeatures3d::DZ, integral, core::vector3i( 4, 4, 4 ), 9 );
      const double expected = + dummySum( i, core::vector3i( 0, 0, 0 ), core::vector3i( 8, 8, 3 ) )
                              - dummySum( i, core::vector3i( 0, 0, 5 ), core::vector3i( 8, 8, 8 ) );
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar2d_vertical_triple()
   {
      // create a basic image
      Image<ui8> i( 9, 9, 1 );
      set( i, core::vector2i( 0, 0 ), core::vector2i( 8, 1 ), 1000 );
      set( i, core::vector2i( 0, 7 ), core::vector2i( 8, 8 ), 2000 );

      set( i, core::vector2i( 0, 2 ), core::vector2i( 2, 6 ), 2 );
      set( i, core::vector2i( 3, 2 ), core::vector2i( 5, 6 ), 8 );
      set( i, core::vector2i( 6, 2 ), core::vector2i( 8, 6 ), 7 );

      IntegralImage integral;
      integral.process( i );

      const double val = HaarFeatures2d::getValue( HaarFeatures2d::VERTICAL_TRIPLE, integral, core::vector2i( 4, 4 ), 9 / 3 );
      const double expected = dummySum( i, core::vector2i( 0, 2 ), core::vector2i( 2, 6 ) ) + 
                              dummySum( i, core::vector2i( 6, 2 ), core::vector2i( 8, 6 ) ) - 2 *
                              dummySum( i, core::vector2i( 3, 2 ), core::vector2i( 5, 6 ) );
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar3d_d2x()
   {
      // create a basic image
      imaging::Volume<ui8> i( 9, 9, 9 );
      set( i, core::vector3i( 0, 0, 0 ), core::vector3i( 8, 8, 8 ), 500 );
      set( i, core::vector3i( 0, 0, 0 ), core::vector3i( 8, 1, 8 ), 1000 );
      set( i, core::vector3i( 0, 7, 0 ), core::vector3i( 8, 8, 8 ), 2000 );

      set( i, core::vector3i( 0, 2, 2 ), core::vector3i( 2, 6, 6 ), 2 );
      set( i, core::vector3i( 3, 2, 2 ), core::vector3i( 5, 6, 6 ), 8 );
      set( i, core::vector3i( 6, 2, 2 ), core::vector3i( 8, 6, 6 ), 7 );

      IntegralImage3d integral;
      integral.process( i );

      const double val = HaarFeatures3d::getValue( HaarFeatures3d::D2X, integral, core::vector3i( 4, 4, 4 ), 9 / 3 );
      const double expected = dummySum( i, core::vector3i( 0, 2, 2 ), core::vector3i( 2, 6, 6 ) ) + 
                              dummySum( i, core::vector3i( 6, 2, 2 ), core::vector3i( 8, 6, 6 ) ) - 2 *
                              dummySum( i, core::vector3i( 3, 2, 2 ), core::vector3i( 5, 6, 6 ) );
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar3d_d2y()
   {
      // create a basic image
      imaging::Volume<ui8> i( 9, 9, 9 );
      set( i, core::vector3i( 0, 0, 0 ), core::vector3i( 8, 8, 8 ), 500 );
      set( i, core::vector3i( 0, 0, 0 ), core::vector3i( 1, 8, 8 ), 1000 );
      set( i, core::vector3i( 7, 0, 0 ), core::vector3i( 8, 8, 8 ), 2000 );

      set( i, core::vector3i( 2, 0, 2 ), core::vector3i( 6, 2, 6 ), 2 );
      set( i, core::vector3i( 2, 3, 2 ), core::vector3i( 6, 5, 6 ), 8 );
      set( i, core::vector3i( 2, 6, 2 ), core::vector3i( 6, 8, 6 ), 7 );

      IntegralImage3d integral;
      integral.process( i );

      const double val = HaarFeatures3d::getValue( HaarFeatures3d::D2Y, integral, core::vector3i( 4, 4, 4 ), 9 / 3 );
      const double expected = dummySum( i, core::vector3i( 2, 0, 2 ), core::vector3i( 6, 2, 6 ) ) + 
                              dummySum( i, core::vector3i( 2, 6, 2 ), core::vector3i( 6, 8, 6 ) ) - 2 *
                              dummySum( i, core::vector3i( 2, 3, 2 ), core::vector3i( 6, 5, 6 ) );
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar3d_d2z()
   {
      // create a basic image
      imaging::Volume<ui8> i( 9, 9, 9 );
      set( i, core::vector3i( 0, 0, 0 ), core::vector3i( 8, 8, 8 ), 500 );
      set( i, core::vector3i( 0, 0, 0 ), core::vector3i( 8, 8, 1 ), 1000 );
      set( i, core::vector3i( 0, 0, 7 ), core::vector3i( 8, 8, 8 ), 2000 );

      set( i, core::vector3i( 2, 2, 0 ), core::vector3i( 6, 6, 2 ), 2 );
      set( i, core::vector3i( 2, 2, 3 ), core::vector3i( 6, 6, 5 ), 8 );
      set( i, core::vector3i( 2, 2, 6 ), core::vector3i( 6, 6, 8 ), 7 );

      IntegralImage3d integral;
      integral.process( i );

      const double val = HaarFeatures3d::getValue( HaarFeatures3d::D2Z, integral, core::vector3i( 4, 4, 4 ), 9 / 3 );
      const double expected = dummySum( i, core::vector3i( 2, 2, 0 ), core::vector3i( 6, 6, 2 ) ) + 
                              dummySum( i, core::vector3i( 2, 2, 6 ), core::vector3i( 6, 6, 8 ) ) - 2 *
                              dummySum( i, core::vector3i( 2, 2, 3 ), core::vector3i( 6, 6, 5 ) );
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar2d_horizontal()
   {
      // create a basic image
      Image<ui8> i( 9, 9, 1 );
      set( i, core::vector2i( 0, 0 ), core::vector2i( 8, 3 ), 1 );
      set( i, core::vector2i( 0, 5 ), core::vector2i( 8, 8 ), 2 );

      IntegralImage integral;
      integral.process( i );

      const double val = HaarFeatures2d::getValue( HaarFeatures2d::HORIZONTAL, integral, core::vector2i( 4, 4 ), 9 );
      const double expected = + dummySum( i, core::vector2i( 0, 0 ), core::vector2i( 8, 3 ) )
                              - dummySum( i, core::vector2i( 0, 5 ), core::vector2i( 8, 8 ) );
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar2d_horizontal_triple()
   {
      // create a basic image
      Image<ui8> i( 9, 9, 1 );
      set( i, core::vector2i( 0, 0 ), core::vector2i( 1, 8 ), 1000 );
      set( i, core::vector2i( 7, 0 ), core::vector2i( 8, 8 ), 2000 );

      set( i, core::vector2i( 2, 0 ), core::vector2i( 6, 2 ), 2 );
      set( i, core::vector2i( 2, 3 ), core::vector2i( 6, 5 ), 8 );
      set( i, core::vector2i( 2, 6 ), core::vector2i( 6, 8 ), 7 );

      IntegralImage integral;
      integral.process( i );

      const double val = HaarFeatures2d::getValue( HaarFeatures2d::HORIZONTAL_TRIPLE, integral, core::vector2i( 4, 4 ), 9 / 3 );
      const double expected = dummySum( i, core::vector2i( 2, 0 ), core::vector2i( 6, 2 ) ) - 2 * 
                              dummySum( i, core::vector2i( 2, 3 ), core::vector2i( 6, 5 ) ) +
                              dummySum( i, core::vector2i( 2, 6 ), core::vector2i( 6, 8 ) );
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar3d_d2xy()
   {
      // create a basic image
      imaging::Volume<ui8> i( 9, 9, 9 );
      set( i, core::vector3i( 0, 0, 0 ), core::vector3i( 8, 8, 8 ), 256 );

      set( i, core::vector3i( 1, 1, 1 ), core::vector3i( 3, 3, 7 ), 2 );
      set( i, core::vector3i( 5, 1, 1 ), core::vector3i( 7, 3, 7 ), 4 );
      set( i, core::vector3i( 1, 5, 1 ), core::vector3i( 3, 7, 7 ), 15 );
      set( i, core::vector3i( 5, 5, 1 ), core::vector3i( 7, 7, 7 ), 45 );


      IntegralImage3d integral;
      integral.process( i );

      const double val = HaarFeatures3d::getValue( HaarFeatures3d::D2XY, integral, core::vector3i( 4, 4, 4 ), 9 / 3 );
      const double v1 = dummySum( i, core::vector3i( 1, 1, 1 ), core::vector3i( 3, 3, 7 ) );
      const double v2 = dummySum( i, core::vector3i( 5, 1, 1 ), core::vector3i( 7, 3, 7 ) );
      const double v3 = dummySum( i, core::vector3i( 1, 5, 1 ), core::vector3i( 3, 7, 7 ) );
      const double v4 = dummySum( i, core::vector3i( 5, 5, 1 ), core::vector3i( 7, 7, 7 ) );
      const double expected = - v2 - v3 + v1 + v4;
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar3d_d2xz()
   {
      // create a basic image
      imaging::Volume<ui8> i( 9, 9, 9 );
      set( i, core::vector3i( 0, 0, 0 ), core::vector3i( 8, 8, 8 ), 256 );

      set( i, core::vector3i( 1, 1, 1 ), core::vector3i( 3, 7, 3 ), 2 );
      set( i, core::vector3i( 5, 1, 1 ), core::vector3i( 7, 7, 3 ), 4 );
      set( i, core::vector3i( 1, 1, 5 ), core::vector3i( 3, 7, 7 ), 15 );
      set( i, core::vector3i( 5, 1, 5 ), core::vector3i( 7, 7, 7 ), 45 );


      IntegralImage3d integral;
      integral.process( i );

      const double val = HaarFeatures3d::getValue( HaarFeatures3d::D2XZ, integral, core::vector3i( 4, 4, 4 ), 9 / 3 );
      const double v1 = dummySum( i, core::vector3i( 1, 1, 1 ), core::vector3i( 3, 7, 3 ) );
      const double v2 = dummySum( i, core::vector3i( 5, 1, 1 ), core::vector3i( 7, 7, 3 ) );
      const double v3 = dummySum( i, core::vector3i( 1, 1, 5 ), core::vector3i( 3, 7, 7 ) );
      const double v4 = dummySum( i, core::vector3i( 5, 1, 5 ), core::vector3i( 7, 7, 7 ) );
      const double expected = - v2 - v3 + v1 + v4;
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar3d_d2yz()
   {
      // create a basic image
      imaging::Volume<ui8> i( 9, 9, 9 );
      set( i, core::vector3i( 0, 0, 0 ), core::vector3i( 8, 8, 8 ), 256 );

      set( i, core::vector3i( 1, 1, 1 ), core::vector3i( 7, 3, 3 ), 2 );
      set( i, core::vector3i( 1, 1, 5 ), core::vector3i( 7, 3, 7 ), 4 );
      set( i, core::vector3i( 1, 5, 1 ), core::vector3i( 7, 7, 3 ), 15 );
      set( i, core::vector3i( 1, 5, 5 ), core::vector3i( 7, 7, 7 ), 45 );


      IntegralImage3d integral;
      integral.process( i );

      const double val = HaarFeatures3d::getValue( HaarFeatures3d::D2YZ, integral, core::vector3i( 4, 4, 4 ), 9 / 3 );
      const double v1 = dummySum( i, core::vector3i( 1, 1, 1 ), core::vector3i( 7, 3, 3 ) );
      const double v2 = dummySum( i, core::vector3i( 1, 1, 5 ), core::vector3i( 7, 3, 7 ) );
      const double v3 = dummySum( i, core::vector3i( 1, 5, 1 ), core::vector3i( 7, 7, 3 ) );
      const double v4 = dummySum( i, core::vector3i( 1, 5, 5 ), core::vector3i( 7, 7, 7 ) );
      const double expected = - v2 - v3 + v1 + v4;
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar2d_checker()
   {
      // create a basic image
      Image<ui8> i( 9, 9, 1 );
      set( i, core::vector2i( 0, 0 ), core::vector2i( 8, 8 ), 256 );

      set( i, core::vector2i( 1, 1 ), core::vector2i( 3, 3 ), 2 );
      set( i, core::vector2i( 5, 1 ), core::vector2i( 7, 3 ), 4 );
      set( i, core::vector2i( 1, 5 ), core::vector2i( 3, 7 ), 15 );
      set( i, core::vector2i( 5, 5 ), core::vector2i( 7, 7 ), 45 );


      IntegralImage integral;
      integral.process( i );

      const double val = HaarFeatures2d::getValue( HaarFeatures2d::CHECKER, integral, core::vector2i( 4, 4 ), 9 / 3 );
      const double v1 = dummySum( i, core::vector2i( 1, 1 ), core::vector2i( 3, 3 ) );
      const double v2 = dummySum( i, core::vector2i( 5, 1 ), core::vector2i( 7, 3 ) );
      const double v3 = dummySum( i, core::vector2i( 1, 5 ), core::vector2i( 3, 7 ) );
      const double v4 = dummySum( i, core::vector2i( 5, 5 ), core::vector2i( 7, 7 ) );
      const double expected = - v2 - v3 + v1 + v4;
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar2d_checker_size2()
   {
      // create a basic image
      Image<ui8> i( 15, 15, 1 );
      set( i, core::vector2i( 0, 0 ), core::vector2i( 14, 14 ), 256 );

      set( i, core::vector2i( 2, 2 ), core::vector2i( 6, 6 ), 2 );
      set( i, core::vector2i( 8, 2 ), core::vector2i( 12, 6 ), 4 );
      set( i, core::vector2i( 2, 8 ), core::vector2i( 6, 12 ), 15 );
      set( i, core::vector2i( 8, 8 ), core::vector2i( 12, 12 ), 45 );


      IntegralImage integral;
      integral.process( i );

      const double val = HaarFeatures2d::getValue( HaarFeatures2d::CHECKER, integral, core::vector2i( 7, 7 ), 5 );
      const double v1 = dummySum( i, core::vector2i( 2, 2 ), core::vector2i( 6, 6 ) );
      const double v2 = dummySum( i, core::vector2i( 8, 2 ), core::vector2i( 12, 6 ) );
      const double v3 = dummySum( i, core::vector2i( 2, 8 ), core::vector2i( 6, 12 ) );
      const double v4 = dummySum( i, core::vector2i( 8, 8 ), core::vector2i( 12, 12 ) );
      const double expected = - v2 - v3 + v1 + v4;
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }

   void testHaar2d_horizontal_triple_size2()
   {
      // create a basic image
      Image<ui8> i( 15, 15, 1 );
      set( i, core::vector2i( 0, 0 ), core::vector2i( 2, 14 ), 1000 );
      set( i, core::vector2i( 12, 0 ), core::vector2i( 14, 14 ), 2000 );

      set( i, core::vector2i( 3, 0 ), core::vector2i( 11, 4 ), 2 );
      set( i, core::vector2i( 3, 5 ), core::vector2i( 11, 9 ), 8 );
      set( i, core::vector2i( 3, 10 ), core::vector2i( 11, 14 ), 7 );

      IntegralImage integral;
      integral.process( i );

      const double val = HaarFeatures2d::getValue( HaarFeatures2d::HORIZONTAL_TRIPLE, integral, core::vector2i( 7, 7 ), 5 );
      const double expected = dummySum( i, core::vector2i( 3, 0 ), core::vector2i( 11, 4 ) ) - 2 * 
                              dummySum( i, core::vector2i( 3, 5 ), core::vector2i( 11, 9 ) ) +
                              dummySum( i, core::vector2i( 3, 10 ), core::vector2i( 11, 14 ) );
      TESTER_ASSERT( core::equal<double>( val, expected ) );
   }


   void testHessianPyramidBasic()
   {
      for ( size_t n = 0; n < 100; ++n )
      {
         const size_t sizex = 32 + rand() % 32;
         const size_t sizey = 64 + rand() % 64;
         FastHessianDetPyramid2d pyramid;
         Image<ui8> i( sizex, sizey, 1 );
         for ( size_t n = 0; n < i.size(); ++n )
         {
            i[ n ] = rand() % 256;
         }

         std::vector<size_t> scales;
         scales.push_back( 9 );
         scales.push_back( 15 );
         scales.push_back( 21 );
         std::vector<size_t> displacements;
         displacements.push_back( 2 );
         displacements.push_back( 4 );
         displacements.push_back( 8 );

         pyramid.construct( i, scales, displacements );

         // test expected pyramid size
         TESTER_ASSERT( pyramid.getPyramidDetHessian().size() == displacements.size() );
         for ( size_t n = 0; n < displacements.size(); ++n )
         {
            TESTER_ASSERT( pyramid.getPyramidDetHessian()[ n ].sizex() * displacements[ n ] + scales[ n ] / 2 < sizex );
            TESTER_ASSERT( pyramid.getPyramidDetHessian()[ n ].sizex() * displacements[ n ] + 2 * scales[ n ] >= sizex );

            TESTER_ASSERT( pyramid.getPyramidDetHessian()[ n ].sizey() * displacements[ n ] + scales[ n ] / 2 < sizey );
            TESTER_ASSERT( pyramid.getPyramidDetHessian()[ n ].sizey() * displacements[ n ] + 2 * scales[ n ] >= sizey );
         }

         // test coordinate system
         for ( size_t n = 0; n < pyramid.getPyramidDetHessian().size(); ++n )
         {
            for ( size_t x = 0; x <  pyramid.getPyramidDetHessian()[ n ].sizex(); ++x )
            {
               for ( size_t y = 0; y <  pyramid.getPyramidDetHessian()[ n ].sizey(); ++y )
               {
                  core::vector2f pos = pyramid.getPositionPyramid2Integral( x, y, n );
                  core::vector2f index = pyramid.getPositionIntegral2Pyramid( pos[ 0 ], pos[ 1 ], n );
                  TESTER_ASSERT( fabs( index[ 0 ] - x ) < 1e-2 && fabs( index[ 1 ] - y ) < 1e-2 );
               }
            }
         }
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestHaarFeature);
TESTER_TEST(testBasic);
TESTER_TEST(testHaar2d_horizontal);
TESTER_TEST(testHaar2d_vertical);
TESTER_TEST(testHaar2d_vertical_triple);
TESTER_TEST(testHaar2d_horizontal_triple);
TESTER_TEST(testHaar2d_checker);
TESTER_TEST(testHaar2d_checker_size2);
TESTER_TEST(testHaar2d_horizontal_triple_size2);
TESTER_TEST(testHessianPyramidBasic);
TESTER_TEST(testHaar3d_dx);
TESTER_TEST(testHaar3d_dy);
TESTER_TEST(testHaar3d_dz);
TESTER_TEST(testHaar3d_d2x);
TESTER_TEST(testHaar3d_d2y);
TESTER_TEST(testHaar3d_d2z);
TESTER_TEST(testHaar3d_d2xy);
TESTER_TEST(testHaar3d_d2xz);
TESTER_TEST(testHaar3d_d2yz);
TESTER_TEST_SUITE_END();
#endif
