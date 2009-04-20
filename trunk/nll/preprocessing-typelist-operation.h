#ifndef NLL_PREPROCESSING_TYPELIST_OPERATIONS_H_
# define NLL_PREPROCESSING_TYPELIST_OPERATIONS_H_

# pragma warning( push )
# pragma warning( disable:4127 ) // conditional expression is constant

//
// Handle all operations required to be done at compile time by the typelist
//
namespace nll
{
namespace preprocessing
{
   //
   // forward declaration
   //
   template <class T1, class T2, class C>
   class Typelist;

   //
   // last type
   //
   template <class TypeList, class Type, bool last>
   struct _GetLastTypelistType
   {
      typedef typename _GetLastTypelistType
      <
         typename TypeList::T2,
         typename TypeList::T2::T2,
         nll::core::Equal<typename TypeList::T2::T2, NullTypelist<typename TypeList::Classifier> >::value
      >::type type;
   };

   template <class TypeList, class Type>
   struct _GetLastTypelistType<TypeList, Type, true>
   {
      typedef typename TypeList::T1   type;
   };

   template <class TypeList>
   struct GetLastTypelistType
   {
      typedef typename  _GetLastTypelistType
      <
         TypeList,
         typename TypeList::T2,
         nll::core::Equal
         <
            typename TypeList::T2,
            NullTypelist<typename TypeList::Classifier >
         >::value
      >::type type;
   };

   //
   // size
   //
   template <class TypeList, class Classifier>
   struct _GetSize
   {
      enum {value = 1 + _GetSize<typename TypeList::T2, Classifier>::value};
   };
   template <class Classifier>
   struct _GetSize<NullTypelist<Classifier>, Classifier>
   {
      enum {value = 0};
   };
   template <class TypeList>
   struct GetSize
   {
      enum{ value = _GetSize<TypeList, typename TypeList::Classifier>::value };
   };

   //
   // typelist accessor
   // TODO : automate with preprocessor metaprogramming
   //
   template <class TypeList, class I, class O, int level>
   struct ProcessFeature
   {
   };

   template <class TypeList, class I, class O>
   struct ProcessFeature<TypeList, I, O, 1>
   {
      static O process( const TypeList& typelist, const I& i )
      {
         return   typelist.getValue().process(i);
      }
   };

   template <class TypeList, class I, class O>
   struct ProcessFeature<TypeList, I, O, 2>
   {
      static O process( const TypeList& typelist, const I& i )
      {
         return   typelist.getNext().getValue().process(
                  typelist.getValue().process(i)
                  );
      }
   };

   template <class TypeList, class I, class O>
   struct ProcessFeature<TypeList, I, O, 3>
   {
      static O process( const TypeList& typelist, const I& i )
      {
         return   typelist.getNext().getNext().getValue().process(
                  typelist.getNext().getValue().process(
                  typelist.getValue().process(i)
                  )
                  );
      }
   };

   template <class TypeList, class I, class O>
   struct ProcessFeature<TypeList, I, O, 4>
   {
      static O process( const TypeList& typelist, const I& i )
      {
         return   typelist.getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getValue().process(
                  typelist.getNext().getValue().process(
                  typelist.getValue().process(i)
                  )
                  )
                  );
      }
   };

