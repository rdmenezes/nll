#include "stdafx.h"
#include <nll/nll.h>

class TestVolume
{
public:
   /**
    Test basic voxel buffer index
    */
   void testBuffer1()
   {
      typedef nll::imaging::VolumeMemoryBuffer<double>   Buffer;

      Buffer buffer( 5, 10, 15 );
      double s = 0;
      for ( Buffer::iterator it = buffer.begin(); it != buffer.end(); ++it )
         *it = ++s;

      s = 0;
      for ( Buffer::const_iterator it = buffer.begin(); it != buffer.end(); ++it )
         TESTER_ASSERT( nll::core::equal( ++s, *it ) );

      buffer( 1, 2, 3 ) = 5.5;
      TESTER_ASSERT( nll::core::equal( buffer( 1, 2, 3 ), 5.5 ) );

      TESTER_ASSERT( buffer.getSize() == nll::core::vector3ui( 5, 10, 15 ) );

      // test read/write of a buffer
      std::stringstream str;
      buffer.write( str );

      Buffer buffer2;
      buffer2.read( str );

      for ( Buffer::iterator it = buffer.begin(), it2 = buffer2.begin(); it != buffer.end(); ++it, ++it2 )
         TESTER_ASSERT( nll::core::equal( *it, *it2 ) );
   }

   /**
    Test test volume sharing memory
    */
   void testVolume1()
   {
      typedef nll::imaging::Volume<int>   Volume;

      Volume vol;
      vol = Volume( 10, 5, 30 );
      vol( 0, 0, 0 ) = -5;
      TESTER_ASSERT( vol.size() == nll::core::vector3ui( 10, 5, 30 ) );
      TESTER_ASSERT( vol( 0, 0, 0 ) == -5 );

      Volume vol2( 40, 50, 10 );
      vol2( 0, 0, 0 ) = -15;
      TESTER_ASSERT( vol2( 0, 0, 0 ) == -15 );

      vol2 = vol;
      TESTER_ASSERT( vol2( 0, 0, 0 ) == -5 );

      Volume vol3;
      vol3.clone( vol2 );
      TESTER_ASSERT( vol3( 0, 0, 0 ) == -5 );

      vol3( 0, 0, 0 ) = 10;
      TESTER_ASSERT( vol3( 0, 0, 0 ) == 10 );
      TESTER_ASSERT( vol2( 0, 0, 0 ) == -5 );
      TESTER_ASSERT( vol( 0, 0, 0 ) == -5 );
   }

   void testVolumeIterator()
   {
      typedef nll::imaging::Volume<int>   Volume;
      Volume buffer( 5, 10, 15 );
      int s = 0;
      for ( Volume::iterator it = buffer.begin(); it != buffer.end(); ++it )
         *it = ++s;

      s = 0;
      for ( Volume::const_iterator it = buffer.begin(); it != buffer.end(); ++it )
         TESTER_ASSERT( nll::core::equal( ++s, *it ) );
   }

   void testVolumeSpatial1()
   {
      typedef nll::imaging::VolumeSpatial<double>  Vol;

      Vol volume;

      Vol::Matrix pst;
      Vol::Matrix rot3x3 = nll::core::identity<float, Vol::Matrix::IndexMapper, Vol::Matrix::Allocator>( 3 );

      pst = Vol::createPatientSpaceTransform( rot3x3, nll::core::vector3f( -10, 5, 30 ), nll::core::vector3f( 10, 20, 30 ) );
      volume = Vol( nll::core::vector3ui( 10, 20, 30 ), pst, 1 );
      TESTER_ASSERT( nll::core::equal<float>( pst( 0, 3 ), -10 ) );
      TESTER_ASSERT( nll::core::equal<float>( pst( 1, 3 ), 5 ) );
      TESTER_ASSERT( nll::core::equal<float>( pst( 2, 3 ), 30 ) );

      TESTER_ASSERT( nll::core::equal<float>( pst( 0, 0 ), 10 ) );
      TESTER_ASSERT( nll::core::equal<float>( pst( 1, 1 ), 20 ) );
      TESTER_ASSERT( nll::core::equal<float>( pst( 2, 2 ), 30 ) );

      TESTER_ASSERT( volume.getOrigin() == nll::core::vector3f( -10, 5, 30 ) );
      TESTER_ASSERT( volume.getSpacing() == nll::core::vector3f( 10, 20, 30 ) );

      // test read write
      std::stringstream ss;
      volume.write( ss );

      Vol volume2;
      volume2.read( ss );
      TESTER_ASSERT( volume2.getBackgroundValue() == volume.getBackgroundValue() );
      TESTER_ASSERT( volume2.getOrigin() == volume.getOrigin() );
      TESTER_ASSERT( volume2.getSpacing() == volume.getSpacing() );
      TESTER_ASSERT( volume2.getSize() == volume.getSize() );

      for ( Vol::iterator it = volume.begin(), it2 = volume2.begin(); it != volume.end(); ++it, ++it2 )
         TESTER_ASSERT( *it == *it2 );

      // inside/outside
      TESTER_ASSERT( !volume.inside( -1, 10, 10 ) );
      TESTER_ASSERT( !volume.inside( 50, 10, 10 ) );
      TESTER_ASSERT( volume.inside( 5, 11, 11 ) );
   }

