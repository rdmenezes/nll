#include "stdafx.h"
#include <nll/nll.h>

template <class I, class O>
class PreprocessingTestAdd : public nll::preprocessing::Preprocessing<I, O>
{
public:
   typedef typename nll::algorithm::Classifier<I>  IClassifier;
   typedef typename nll::algorithm::Classifier<O>  OClassifier;
   typedef typename nll::algorithm::Classifier<I>::Database InputDatabase;
   typedef typename nll::algorithm::Classifier<O>::Database OutputDatabase;

   PreprocessingTestAdd( bool* isOptimized = 0 ) : _isOptimized( isOptimized )
   {
   }
   virtual O process( const I& i ) const
   {
      return (O)(i + 1);
   }
   virtual OutputDatabase optimize( const InputDatabase& i, const OClassifier* classifier, const typename OClassifier::ClassifierParameters& parameters )
   {
      *_isOptimized = true;
      OutputDatabase odat = processDatabase( i );
      classifier->evaluate( parameters, odat );
      return odat;
   }
   virtual void write( std::ostream& ) const
   {
   }
   virtual void read( std::istream& )
   {
   }
private:
   bool* _isOptimized;
};

template <class I, class O>
class PreprocessingTestMul : public nll::preprocessing::Preprocessing<I, O>
{
public:
   typedef typename nll::algorithm::Classifier<I>  IClassifier;
   typedef typename nll::algorithm::Classifier<O>  OClassifier;
   typedef typename nll::algorithm::Classifier<I>::Database InputDatabase;
   typedef typename nll::algorithm::Classifier<O>::Database OutputDatabase;

   PreprocessingTestMul( bool* isOptimized = 0 ) : _isOptimized( isOptimized )
   {
   }
   virtual O process( const I& i ) const
   {
      return (O)(i * 2);
   }
   virtual OutputDatabase optimize( const InputDatabase& i, const OClassifier* , const typename OClassifier::ClassifierParameters& )
   {
      *_isOptimized = true;
      OutputDatabase odat = processDatabase( i );
      return odat;
   }
   virtual void write( std::ostream& ) const
   {
   }
   virtual void read( std::istream& )
   {
   }
private:
   bool* _isOptimized;
};

template <class I, class O>
class PreprocessingTestConvert : public nll::preprocessing::Preprocessing<I, O>
{
public:
   typedef typename nll::algorithm::Classifier<I>  IClassifier;
   typedef typename nll::algorithm::Classifier<O>  OClassifier;
   typedef typename nll::algorithm::Classifier<I>::Database InputDatabase;
   typedef typename nll::algorithm::Classifier<O>::Database OutputDatabase;

   PreprocessingTestConvert( bool* isOptimized = 0 ) : _isOptimized( isOptimized )
   {
   }
   virtual O process( const I& i ) const
   {
      O o( 1 );
      o[ 0 ] = static_cast<typename O::value_type>( i );
      return o;
   }
   virtual OutputDatabase optimize( const InputDatabase& i, const OClassifier* , const typename OClassifier::ClassifierParameters& )
   {
      *_isOptimized = true;
      OutputDatabase odat = processDatabase( i );
      return odat;
   }
   virtual void write( std::ostream& ) const
   {
   }
   virtual void read( std::istream& )
   {
   }
private:
   bool* _isOptimized;
};

