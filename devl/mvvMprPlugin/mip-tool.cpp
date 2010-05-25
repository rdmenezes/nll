#include "mip-tool.h"
#include "mip.h"

namespace mvv
{
namespace platform
{
   MipTool::~MipTool()
   {
      removeConnections();
   }

   void MipTool::connect( Mip* mip )
   {
      _addSimpleLink( mip );

      Mip::ToolLinks* v = mip;
      v->_addSimpleLink( this );
      
   }

   void MipTool::disconnect( Mip* mip )
   {
      _eraseSimpleLink( mip );

      Mip::ToolLinks* v = mip;
      v->_eraseSimpleLink( this );
   }
}
}