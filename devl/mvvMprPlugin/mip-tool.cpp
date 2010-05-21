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
      // TODO up!
      /*
      _addSimpleLink( mip );

      Mip::Linkable* v = mip;
      v->_addSimpleLink( this );
      */
   }

   void MipTool::disconnect( Mip* mip )
   {
      // TODO up!
      /*
      _eraseSimpleLink( mip );

      Mip::Linkable* v = mip;
      v->_eraseSimpleLink( this );
      */
   }
}
}