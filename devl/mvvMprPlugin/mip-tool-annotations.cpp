#include "mip-tool-annotations.h"
#include "mip.h"

namespace mvv
{
namespace platform
{
   void MipToolAnnotations::refreshConnectedMip()
   {
      for ( MipTool::LinkStorage::iterator it = MipTool::_links.begin(); it != MipTool::_links.end(); ++it )
      {
         (*it)->refreshTools();
      }
   }
}
}