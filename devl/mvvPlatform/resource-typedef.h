#ifndef MVV_PLATFORM_RESOURCE_TYPEDEF_H_
# define MVV_PLATFORM_RESOURCE_TYPEDEF_H_

# include "resource-set.h"
# include "resource-map.h"
# include "resource-vector.h"
# include "transfer-function.h"

namespace mvv
{
namespace platform
{
   /// defines a set of order in no particular order.
   typedef ResourceSetRef<Order> ResourceOrders;

   /// defines a map of transfer function. Given a volume name, retrieve its associated transfer function
   typedef ResourceMap<SymbolVolume, RefcountedTyped< TransferFunction > > ResourceTransferFunction;

   /// defines a map of floats. Given a volume name, return the assiated floating value
   typedef ResourceMap<SymbolVolume, f32 > ResourceFloats;
}
}

#endif