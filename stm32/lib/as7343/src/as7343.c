/**
 ******************************************************************************
 * @file     as7343.c
 * @author   Eric Tran
 * @brief    Sensor driver for the as7343 sensor.
 *
 *           This library is designed to read measurements from a Multi-Spectral Sensor
 *           https://github.com/sparkfun/SparkFun_AS7343_Arduino_Library/blob/main/examples/Example_01_BasicReadings/Example_01_BasicReadings.ino
 * @date     7/3/2026
 ******************************************************************************
 */

#include "as7343.h"
#include "i2c.h"
#include "sys_app.h"

// user includes
#include "transcoder.h"
#include "userConfig.h"
#include "sensor.h"
#include "sensors.h"

static volatile AS7343Data sensorReadings = {};

// private functions
unsigned char I2C_ReadRegister(unsigned char I2CAddress, unsigned char deviceRegisterAddress);
unsigned char I2C_WriteReg(unsigned char I2CAddress, unsigned char deviceRegisterAddress, uint8_t data);

///////////////////////////////////////////////////////////////////////////////////



int8_t AS7343Init(void)
{
    // HAL_StatusTypeDef ret;
    APP_LOG(TS_OFF, VLEVEL_M, "Starting Init\r\n");
    HAL_Delay(1); // wait atleast 200uS on initial power up

    // immediately resets the sensor 
    sfe_as7343_reg_control_t controlReg = {0};
    controlReg.sw_reset = 1;
    
    APP_LOG(TS_OFF, VLEVEL_M, "Reseting Sensor\r\n");
    if(I2C_WriteReg(kAS7343Addr, ksfAS7343RegControl, controlReg.byte) == ERROR)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "Error occured during reset\r\n");
        return ERROR;
    }

    HAL_Delay(1);
    controlReg.sw_reset = 0;
    
    APP_LOG(TS_OFF, VLEVEL_M, "Clearing Reset Bit\r\n");
    if(I2C_WriteReg(kAS7343Addr, ksfAS7343RegControl, controlReg.byte) == ERROR)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "Error occured during reset\r\n");
        return ERROR;
    }

    // enables the sensor, waking it up from sleep
    sfe_as7343_reg_enable_t enableReg = {0};
    enableReg.pon = 1;

    APP_LOG(TS_OFF, VLEVEL_M, "Enabling Sensor\r\n");
    if (I2C_WriteReg(kAS7343Addr, ksfAS7343RegEnable, enableReg.byte) == ERROR)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "Error occured during power up\r\n");
        return ERROR;
    }

    // configuring the sensor
    sfe_as7343_reg_cfg20_t autoSmux = {0};
    autoSmux.auto_smux = 3; // setting auto_smux to oversee 18 channels
    
    APP_LOG(TS_OFF, VLEVEL_M, "Enabling Audo Smux\r\n");
    if(I2C_WriteReg(kAS7343Addr, ksfAS7343RegCfg20, autoSmux.byte) == ERROR)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "Error occured during Configuring auto smux\r\n");
        return ERROR;
    }

    sfe_as7343_reg_astep_t aStep = {0};
    aStep.astep_l = 0x57;
    aStep.astep_h = 0x02;

    I2C_WriteReg(kAS7343Addr, ksfAS7343RegAStep, aStep.word); // integration time per step with 2.78(n+1)uS per count
    I2C_WriteReg(kAS7343Addr, ksfAS7343RegATime, 0x0A); // total steps in integration

    I2C_WriteReg(kAS7343Addr, ksfAS7343RegWTime, 0x5E);
    
    // Verifying i2c connection to sensor
    sfe_as7343_reg_cfg0_t regBank = {0};
    regBank.reg_bank = 1; // setting reg_bank bit so registers 0x20 to 0x7F are accessible

    if(I2C_WriteReg(kAS7343Addr, ksfAS7343RegCfg0, regBank.byte) == ERROR)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "Error occured during register bank change\r\n");
        return ERROR;
    }


    APP_LOG(TS_OFF, VLEVEL_M, "Verifying I2C connection\r\n");
    if(I2C_ReadRegister(kAS7343Addr, ksfAS7343RegID) != kDefaultAS7343DeviceID)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "Unexpected device ID %x \n", I2C_ReadRegister(kAS7343Addr, ksfAS7343RegID));

        // Clearing regBank bit to allow for normal operations
        regBank.reg_bank = 0; // clearing reg_bank bit so registers 0x80 and above are accessible

        if(I2C_WriteReg(kAS7343Addr, ksfAS7343RegCfg0, regBank.byte) == ERROR)
        {
            APP_LOG(TS_OFF, VLEVEL_M, "Error occured clearing register bank bit\r\n");
            return ERROR;
        }

        return ERROR;
    }
    
    APP_LOG(TS_OFF, VLEVEL_M, "Connection Verified. Device ID: %x\r\n", I2C_ReadRegister(kAS7343Addr, ksfAS7343RegID));

    // Clearing regBank bit to allow for normal operations
    regBank.reg_bank = 0; // clearing reg_bank bit so registers 0x80 and above are accessible

    if(I2C_WriteReg(kAS7343Addr, ksfAS7343RegCfg0, regBank.byte) == ERROR)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "Error occured clearing register bank bit\r\n");
        return ERROR;
    }

    AS7343Active();

    return SUCCESS;
}

