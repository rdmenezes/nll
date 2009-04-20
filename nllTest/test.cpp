#include "stdafx.h"

#include <fstream>
#include <sstream>
#include <nll/nll.h>

#pragma warning( push )
#pragma warning( disable:4996 ) // unsafe function

/**
 * Test harness for nllTest.
 *
 * @version 1.0
 * @author 
 */

static nll::core::Matrix<float> testRef(nll::core::Matrix<float> m, float* buf, nll::ui32 count)
{
   TESTER_ASSERT( m.getBuf() == buf && m.getRefCount() == count );
   return m;
}


struct Test1
{
   Test1(){_p = 0;}
   Test1(int s)
   {
      _p = new int[s];
   }
   ~Test1()
   {
      if (_p)
      {
         delete [] _p;
      }
   }
   void operator=(Test1& cpy)
   {
      _p = cpy._p;
      cpy._p = 0;
   }

   int* _p;
};

class TestnllCore
{
public:
   void testBuffer1D()
   {
      srand(1);
      nll::core::Buffer1D<float> buf1( 10 );
      nll::core::Buffer1D<float> buf3( 15 );
      nll::core::Buffer1D<float> buf2 = buf1;
      TESTER_ASSERT( buf1.getBuf() == buf2.getBuf() && buf1.getRefCount() == 2 && buf2.getRefCount() == 2 );
      buf2 = buf3;
      TESTER_ASSERT( buf2.getBuf() == buf3.getBuf() && buf2.getRefCount() == 2 && buf3.getRefCount() == 2 && buf1.getRefCount() == 1 );
      nll::core::Buffer1D<float> buf4( 20 );
      nll::core::Buffer1D<float> buf5 = testRef( nll::core::Matrix<float>(buf4), buf4.getBuf(), buf4.getRefCount() + 1 );
      TESTER_ASSERT( buf4.getBuf() == buf5.getBuf() && buf4.getRefCount() == 2 );

      double t1[] = { 0, 1.5, -10, 8 };
      nll::core::Buffer1D<double> buf6;
      buf6.import( t1, 4 );
      TESTER_ASSERT( buf6.getRefCount() == 1 && buf6.getBuf() != t1 );
      for ( int n = 0; n < 4; ++n )
         TESTER_ASSERT( nll::core::equal( buf6[ n ], t1[ n ] ) );
      int n1 = 0;
      for ( nll::core::Buffer1D<double>::const_iterator it = buf6.begin(); it != buf6.end(); ++it, ++n1 )
         TESTER_ASSERT( nll::core::equal( *it, t1[ n1 ] ) );

      nll::core::Buffer1D<double> buf7(10000);
      for (int nn = 0; nn < 100; ++nn)
      for (int n = 0; n < 10000; ++n)
         buf7[n] = n;

      nll::core::Buffer1D<Test1> buf8(2);
      Test1 a1 = Test1(10);
      buf8[0] = a1;
      buf8[1] = a1;
   }