   void testIndexToPos()
   {
      for ( unsigned nn = 0; nn < 100; ++nn )
      {
         nll::core::vector3ui size( 40, 50, 35 );
         typedef nll::imaging::VolumeSpatial<double>  Volume; 

         // define a random rotation matrix around x axis
         // define a random translation vector in [0..99]
         // define a random spacing [2, 0.01]
         double a = static_cast<double>( rand() ) / RAND_MAX * 3.141 / 2;

         nll::core::vector3f spacingT( rand() % 2 / (float)10 + 0.01f,
                                       rand() % 2 / (float)10 + 0.01f,
                                       rand() % 2 / (float)10 + 0.01f );
         nll::core::vector3f origin( static_cast<float>( rand() % 100 ),
                                     static_cast<float>( rand() % 100 ),
                                     static_cast<float>( rand() % 100 ) );
         nll::core::Matrix<float> rotation( 3, 3 );
         rotation( 0, 0 ) = 1;
         rotation( 1, 1 ) = (float)cos( a );
         rotation( 2, 1 ) = (float)sin( a );
         rotation( 1, 2 ) = (float)-sin( a );
         rotation( 2, 2 ) = (float)cos( a );

         nll::core::Matrix<float> pst = Volume::createPatientSpaceTransform( rotation, origin, spacingT );

         // create the wrapper
         Volume image( size, pst );
         for ( Volume::iterator it = image.begin(); it != image.end(); ++it )
            *it = rand() % 5000;

         nll::core::vector3f spacing = image.getSpacing();
         for ( unsigned n = 0; n < 100; ++n )
         {
            nll::core::vector3f index( static_cast<float>( rand() % ( size[ 0 ] - 1 ) ),
                                       static_cast<float>( rand() % ( size[ 1 ] - 1 ) ),
                                       static_cast<float>( rand() % ( size[ 2 ] - 1 ) ) );

            nll::core::vector3f position = image.indexToPosition( index );
            nll::core::vector3f indexTransf = image.positionToIndex( position );
            TESTER_ASSERT( fabs( index[ 0 ] - indexTransf[ 0 ] ) < 1e-3 );
            TESTER_ASSERT( fabs( index[ 1 ] - indexTransf[ 1 ] ) < 1e-3 );
            TESTER_ASSERT( fabs( index[ 2 ] - indexTransf[ 2 ] ) < 1e-3 );
         }
      }
   }