class TestNllPreprocessing
{
public:
   //
   // create a set of Image, add an adaptor from Image -> Buffer1D and
   // select some of the features of the buffer, finally learn the database
   //
   void testNllPreprocessingBase()
   {
      typedef nll::algorithm::Classifier<char>       Classifier;
      typedef PreprocessingTestAdd<float, int>       Preprocessing1;
      typedef PreprocessingTestMul<int, char>        Preprocessing2;
      typedef PreprocessingTestAdd<char, char>       Preprocessing3;

      typedef TYPELIST_2(Preprocessing1, Preprocessing2, Classifier) PreprocessingPipeline2;

      
      Classifier* c = 0;
      Preprocessing1 p1;
      Preprocessing2 p2;
      Preprocessing3 p3;

      // test preprocessing size = 1
      typedef TYPELIST_1(Preprocessing2, Classifier) PreprocessingPipeline1;
      PreprocessingPipeline1 pipeline1 = nll::preprocessing::make_typelist(p2, c);

      char r1 = pipeline1.process( 3 );
      TESTER_ASSERT( r1 == 6 );

      // test preprocessing size = 2
      PreprocessingPipeline2 pipeline = nll::preprocessing::make_typelist(p1, p2, c);

      r1 = pipeline.process( 3.3f );
      TESTER_ASSERT( r1 == 8 );

      PreprocessingPipeline2::Database dat;
      dat.add( PreprocessingPipeline2::Database::Sample( 0, 0, PreprocessingPipeline2::Database::Sample::LEARNING ) );
      dat.add( PreprocessingPipeline2::Database::Sample( 4, 1, PreprocessingPipeline2::Database::Sample::LEARNING ) );
      dat.add( PreprocessingPipeline2::Database::Sample( 8, 0, PreprocessingPipeline2::Database::Sample::LEARNING ) );

      Classifier::Database pdat = pipeline.process( dat );
      TESTER_ASSERT( pdat.size() == 3 );
      TESTER_ASSERT( pdat[ 0 ].input == 2 );
      TESTER_ASSERT( pdat[ 1 ].input == 10 );
      TESTER_ASSERT( pdat[ 2 ].input == 18 );
      TESTER_ASSERT( pdat[ 0 ].output == 0 );
      TESTER_ASSERT( pdat[ 1 ].output == 1 );
      TESTER_ASSERT( pdat[ 2 ].output == 0 );

      // test preprocessing size = 3
      typedef TYPELIST_3(Preprocessing1, Preprocessing2, Preprocessing3, Classifier) PreprocessingPipeline3;
      PreprocessingPipeline3 pipeline3 = nll::preprocessing::make_typelist(p1, p2, p3, c);

      r1 = pipeline3.process( 3 );
      TESTER_ASSERT( r1 == 9 );

      // test preprocessing size = 4
      typedef TYPELIST_4(Preprocessing1, Preprocessing2, Preprocessing3, Preprocessing3, Classifier) PreprocessingPipeline4;
      PreprocessingPipeline4 pipeline4 = nll::preprocessing::make_typelist(p1, p2, p3, p3, c);

      r1 = pipeline4.process( 3 );
      TESTER_ASSERT( r1 == 10 );

      // TODO : FIXME
      // test preprocessing size = 5
      typedef TYPELIST_5(Preprocessing1, Preprocessing2, Preprocessing3, Preprocessing3, Preprocessing3, Classifier) PreprocessingPipeline5;
      PreprocessingPipeline5 pipeline5 = nll::preprocessing::make_typelist(p1, p2, p3, p3, p3, c);

      r1 = pipeline5.process( 3 );
      TESTER_ASSERT( r1 == 11 );

      
      // test preprocessing size = 6
      typedef TYPELIST_6(Preprocessing1, Preprocessing2, Preprocessing3, Preprocessing3, Preprocessing3, Preprocessing3, Classifier) PreprocessingPipeline6;
      PreprocessingPipeline6 pipeline6 = nll::preprocessing::make_typelist(p1, p2, p3, p3, p3, p3, c);

      r1 = pipeline6.process( 3 );
      TESTER_ASSERT( r1 == 12 );

      // test preprocessing size = 7
      typedef TYPELIST_7(Preprocessing1, Preprocessing2, Preprocessing3, Preprocessing3, Preprocessing3, Preprocessing3, Preprocessing3, Classifier) PreprocessingPipeline7;
      PreprocessingPipeline7 pipeline7 = nll::preprocessing::make_typelist(p1, p2, p3, p3, p3, p3, p3, c);

      r1 = pipeline7.process( 3 );
      TESTER_ASSERT( r1 == 13 );
      
      // test preprocessing size = 8
      typedef TYPELIST_8(Preprocessing1, Preprocessing2, Preprocessing3, Preprocessing3, Preprocessing3, Preprocessing3, Preprocessing3, Preprocessing3, Classifier) PreprocessingPipeline8;
      PreprocessingPipeline8 pipeline8 = nll::preprocessing::make_typelist(p1, p2, p3, p3, p3, p3, p3, p3, c);

      r1 = pipeline8.process( 3 );
      TESTER_ASSERT( r1 == 14 );
   }