   void testMatrix()
   {  
      char bufn[] = {1, 2, 3};
      nll::core::Matrix<char> mtest( bufn, 1, 3, false );
      nll::core::transpose( mtest );
      TESTER_ASSERT( mtest.sizex() == 1 && mtest.sizey() == 3 );
      TESTER_ASSERT( mtest( 0, 0 ) == 1 && mtest( 1, 0 ) == 2 && mtest( 2, 0 ) == 3 );

      nll::core::Matrix<char> mtest2( 1, 3 );
      mtest2[ 0 ] = 1;
      mtest2[ 1 ] = 2;
      mtest2[ 2 ] = 3;
      nll::core::transpose( mtest2 );
      TESTER_ASSERT( mtest2.sizex() == 1 && mtest2.sizey() == 3 );
      TESTER_ASSERT( mtest2( 0, 0 ) == 1 && mtest2( 1, 0 ) == 2 && mtest2( 2, 0 ) == 3 );

      nll::core::Matrix<float, nll::core::IndexMapperRowMajorFlat2D> m( 4, 5 );
      int n = 0;
      std::stringstream s;
      for (nll::core::Matrix<float>::iterator i = m.begin(); i != m.end(); ++i, ++n)
         *i = static_cast<float> ( n );

      m.write( s );
      nll::core::Matrix<float, nll::core::IndexMapperRowMajorFlat2D> m2;
      m2.read( s );
      TESTER_ASSERT( m2 == m );

      nll::core::Matrix<float, nll::core::IndexMapperRowMajorFlat2D> m3( 2, 2 );
      m3( 0, 1 ) = 8;
      m3( 1, 1 ) = -8;
      m3( 0, 0 ) = 88;
      m3( 1, 0 ) = 23;
      nll::core::Matrix<float, nll::core::IndexMapperRowMajorFlat2D> m4;
      m4.clone( m3 );
      bool inv = nll::core::inverse( m3 );
      
      nll::core::Matrix<float, nll::core::IndexMapperRowMajorFlat2D> m5 = nll::core::mul( m3, m4 );
      nll::core::sub( m5, nll::core::identity<float, nll::core::IndexMapperRowMajorFlat2D> ( 2 ) );

      TESTER_ASSERT( inv );
      double norm = nll::core::generic_norm2<float*, double>( m5.getBuf(), 2 * 2 );
      TESTER_ASSERT( norm <= 0.00001 );

      m3[2] = 88;
      nll::core::Buffer1D<int> b1 = nll::core::convert<float, nll::core::IndexMapperFlat1D, int, nll::core::IndexMapperFlat1D>(m3);
      TESTER_ASSERT( b1( 2 ) == 88 );

      nll::core::Matrix<double, nll::core::IndexMapperRowMajorFlat2D> m6 = nll::core::convertMatrix<float, nll::core::IndexMapperRowMajorFlat2D, double, nll::core::IndexMapperRowMajorFlat2D>(m3);
      TESTER_ASSERT( m6.sizex() == m3.sizex() );
      TESTER_ASSERT( m6.sizey() == m3.sizey() );
      for (unsigned int nx = 0; nx < m3.sizex(); ++nx)
         for (unsigned int ny = 0; ny < m3.sizey(); ++ny)
         {
            TESTER_ASSERT( m6( ny, nx ) == m3( ny, nx ) );
         }

      typedef long long                TYPE1;
      typedef nll::core::Matrix<TYPE1> TYPEMAT1;
      TYPEMAT1 mat1(4, 4);
      TYPEMAT1 mat2 = nll::core::identity<TYPE1, TYPEMAT1::IndexMapper>(4);
      int val = 0;
      
      for (TYPEMAT1::iterator it = mat1.begin(); it != mat1.end(); ++it, ++val)
         *it = val;
      nll::core::add(mat1, mat2);
      nll::core::sub(mat1, mat2);
      nll::core::mul<TYPE1, TYPEMAT1::IndexMapper>(mat1, 3.0);
      nll::core::div<TYPE1, TYPEMAT1::IndexMapper>(mat1, 3.0);
      val = 0;
      for (TYPEMAT1::iterator it = mat1.begin(); it != mat1.end(); ++it, ++val)
         TESTER_ASSERT( *it == val );

      nll::core::Matrix<float, nll::core::IndexMapperRowMajorFlat2D> test1(4, 3);
      val = 0;
      for (nll::core::Matrix<float>::iterator it = test1.begin(); it != test1.end(); ++it, ++val)
         *it = (float)val;
      nll::core::Buffer1D<float> meanRow = nll::core::meanRow(test1, 0, 2);
      float buf2[] = {3.0f, 4.0f, 5.0f};
      nll::core::Buffer1D<float> meanRefRow(buf2, 3, false);
      TESTER_ASSERT( meanRefRow == meanRow );

      nll::core::Buffer1D<float> meanCol = nll::core::meanCol(test1, 0, 2);
      float buf3[] = {1.0f, 4.0f, 7.0f, 10.0f};
      nll::core::Buffer1D<float> meanRefCol(buf3, 4, false);
      TESTER_ASSERT( meanRefCol == meanCol );

      float buf4[] =
      {
         4.0f, 2.0f, 0.6f,
         4.2f, 2.1f, 0.59f,
         3.9f, 2.0f, 0.58f,
         4.3f, 2.1f, 0.62f,
         4.1f, 2.2f, 0.63f
      };
      nll::core::Matrix<float, nll::core::IndexMapperRowMajorFlat2D> test2(buf4, 5, 3, false);
      float buf5[] =
      {
         0.025f, 0.0075f, 0.00175f,
         0.0075f, 0.0070f, 0.00135f,
         0.00175f, 0.00135f, 0.00043f
      };
      nll::core::Matrix<float, nll::core::IndexMapperRowMajorFlat2D> test2Result(buf5, 3, 3, false);

      nll::core::Matrix<float, nll::core::IndexMapperRowMajorFlat2D> r = nll::core::covariance(test2, 0, test2.sizey() - 1);
      TESTER_ASSERT( nll::core::covariance(test2, 0, test2.sizey() - 1).equal(test2Result, 0.0001f) );
      TESTER_ASSERT( nll::core::covariance(test2, 0, test2.sizey() - 1) == test2Result );
      

      test2Result(0, 0) = 10;
      TESTER_ASSERT( !(nll::core::covariance(test2, 0, test2.sizey() - 1) == test2Result) );
      test2Result(0, 0) = 0.025f;

      float buf6[] =
      {
         -1.0f, -1.0f, -1.0f,
         4.0f, 2.0f, 0.6f,
         4.2f, 2.1f, 0.59f,
         3.9f, 2.0f, 0.58f,
         4.3f, 2.1f, 0.62f,
         4.1f, 2.2f, 0.63f,
         -1.0f, -1.0f, -1.0f
      };
      nll::core::Matrix<float, nll::core::IndexMapperRowMajorFlat2D> test3(buf6, 7, 3, false);
      TESTER_ASSERT( nll::core::covariance(test3, 1, test3.sizey() - 2).equal(test2Result));

      nll::core::Matrix<char> d(4, 1);
      d(0, 0) = 'g';
      d(1, 0) = 'a';
      d(2, 0) = 'l';
      d(3, 0) = 'b';
      std::vector<nll::ui32> rang(4);
      rang[0] = 2;
      rang[1] = 0;
      rang[2] = 3;
      rang[3] = 1;
      nll::core::sortRow( d, rang );
      TESTER_ASSERT( d(0, 0) == 'a' );
      TESTER_ASSERT( d(1, 0) == 'b' );
      TESTER_ASSERT( d(2, 0) == 'g' );
      TESTER_ASSERT( d(3, 0) == 'l' );

      nll::core::Matrix<char> d2(1, 4);
      d2(0, 0) = 'g';
      d2(0, 1) = 'a';
      d2(0, 2) = 'l';
      d2(0, 3) = 'b';
      nll::core::sortCol( d2, rang );
      TESTER_ASSERT( d2(0, 0) == 'a' );
      TESTER_ASSERT( d2(0, 1) == 'b' );
      TESTER_ASSERT( d2(0, 2) == 'g' );
      TESTER_ASSERT( d2(0, 3) == 'l' );
   }

   void testDatabase()
   {
      std::stringstream stream;
      typedef nll::core::ClassificationSample<nll::core::Buffer1D<float>, int> ClassificationSample;
      typedef nll::core::Database<ClassificationSample> Database;


      ClassificationSample s1( nll::core::make_buffer1D<float>( 0 ),
                               0,
                               ClassificationSample::TESTING,
                               nll::core::make_buffer1D_from_string("test1"));
      ClassificationSample s2( nll::core::make_buffer1D<float>( 10 ),
                               1,
                               ClassificationSample::LEARNING,
                               nll::core::make_buffer1D_from_string("test2"));
      Database dat;
      dat.add(s1);
      dat.add(s2);
      dat.write(stream);

      Database dat2;
      dat2.read(stream);

      TESTER_ASSERT(dat2.size() == 2);
      TESTER_ASSERT(dat[0] == dat2[0]);
	   TESTER_ASSERT(dat[1] == dat2[1]);

      TESTER_ASSERT( nll::core::getNumberOfClass( dat ) == 2 );
   }