   /*
   void testInterpolator()
   {
      typedef nll::imaging::Volume<double>  Volume;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume> Interpolator;

      Volume volume( 30, 30, 30, -1 );
      Interpolator interpolator( volume );


      TESTER_ASSERT( interpolator( -1, 0, 0 ) == -1 );

      volume( 10, 10, 10 ) = 10;
      volume( 9, 10, 10 ) = 11;
      volume( 11, 10, 10 ) = 17;
      volume( 10, 10, 11 ) = 12;
      volume( 10, 9, 10 ) = 13;
      volume( 10, 11, 10 ) = 14;
      volume( 10, 10, 11 ) = 15;
      volume( 10, 10, 9 ) = 16;
      TESTER_ASSERT( interpolator( 10, 10, 10 ) == 10 );
      TESTER_ASSERT( interpolator( 10.4f, 10.4f, 10.4f ) == 10 );
      TESTER_ASSERT( interpolator( 9.6f, 9.6f, 9.6f ) == 10 );
      TESTER_ASSERT( interpolator( 10.5f, 9.6f, 9.6f ) == 17 );
   }

   void testInterpolatorTriLinear()
   {
      typedef nll::imaging::Volume<double>  Volume;
      typedef nll::imaging::InterpolatorTriLinear<Volume> Interpolator;

      Volume volume( 30, 30, 30, -1 );
      Interpolator interpolator( volume );

      volume( 5, 5, 5 ) = 10;
      volume( 6, 5, 5 ) = 11;
      volume( 5, 6, 5 ) = 20;
      volume( 5, 5, 6 ) = 40;

      float dev = 0; //0.5 - NLL_IMAGE_BIAS;// deviation to move the center to the center of the voxel + bias
      TESTER_ASSERT( interpolator( 5.5f + dev, 5 + dev, 5 + dev ) == 10.5 );
      TESTER_ASSERT( interpolator( 5 + dev, 5.5f + dev, 5 + dev ) == 15 );
      TESTER_ASSERT( interpolator( 5 + dev, 5 + dev, 5.5f + dev ) == 25 );

      TESTER_ASSERT( interpolator( -10, 0, 0 ) == -1 );
   }*/

   /**
    Test the reconstruction of a slice on a PET volume.
    */
   void testMpr()
   {
      // PETtest-NAC.mf2
      //
      const std::string volname = NLL_TEST_PATH "data/medical/pet-NAC.mf2";
      const std::string output = NLL_TEST_PATH "data/mpr1.bmp";
      const std::string output2 = NLL_TEST_PATH "data/mpr1-1.bmp";
      typedef nll::imaging::VolumeSpatial<double>           Volume;
      typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator>       Mpr;

      Volume volume;

      std::cout << "loadind..." << std::endl;
      bool loaded = nll::imaging::loadSimpleFlatFile( volname, volume );
      assert( loaded );

      std::cout << "loaded" << std::endl;
      Mpr mpr( volume, 512, 512 );

      nll::core::Timer mprTime;
      Mpr::Slice slice = mpr.getSlice( nll::core::vector3f( 0, 0, 43 ),
                                       nll::core::vector3f( 1, 0, 0 ),
                                       nll::core::vector3f( 0, 1, 0 ),
                                       nll::core::vector2f( 4, 4 ) );
      mprTime.end();
      std::cout << "mpr time full=" << mprTime.getCurrentTime() << std::endl;
      slice( 1, 1, 0 ) = 1e6;

      nll::core::Image<nll::i8> bmp( slice.sizex(), slice.sizey(), 1 );
      for ( unsigned y = 0; y < bmp.sizex(); ++y )
         for ( unsigned x = 0; x < bmp.sizex(); ++x )
            bmp( x, y, 0 ) = (nll::i8)NLL_BOUND( ( (double)slice( x, y, 0 )*3 + 20000 ) / 200, 0, 255 );
      nll::core::extend( bmp, 3 );
      nll::core::writeBmp( bmp, output );

      nll::core::Timer resampelTime;
      nll::core::rescaleBilinear( slice, 4096, 4096 );
      std::cout << "resample time=" << resampelTime.getCurrentTime() << std::endl;

      nll::core::Image<nll::ui8> bmp2( slice.sizex(), slice.sizey(), 1 );
      for ( unsigned y = 0; y < bmp2.sizey(); ++y )
         for ( unsigned x = 0; x < bmp2.sizex(); ++x )
            bmp2( x, y, 0 ) = (nll::ui8)NLL_BOUND( ( (double)slice( x, y, 0 )*3 + 20000 ) / 200, 0, 255 );
      nll::core::extend( bmp2, 3 );

      nll::core::writeBmp( bmp2, output2 );
   }

