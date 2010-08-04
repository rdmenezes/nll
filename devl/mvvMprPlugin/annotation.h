#ifndef MVV_PLATFORM_ANNOTATION_H_
# define MVV_PLATFORM_ANNOTATION_H_

# include "mvvMprPlugin.h"
# include "types.h"
# include <mvvPlatform/resource-typedef.h>

namespace mvv
{
namespace platform
{
   /**
    @brief Annotation class, to display annotation on a segment/mip
    */
   class MVVMPRPLUGIN_API Annotation
   {
   public:
      virtual void updateSegment( ResourceSliceuc segment ) = 0;
      virtual void setPosition( const nll::core::vector3f& pos ) = 0;
      virtual const nll::core::vector3f& getPosition() = 0;

      virtual ~Annotation()
      {}
   };

   typedef ResourceSet<RefcountedTyped<Annotation> >   ResourceAnnotations;
}
}

#endif