   void testClassifier()
   {
      srand( 0 );
      typedef nll::algorithm::Classifier<float*>   Classifier;

      class ClassifierTest : public Classifier
      {
         typedef float* T;
      public:
         virtual Classifier* deepCopy() const { return (Classifier*)this; }
         virtual void read( std::istream&  )
         {}
         virtual void write( std::ostream&  ) const
         {}
         virtual Classifier::Class test( const T&  ) const
         { return 0; }
         virtual void learn( const Database& , const nll::core::Buffer1D<nll::f64>& )
         {}
         virtual double evaluate( const nll::core::Buffer1D<nll::f64>&,  const Database& ) const { return 0; }
         ClassifierTest( const nll::algorithm::ParameterOptimizers& p ) : Classifier( p ){}

         static const nll::algorithm::ParameterOptimizers createParameters()
         {
            nll::algorithm::ParameterOptimizers params;
            return params;
         }
      };

      ClassifierTest c1( ClassifierTest::createParameters() );
      ClassifierTest::Database dat;
      Classifier* classifier1 = &c1;
      //TESTER_ASSERT( classifier1->checkDatabaseConsistency(dat) );
      dat.add(ClassifierTest::Database::Sample(0, 0, ClassifierTest::Database::Sample::LEARNING));
      dat.add(ClassifierTest::Database::Sample(0, 1, ClassifierTest::Database::Sample::LEARNING));
      dat.add(ClassifierTest::Database::Sample(0, 1, ClassifierTest::Database::Sample::TESTING));
      //TESTER_ASSERT( classifier1->checkDatabaseConsistency(dat) );

      dat.add(ClassifierTest::Database::Sample(0, 4, ClassifierTest::Database::Sample::LEARNING));
      //TESTER_ASSERT( !classifier1->checkDatabaseConsistency(dat) );

      dat.clear();
      dat.add(ClassifierTest::Database::Sample(0, 0, ClassifierTest::Database::Sample::TESTING));
      //TESTER_ASSERT( !classifier1->checkDatabaseConsistency(dat) );
      dat.add(ClassifierTest::Database::Sample(0, 0, ClassifierTest::Database::Sample::LEARNING));
      //TESTER_ASSERT( classifier1->checkDatabaseConsistency(dat) );
      Classifier::Result result1 = classifier1->test( dat );
      TESTER_ASSERT( result1.testingError == 0 && result1.learningError == 0 && result1.validationError == -1);

      dat.add(ClassifierTest::Database::Sample(0, 1, ClassifierTest::Database::Sample::LEARNING));
      dat.add(ClassifierTest::Database::Sample(0, 1, ClassifierTest::Database::Sample::TESTING));
      Classifier::Result result2 = classifier1->test(dat);
      TESTER_ASSERT( nll::core::equal(result2.learningError, 0.5) &&
                      nll::core::equal(result2.testingError, 0.5) &&
                      nll::core::equal(result2.validationError, -1.0));

      //
      // TODO CODE MORE CROSSFOLD TEST
      // TEST2 cross validation : generate 60 examples from 3 classes
      //
      nll::core::LoggerHandler::instance().createFileLogger("data/testLogger2.txt");
      class ClassifierTest2 : public nll::algorithm::Classifier<int>
      {
         typedef int T;
         typedef nll::algorithm::Classifier<int> Classifier;

      public:
         virtual Classifier* deepCopy() const { return new ClassifierTest2(_parametersPrototype); }
         virtual void read( std::istream&  )
         {}
         virtual void write( std::ostream&  ) const
         {}
         virtual Classifier::Class test( const T& val ) const
         { return val; }
         virtual void learn( const Database& dat, const nll::core::Buffer1D<nll::f64>& )
         {
            std::map<nll::ui32, nll::ui32> learning;
            for ( nll::ui32 n = 0; n < dat.size(); ++n )
               if ( dat[ n ].type == Database::Sample::TESTING )
                  ++learning[ dat[ n ].output ];
            for (std::map<nll::ui32, nll::ui32>::const_iterator it = learning.begin(); it != learning.end(); ++it)
            {
               TESTER_ASSERT( it->second >= 2 );
            }
         }
         ClassifierTest2( const nll::algorithm::ParameterOptimizers& p ) : Classifier( p ){}

         static const nll::algorithm::ParameterOptimizers createParameters()
         {
            nll::algorithm::ParameterOptimizers params;
            return params;
         }

         virtual double evaluate( const nll::core::Buffer1D<nll::f64>&,  const Database& ) const { return 0; }
      };

      srand( 0 );
      ClassifierTest2::Database dat1;
      
      for ( nll::ui32 n = 0; n < 40; ++n )
         dat1.add(ClassifierTest2::Database::Sample(0, 0, ClassifierTest2::Database::Sample::LEARNING));
      for ( nll::ui32 n = 0; n < 40; ++n )
         dat1.add(ClassifierTest2::Database::Sample(1, 1, ClassifierTest2::Database::Sample::LEARNING));
      for ( nll::ui32 n = 0; n < 40; ++n )
         dat1.add(ClassifierTest2::Database::Sample(2, 2, ClassifierTest2::Database::Sample::LEARNING));
      for ( nll::ui32 n = 0; n < 40; ++n )
         dat1.add(ClassifierTest2::Database::Sample(0, 1, ClassifierTest2::Database::Sample::TESTING));

      ClassifierTest2 c2( ClassifierTest::createParameters() );
      nll::algorithm::Classifier<int>* classifier2 = &c2;
      nll::core::Buffer1D<double> lparameters;
      ClassifierTest2::Result r = classifier2->test( dat1, lparameters, 6 );
      TESTER_ASSERT( nll::core::absolute(r.learningError ) <= 0 );
      TESTER_ASSERT( nll::core::absolute(r.testingError - (-1) ) < 0.001 );
      TESTER_ASSERT( nll::core::absolute(r.validationError - (-1)) < 0.001 );

      for ( nll::ui32 n = 0; n < 20; ++n )
         dat1.add(ClassifierTest2::Database::Sample(0, 2, ClassifierTest2::Database::Sample::VALIDATION));
      r = classifier2->test( dat1, lparameters, 6 );
      double err = r.learningError - 20.0 / 140;
      TESTER_ASSERT( nll::core::absolute( err ) <= 0.01 );
   }

   void testStaticVector()
   {
      float buf[] = {1, 2, 3};
      float* buf2 = buf;
      nll::core::StaticVector<float, 3> v2(buf2);
      nll::core::StaticVector<float, 3> v1;
      v1[0] = 5;
      v1[1] = -5;
      v1[2] = 8;

      TESTER_ASSERT( v2.getBuf() != buf );
      for (int n = 0; n < 3; ++n)
         TESTER_ASSERT( v2[n] == buf[n] );

      nll::core::StaticVector<float, 3> v3 = v2 + v1;
      for (int n = 0; n < 3; ++n)
         TESTER_ASSERT( v3[n] == v2[n] + v1[n] );
      TESTER_ASSERT( v3.getBuf() != v2.getBuf() );
      v1.mul(2.0f);
      TESTER_ASSERT( v1[0] == 10 );
      TESTER_ASSERT( nll::core::equal<float>( (float)v2.norm2(), (float)sqrt(buf[0] * buf[0] + buf[1] * buf[1] + buf[2] * buf[2])) );

      
      v1.add(v2);
      v1.sub(v2);
      v1.mul(3.0);
      v1.div(3.0);
      TESTER_ASSERT( v1[0] == 10 );
      TESTER_ASSERT( v1[1] == -10 );
      TESTER_ASSERT( v1[2] == 16 );

      v1 = v1 + v2;
      v1 = v1 - v2;
      v1 = v1 * 3.0;
      v1 = v1 / 3.0;
      TESTER_ASSERT( v1[0] == 10 );
      TESTER_ASSERT( v1[1] == -10 );
      TESTER_ASSERT( v1[2] == 16 );

      TESTER_ASSERT( nll::core::equal<double>(v1.dot(v1), v1.norm2() * v1.norm2()) );
   }

   typedef nll::core::StaticVector<float, 2> Point2D;
   Point2D randomPoint2D(float x, float y, int dist)
   {
      Point2D vec(2);
      vec[0] = x + static_cast<float>((rand() % dist)) - dist / 2;
      vec[1] = y + static_cast<float>((rand() % dist)) - dist / 2;
      return vec;
   }

   void testMath()
   {
      srand(0);
      TESTER_ASSERT( nll::core::equal<double> (nll::core::PI, 3.1415926535897932384626433) );

      double mean = 10;
      double variance = 3;
      int size = 20000;
      nll::core::Matrixd vals( size, 1 );
      for ( int n = 0; n < size; ++n )
         vals( n, 0 ) = nll::core::generateGaussianDistribution( mean, variance );
      nll::core::Buffer1D<double> me = nll::core::meanRow( vals, 0, size - 1 );
      nll::core::Matrixd var = nll::core::covariance( vals, 0, size - 1 );

      TESTER_ASSERT( nll::core::equal(me(0), mean, 0.2) );
      TESTER_ASSERT( nll::core::equal(sqrt(var(0, 0)), variance, 0.2) );
   }

