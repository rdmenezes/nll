#include "ast-decl-var.h"

namespace mvv
{
namespace parser
{
   AstDeclVar::~AstDeclVar()
   {
      delete _type;
      delete _init;
      delete _declarationList;
      delete _objectInit;
   }
}
}