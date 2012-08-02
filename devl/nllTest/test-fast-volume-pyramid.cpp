#include <nll/nll.h>
#include <tester/register.h>
#include "config.h"

using namespace nll;

class TestFastVolumePyramid
{
public:
   typedef algorithm::VolumePyramid<float> Pyramid;
   typedef Pyramid::Volume Volume;
   typedef Volume::Matrix Matrix;
   typedef Volume::Matrix::value_type MatrixType;

   Volume downSample( const Volume& volume, size_t level )
   {
      Matrix pst;
      pst.clone( volume.getPst() );
      const ui32 factori = 1 << level;
      const ui32 area = factori * factori * factori;

      // set the correct spacing
      for ( size_t x = 0; x < 3; ++x )
      {
         for ( size_t y = 0; y < 3; ++y )
         {
            pst( y, x ) *= factori;
         }
      }

      // now build the volume
      Volume v( core::vector3ui( volume.sizex() / factori,
                                 volume.sizey() / factori,
                                 volume.sizez() / factori ),
                  pst,
                  volume.getBackgroundValue() );

      for ( ui32 z = 0; z < v.getSize()[ 2 ]; ++z )
      {
         for ( ui32 y = 0; y < v.getSize()[ 1 ]; ++y )
         {
            for ( ui32 x = 0; x < v.getSize()[ 0 ]; ++x )
            {
               double accum = 0;
               for ( ui32 dz = 0; dz < factori; ++dz )
               {
                  for ( ui32 dy = 0; dy < factori; ++dy )
                  {
                     for ( ui32 dx = 0; dx < factori; ++dx )
                     {
                        const double val = volume( x * factori + dx,
                                                   y * factori + dy,
                                                   z * factori + dz );
                        accum += val;
                     }
                  }
               }
               v( x, y, z ) = accum / area;
            }
         }
      }

      return v;
   }

   void testPyramid()
   {
      Volume v;
      const bool loaded = imaging::loadSimpleFlatFile( NLL_TEST_PATH "data/medical/MR-1.mf2", v );
      ensure( loaded, "error!" );
      core::vector3ui size = v.getSize();
      

      Pyramid pyramid;
      pyramid.construct( v, 4 );


      for ( ui32 level = 1; level < pyramid.size(); ++level )
      {
         size /= 2;

         std::cout << "pyramid size=" << size << " level=" << level << std::endl;
         TESTER_ASSERT( size == pyramid[ level ].size() );

         const Volume downSampled = downSample( v, level );
         pyramid[ level ].getPst().equal( downSampled.getPst() );

         for ( size_t n = 0; n < downSampled.getStorage().size(); ++n )
         {
            const double vexpected = downSampled.getStorage()[ n ];
            const double vfound = pyramid[ level ].getStorage()[ n ];
            TESTER_ASSERT( core::equal<double>( vfound, vexpected, 1e-2 ) );
         }

         imaging::saveSimpleFlatFile( NLL_TEST_PATH "data/pyramid-" + core::val2str( level ) + ".mf2", pyramid[ level ] );
      }

      imaging::saveSimpleFlatFile( NLL_TEST_PATH "data/pyramid-1.mf2", pyramid[ 0 ] );
   }
};


#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestFastVolumePyramid);
TESTER_TEST(testPyramid);
TESTER_TEST_SUITE_END();
#endif