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
    @brief Hold a list of values
    */
   template <class T>
   class ResourceVector : public Resource< std::vector<T> >
   {
      typedef Resource< std::vector<T> >  Base;

   public:
      typedef typename Base::value_type::value_type   value_type;

   public:
      ResourceVector() : Base( new Base::value_type(), true )
      {}

      ResourceVector( ui32 n ) : Base( new Base::value_type( n ), true )
      {}

      void push_back( T val )
      {
         Base::getValue().push_back( val );
         notify();
      }

      T operator[]( ui32 n ) const
      {
         return Base::getValue()[ n ];
      }

      void setValue( T val, ui32 index )
      {
         if ( val != Base::getValue()[ index ] )
         {
            Base::getValue()[ index ] = val;
            notify();
         }
      }

      typename std::vector<T>::const_iterator begin() const
      {
         return Base::getValue().begin();
      }

      typename std::vector<T>::const_iterator end() const
      {
         return Base::getValue().end();
      }

      ui32 size() const
      {
         return static_cast<ui32>( Base::getValue().size() );
      }
   };

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

      void setValue( const Base::value_type& val )
      {
         if ( val != Base::getValue() )
         {
            Base::getValue() = val;
            notify();
         }
      }

      void setValue( ui32 n, value_type val )
      {
         if ( !nll::core::equal<value_type>( Base::getValue()[ n ], val, 1e-6f ) )
         {
            Base::getValue()[ n ] = val;
            notify();
         }
      }

      value_type getValue( ui32 n ) const
      {
         return Base::getValue()[ n ];
      }

      Base::value_type getValue() const
      {
         return Base::getValue();
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

      void setValue( const Base::value_type& val )
      {
         if ( val != Base::getValue() )
         {
            Base::getValue() = val;
            notify();
         }
      }

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

      Base::value_type getValue() const
      {
         return Base::getValue();
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

      void setValue( const Base::value_type& val )
      {
         if ( val != Base::getValue() )
         {
            Base::getValue() = val;
            notify();
         }
      }

      void setValue( ui32 n, value_type val )
      {
         if ( !nll::core::equal<value_type>( Base::getValue()[ n ], val, 1e-6f ) )
         {
            Base::getValue()[ n ] = val;
            notify();
         }
      }

      value_type getValue( ui32 n ) const
      {
         return Base::getValue()[ n ];
      }

      Base::value_type getValue() const
      {
         return Base::getValue();
      }
   };
}
}

#endif