   template <class T>
   class ClassifierTest : public nll::algorithm::Classifier<T>
   {
   public:
      typedef typename nll::algorithm::Classifier<T>  Classifier;
      typedef typename nll::algorithm::Classifier<T>::Database Database;

      static nll::algorithm::ParameterOptimizers buildParameters()
      {
         nll::algorithm::ParameterOptimizers parameters;
         return parameters;
      }

   public:
      ClassifierTest( const Database& refDat ) : Classifier( buildParameters() ), _refDat( refDat )
      {}
      virtual Classifier* deepCopy() const
      {
         return new ClassifierTest( _refDat );
      }

      virtual void read( std::istream& )
      {
      }

      virtual void write( std::ostream& ) const
      {
      }

      virtual typename Classifier::Class test( const typename Classifier::Point& ) const
      {
         return 0;
      }

      virtual void learn( const Database& dat, const nll::core::Buffer1D<nll::f64>& )
      {
         TESTER_ASSERT( dat == _refDat );
      }
      virtual double evaluate( const nll::core::Buffer1D<nll::f64>& params, const Database& dat) const
      {
         Classifier* c = deepCopy();
         c->learn( dat, params );
         delete c;
         return 0;
      }

   private:
      Database _refDat;
   };

   //
   // test if the database is correctly computed accross the typelist
   // test optimization dispatch
   //
   void testNllPreprocessingOptimizationScheme()
   {
      typedef nll::algorithm::Classifier<nll::core::Buffer1D<char> >       Classifier;
      typedef PreprocessingTestAdd<float, int>                             Preprocessing1;
      typedef PreprocessingTestMul<int, char>                              Preprocessing2;
      typedef PreprocessingTestConvert<char, nll::core::Buffer1D<char> >   Preprocessing3;

      typedef TYPELIST_3(Preprocessing1, Preprocessing2, Preprocessing3, Classifier) PreprocessingPipeline;

      PreprocessingPipeline::Database dat;
      dat.add(PreprocessingPipeline::Database::Sample( 0, 0, PreprocessingPipeline::Database::Sample::LEARNING ) );
      dat.add(PreprocessingPipeline::Database::Sample( 1, 0, PreprocessingPipeline::Database::Sample::LEARNING ) );
      dat.add(PreprocessingPipeline::Database::Sample( 2, 0, PreprocessingPipeline::Database::Sample::LEARNING ) );
      dat.add(PreprocessingPipeline::Database::Sample( 3, 1, PreprocessingPipeline::Database::Sample::LEARNING ) );
      dat.add(PreprocessingPipeline::Database::Sample( 3, 1, PreprocessingPipeline::Database::Sample::TESTING ) );

      Classifier::Database refDat;
      refDat.add(Classifier::Database::Sample( nll::core::make_buffer1D<char>( 2 ), 0, Classifier::Database::Sample::LEARNING ) );
      refDat.add(Classifier::Database::Sample( nll::core::make_buffer1D<char>( 4 ), 0, Classifier::Database::Sample::LEARNING ) );
      refDat.add(Classifier::Database::Sample( nll::core::make_buffer1D<char>( 6 ), 0, Classifier::Database::Sample::LEARNING ) );
      refDat.add(Classifier::Database::Sample( nll::core::make_buffer1D<char>( 8 ), 1, Classifier::Database::Sample::LEARNING ) );
      refDat.add(Classifier::Database::Sample( nll::core::make_buffer1D<char>( 8 ), 1, Classifier::Database::Sample::TESTING ) );

      Classifier* c = new ClassifierTest<nll::core::Buffer1D<char> >( refDat );
      Classifier::ClassifierParameters params;
      bool hasBeenOptimized[ 3 ] = {false, false, false};
      Preprocessing1 p1( hasBeenOptimized + 0 );
      Preprocessing2 p2( hasBeenOptimized + 1 );
      Preprocessing3 p3( hasBeenOptimized + 2 );


      nll::preprocessing::PreprocessingOptimizationScheme scheme;
      scheme.add( p1.getId() );
      scheme.add( p2.getId() );
      scheme.add( p3.getId() );

      PreprocessingPipeline pipeline = nll::preprocessing::make_typelist( p1, p2, p3, c );
      pipeline.optimize( dat, params, scheme );

      TESTER_ASSERT( hasBeenOptimized[ 0 ] );
      TESTER_ASSERT( hasBeenOptimized[ 1 ] );
      TESTER_ASSERT( hasBeenOptimized[ 2 ] );
   }