   /**
    Test the accuracy of the interpolator using nearest neighbour
    */
   /*
   // NOT a valid test anymore
   void testMpr2()
   {
      typedef nll::imaging::VolumeSpatial<double>           Volume;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume>   Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator>       Mpr;

      nll::core::Matrix<double> pst( 4, 4 );
      for ( unsigned n = 0; n < 4; ++n )
         pst( n, n ) = 1;

      Volume volume( nll::core::vector3ui( 2, 2, 2 ), pst );
      volume( 0, 0, 0 ) = 200;
      volume( 1, 0, 0 ) = 10;
      volume( 1, 1, 0 ) = 200;
      volume( 0, 1, 0 ) = 10;

      Mpr mpr( volume, 32, 32 );
      Mpr::Slice slice = mpr.getSlice( nll::core::vector3f( 0, 0, 0 ),
                                       nll::core::vector3f( 1, 0, 0 ),
                                       nll::core::vector3f( 0, 1, 0 ),
                                       nll::core::vector2d( 16.00, 16.00 ) );

      const std::string output = NLL_TEST_PATH "data/mpr-resamples-max.bmp";
      nll::core::Image<nll::i8> bmp( slice.sizex(), slice.sizey(), 1 );
      for ( unsigned y = 0; y < bmp.sizey(); ++y )
         for ( unsigned x = 0; x < bmp.sizex(); ++x )
            bmp( x, y, 0 ) = (nll::i8)NLL_BOUND( (double)slice( x, y, 0 ), 0, 255 );
      nll::core::extend( bmp, 3 );
      nll::core::writeBmp( bmp, output );

      for ( unsigned n = 0; n < 16; ++n )
         for ( unsigned nn = 0; nn < 16; ++nn )
         {
            TESTER_ASSERT( nll::core::equal<double>( slice( n, nn, 0 ), 200 ) );
            TESTER_ASSERT( nll::core::equal<double>( slice( n + 16, nn, 0 ), 10 ) );
            TESTER_ASSERT( nll::core::equal<double>( slice( n, nn + 16, 0 ), 10 ) );
            TESTER_ASSERT( nll::core::equal<double>( slice( n + 16, nn + 16, 0 ), 200 ) );
         }
   }*/

   /**
    Should produce a perfect chessboard

    (1.5, 1.5, 0) is the center of the 4x4 chessboard (we want to be 1 + 1/2 voxel off on x and y)
    */
   void testMpr3()
   {
      typedef nll::imaging::VolumeSpatial<double>           Volume;
      //typedef nll::imaging::InterpolatorNearestNeighbour<Volume>   Interpolator;
      typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator>       Mpr;

      nll::core::Matrix<float> pst( 4, 4 );
      for ( unsigned n = 0; n < 4; ++n )
         pst( n, n ) = 1;

      Volume volume( nll::core::vector3ui( 4, 4, 4 ), pst );
      volume( 0, 0, 0 ) = 255;
      volume( 1, 0, 0 ) = 10;
      volume( 1, 1, 0 ) = 255;
      volume( 0, 1, 0 ) = 10;

      volume( 0+2, 0, 0 ) = 255;
      volume( 1+2, 0, 0 ) = 10;
      volume( 1+2, 1, 0 ) = 255;
      volume( 0+2, 1, 0 ) = 10;

      volume( 0, 0+2, 0 ) = 255;
      volume( 1, 0+2, 0 ) = 10;
      volume( 1, 1+2, 0 ) = 255;
      volume( 0, 1+2, 0 ) = 10;

      volume( 0+2, 0+2, 0 ) = 255;
      volume( 1+2, 0+2, 0 ) = 10;
      volume( 1+2, 1+2, 0 ) = 255;
      volume( 0+2, 1+2, 0 ) = 10;

      Mpr mpr( volume, 32, 32 );
      Mpr::Slice slice = mpr.getSlice( nll::core::vector3f( 1.5, 1.5, 0 ),
                                       nll::core::vector3f( 1, 0, 0 ),
                                       nll::core::vector3f( 0, 1, 0 ),
                                       nll::core::vector2f( 8.00, 8.00 ) );


      const std::string output = NLL_TEST_PATH "data/mpr-resamples-max-linear.bmp";
      nll::core::Image<nll::i8> bmp( slice.sizex(), slice.sizey(), 1 );
      for ( unsigned y = 0; y < bmp.sizey(); ++y )
         for ( unsigned x = 0; x < bmp.sizex(); ++x )
            bmp( x, y, 0 ) = (nll::i8)NLL_BOUND( (double)slice( x, y, 0 ), 0, 255 );
      nll::core::extend( bmp, 3 );
      nll::core::writeBmp( bmp, output );
   }

