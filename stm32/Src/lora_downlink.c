#include "lora_downlink.h"
#include "LmHandlerTypes.h"
#include "stdbool.h"

static LmHandlerAppData_t currentAppData;
static char downlinkNewData = false;

char downlinkIsNewDataReady(void)
{
    if(downlinkNewData)
    {
        downlinkNewData = false;
        return true;
    }

    return false;
}

LmHandlerAppData_t getDownlinkData(void)
{
    return currentAppData;
}

void saveNewDownlinkData(LmHandlerAppData_t *appData)
{
    downlinkNewData = true; // notifies downlinkIsNewDataReady of a new downlink
    currentAppData = *appData;
}