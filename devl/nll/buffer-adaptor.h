#ifndef NLL_BUFFER_ADAPTOR_H_
# define NLL_BUFFER_ADAPTOR_H_

#pragma warning( push )
#pragma warning( disable:4512 ) // couldn't generate assignment operator

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Defines an adaptor from an array of array to an array
    @todo C++ 0x use Concepts. <code>Array</code> must define operator[], size(), typedef value_type
    
    value_type must also be itself an array type
    */
   template <class Array>
   class BufferSelectionAdaptor
   {
   public:
      typedef typename Array::value_type::value_type  value_type;

   public:
      /**
       @param a the array to be wrapped. Internally a reference to the original array is taken
       @param index the index to wrap as a new array
       */
      BufferSelectionAdaptor( Array& a, ui32 index ) : _array( a ), _index( index )
      {}

      inline ui32 size() const
      {
         return 
      }

      typename value_type& operator[]( ui32 i )
      {
         return _array[ _index ][ i ];
      }

      const value_type& operator[]( ui32 i ) const
      {
         return _array[ _index ][ i ];
      }

   private:
      ui32     _index;
      Array&   _array;
   };


   /**
    @ingroup core
    @brief Defines an adaptor from an array of array to an array using a const array only
    @todo C++ 0x use Concepts. <code>Array</code> must define operator[], size(), typedef value_type
    
    value_type must also be itself an array type
    */
   template <class Array>
   class BufferSelectionConstAdaptor
   {
   public:
      typedef typename Array::value_type::value_type  value_type;

   public:
      /**
       @param a the array to be wrapped. Internally a reference to the original array is taken
       @param index the index to wrap as a new array
       */
      BufferSelectionConstAdaptor( const Array& a, ui32 index ) : _array( a ), _index( index )
      {}

      inline ui32 size() const
      {
         return 
      }

      const value_type& operator[]( ui32 i ) const
      {
         return _array[ _index ][ i ];
      }

   private:
      ui32            _index;
      const Array&    _array;
   };
}
}

#pragma warning( pop )

#endif