// mvvPlatform.cpp : Defines the entry point for the DLL.
//

#include "notifiable.h"
#include "mvvPlatform.h"
#include "symbol.h"
#include "refcounted.h"
#include "context.h"
#include "order.h"
#include "order-provider.h"
#include "order-consumer.h"
#include "order-dispatcher.h"
#include "order-creator.h"
#include "engine-order.h"
#include "event-mouse.h"
#include "event-mouse-receiver.h"
#include "order-dispatcher-impl.h"
#include "order-provider-impl.h"
#include "layout-widget.h"
#include "context-volumes.h"
#include "engine-order-volume-loading.h"
#include "resource-vector.h"
#include "resource-set.h"
#include "resource-volumes.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

MVVPLATFORM_API int dummy = 0;