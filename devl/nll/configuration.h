#ifndef NLL_CORE_CONFIGURATION_H_
# define NLL_CORE_CONFIGURATION_H_

# ifndef NLL_DISABLE_SSE_SUPPORT
#  include <xmmintrin.h>
#  include <emmintrin.h>
#endif

# define _CPU_FEATURE_MMX    0x0001
# define _CPU_FEATURE_SSE    0x0002
# define _CPU_FEATURE_SSE2   0x0004
# define _CPU_FEATURE_3DNOW  0x0008

namespace nll
{
namespace core
{
   namespace impl
   {
      inline bool _os_support( int feature )
      {
#if defined( _MSC_VER ) && !defined( NLL_DISABLE_SSE_SUPPORT )
          __try {
              switch ( feature ) {
              case _CPU_FEATURE_SSE:
                  __asm {
                      xorps xmm0, xmm0        // executing SSE instruction
                  }
                  break;
              case _CPU_FEATURE_SSE2:
                  __asm {
                      xorpd xmm0, xmm0        // executing SSE2 instruction
                  }
                  break;
              case _CPU_FEATURE_3DNOW:
                  __asm {
                      pfrcp mm0, mm0          // executing 3DNow! instruction
                      emms
                  }
                  break;
              case _CPU_FEATURE_MMX:
                  __asm {
                      pxor mm0, mm0           // executing MMX instruction
                      emms
                  }
                  break;
              }
          }
          __except(1)   // we don't want to include <windows.h> so we harcode the constant
          {
             return false;
          }
          return true;
# else
          return false;
# endif
      }
   }

   /**
    @ingroup core
    @brief Detect & publish the processor capabilities

    This class is used at runtime to launch optimized routines
    */
   class _Configuration
   {
   public:
      _Configuration()
      {
         // check if the processor capability
         _supportSSE   = impl::_os_support( _CPU_FEATURE_SSE );
         _supportSSE2  = impl::_os_support( _CPU_FEATURE_SSE2 );
         _supportMMX   = impl::_os_support( _CPU_FEATURE_MMX );
         _support3DNOW = impl::_os_support( _CPU_FEATURE_3DNOW );

      }

      bool isSupportedSSE() const
      {
         return _supportSSE;
      }

      bool isSupportedSSE2() const
      {
         return _supportSSE2;
      }

      bool isSupportedMMX() const
      {
         return _supportMMX;
      }

      bool isSupported3DNOW() const
      {
         return _support3DNOW;
      }

      void disableSSE()
      {
         _supportSSE = false;
      }

   private:
      bool  _supportSSE;
      bool  _supportSSE2;
      bool  _supportMMX;
      bool  _support3DNOW;
   };

   typedef Singleton<_Configuration>   Configuration;
}
}

#endif