   void testNllPreprocessingRawNormalize()
   {
      typedef nll::core::Buffer1D<float>              Point;
      typedef nll::algorithm::ClassifierMlp<Point>    Classifier;

      typedef nll::preprocessing::PreprocessingRawNormalize<Point>      Preprocessing1;
      typedef TYPELIST_1(Preprocessing1, Classifier)                    PreprocessingPipeline;

      Preprocessing1 normalize;
      Classifier* classifier = new nll::algorithm::ClassifierMlp<Point>();
      PreprocessingPipeline pipeline( normalize, classifier );

      nll::preprocessing::PreprocessingOptimizationScheme scheme;
      scheme.add( normalize.getId() );

      Classifier::Database dat;
      for ( nll::ui32 n = 0; n < 10; ++n )
         dat.add( Classifier::Database::Sample( nll::core::make_buffer1D<float>( (float)n, (float)n - 1, (float)n * 3 ), 0, Classifier::Database::Sample::LEARNING ) );

      pipeline.optimize( dat, Classifier::ClassifierParameters(), scheme );
      Classifier::Database ndat = pipeline.process( dat );

      for ( nll::ui32 n = 0; n < dat[ 0 ].input.size(); ++n )
      {
         nll::core::Matrix<double> matPoint( ndat.size(), 1 );
         for ( nll::ui32 nn = 0; nn < ndat.size(); ++nn )
            matPoint( nn, 0 ) = ndat[ nn ].input[ n ];
         nll::core::Matrix<double> var;
         nll::core::Buffer1D<double> mean;
         var = nll::core::covariance( matPoint, 0, dat.size() - 1, &mean );

         TESTER_ASSERT( nll::core::equal<nll::f64>( mean( 0 ), 0 ) );
         TESTER_ASSERT( nll::core::equal<nll::f64>( var( 0, 0 ), 1, 0.001 ) );
      }
   }

   void testNllPreprocessingConcatenate()
   {
      typedef nll::algorithm::Classifier<nll::core::Buffer1D<char> >       Classifier;
      typedef PreprocessingTestAdd<float, int>                             Preprocessing1;
      typedef PreprocessingTestMul<int, char>                              Preprocessing2;
      typedef PreprocessingTestConvert<char, nll::core::Buffer1D<char> >   Preprocessing3;
      typedef TYPELIST_3(Preprocessing1, Preprocessing2, Preprocessing3, Classifier) PreprocessingPipeline;

      typedef PreprocessingTestAdd<float, float>                           Preprocessing0;


      Classifier::Database refDat;
      Classifier* c = new ClassifierTest<nll::core::Buffer1D<char> >( refDat );
      Classifier::ClassifierParameters params;
      Preprocessing1 p1;
      Preprocessing2 p2;
      Preprocessing3 p3;
      nll::preprocessing::PreprocessingOptimizationScheme scheme;


      PreprocessingPipeline pipeline = nll::preprocessing::make_typelist( p1, p2, p3, c );

      Preprocessing0 p0;
      TYPELIST_CONCATENATE_TYPE(Preprocessing0, PreprocessingPipeline) newPipeline = TYPELIST_CONCATENATE_VAL(Preprocessing0, PreprocessingPipeline, p0, pipeline);
      TESTER_ASSERT( newPipeline.process( 3 )[ 0 ] == 10 );
   }

