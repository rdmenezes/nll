#include "stdafx.h"
#include <nll/nll.h>

class TestTransformation
{
public:
   void testAffine()
   {
      nll::core::vector3f t( 10, -5, 1 );
      nll::imaging::Transformation::Matrix tfm = nll::core::identityMatrix<nll::imaging::Transformation::Matrix>( 4 );
      tfm( 0, 3 ) = t[ 0 ];
      tfm( 1, 3 ) = t[ 1 ];
      tfm( 2, 3 ) = t[ 2 ];

      nll::imaging::TransformationAffine affine1( tfm );
      for ( int n = 0; n < 100; ++n )
      {
         nll::core::vector3f p( (float)(rand()%100), (float)(rand()%100), (float)(rand()%100) );
         TESTER_ASSERT( affine1.transformDeformableOnly( p ) == p );
         TESTER_ASSERT( affine1.transform( p ) == ( p + t ) );

         nll::imaging::Transformation* c = affine1.clone();
         TESTER_ASSERT( c->transform( p ) == ( p + t ) );
      }
   }

   
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestTransformation);
TESTER_TEST(testAffine);
TESTER_TEST_SUITE_END();
#endif

