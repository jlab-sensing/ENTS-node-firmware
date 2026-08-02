#include "lora_downlink.h"
#include "LmHandlerTypes.h"

static LmHandlerAppData_t currentAppData;
static char downlinkNewData = FALSE;

char downlinkIsNewDataReady(void)
{
    if(downlinkNewData)
    {
        downlinkNewData = FALSE;
        return true;
    }

    return FALSE;
}

LmHandlerAppData_t getDownlinkData(void)
{
    return currentAppData;
}

void saveNewDownlinkData(LmHandlerAppData_t *appData)
{
    downlinkNewData = TRUE; // notifies downlinkIsNewDataReady of a new downlink
    currentAppData = *appData;
}