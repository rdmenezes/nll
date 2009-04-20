#ifndef NLL_TYPE_TRAITS_INHERITED_H_
# define NLL_TYPE_TRAITS_INHERITED_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief Test at compile time if a class is derived vrom another one.

    if value == 1 then the class D is derived from B
   */
   template<class D, class B>
   class IsDerivedFrom
   {
   private:
     class Yes { char a[1]; };
     class No { char a[10]; };

     static Yes Test( B* ); // undefined
     static No Test( ... ); // undefined

   public:
     enum {
            value = sizeof(Test(static_cast<D*>(0))) == sizeof(Yes) ? 1 : 0 ///< if value == 1 then the class D is derived from B
          };
   };

   template<class T>
   class IsDerivedFrom<T,T>
   {
   public:
     enum { value = 0 };
   };
}
}

#endif
