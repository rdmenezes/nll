#ifndef NLL_LOG_H_
# define NLL_LOG_H_

# define NLL_CAUTION_PRINT

namespace nll
{
namespace core
{

# ifdef NLL_CAUTION_PRINT
#  define caution(_Expression, _String)                        \
   if (!(_Expression))											         \
   {                                                           \
      std::cout << "------------" << std::endl;                \
      std::cout << "Warning : " << _String << std::endl;       \
      std::cout << "  Location : " << __FILE__ << std::endl;   \
      std::cout << "  Line     : " << __LINE__ << std::endl;   \
   }
# else
#  define caution(_Expression, _String)   (void*)0
# endif

}
}

#endif