   template <class TypeList, class I, class O>
   struct ProcessFeature<TypeList, I, O, 5>
   {
      static O process( const TypeList& typelist, const I& i )
      {
         return   typelist.getNext().getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getValue().process(
                  typelist.getNext().getValue().process(
                  typelist.getValue().process(i)
                  )
                  )
                  )
                  );
      }
   };

   template <class TypeList, class I, class O>
   struct ProcessFeature<TypeList, I, O, 6>
   {
      static O process( const TypeList& typelist, const I& i )
      {
         return   typelist.getNext().getNext().getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getValue().process(
                  typelist.getNext().getValue().process(
                  typelist.getValue().process(i)
                  )
                  )
                  )
                  )
                  );
      }
   };

   template <class TypeList, class I, class O>
   struct ProcessFeature<TypeList, I, O, 7>
   {
      static O process( const TypeList& typelist, const I& i )
      {
         return   typelist.getNext().getNext().getNext().getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getValue().process(
                  typelist.getNext().getValue().process(
                  typelist.getValue().process(i)
                  )
                  )
                  )
                  )
                  )
                  );
      }
   };

   template <class TypeList, class I, class O>
   struct ProcessFeature<TypeList, I, O, 8>
   {
      static O process( const TypeList& typelist, const I& i )
      {
         return   typelist.getNext().getNext().getNext().getNext().getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getNext().getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getNext().getValue().process(
                  typelist.getNext().getNext().getValue().process(
                  typelist.getNext().getValue().process(
                  typelist.getValue().process(i)
                  )
                  )
                  )
                  )
                  )
                  )
                  );
      }
   };

   //
   // TODO : add more!!!
   // automate this!
   //
   #define TYPELIST_1(T1, C)              nll::preprocessing::Typelist<T1, nll::preprocessing::NullTypelist<C>, C>
   #define TYPELIST_2(T1, T2, C)          nll::preprocessing::Typelist<T1, TYPELIST_1(T2, C), C>
   #define TYPELIST_3(T1, T2, T3, C)      nll::preprocessing::Typelist<T1, TYPELIST_2(T2, T3, C), C>
   #define TYPELIST_4(T1, T2, T3, T4, C)  nll::preprocessing::Typelist<T1, TYPELIST_3(T2, T3, T4, C), C>

   #define TYPELIST_5(T1, T2, T3, T4, T5, C)                nll::preprocessing::Typelist<T1, TYPELIST_4(T2, T3, T4, T5, C), C>
   #define TYPELIST_6(T1, T2, T3, T4, T5, T6, C)            nll::preprocessing::Typelist<T1, TYPELIST_5(T2, T3, T4, T5, T6, C), C>
   #define TYPELIST_7(T1, T2, T3, T4, T5, T6, T7, C)        nll::preprocessing::Typelist<T1, TYPELIST_6(T2, T3, T4, T5, T6, T7, C), C>
   #define TYPELIST_8(T1, T2, T3, T4, T5, T6, T7, T8, C)    nll::preprocessing::Typelist<T1, TYPELIST_7(T2, T3, T4, T5, T6, T7, T8, C), C>

   template <class T1, class C>
   TYPELIST_1(T1, C) make_typelist( T1 t1, C* c )
   {
      STATIC_ASSERT( (core::Equal<typename C::Point, typename T1::OutputType>::value) );
      return TYPELIST_1(T1, C)(t1, NullTypelist<C>(c));
   }

   template <class T1, class T2, class C>
   TYPELIST_2(T1, T2, C) make_typelist( T1 t1, T2 t2, C* c )
   {
      STATIC_ASSERT( (core::Equal<typename T1::OutputType, typename T2::InputType>::value) );
      return TYPELIST_2(T1, T2, C)(t1, make_typelist(t2, c));
   }

   template <class T1, class T2, class T3, class C>
   TYPELIST_3(T1, T2, T3, C) make_typelist( T1 t1, T2 t2, T3 t3, C* c )
   {
      STATIC_ASSERT( (core::Equal<typename T1::OutputType, typename T2::InputType>::value) );
      return TYPELIST_3(T1, T2, T3, C)(t1, make_typelist(t2, t3, c));
   }

   template <class T1, class T2, class T3, class T4, class C>
   TYPELIST_4(T1, T2, T3, T4, C) make_typelist( T1 t1, T2 t2, T3 t3, T4 t4, C* c )
   {
      STATIC_ASSERT( (core::Equal<typename T1::OutputType, typename T2::InputType>::value) );
      return TYPELIST_4(T1, T2, T3, T4, C)(t1, make_typelist(t2, t3, t4, c));
   }

   template <class T1, class T2, class T3, class T4, class T5, class C>
   TYPELIST_5(T1, T2, T3, T4, T5, C) make_typelist( T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, C* c )
   {
      STATIC_ASSERT( (core::Equal<typename T1::OutputType, typename T2::InputType>::value) );
      return TYPELIST_5(T1, T2, T3, T4, T5, C)(t1, make_typelist(t2, t3, t4, t5, c));
   }

   template <class T1, class T2, class T3, class T4, class T5, class T6, class C>
   TYPELIST_6(T1, T2, T3, T4, T5, T6, C) make_typelist( T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, C* c )
   {
      STATIC_ASSERT( (core::Equal<typename T1::OutputType, typename T2::InputType>::value) );
      return TYPELIST_6(T1, T2, T3, T4, T5, T6, C)(t1, make_typelist(t2, t3, t4, t5, t6, c));
   }

   template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class C>
   TYPELIST_7(T1, T2, T3, T4, T5, T6, T7, C) make_typelist( T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, C* c )
   {
      STATIC_ASSERT( (core::Equal<typename T1::OutputType, typename T2::InputType>::value) );
      return TYPELIST_7(T1, T2, T3, T4, T5, T6, T7, C)(t1, make_typelist(t2, t3, t4, t5, t6, t7, c));
   }

   template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class C>
   TYPELIST_8(T1, T2, T3, T4, T5, T6, T7, T8, C) make_typelist( T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, C* c )
   {
      STATIC_ASSERT( (core::Equal<typename T1::OutputType, typename T2::InputType>::value) );
      return TYPELIST_8(T1, T2, T3, T4, T5, T6, T7, T8, C)(t1, make_typelist(t2, t3, t4, t5, t6, t7, t8, c));
   }
}
}

# pragma warning( pop )

#endif
