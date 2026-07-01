#include "port.h"
#include "mb.h"
#include "mbport.h"

static eMBEventType eQueuedEvent;
static BOOL     xEventInQueue;

BOOL
xMBPortEventInit(void)
{
    xEventInQueue = FALSE;
    return TRUE;
}

BOOL
xMBPortEventPost(eMBEventType eEvent)
{
    ENTER_CRITICAL_SECTION();
    xEventInQueue = TRUE;
    eQueuedEvent = eEvent;
    EXIT_CRITICAL_SECTION();
    return TRUE;
}

BOOL
xMBPortEventGet(eMBEventType *eEvent)
{
    BOOL xEventHappened = FALSE;

    ENTER_CRITICAL_SECTION();
    if (xEventInQueue) {
        *eEvent = eQueuedEvent;
        xEventInQueue = FALSE;
        xEventHappened = TRUE;
    }
    EXIT_CRITICAL_SECTION();
    return xEventHappened;
}
