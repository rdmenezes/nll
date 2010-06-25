#ifndef READ_RESULT_H_
# define READ_RESULT_H_

# include <vector>
# include <map>
# include <fstream>
# include "globals.h"
# include "features.h"

namespace nll
{
namespace detect
{
   class CorrectPosition
   {
   public:
      // the learning data file to use to detect and correct outliers
      CorrectPosition( const std::string& measures )
      {
      }
   };
}
}

#endif