   void testImage()
   {
      typedef nll::core::Image<unsigned char> image;
      image i1;
      TESTER_ASSERT( nll::core::readBmp(i1, "data/image/test-image1.bmp") );
      TESTER_ASSERT( i1.equal(0, 0, image::white()) );
      TESTER_ASSERT( i1.equal(0, i1.sizey() - 1, image::red()) );

      image i2;
      i2.clone(i1);
      nll::core::binarize(i2, nll::core::ThresholdGreater<nll::ui32> (200));
      TESTER_ASSERT( i2.equal(0, 0, image::white()) );
      TESTER_ASSERT( i2.equal(1, 0, image::white()) );
      TESTER_ASSERT( i2.equal(0, i1.sizey() - 1, image::black()) );

      image i3;
      i3.clone(i1);
      nll::core::binarize(i3, nll::core::ThresholdGreater<nll::ui32> (80));
      TESTER_ASSERT( i3.equal(0, 0, image::white()) );
      TESTER_ASSERT( i3.equal(1, 0, image::white()) );
      TESTER_ASSERT( i3.equal(0, i3.sizey() - 1, image::white()) );

      image i3a;
      i3a.clone(i1);
      nll::core::decolor(i3a);
      nll::core::binarize(i3a, nll::core::ThresholdBetween<nll::ui32> (10, 200));
      nll::core::extend(i3a, 3);
      nll::core::writeBmp(i3a, "data/thresholdtest.bmp");


      nll::core::writeBmp(i1, "data/tmp.bmp");
      image i4;
      nll::core::readBmp(i4, "data/tmp.bmp");
      TESTER_ASSERT( i4 == i1 );
      TESTER_ASSERT( !(i2 == i1) );

      nll::core::threshold(i4, nll::core::ThresholdGreater<nll::ui32> (80));
      TESTER_ASSERT( i4.equal(0, i3.sizey() - 1, image::red()) );
      nll::core::threshold(i4, nll::core::ThresholdGreater<nll::ui32> (200));
      TESTER_ASSERT( i4.equal(0, i3.sizey() - 1, image::black()) );

      nll::core::readBmp(i4, "data/image/test-image1.bmp");
      nll::core::convolve(i4, nll::core::buildGaussian());
      TESTER_ASSERT( i4(1, 1, 0) > 30 );
      TESTER_ASSERT( (int)i4(2, 1, 0) > 20 );
      nll::core::writeBmp(i4, "data/tmp.bmp");

      // resampling
      image i5, i6, i7, i8, i9;
      nll::core::readBmp(i5, "data/image/test-image1.bmp");
      i6.clone(i5);
      i9.clone(i5);
      nll::core::rescaleFast(i9, 256, 16);
      nll::core::writeBmp(i9, "data/test4.bmp");

      nll::core::Image<float> flImage;
      nll::core::Image<nll::ui32> uiImage;
      nll::core::Image<nll::i32> iImage;
      nll::core::convert(i5, flImage);
      nll::core::convert(i5, uiImage);
      nll::core::convert(i5, iImage);
      nll::core::rescaleFast(flImage, 32, 32);
      nll::core::rescaleFast(uiImage, 32, 32);
      nll::core::rescaleFast(iImage, 32, 32);
      nll::core::rescaleFast(i6, 32, 32);
      nll::core::convert(flImage, i5);
      nll::core::convert(uiImage, i7);
      nll::core::convert(iImage, i8);
      

      nll::core::writeBmp(i5, "data/test2.bmp");
      nll::core::writeBmp(i7, "data/test3.bmp");

      TESTER_ASSERT( i5 == i6 );
      TESTER_ASSERT( i5 == i8 );
      TESTER_ASSERT( i5 == i7 );
   }

   template <class Point, int SIZE>
   struct Metric2D
   {
      double distance( const Point& p1, const Point& p2 )
      {
         return nll::core::generic_norm2<Point, nll::f64>( p1, p2, static_cast<unsigned>( SIZE ) );
      }
   };

   void testKmeans()
   {
      typedef nll::algorithm::BuildKMeansUtility<Point2D, Metric2D<Point2D, 2> >		KMeansTestHelper;
      typedef nll::algorithm::KMeans<KMeansTestHelper>	                           KMeans2D;

      Metric2D<Point2D, 2> metric;
      KMeansTestHelper helper( 2, metric );

      KMeans2D kmeans( helper );
      KMeans2D::Points points;
	   for (int n = 0; n < 100; ++n)
		   points.push_back(randomPoint2D(0, 100, 50));
	   for (int n = 0; n < 100; ++n)
		   points.push_back(randomPoint2D(-200, 50, 50));
	   for (int n = 0; n < 100; ++n)
		   points.push_back(randomPoint2D(0, -200, 50));
	   KMeans2D::KMeansResult res = kmeans(points, 3);
	   int cluster_nb[3];
	   for (int n = 0; n < 3; ++n)
		   cluster_nb[n] = 0;
	   for (int n = 0; n < (int)res.first.size(); ++n)
		   ++cluster_nb[res.first[n]];

	   for (int n = 0; n < 3; ++n)
         TESTER_ASSERT( cluster_nb[n] == 100 );
   }

   typedef std::vector<nll::f32>			GmmTestPoint;
   typedef std::vector<GmmTestPoint>	GmmTestPoints;
   
