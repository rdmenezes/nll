#ifndef MVV_PLATFORM_RESOURCE_VECTOR_H_
# define MVV_PLATFORM_RESOURCE_VECTOR_H_

# include <nll/nll.h>
# include "resource.h"

namespace mvv
{
namespace platform
{
   /**
    @ingroup platform
    @brief Hold a vector, it is modified when a new value is set
    */
   class MVVPLATFORM_API ResourceVector3f : public Resource<nll::core::vector3f>
   {
      typedef Resource<nll::core::vector3f>  Base;

   public:
      typedef Base::value_type::value_type   value_type;

   public:
      ResourceVector3f() : Base( new Base::value_type(), true )
      {}

      ResourceVector3f( value_type v0,
                        value_type v1,
                        value_type v2 ) : Base( new Base::value_type( v0, v1, v2 ) )
      {}

      void setValue( ui32 n, value_type val )
      {
         if ( !nll::core::equal<value_type>( Base::getValue()[ n ], val, 1e-6 ) )
         {
            Base::getValue()[ n ] = val;
            notify();
         }
      }

      value_type getValue( ui32 n ) const
      {
         return Base::getValue()[ n ];
      }
   };

   /**
    @ingroup platform
    @brief Hold a vector, it is modified when a new value is set
    */
   class MVVPLATFORM_API ResourceVector2ui : public Resource<nll::core::vector2ui>
   {
      typedef Resource<nll::core::vector2ui>  Base;

   public:
      typedef Base::value_type::value_type   value_type;

   public:
      ResourceVector2ui() : Base( new Base::value_type(), true )
      {}

      ResourceVector2ui( value_type v0,
                        value_type v1 ) : Base( new Base::value_type( v0, v1 ) )
      {}

      void setValue( ui32 n, value_type val )
      {
         if ( val != Base::getValue()[ n ] )
         {
            Base::getValue()[ n ] = val;
            notify();
         }
      }

      value_type getValue( ui32 n ) const
      {
         return Base::getValue()[ n ];
      }
   };

   /**
    @ingroup platform
    @brief Hold a vector, it is modified when a new value is set
    */
   class MVVPLATFORM_API ResourceVector2f : public Resource<nll::core::vector2f>
   {
      typedef Resource<nll::core::vector2f>  Base;

   public:
      typedef Base::value_type::value_type   value_type;

   public:
      ResourceVector2f() : Base( new Base::value_type(), true )
      {}

      ResourceVector2f( value_type v0,
                        value_type v1 ) : Base( new Base::value_type( v0, v1 ) )
      {}

      void setValue( ui32 n, value_type val )
      {
         if ( !nll::core::equal<value_type>( Base::getValue()[ n ], val, 1e-6 ) )
         {
            Base::getValue()[ n ] = val;
            notify();
         }
      }

      value_type getValue( ui32 n ) const
      {
         return Base::getValue()[ n ];
      }
   };
}
}

#endif