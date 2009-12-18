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

#if defined(_MSC_VER) && defined(_DEBUG)
#define new DEBUG_NEW
#endif

MVVPLATFORM_API int dummy = 0;