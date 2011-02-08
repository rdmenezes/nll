#include "stdafx.h"
#include <tester/register.h>
#include <nll/nll.h>
#include "database-benchmark.h"
#include "utils.h"

namespace nll
{
namespace tutorial
{
   struct TestKernelPcaUsps
   { 
      typedef nll::benchmark::BenchmarkDatabases::Database Database;
         typedef nll::benchmark::BenchmarkDatabases::Database::Sample::Input  Input;

      static core::Image<ui8> getImage( const Input& i )
      {
         core::Image<ui8> im( 16, 16, 1 );
         for ( ui32 y = 0; y < 16; ++y )
         {
            for ( ui32 x = 0; x < 16; ++x )
            {
               im( x, y, 0 ) = static_cast<ui8>( NLL_BOUND( i[ x + y * 16 ] * 127.5, 0.0, 255.0) ); // it is scaled 0-2
            }
         }
         core::extend( im, 3 );
         return im;
      }


      // demonstrate denoising capabilities on the USPS dataset
      void test1()
      {
         typedef nll::algorithm::Classifier<Input>                            Classifier;

         // find the correct benchmark
         const nll::benchmark::BenchmarkDatabases::Benchmark* benchmark = nll::benchmark::BenchmarkDatabases::instance().find( "usps" );
         ensure( benchmark, "can't find benchmark" );
         Classifier::Database dat = benchmark->database;
         std::cout << "size=" << dat.size() << std::endl;

         // generate a random index
         const ui32 nbLearning = 4000;
         std::vector<ui32> index = core::generateUniqueList( 0, nbLearning );

         Classifier::Database datSmall;
         for ( ui32 n = 0; n < nbLearning; ++n )
         {
            datSmall.add( dat[ index[ n ] ] );
         }

         typedef nll::core::DatabaseInputAdapterRead<Database> Adapter;

         Adapter adapter( datSmall );
         typedef nll::algorithm::KernelRbf<Input>  Kernel;
         typedef nll::algorithm::KernelPca<Input, Kernel> KernelPca;

         KernelPca kpca;
         Kernel kernel( 500 );
         kpca.compute( adapter, 100, kernel );

         typedef algorithm::KernelRbf<Input> Kernel;
         typedef algorithm::KernelPreImageMDS<Input, Kernel> PreImageGenerator;

         PreImageGenerator preimage( kpca );

         double meanPsnrNoisy = 0;
         double meanPsnrDenoised = 0;

         const ui32 nbTest = 700;
         for ( ui32 n = 0; n < nbTest; ++n )
         {
            const ui32 index = nbLearning + n;

            Input orig;
            orig.clone( dat[ index ].input );

            core::awgn( dat[ index ].input, 0.7 );
            core::writeBmp( getImage( dat[ index ].input ), "c:/tmp3/" + core::val2str( n ) + ".bmp" );

            const double psnrNoisy = core::psnr( getImage( orig ), getImage( dat[ index ].input ) );
            meanPsnrNoisy += psnrNoisy;
            std::cout << "psnr noisy=" << psnrNoisy << std::endl;

            Input feature = kpca.transform( dat[ index ].input );
            Input denoised = preimage.preimage( feature, 3 );
            core::writeBmp( getImage( denoised ), "c:/tmp3/" + core::val2str( n ) + "-d.bmp" );

            const double psnrDenoised = core::psnr( getImage( orig ), getImage( denoised ) );
            meanPsnrDenoised += psnrDenoised;
            std::cout << "psnr denoised=" << psnrDenoised << std::endl;
         }

         meanPsnrDenoised /= nbTest;
         meanPsnrNoisy /= nbTest;

         std::cout << "psnrNoisy=" <<  meanPsnrNoisy << std::endl;
         std::cout << "psnrDenoised=" << meanPsnrDenoised << std::endl;
         Tester_ASSERT( meanPsnrNoisy < meanPsnrDenoised );
      }
   };

   TESTER_TEST_SUITE( TestKernelPcaUsps );
    TESTER_TEST( test1 );
   TESTER_TEST_SUITE_END();
}
}