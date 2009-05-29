#ifndef NLL_UNREACHABLE_H_
# define NLL_UNREACHABLE_H_

# include <iostream>

namespace nll
{
namespace core
{
   /// define an unreachable state
   #   define unreachable(_String)								            \
				std::cout << "------------" << std::endl;				      \
				std::cout << "Unreachable : " << (_String) << std::endl;	\
				std::cout << "  Location : " << __FILE__ << std::endl;   \
				std::cout << "  Line     : " << __LINE__ << std::endl;
}
}

#endif