   /**
    Should produce a perfect chessboard

    (1.5, 1.5, 0) is the center of the 4x4 chessboard (we want to be 1 + 1/2 voxel off on x and y)
    */
   void testMpr5()
   {
      typedef nll::imaging::VolumeSpatial<double>           Volume;
      typedef nll::imaging::InterpolatorNearestNeighbour<Volume>   Interpolator;
      //typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator>       Mpr;

      nll::core::Matrix<float> pst( 4, 4 );
      for ( unsigned n = 0; n < 4; ++n )
         pst( n, n ) = 1;

      Volume volume( nll::core::vector3ui( 4, 4, 4 ), pst );
      volume( 0, 0, 0 ) = 255;
      volume( 1, 0, 0 ) = 10;
      volume( 1, 1, 0 ) = 255;
      volume( 0, 1, 0 ) = 10;

      volume( 0+2, 0, 0 ) = 255;
      volume( 1+2, 0, 0 ) = 10;
      volume( 1+2, 1, 0 ) = 255;
      volume( 0+2, 1, 0 ) = 10;

      volume( 0, 0+2, 0 ) = 255;
      volume( 1, 0+2, 0 ) = 10;
      volume( 1, 1+2, 0 ) = 255;
      volume( 0, 1+2, 0 ) = 10;

      volume( 0+2, 0+2, 0 ) = 255;
      volume( 1+2, 0+2, 0 ) = 10;
      volume( 1+2, 1+2, 0 ) = 255;
      volume( 0+2, 1+2, 0 ) = 10;

      Mpr mpr( volume, 32, 32 );
      Mpr::Slice slice = mpr.getSlice( nll::core::vector3f( 1.5, 1.5, 0 ),
                                       nll::core::vector3f( 1, 0, 0 ),
                                       nll::core::vector3f( 0, 1, 0 ),
                                       nll::core::vector2f( 8.00, 8.00 ) );


      const std::string output = NLL_TEST_PATH "data/mpr-resamples-max-nn.bmp";
      nll::core::Image<nll::i8> bmp( slice.sizex(), slice.sizey(), 1 );
      for ( unsigned y = 0; y < bmp.sizey(); ++y )
         for ( unsigned x = 0; x < bmp.sizex(); ++x )
            bmp( x, y, 0 ) = (nll::i8)NLL_BOUND( (double)slice( x, y, 0 ), 0, 255 );
      nll::core::extend( bmp, 3 );
      nll::core::writeBmp( bmp, output );
   }

   /**
    This example produce strange results but is nevertheless correct!
    The problem is for the last voxel resampled from the input, is that there
    is no following voxels to interpolate, resulting in an interpolation with 0
    */
   void testResampling2d()
   {
      nll::core::Image<nll::ui8> i4(4, 4, 1);
      i4( 0, 0, 0 ) = 255;
      i4( 1, 0, 0 ) = 50;
      i4( 2, 0, 0 ) = 255;
      i4( 3, 0, 0 ) = 50;

      i4( 0, 1, 0 ) = 50;
      i4( 1, 1, 0 ) = 255;
      i4( 2, 1, 0 ) = 50;
      i4( 3, 1, 0 ) = 255;

      i4( 0, 2, 0 ) = 255;
      i4( 1, 2, 0 ) = 50;
      i4( 2, 2, 0 ) = 255;
      i4( 3, 2, 0 ) = 50;

      i4( 0, 3, 0 ) = 50;
      i4( 1, 3, 0 ) = 255;
      i4( 2, 3, 0 ) = 50;
      i4( 3, 3, 0 ) = 255;

      const unsigned factor = 64;
      nll::core::rescale<nll::ui8,
                         nll::core::IndexMapperRowMajorFlat2DColorRGBn,
                         nll::core::InterpolatorLinear2D<nll::ui8, nll::core::Image<nll::ui8>::IndexMapper, nll::core::Image<nll::ui8>::Allocator>
                        >(i4, 4 * factor, 4 * factor);
      nll::core::extend( i4, 3 );
      nll::core::writeBmp( i4, NLL_TEST_PATH "data/mresampl.bmp" );
   }