   inline static GmmTestPoints load(const std::string& file)
   {
	   std::string buf;
	   GmmTestPoints	vs;
	   std::fstream f(file.c_str());

	   assert(f.is_open());
	   while (!f.eof())
	   {
		   GmmTestPoint v(13);
		   getline(f, buf);
		   nll::ui32 nb = sscanf(buf.c_str(), "%f %f %f %f %f %f %f %f %f %f %f %f %f", 
			   &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10], &v[11], &v[12]);
		   if (f.eof())
			   break;
		   assert(nb == 13); // else missing data
		   vs.push_back(v);
	   }
	   return vs;
   }

   
   void testGmm()
   {

# define GMM_TEST_PATH "data/gmm/"

      static const nll::i8* gmm_train[] =
      {
	      GMM_TEST_PATH "spk01-train.mfc",
	      GMM_TEST_PATH "spk02-train.mfc",
	      GMM_TEST_PATH "spk03-train.mfc",
	      GMM_TEST_PATH "spk04-train.mfc",
	      GMM_TEST_PATH "spk05-train.mfc",
	      GMM_TEST_PATH "spk06-train.mfc",
	      GMM_TEST_PATH "spk07-train.mfc",
	      GMM_TEST_PATH "spk08-train.mfc",
	      GMM_TEST_PATH "spk09-train.mfc",
	      GMM_TEST_PATH "spk10-train.mfc",
      };

      static const nll::i8* gmm_sample[] =
      {
	      GMM_TEST_PATH "unk01-test.mfc",
	      GMM_TEST_PATH "unk02-test.mfc",
	      GMM_TEST_PATH "unk03-test.mfc",
	      GMM_TEST_PATH "unk04-test.mfc",
	      GMM_TEST_PATH "unk05-test.mfc",
	      GMM_TEST_PATH "unk06-test.mfc",
	      GMM_TEST_PATH "unk07-test.mfc",
	      GMM_TEST_PATH "unk08-test.mfc",
	      GMM_TEST_PATH "unk09-test.mfc",
	      GMM_TEST_PATH "unk10-test.mfc",
      };

      static const nll::i32 gmm_results[] =
      {
	      2,
	      0,
	      1,
	      7,
	      9,
	      6,
	      4,
	      5,
	      3,
	      8
      };

      const nll::ui32 size = nll::core::getStaticBufferSize(gmm_train);
	   GmmTestPoints* ps = new GmmTestPoints[10];
      for (nll::ui32 n = 0; n < 10; ++n)
		   ps[n] = load(gmm_train[n]);

      const nll::ui32 size2 = nll::core::getStaticBufferSize(gmm_sample);
	   GmmTestPoints* pss = new GmmTestPoints[10];
	   for (nll::ui32 n = 0; n < 10; ++n)
		   pss[n] = load(gmm_sample[n]);

      nll::algorithm::Gmm* gmms[10];
      for (nll::ui32 n = 0; n < size; ++n)
	   {
		   std::cout << " build model:" << n << std::endl;
		   gmms[n] = new nll::algorithm::Gmm();
		   gmms[n]->em( ps[n], 12, 16, 5 );
	   }

      for (nll::ui32 n = 0; n < size2; ++n)
	   {
		   nll::f64 l = INT_MIN;
		   nll::i32 choice = -1;
         for (nll::ui32 nn = 0; nn < size; ++nn)
		   {
			   nll::f64 tmp = gmms[nn]->likelihood(pss[n]);
			   std::cout << "[" << n << "]->" << nn << "=" << tmp << std::endl;
			   if (tmp > l)
			   {
				   l = tmp;
				   choice = nn;
			   }
		   }
		   std::cout << "choice[" << n << "]=" << choice << std::endl;

		   if (gmm_results[n] != choice)
			   std::cout << "error: expected choice=" << gmm_results[n] << std::endl;
         TESTER_ASSERT( gmm_results[n] == choice );
	   }
   }


   void testLogger()
   {
      nll::ui32 id = nll::core::LoggerHandler::instance().createFileLogger("data/testLogger.txt");
      nll::core::LoggerHandler::instance().getLogger(id).write("test1");
      nll::core::LoggerHandler::instance().getLogger(id).write("test2");
      nll::core::LoggerHandler::instance().getLogger(id).write("test3");
      nll::core::LoggerHandler::instance().destroyLogger(id);
      std::ifstream f("data/testLogger.txt");
      std::string str;
      f >> str;
      TESTER_ASSERT( str == std::string("test1") );
      f >> str;
      TESTER_ASSERT( str == std::string("test2") );
      f >> str;
      TESTER_ASSERT( str == std::string("test3") );
   }

   void testKdTree()
   {
      typedef nll::algorithm::KdTree<float*> KdTreeAlgo;//3
      typedef nll::algorithm::KdTree<Point2D> KdTreeAlgo2;//2
      typedef std::vector<float> Point;
      typedef nll::algorithm::KdTree<Point> KdTreeTest;//1

      std::vector<Point> points;
	   {
		   Point p(1);
		   p[0] = 1;
		   points.push_back(p);
		   p[0] = 2;
		   points.push_back(p);
		   p[0] = 10;
		   points.push_back(p);
		   p[0] = 11;
		   points.push_back(p);
		   p[0] = -1;
		   points.push_back(p);
		   p[0] = -2;
		   points.push_back(p);
	   }
	   KdTreeTest kdtree;
	   kdtree.build(points, 1);

	   static const unsigned int result_diff[][4] =
	   {
		   {0, 1, 4, 5},
		   {1, 0, 4, 5},
		   {2, 3, 1, 0},
		   {3, 2, 1, 0},
		   {4, 5, 0, 1},
		   {5, 4, 0, 1}
	   };

	   for (int n = 0; n < 6; ++n)
	   {
         KdTreeTest::NearestNeighborList nnlist = kdtree.findNearestNeighbor(points[ n ], 4);

		   int nn = 0;
		   for (KdTreeTest::NearestNeighborList::const_iterator i = nnlist.begin(); i != nnlist.end(); ++i, ++nn)
			   TESTER_ASSERT( i->id == result_diff[n][nn] );
	   }

      {
         Point p = nll::core::make_vector<float>( 0.1f );
         KdTreeTest::NearestNeighborList list = kdtree.findNearestNeighbor( p, 2 );
         TESTER_ASSERT( list.begin()->id == 0 );
         TESTER_ASSERT( list.rbegin()->id == 4 );
      }

      {
         Point p = nll::core::make_vector<float>( 1.4f );
         KdTreeTest::NearestNeighborList list = kdtree.findNearestNeighbor( p, 2 );
         TESTER_ASSERT( list.begin()->id == 0 );
         TESTER_ASSERT( list.rbegin()->id == 1 );
      }

      {
         Point p = nll::core::make_vector<float>( 8 );
         KdTreeTest::NearestNeighborList list = kdtree.findNearestNeighbor( p, 2 );
         TESTER_ASSERT( list.begin()->id == 2 );
         TESTER_ASSERT( list.rbegin()->id == 3 );
      }
   }

   void testGabor()
   {
      typedef nll::core::Image<nll::ui8>     Image ;

      // test low level access
      nll::algorithm::GaborFilterDescriptors gdescs;
      gdescs.push_back(nll::algorithm::GaborFilterDescriptor(0.1f, 0, 8));
      gdescs.push_back(nll::algorithm::GaborFilterDescriptor(0.1f, (nll::f32)nll::core::PI / 4, 8));

      std::vector<nll::core::Matrix<nll::f32> > convs = nll::algorithm::computeGaborFilters<nll::f32, nll::core::IndexMapperColumnMajorFlat2D>(gdescs);
      TESTER_ASSERT( convs.size() == 2 );

      // test API
      nll::algorithm::GaborFilters<nll::ui8, Image::IndexMapper, nll::f64> gabors;
      gabors.addFilter( nll::algorithm::GaborFilterDescriptor(0.1f, 0, 8) );
      gabors.addFilter( nll::algorithm::GaborFilterDescriptor(0.23f, 0, 8) );
      TESTER_ASSERT( gabors.size() == 2 );

      // test convolution
      nll::core::Image<nll::ui8> im1;
      nll::core::readBmp(im1, "data/image/test-image1.bmp");
      nll::core::decolor(im1);
      nll::core::convolveBorder(im1, gabors[1]);
      nll::core::extend(im1, 3);
      nll::core::writeBmp(im1, "data/tmp-img.bmp");

      // test list convolutions
      std::cout << "gabor:" << std::endl;
      nll::algorithm::GaborFilterDescriptors gdescs2;
      nll::f32 kernelSize = 8.0f;
      nll::f32 frequency = 0.20f;
      gdescs2.push_back(nll::algorithm::GaborFilterDescriptor(0.01f, 0, kernelSize));
      gdescs2.push_back(nll::algorithm::GaborFilterDescriptor(frequency, (nll::f32)nll::core::PI / 4, kernelSize));
      gdescs2.push_back(nll::algorithm::GaborFilterDescriptor(0.05f, (nll::f32)nll::core::PI / 8, 60.0f));

      nll::algorithm::GaborFilters<nll::ui8, Image::IndexMapper, nll::ui8> gabors2( gdescs2 );

      Image im2;
      nll::core::readBmp( im2, "data/image/test-image2.bmp" );
      nll::core::decolor( im2 );

      
      nll::core::Timer timer1;
      Image buf = gabors2.convolve( im2 );
      timer1.end();
      std::cout << "gabor=" << timer1.getCurrentTime() << std::endl;
      for ( nll::ui32 n = 0; n < buf.getNbComponents(); ++n )
      {
         Image cmp = nll::core::extractChannel( buf, n );
         nll::core::extend( cmp, 3 );
         nll::core::writeBmp( cmp, "data/gabor-" + nll::core::val2str( n ) + ".bmp" );
      }
   }

   void testSequenceConverter()
   {
      nll::ui32 t1[] = {1, 0, 2, 3, 4};
      std::vector<nll::ui8> t2;
      for (nll::ui32 n = 0; n < nll::core::getStaticBufferSize(t1); ++n)
         t2.push_back((nll::ui8)t1[n]);
      nll::core::Buffer1D<nll::f32> v1 = nll::core::convert<nll::ui32[5], nll::core::Buffer1D<nll::f32> >(t1, nll::core::getStaticBufferSize(t1));
      nll::core::Buffer1D<nll::f32> v2;
      nll::core::Buffer1D<nll::ui8> v3;
      nll::core::convert(t2, v2, (nll::ui32)t2.size());
      TESTER_ASSERT( v1 == v2 );
      nll::core::convert(v2, v3, v2.size());
      TESTER_ASSERT( v1.equal( v3 ) );
   }

   void testImageBinaryOperation()
   {
      nll::ui8 white[]  =  {255, 255, 255};
      nll::ui8 red[]    =  {0, 0, 255};
      nll::ui8 blue[]   =  {0, 255, 0};
      nll::ui8 br[]     =  {0, 255, 255};

      nll::core::Image<nll::ui8> i1(3, 3, 3 );
      nll::core::Image<nll::ui8> i2(3, 3, 3 );
      i1.setPixel( 0, 0, white );
      i1.setPixel( 1, 0, red );
      i1.setPixel( 2, 0, blue );
      i1.setPixel( 0, 1, blue );
      
      i2.setPixel( 0, 0, white );
      i2.setPixel( 1, 0, white );
      i2.setPixel( 2, 0, white );
      i2.setPixel( 0, 1, red );
      nll::core::Image<nll::ui8> i3 = nll::core::transform(i1, i2, nll::core::BinaryAdd<nll::ui8>());

      TESTER_ASSERT( !nll::core::isColorEqual(white, red, 3) );
      TESTER_ASSERT( nll::core::isColorEqual(white, white, 3) );

      TESTER_ASSERT( nll::core::isColorEqual(i3.point(0, 0), white, 3) );
      TESTER_ASSERT( nll::core::isColorEqual(i3.point(1, 0), white, 3) );
      TESTER_ASSERT( nll::core::isColorEqual(i3.point(2, 0), white, 3) );
      TESTER_ASSERT( nll::core::isColorEqual(i3.point(0, 1), br, 3) );

      nll::core::Image<nll::ui8> ii1("data/image/test-image2.bmp");
      nll::core::Image<nll::ui8> ii2("data/image/test-image1.bmp");
      nll::core::Image<nll::ui8> ii3 = nll::core::transform(ii1, ii2, nll::core::BinaryAdd<nll::ui8>());

      nll::core::transformUnaryFast(ii3, nll::core::TransformationRotation(-0.3f, nll::core::vector2f(0, 0)));
      nll::core::convolve(ii3, nll::core::buildGaussian());
      nll::core::rescaleFast(ii3, 512, 512);
      nll::core::convolve(ii3, nll::core::buildGaussian());
      nll::core::writeBmp(ii3, "data/test5.bmp");

      nll::core::Image<nll::ui8> ii4("data/image/test-image3.bmp");
      nll::core::rescaleFast(ii4, 128, 128);
      nll::core::writeBmp(ii4, "data/resample1.bmp");
   }
   
   void testMatrixOperators()
   {
      nll::core::Matrix<float> f1(3, 3);
      f1(0, 0) = 2;
      f1(1, 0) = 4;
      nll::core::Matrix<float> f2 = nll::core::identity<float, nll::core::Matrix<float>::IndexMapper>(3);

      nll::core::Matrix<float> f3 = f1 + f2;
      nll::core::Matrix<float> f4;
      f4.clone(f1);
      TESTER_ASSERT( nll::core::equal<float>(f3(0, 0), 3) );
      TESTER_ASSERT( nll::core::equal<float>(f3(1, 1), 1) );
      TESTER_ASSERT( nll::core::equal<float>(f3(1, 0), 4) );

      f3 = f3 - f2;
      f3 = f3 * 3.0f;
      f3 = 3.0f * f3;
      f3 *= 3.0f;
      f3 /= 3.0f;
      f3 = f3 / 9.0;
      TESTER_ASSERT( f4.equal(f3) );

      f3 = f3 * f4;
   }

   void testInterpolatorPerf()
   {
      nll::core::Image<nll::ui8> img("data/image/test-image1.bmp");
      nll::core::Image<nll::ui8> img2;
      img2.clone(img);

      nll::core::Timer t1;
      nll::core::rescaleBilinear( img, 1024, 1024 );
      nll::core::rescaleNearestNeighbor( img2, 1024, 1024 );
      t1.end();

      nll::core::writeBmp( img, "data/interp-perf.bmp" );
      std::cout << "Perf Interpolator=" << t1.getTime() << std::endl;
   }

   void testInterpolator()
   {
      nll::core::Timer t1;

      nll::ui8 white[]  =  {255, 255, 255};
      nll::ui8 red[]    =  {0, 0, 255};
      nll::ui8 blue[]   =  {0, 255, 0};
      nll::ui8 green[]   =  {255, 0, 0};

      nll::core::Image<nll::ui8> i(4, 4, 3);
      i.setPixel(0, 0, white);
      i.setPixel(1, 0, white);
      i.setPixel(0, 1, white);
      i.setPixel(1, 1, white);
      nll::core::InterpolatorLinear2D<nll::ui8, nll::core::Image<nll::ui8>::IndexMapper> linearInterpolator( i );
      double v1 = linearInterpolator.interpolate(0.5, 0.5, 0);
      TESTER_ASSERT( nll::core::equal(v1, 255.0) );

      nll::core::Image<nll::ui8> i2("data/image/test-image3.bmp");
      nll::core::rescale<nll::ui8,
                         nll::core::IndexMapperRowMajorFlat2DColorRGBn,
                         nll::core::InterpolatorLinear2D<nll::ui8, nll::core::Image<nll::ui8>::IndexMapper>
                        >(i2, 128, 128);
      nll::core::writeBmp(i2, "data/rescale-interp1.bmp");

      nll::core::Image<nll::ui8> i3("data/image/test-image1.bmp");
      nll::core::rescale<nll::ui8,
                         nll::core::IndexMapperRowMajorFlat2DColorRGBn,
                         nll::core::InterpolatorLinear2D<nll::ui8, nll::core::Image<nll::ui8>::IndexMapper>
                        >(i3, 32, 32);
      nll::core::writeBmp(i3, "data/rescale-interp2.bmp");

      nll::core::Image<nll::ui8> i4(3, 3, 3);
      i4.setPixel(0, 0, red);
      i4.setPixel(1, 0, blue);
      i4.setPixel(2, 0, red);
      i4.setPixel(0, 1, blue);
      i4.setPixel(1, 1, green);
      i4.setPixel(2, 1, blue);
      i4.setPixel(0, 2, red);
      i4.setPixel(1, 2, blue);
      i4.setPixel(2, 2, red);
      nll::core::Image<nll::ui8> i5;
      i5.clone(i4);
      nll::core::rescale<nll::ui8,
                         nll::core::IndexMapperRowMajorFlat2DColorRGBn,
                         nll::core::InterpolatorLinear2D<nll::ui8, nll::core::Image<nll::ui8>::IndexMapper>
                        >(i4, 128, 128);
      nll::core::writeBmp(i4, "data/rescale-interp3.bmp");
      nll::core::rescale<nll::ui8,
                         nll::core::IndexMapperRowMajorFlat2DColorRGBn,
                         nll::core::InterpolatorNearestNeighbor2D<nll::ui8, nll::core::Image<nll::ui8>::IndexMapper>
                        >(i5, 128, 128);
      nll::core::writeBmp(i5, "data/rescale-interp4.bmp");

      nll::core::Image<nll::ui8> i6("data/image/test-image2.bmp");
      nll::core::Image<nll::ui8> i7("data/image/test-image2.bmp");
      for (int n = 0; n < 10; ++n)
      {
         nll::core::transformUnaryBilinear( i6, nll::core::TransformationRotation(-0.05f, nll::core::vector2f(0, 0)) );
         nll::core::transformUnaryNearestNeighbor( i7, nll::core::TransformationRotation(-0.05f, nll::core::vector2f(0, 0)) );
      }

      nll::core::writeBmp(i6, "data/rescale-interp5.bmp");
      nll::core::writeBmp(i7, "data/rescale-interp5-NN.bmp");

      t1.end();
      std::cout << "time=" << t1.getTime() << std::endl;
   }

   void testPSNR()
   {
      typedef nll::core::Image<nll::ui8> Image;

      Image img1("data/image/test-image2.bmp");
      Image img2;

      img2.clone(img1);
      nll::core::rescaleNearestNeighbor(img2, 32, 32);
      nll::core::rescaleNearestNeighbor(img2, img1.sizex(), img1.sizey());
      double psnr2 = nll::core::psnr(img1, img2);
      nll::core::writeBmp(img2, "data/reconstructed-iterp2.bmp");

      img2.clone(img1);
      nll::core::rescaleFast(img2, 32, 32);
      nll::core::rescaleFast(img2, img1.sizex(), img1.sizey());
      double psnr3 = nll::core::psnr(img1, img2);
      nll::core::writeBmp(img2, "data/reconstructed-fast3.bmp");

      img2.clone(img1);
      nll::core::rescaleBilinear(img2, 32, 32);
      nll::core::rescaleBilinear(img2, img1.sizex(), img1.sizey());
      double psnr1 = nll::core::psnr(img1, img2);
      nll::core::writeBmp(img2, "data/reconstructed-iterp1.bmp");

      TESTER_ASSERT( psnr1 > psnr2 );
      TESTER_ASSERT( psnr1 > psnr3 );
   }

   void testSVD()
   {
      typedef nll::core::Matrix<double> Matrixd;
      Matrixd m(2, 2), g, m_copy;
      m(0, 0) = 1;
      m(0, 1) = 1;
      m(1, 0) = 0;
      m(1, 1) = 1;
      m_copy.clone(m);
      nll::core::Buffer1D<double> x;
      
      bool result = nll::core::svdcmp(m, x, g);
      TESTER_ASSERT( result );

      Matrixd diag(m.sizex(), m.sizex());
      for (unsigned int n = 0; n < m.sizex(); ++n)
         diag(n, n) = x[n];
      nll::core::transpose(g);
      Matrixd tmp = m * diag;
      tmp = tmp * g;
      TESTER_ASSERT( tmp.equal(m_copy, 0.00001) );
   }

   class TestBase
   {
   };

   class TestTest : public TestBase
   {
   };

   void testTraitsInheritence()
   {
      bool val1 = (nll::core::IsDerivedFrom<TestTest, TestBase>::value);
      bool val2 = !(nll::core::IsDerivedFrom<TestBase, TestTest>::value);
      TESTER_ASSERT( val1 );
      TESTER_ASSERT( val2 );
   }

   void testCovariance()
   {
      double v1[] = {60, 61, 62, 63, 65};
      double v2[] = {3.1, 3.6, 3.8, 4, 4.1};
      nll::core::Matrix<double> m1( nll::core::Buffer1D<double>( v1, 5, false ) );
      nll::core::Matrix<double> m2( nll::core::Buffer1D<double>( v2, 5, false ) );

      double correlation = nll::core::correlation( m1, m2 );
      TESTER_ASSERT( nll::core::equal( correlation, 0.911872, 0.0001 ) );
   }

   void testDatabaseFilter()
   {
      typedef nll::core::Buffer1D<double> Input;
      typedef nll::algorithm::Classifier< Input >::Database Database;

      Database d;
      d.add( Database::Sample( nll::core::make_buffer1D<double>( 1 ), 0, Database::Sample::LEARNING ) );
      d.add( Database::Sample( nll::core::make_buffer1D<double>( 2 ), 0, Database::Sample::TESTING ) );
      d.add( Database::Sample( nll::core::make_buffer1D<double>( 5 ), 0, Database::Sample::TESTING ) );
      d.add( Database::Sample( nll::core::make_buffer1D<double>( 3 ), 0, Database::Sample::VALIDATION ) );
      d.add( Database::Sample( nll::core::make_buffer1D<double>( 4 ), 0, Database::Sample::LEARNING ) );

      Database dat_l = nll::core::filterDatabase( d, nll::core::make_vector<nll::ui32>( Database::Sample::LEARNING ), Database::Sample::TESTING );
      TESTER_ASSERT( dat_l.size() == 2 );
      TESTER_ASSERT( dat_l[ 0 ].input[ 0 ] == 1 );
      TESTER_ASSERT( dat_l[ 1 ].input[ 0 ] == 4 );
      TESTER_ASSERT( dat_l[ 0 ].type == Database::Sample::TESTING );
      TESTER_ASSERT( dat_l[ 1 ].type == Database::Sample::TESTING );

      TESTER_ASSERT( d[ 0 ].type == Database::Sample::LEARNING );
      TESTER_ASSERT( d[ 4 ].type == Database::Sample::LEARNING );

      Database dat_t = nll::core::filterDatabase( d, nll::core::make_vector<nll::ui32>( Database::Sample::TESTING, Database::Sample::VALIDATION ), Database::Sample::VALIDATION );
      TESTER_ASSERT( dat_t.size() == 3 );
      TESTER_ASSERT( dat_t[ 0 ].input[ 0 ] == 2 );
      TESTER_ASSERT( dat_t[ 1 ].input[ 0 ] == 5 );
      TESTER_ASSERT( dat_t[ 2 ].input[ 0 ] == 3 );

      TESTER_ASSERT( dat_t[ 0 ].type == Database::Sample::VALIDATION );
      TESTER_ASSERT( dat_t[ 1 ].type == Database::Sample::VALIDATION );
      TESTER_ASSERT( dat_t[ 2 ].type == Database::Sample::VALIDATION );
   }

   void testDatabaseFilterSelect()
   {
      typedef nll::core::Buffer1D<double> Input;
      typedef nll::algorithm::Classifier< Input >::Database Database;

      Database d;
      d.add( Database::Sample( nll::core::make_buffer1D<double>( 0, 1, 2, 3, 4 ),
                               0,
                               Database::Sample::LEARNING ) );
      d.add( Database::Sample( nll::core::make_buffer1D<double>( 5, 6, 7, 8, 9 ),
                               0,
                               Database::Sample::TESTING ) );
      d.add( Database::Sample( nll::core::make_buffer1D<double>( 10, 11, 12, 13, 14 ),
                               1,
                               Database::Sample::TESTING ) );
      Database d2 = nll::core::filterDatabase( d, nll::core::make_buffer1D<bool>( true,
                                                                                  false,
                                                                                  false,
                                                                                  true,
                                                                                  true ) );
      TESTER_ASSERT( d2.size() == 3 );
      TESTER_ASSERT( d2[ 0 ].input.size() == 3 );
      
      TESTER_ASSERT( d2[ 0 ].type == Database::Sample::LEARNING );
      TESTER_ASSERT( d2[ 1 ].type == Database::Sample::TESTING );
      TESTER_ASSERT( d2[ 2 ].type == Database::Sample::TESTING );

      TESTER_ASSERT( d2[ 0 ].output == 0 );
      TESTER_ASSERT( d2[ 1 ].output == 0 );
      TESTER_ASSERT( d2[ 2 ].output == 1 );

      TESTER_ASSERT( d2[ 0 ].input[ 0 ] == 0 );
      TESTER_ASSERT( d2[ 0 ].input[ 1 ] == 3 );
      TESTER_ASSERT( d2[ 0 ].input[ 2 ] == 4 );

      TESTER_ASSERT( d2[ 1 ].input[ 0 ] == 5 );
      TESTER_ASSERT( d2[ 1 ].input[ 1 ] == 8 );
      TESTER_ASSERT( d2[ 1 ].input[ 2 ] == 9 );

      TESTER_ASSERT( d2[ 2 ].input[ 0 ] == 10 );
      TESTER_ASSERT( d2[ 2 ].input[ 1 ] == 13 );
      TESTER_ASSERT( d2[ 2 ].input[ 2 ] == 14 );
   }

   void testSampling()
   {
      srand( 1 );
      nll::core::Buffer1D<double> p = nll::core::make_buffer1D<double>( 0.1, 0.25, 0.25, 0.4 );
      nll::core::Buffer1D<nll::ui32> indexes = nll::core::sampling( p, 10000 );

      std::vector<unsigned> nb( p.size() );
      for ( unsigned n = 0; n < indexes.size(); ++n )
         ++nb[ indexes[ n ] ];
      for ( unsigned n = 0; n < p.size(); ++ n )
         TESTER_ASSERT( fabs( p[ n ] * 10000 - nb[ n ] ) < 1000 );
   }

   void testDatabaseInputAdapter()
   {
      typedef nll::core::Buffer1D<double> Input;
      typedef nll::algorithm::Classifier< Input >::Database Database;

      Database d;
      d.add( Database::Sample( nll::core::make_buffer1D<double>( 0 ),
                               0,
                               Database::Sample::LEARNING ) );

      d.add( Database::Sample( nll::core::make_buffer1D<double>( 1 ),
                               0,
                               Database::Sample::TESTING ) );
      d.add( Database::Sample( nll::core::make_buffer1D<double>( 2 ),
                               0,
                               Database::Sample::TESTING ) );
      d.add( Database::Sample( nll::core::make_buffer1D<double>( 3 ),
                               0,
                               Database::Sample::VALIDATION ) );
      d.add( Database::Sample( nll::core::make_buffer1D<double>( 4 ),
                               0,
                               Database::Sample::LEARNING ) );

      nll::core::DatabaseInputAdapterType<Database> adapter( d, nll::core::make_vector<nll::ui32>( 0, 2 ) );
      TESTER_ASSERT( adapter.size() == 3 );
      TESTER_ASSERT( adapter[ 0 ][ 0 ] == 1 );
      TESTER_ASSERT( adapter[ 1 ][ 0 ] == 2 );
      TESTER_ASSERT( adapter[ 2 ][ 0 ] == 3 );
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestnllCore);
TESTER_TEST(testSampling);
TESTER_TEST(testTraitsInheritence);
TESTER_TEST(testBuffer1D);
TESTER_TEST(testMatrix);
TESTER_TEST(testDatabase);
TESTER_TEST(testDatabaseFilter);
TESTER_TEST(testDatabaseFilterSelect);
TESTER_TEST(testDatabaseInputAdapter);
TESTER_TEST(testClassifier);
TESTER_TEST(testKmeans);
TESTER_TEST(testStaticVector);
TESTER_TEST(testMath);
TESTER_TEST(testImage);
TESTER_TEST(testKdTree);
TESTER_TEST(testLogger);
TESTER_TEST(testGabor);
TESTER_TEST(testSequenceConverter);
TESTER_TEST(testImageBinaryOperation);
TESTER_TEST(testMatrixOperators);
TESTER_TEST(testInterpolatorPerf);
TESTER_TEST(testInterpolator);
TESTER_TEST(testPSNR);
TESTER_TEST(testSVD);
TESTER_TEST(testCovariance);
# ifndef DONT_RUN_SLOW_TEST
TESTER_TEST(testGmm);
# endif
TESTER_TEST_SUITE_END();
#endif

#pragma warning( pop )
