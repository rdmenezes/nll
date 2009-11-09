#include "stdafx.h"
#include "nll.h"

# ifndef NLL_DISABLE_SSE_SUPPORT
#include <intrin.h>
#endif

namespace nll
{
namespace core
{
   namespace impl
   {
      enum CPUFeatures
      {
         FPU       = 0x00000001,
         VME       = 0x00000002,
         DE        = 0x00000004,
         PSE       = 0x00000008,
         TSC       = 0x00000010,
         MSR       = 0x00000020,
         PAE       = 0x00000040,
         MCE       = 0x00000080,
         CX8       = 0x00000100,
         APIC      = 0x00000200,

         SEP       = 0x00000800,
         MTRR      = 0x00001000,
         PGE       = 0x00002000,
         MCA       = 0x00004000,
         CMOV      = 0x00008000,
         PAT       = 0x00010000,
         PSE36     = 0x00020000,
         PSN       = 0x00040000,
         CLFL      = 0x00080000,

         DTES      = 0x00200000,
         ACPI      = 0x00400000,
         MMX       = 0x00800000,
         FXSR      = 0x01000000,
         SSE       = 0x02000000,
         SSE2      = 0x04000000,
         SS        = 0x08000000,
         HTT       = 0x10000000,
         TM        = 0x20000000,
         IA64      = 0x40000000,
         PBE       = 0x80000000
      };
      enum CPUFeatures2
      {
         SSE3      = 0x00000001,
         MONITOR   = 0x00000008,
         DSCPL     = 0x00000010
      };

# if !defined ( NLL_DISABLE_SSE_SUPPORT )
      static inline unsigned long getCPUFeatures( int i )
      {
         static int CPUInfo[ 4 ];
         static bool b = false;
         if ( !b )
         {
            __cpuid( CPUInfo, 1 );
            b = true;
         }
         return static_cast< unsigned long >( CPUInfo[ i ] );
      }

      static const unsigned long features = getCPUFeatures( 3 );
      static const unsigned long features2 = getCPUFeatures( 2 );
# else
      /**
       Disable all feature detection
       */
      static const unsigned long features = 0;
      static const unsigned long features2 = 0;

#endif

      _Configuration::_Configuration()
      {
         _supportMMX = ( features & MMX ) != 0;
         _supportSSE = ( features & SSE ) != 0;
         _supportSSE2 = ( features & SSE2 ) != 0;
         _supportSSE3 = ( features2 & SSE3 ) != 0;
      }

   }
}
}