   void testMpr4()
   {
      const std::string volname = NLL_TEST_PATH "data/medical/pet-NAC.mf2";
      typedef nll::imaging::VolumeSpatial<float>           Volume;
      typedef nll::imaging::InterpolatorTriLinear<Volume>   Interpolator;
      //typedef nll::imaging::InterpolatorTriLinearDummy<Volume>   Interpolator;
      //typedef nll::imaging::InterpolatorNearestNeighbour<Volume>   Interpolator;
      typedef nll::imaging::Mpr<Volume, Interpolator>       Mpr;

      Volume volume;

      std::cout << "loadind..." << std::endl;
      nll::imaging::loadSimpleFlatFile( volname, volume );

      std::cout << "loaded" << std::endl;
      Mpr mpr( volume, 1024*4, 1024*4 );

      for ( unsigned z = 0; z < volume.getSize()[ 1 ]; ++z )
      {
         nll::core::Timer mprTime;
         Mpr::Slice slice = mpr.getSlice( nll::core::vector3f( 0, (float)z, 0 ),
                                          nll::core::vector3f( 1, 0, 0 ),
                                          nll::core::vector3f( 0, 0, 1 ),
                                          nll::core::vector2f( 8, 8 ) );
         mprTime.end();
         std::cout << "mpr time=" << mprTime.getCurrentTime() << std::endl;
         slice( 1, 1, 0 ) = 1e6;

         nll::core::Image<nll::i8> bmp( slice.sizex(), slice.sizey(), 1 );
         for ( unsigned y = 0; y < bmp.sizey(); ++y )
            for ( unsigned x = 0; x < bmp.sizex(); ++x )
               bmp( x, y, 0 ) = (nll::i8)NLL_BOUND( ( (double)slice( x, y, 0 )*4 + 15000 ) / 200, 0, 255 );
         nll::core::extend( bmp, 3 );
         nll::core::writeBmp( bmp, NLL_TEST_PATH "data/mpr-slice-" + nll::core::val2str( z ) + ".bmp" );
      }
   }

   void testVolumeIterators()
   {
      typedef nll::imaging::Volume<char>           Volume;

      Volume i1( 5, 6, 3 );
      const char* ubf = &i1( 0, 0, 0 );
      int k = 0;
      for ( unsigned z = 0; z < i1.size()[ 2 ]; ++z )
         for ( unsigned y = 0; y < i1.size()[ 1 ]; ++y )
            for ( unsigned x = 0; x < i1.size()[ 0 ]; ++x, ++k )
               i1( x, y, z ) = (char)k;

      unsigned m = 0;
      for ( Volume::DirectionalIterator it = i1.beginDirectional(); it != i1.endDirectional(); ++it, ++m )
      {
         TESTER_ASSERT( *it == ubf[ m ] );
      }

      m = 0;
      for ( unsigned z = 0; z < i1.size()[ 2 ]; ++z )
         for ( unsigned y = 0; y < i1.size()[ 1 ]; ++y )
            for ( unsigned x = 0; x < i1.size()[ 0 ]; ++x )
               TESTER_ASSERT( *i1.getIterator( x, y, z ) == ubf[ m++ ] );

      TESTER_ASSERT( i1.getIterator( 0, 0, 0 ).pickz( 2 ) == i1( 0, 0, 2 ) );
      TESTER_ASSERT( i1.getIterator( 0, 0, 0 ).pickx( 2 ) == i1( 2, 0, 0 ) );
      TESTER_ASSERT( i1.getIterator( 0, 0, 0 ).picky( 3 ) == i1( 0, 3, 0 ) );

      Volume i2( 2048 * 4, 2048 * 4, 3 );
      i2( 100, 100, 1 ) = 42;

      nll::core::Timer t1;
      m = 0;
      for ( unsigned z = 0; z < i2.size()[ 2 ]; ++z )
         for ( unsigned y = 0; y < i2.size()[ 1 ]; ++y )
            for ( unsigned x = 0; x < i2.size()[ 0 ]; ++x )
               m += i2( x, y, z );
      TESTER_ASSERT( m == 42 );
      double time1t = t1.getCurrentTime();
      std::cout << "volt1=" << t1.getCurrentTime() << std::endl;

      nll::core::Timer t2;      
      m = 0;
      for ( Volume::DirectionalIterator it = i2.beginDirectional(); it != i2.endDirectional(); ++it )
         m += *it;
      double time2t = t2.getCurrentTime();
      std::cout << "volt2=" << t2.getCurrentTime() << std::endl;
      TESTER_ASSERT( m == 42 );
      TESTER_ASSERT( time2t < time1t );
   }

