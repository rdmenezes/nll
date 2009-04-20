#ifndef STATIC_ASSERT_H_
# define STATIC_ASSERT_H_

namespace nll
{
namespace core
{
   template <bool x> struct STATIC_ASSERTION_FAILURE;
   template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
   template<int x> struct static_assert_test{};

   #define STATIC_ASSERT( B ) \
   typedef nll::core::static_assert_test<\
      sizeof(nll::core::STATIC_ASSERTION_FAILURE< (bool)( B ) >)\
      > static_assert_typedef_

/*
   template<bool> struct CompileTimeError;
   template<> struct CompileTimeError<true> {};

   #define STATIC_ASSERT(exp) (nll::core::CompileTimeError< (exp) >())
   */
}
}

#endif
