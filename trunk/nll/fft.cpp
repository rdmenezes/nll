#include "nll.h"

#ifndef NLL_DONT_USE_LIBFFTW3

// bind to FFTW3 implementation
# include <fftw/fftw3.h>
#pragma comment(lib, "libfftw-3.3.lib")


namespace nll
{
namespace algorithm
{
   //
   // use the FFTW3 to compute the FFT
   //
   void Fft1D::forward( const core::Buffer1D<double>& real, ui32 nfft, core::Buffer1D<double>& outComplex )
   {
      nfft = real.size();  // TODO handle nfft correctly... no padding for now
      ensure( nfft >= real.size(), "nfft must be > real.size()" );

      double* in = reinterpret_cast<double*>( fftw_malloc( sizeof ( double ) * nfft ) );
      std::copy( real.begin(), real.end(), in );

      const ui32 nc = ( nfft / 2 ) + 1;
      fftw_complex* out = reinterpret_cast<fftw_complex*>( fftw_malloc( sizeof ( fftw_complex ) * nc ) );
      fftw_plan plan_forward = fftw_plan_dft_r2c_1d( nfft, in, out, FFTW_ESTIMATE );

      fftw_execute( plan_forward );
      outComplex = core::Buffer1D<double>( 2 * nc );
      for ( ui32 n = 0; n < nc; ++n )
      {
         const ui32 n2 = 2 * n;
         outComplex[ n2 + 0 ] = out[ n ][ 0 ];
         outComplex[ n2 + 1 ] = out[ n ][ 1 ];
      }

      fftw_destroy_plan( plan_forward );
      fftw_free( in );
      fftw_free( out );
   }

   void Fft1D::backward( const core::Buffer1D<double>& complex, ui32 nfft, core::Buffer1D<double>& outReal )
   {
      nfft = complex.size();  // TODO handle nfft correctly... no padding for now
      ensure( nfft >= complex.size(), "nfft must be > complex.size()" );

      const ui32 sizeOut = complex.size() - 2;
      const ui32 sizein = complex.size() / 2;

      fftw_complex* in = reinterpret_cast<fftw_complex*>( fftw_malloc( sizeof ( fftw_complex ) * sizein ) );
      for ( ui32 n = 0; n < sizein; ++n )
      {
         in[ n ][ 0 ] = complex[ n * 2 + 0 ];
         in[ n ][ 1 ] = complex[ n * 2 + 1 ];
      }

      double* out = reinterpret_cast<double*>( fftw_malloc( sizeof ( double ) * sizeOut ) );

      fftw_plan plan_backward = fftw_plan_dft_c2r_1d( sizeOut, in, out, FFTW_ESTIMATE );

      fftw_execute( plan_backward );
      outReal = core::Buffer1D<double>( sizeOut );
      for ( ui32 n = 0; n < sizeOut; ++n )
      {
         outReal[ n ] = out[ n ];
      }

      fftw_destroy_plan( plan_backward );
      fftw_free( in );
      fftw_free( out );
   }
}
}

#else

namespace nll
{
namespace algorithm
{
   //
   // Default FFT: no implementation
   //
   void Fft1D::forward( const core::Buffer1D<double>& real, ui32 nfft, core::Buffer1D<double>& )
   {
      throw std::runtime_error( "No FFT library binded" );
   }

   void Fft1D::backward( const core::Buffer1D<double>& real, ui32 nfft, core::Buffer1D<double>& )
   {
      throw std::runtime_error( "No FFT library binded" );
   }
}
}

#endif