   void testNllPreprocessingCenter()
   {
      const nll::ui8 grey[ 3 ] = { 150, 150, 150};
      typedef nll::core::Image<nll::ui8>                                                  Image;
      typedef nll::algorithm::Classifier<Image>                                           Classifier;
      typedef nll::preprocessing::PreprocessingImageCenter<nll::ui8, Image::IndexMapper>  PreprocessingUnit1;
      typedef TYPELIST_1(PreprocessingUnit1, Classifier) PreprocessingPipeline;

      nll::preprocessing::IsBackground<nll::ui8>   isBackground( Image::black(), 3 );
      PreprocessingUnit1 p1( isBackground, Image::black() );
      Classifier* c = new ClassifierTest<Image>( Classifier::Database() );

      PreprocessingPipeline pipeline = nll::preprocessing::make_typelist( p1, c );

      Image i1( 10, 10, 3 );
      i1.setPixel( 0, 0, Image::white() );
      i1.setPixel( 4, 0, grey );

      i1 = pipeline.process( i1 );
      nll::core::writeBmp( i1, NLL_TEST_PATH "data/test_s1.bmp");
      TESTER_ASSERT( i1( 4, 5, 0 ) == 255 );
      TESTER_ASSERT( i1( 4, 5, 1 ) == 255 );
      TESTER_ASSERT( i1( 4, 5, 2 ) == 255 );

      TESTER_ASSERT( i1( 8, 5, 0 ) == 150 );
      TESTER_ASSERT( i1( 8, 5, 1 ) == 150 );
      TESTER_ASSERT( i1( 8, 5, 2 ) == 150 );

      for ( int nx = 0; nx < 10; ++nx )
         for ( int ny = 0; ny < 10; ++ny )
            for ( int c = 0; c < 3; ++c )
               if ( !(nx == 4 && ny == 5 ) && !(nx == 8 && ny == 5 ) )
                  TESTER_ASSERT( i1( nx, ny, c ) == 0 );
   }

   void testNllPreprocessingBridgeImageVector()
   {
      typedef nll::core::Image<nll::ui8>           Image;
      typedef nll::core::Buffer1D<nll::ui8>        Vector;
      typedef nll::algorithm::Classifier<Vector>   Classifier;
      typedef nll::preprocessing::PreprocessingBridgeImageVector<nll::ui8, Image::IndexMapper, nll::ui8>  PreprocessingUnit1;
      typedef TYPELIST_1(PreprocessingUnit1, Classifier) PreprocessingPipeline;

      PreprocessingUnit1 p1;
      Classifier* c = new ClassifierTest<Vector>( Classifier::Database() );

      PreprocessingPipeline pipeline = nll::preprocessing::make_typelist( p1, c );
      Image i1(3, 2, 3);
      for ( nll::ui32 n = 0; n < i1.size(); ++n )
         i1[ n ] = static_cast<nll::ui8>( n );

      Vector res = pipeline.process( i1 );
      for ( nll::ui32 n = 0; n < 3 * 2 * 3; ++n )
         TESTER_ASSERT( res[ n ] == n );
   }

   void testNllPreprocessingImagePlace()
   {
      typedef nll::core::Image<nll::ui8>           Image;
      typedef nll::algorithm::Classifier<Image>   Classifier;
      typedef nll::preprocessing::PreprocessingImagePlace<nll::ui8, Image::IndexMapper>  PreprocessingUnit1;
      typedef TYPELIST_1(PreprocessingUnit1, Classifier) PreprocessingPipeline;

      PreprocessingUnit1 p1( 32, 32, Image::white() );
      Classifier* c = new ClassifierTest<Image>( Classifier::Database() );

      PreprocessingPipeline pipeline = nll::preprocessing::make_typelist( p1, c );
      Image i1(3, 3, 3);
      i1.setPixel( 1, 2, Image::red() );
      i1.setPixel( 2, 2, Image::white() );
      Image i2 = pipeline.process( i1 );
      TESTER_ASSERT( i2.sizex() == 32 && i2.sizey() == 32 && i2.getNbComponents() == 3 );
      TESTER_ASSERT( nll::core::isColorEqual( i2.point( 1, 2 ), Image::red(), 3 ) );
      TESTER_ASSERT( nll::core::isColorEqual( i2.point( 2, 2 ), Image::white(), 3 ) );
      TESTER_ASSERT( nll::core::isColorEqual( i2.point( 31, 31 ), Image::white(), 3 ) );
   }

