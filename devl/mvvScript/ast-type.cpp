# include "ast-files.h"

namespace mvv
{
namespace parser
{
   AstFunctionType::~AstFunctionType()
   {
      delete _returnType;
      delete _args;
   }
}
}