   void testVolumeConstIterators()
   {
      typedef nll::imaging::Volume<char>           Volume;

      Volume i1( 5, 6, 3 );
      const char* ubf = &i1( 0, 0, 0 );
      int k = 0;
      for ( unsigned z = 0; z < i1.size()[ 2 ]; ++z )
         for ( unsigned y = 0; y < i1.size()[ 1 ]; ++y )
            for ( unsigned x = 0; x < i1.size()[ 0 ]; ++x, ++k )
               i1( x, y, z ) = (char)k;

      unsigned m = 0;
      for ( Volume::ConstDirectionalIterator it = i1.beginDirectional(); it != i1.endDirectional(); ++it, ++m )
      {
         TESTER_ASSERT( *it == ubf[ m ] );
      }

      m = 0;
      for ( unsigned z = 0; z < i1.size()[ 2 ]; ++z )
         for ( unsigned y = 0; y < i1.size()[ 1 ]; ++y )
            for ( unsigned x = 0; x < i1.size()[ 0 ]; ++x )
               TESTER_ASSERT( *i1.getIterator( x, y, z ) == ubf[ m++ ] );

      TESTER_ASSERT( i1.getIterator( 0, 0, 0 ).pickz( 2 ) == i1( 0, 0, 2 ) );
      TESTER_ASSERT( i1.getIterator( 0, 0, 0 ).pickx( 2 ) == i1( 2, 0, 0 ) );
      TESTER_ASSERT( i1.getIterator( 0, 0, 0 ).picky( 3 ) == i1( 0, 3, 0 ) );

      Volume i2( 2048 * 4, 2048 * 4, 3 );
      i2( 100, 100, 1 ) = 42;

      nll::core::Timer t1;
      m = 0;
      for ( unsigned z = 0; z < i2.size()[ 2 ]; ++z )
         for ( unsigned y = 0; y < i2.size()[ 1 ]; ++y )
            for ( unsigned x = 0; x < i2.size()[ 0 ]; ++x )
               m += i2( x, y, z );
      TESTER_ASSERT( m == 42 );
      double t1t = t1.getCurrentTime();
      std::cout << "volt1=" << t1.getCurrentTime() << std::endl;

      nll::core::Timer t2;      
      m = 0;
      for ( Volume::ConstDirectionalIterator it = i2.beginDirectional(); it != i2.endDirectional(); ++it )
         m += *it;
      double t2t = t2.getCurrentTime();
      std::cout << "volt2=" << t2.getCurrentTime() << std::endl;
      TESTER_ASSERT( m == 42 );
      TESTER_ASSERT( t2t < t1t );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestVolume);
 TESTER_TEST(testVolumeIterators);
 TESTER_TEST(testBuffer1);
 TESTER_TEST(testVolume1);
 TESTER_TEST(testVolumeIterator);
 TESTER_TEST(testVolumeSpatial1);
 TESTER_TEST(testIndexToPos);
 //TESTER_TEST(testInterpolator);
 //TESTER_TEST(testInterpolatorTriLinear);
 TESTER_TEST(testMpr);
 TESTER_TEST(testMpr3);
 TESTER_TEST(testMpr5);
 TESTER_TEST(testResampling2d);
 TESTER_TEST(testMpr4);
TESTER_TEST_SUITE_END();
#endif