   void testNllPreprocessingImageResample()
   {
      typedef nll::core::Image<nll::ui8>           Image;
      typedef nll::algorithm::Classifier<Image>    Classifier;
      typedef nll::preprocessing::PreprocessingImageResample<nll::ui8, Image::IndexMapper>  PreprocessingUnit1;
      typedef TYPELIST_1(PreprocessingUnit1, Classifier) PreprocessingPipeline;

      PreprocessingUnit1 p1(16, 16);
      Classifier* c = new ClassifierTest<Image>( Classifier::Database() );

      PreprocessingPipeline pipeline = nll::preprocessing::make_typelist( p1, c );
      Image i1( NLL_TEST_PATH "data/image/test-image1.bmp");

      Image res = pipeline.process( i1 );
      nll::core::rescaleFast( i1, 16, 16 );
      TESTER_ASSERT( i1 == res );

      nll::core::writeBmp( res, NLL_TEST_PATH "data/test-resample-pre.bmp" );
   }

   void testNllPreprocessingImageGabor()
   {
      typedef nll::core::Image<nll::ui8>           Image;
      typedef nll::algorithm::Classifier<Image>    Classifier;

      typedef nll::preprocessing::PreprocessingImageGabor<nll::ui8, Image::IndexMapper, nll::ui8>  PreprocessingUnit1;
      typedef TYPELIST_1(PreprocessingUnit1, Classifier) PreprocessingPipeline;

      nll::f32 kernelSize = 8.0f;
      nll::f32 frequency = 0.20f;
      PreprocessingUnit1::GaborFiltersBank  gaborsBank;
      gaborsBank.addFilter(nll::algorithm::GaborFilterDescriptor(frequency, 0, kernelSize));
      gaborsBank.addFilter(nll::algorithm::GaborFilterDescriptor(frequency, (nll::f32)nll::core::PI / 4, kernelSize));
      gaborsBank.addFilter(nll::algorithm::GaborFilterDescriptor(0.05f, (nll::f32)nll::core::PI / 8, 60.0f));

      PreprocessingUnit1 p1( 1000, gaborsBank, gaborsBank );
      Classifier* c = new ClassifierTest<Image>( Classifier::Database() );

      PreprocessingPipeline pipeline = nll::preprocessing::make_typelist( p1, c );
      Image i1( NLL_TEST_PATH "data/image/test-image2.bmp");
      nll::core::decolor( i1 );
      Image vec = pipeline.process( i1 );

      for ( nll::ui32 n = 0; n < vec.getNbComponents(); ++n )
      {
         Image ii = nll::core::extractChannel( vec, n );
         nll::core::extend( ii, 3 );
         nll::core::writeBmp( ii, NLL_TEST_PATH "data/gabor-" + nll::core::val2str( n ) + "-2.bmp" );
      }
   }
};

#ifndef DONT_RUN_TEST
TESTER_TEST_SUITE(TestNllPreprocessing);
TESTER_TEST(testNllPreprocessingBase);
TESTER_TEST(testNllPreprocessingOptimizationScheme);
TESTER_TEST(testNllPreprocessingRawNormalize);
TESTER_TEST(testNllPreprocessingConcatenate);
TESTER_TEST(testNllPreprocessingCenter);
TESTER_TEST(testNllPreprocessingBridgeImageVector);
TESTER_TEST(testNllPreprocessingImagePlace);
TESTER_TEST(testNllPreprocessingImageResample);
TESTER_TEST(testNllPreprocessingImageGabor);
TESTER_TEST_SUITE_END();
#endif