void AS7343GetMeasurement(AS7343Data *channelData)
{
    for(int i = 0; i < ksfAS7343NumChannels; i++)
    {
        channelData->channelLow[i] = I2C_ReadRegister(kAS7343Addr, ksfAS7343RegData0 + (2*i)); // gets the low byte of the channel
        channelData->channelHigh[i] = I2C_ReadRegister(kAS7343Addr, ksfAS7343RegData0 + ((2*i) + 1)); // gets the high byte of the channel
        channelData->channelCombined[i] = (channelData->channelLow[i] | ((channelData->channelHigh[i]) << 8 ));
    }      
}

void AS7343Active(void)
{
    sfe_as7343_reg_enable_t enableReg = {0};
    enableReg.sp_en = 1;

    unsigned char currentConfiguration = I2C_ReadRegister(kAS7343Addr, ksfAS7343RegEnable); // stores current byte data
    enableReg.byte = enableReg.byte | currentConfiguration; // ensures that current byte data stays intact
    
    APP_LOG(TS_OFF, VLEVEL_M, "Activating AS7343r\n");
    if(I2C_WriteReg(kAS7343Addr, ksfAS7343RegEnable, enableReg.byte) == ERROR)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "Error occured during setting sp_en\n");
        return;
    }
    
    APP_LOG(TS_OFF, VLEVEL_M, "Finished Activation");
}


void AS7343Idle(void)
{
    sfe_as7343_reg_enable_t enableReg = {0};
    enableReg.sp_en = 0;
    
    unsigned char currentConfiguration = I2C_ReadRegister(kAS7343Addr, ksfAS7343RegEnable); // stores current byte data
    enableReg.byte = enableReg.byte | currentConfiguration; // ensures that current byte data stays intact
    
    APP_LOG(TS_OFF, VLEVEL_M, "Setting AS7343r to idle\n");
    if(I2C_WriteReg(kAS7343Addr, ksfAS7343RegEnable, enableReg.byte) == ERROR)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "Error occured during clearing sp_en\n");
        return;
    }

    APP_LOG(TS_OFF, VLEVEL_M, "Finished state transition\n");
}

void AS7343LEDOn(void)
{
    sfe_as7343_reg_led_t LED = {0};
    LED.led_act = 1;

    APP_LOG(TS_OFF, VLEVEL_M, "Turning on LED\n");
    if(I2C_WriteReg(kAS7343Addr, ksfAS7343RegLed, LED.byte) == ERROR)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "Error occured during Enabling LED\r\n");
        return;
    }
    
    APP_LOG(TS_OFF, VLEVEL_M, "LED ON\n");
}

void AS7343LEDOff(void)
{
    sfe_as7343_reg_led_t LED = {0};
    LED.led_act = 0;

    APP_LOG(TS_OFF, VLEVEL_M, "Turning off LED\n");
    if(I2C_WriteReg(kAS7343Addr, ksfAS7343RegLed, LED.byte) == ERROR)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "Error occured during Enabling LED\r\n");
        return;
    }
    
    APP_LOG(TS_OFF, VLEVEL_M, "LED off\n");
}

size_t AS7343Measure(uint8_t *data, SysTime_t ts, uint32_t idx) {
  // read sensor
  AS7343Data sens_data = {};
  AS7343GetMeasurement(&sens_data);

  const UserConfiguration* cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = cfg->cell_id;

  SensorStatus sen_status = SENSOR_OK;
  size_t data_size = 0;
  size_t data_length = 0;

  // spectral data sensor measurement
  SensorMeasurement sensorMeasurement[18] = SensorMeasurement_init_zero;

  for(int i = 0; i< 18; i++)
  {
    sensorMeasurement[i].type = SensorType_AS7343_SPECTRAL_DATA; 

    sensorMeasurement[i].which_value = SensorMeasurement_unsigned_int_tag;
    sensorMeasurement[i].value.unsigned_int = sens_data.channelCombined[i];

    sensorMeasurement[i].idx = i;

  }


  // encoding  
  sen_status = EncodeRepeatedSensorMeasurements(
      meta, sensorMeasurement, 18, data, data_size, &data_length);

  if (sen_status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_length);

  return data_length;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char I2C_ReadRegister(unsigned char I2CAddress, unsigned char deviceRegisterAddress)
{
    HAL_StatusTypeDef ret;
    
    I2CAddress = I2CAddress << 1; // use 8-bit address
    uint8_t *data = &deviceRegisterAddress;

    // start condition
    ret = HAL_I2C_Master_Transmit(&hi2c1, I2CAddress, data, 1, HAL_MAX_DELAY); // wait for it to end, this is internal and can not stall
    if (ret != HAL_OK)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "I2C Tx Error on read start condition\r\n");
        return ERROR;
    }

    // get byte
    ret = HAL_I2C_Master_Receive(&hi2c1, I2CAddress, data, 1, HAL_MAX_DELAY); // wait for it to end, this is internal and can not stall
    if (ret != HAL_OK)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "I2C Rx Error on read byte\r\n");
        return ERROR;
    }

    return *data;
}


unsigned char I2C_WriteReg(unsigned char I2CAddress, unsigned char deviceRegisterAddress, uint8_t data)
{
    HAL_StatusTypeDef ret;
    I2CAddress = I2CAddress << 1; // use 8-bit address

    ret = HAL_I2C_Mem_Write(&hi2c1, I2CAddress, deviceRegisterAddress, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK)
    {
        APP_LOG(TS_OFF, VLEVEL_M, "I2C Tx Error on write data\r\n");
        return ERROR;
    }

    return SUCCESS;
}
