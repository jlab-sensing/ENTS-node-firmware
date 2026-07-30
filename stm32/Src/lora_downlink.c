#include "lora_downlink.h"
#include "LmHandlerTypes.h"

static LmHandlerAppData_t currentAppData;
static char downlinkNewData = FALSE;


char downlink_IsnewDataReady(void)
{
    if(downlinkNewData)
    {
        downlinkNewData = FALSE;
        return true;
    }

    return FALSE;
}

LmHandlerAppData_t saveDownlinkData(LmHandlerAppData_t *appData)
{

    return currentAppData;
}
