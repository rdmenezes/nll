#ifndef NLL_CORE_CONFIGURATION_H_
# define NLL_CORE_CONFIGURATION_H_

# ifndef NLL_DISABLE_SSE_SUPPORT
#  include <xmmintrin.h>
#  include <emmintrin.h>
#endif

namespace nll
{
namespace core
{
   namespace impl
   {
      /**
       @ingroup core
       @brief Detect & publish the processor capabilities

       This class is used at runtime to launch optimized routines
       */
      class NLL_API _Configuration
      {
      public:
         // detect CPU features at initialisation
         _Configuration();
         
         bool isSupportedSSE() const
         {
            return _supportSSE;
         }

         bool isSupportedSSE2() const
         {
            return _supportSSE2;
         }

         bool isSupportedSSE3() const
         {
            return _supportSSE3;
         }

         bool isSupportedMMX() const
         {
            return _supportMMX;
         }

         void disableSSE()
         {
            _supportSSE = false;
            _supportSSE2 = false;
            _supportSSE3 = false;
         }

      private:
         bool  _supportSSE;
         bool  _supportSSE2;
         bool  _supportSSE3;
         bool  _supportMMX;
      };
   }

   typedef Singleton<impl::_Configuration>   Configuration;
}
}

#endif
