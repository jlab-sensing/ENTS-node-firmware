/**
 ******************************************************************************
 * @file     as7343_defs.h
 * @author   Eric Tran
 * @brief    Register Addresses for the as7343 sensor.
 *
 *           This library is designed to read measurements from a Multi-Spectral Sensor
 *           https://github.com/sparkfun/SparkFun_AS7343_Arduino_Library/blob/main/examples/Example_01_BasicReadings/Example_01_BasicReadings.ino
 * @date     7/3/2026
 ******************************************************************************
 */

#ifndef _AS7343_DEFS_H
#define _AS7343_DEFS_H


#include "i2c.h"

/**
 * Register Addresses copied from Sparkfun Github
 * https://github.com/sparkfun/SparkFun_AS7343_Arduino_Library/blob/d4185477539efce74c9453117443b75811f00eee/src/sfTk/sfDevAS7343.h
 * 
 */
/******************************************************************************/
/*! @name         I2C Addressing             */
/******************************************************************************/
#define kAS7343Addr 0x39 // I2C address for the AS7343.
#define kDefaultAS7343DeviceID 0x81 // When polling the ID register, this should be returned on boot.

/******************************************************************************/
/*! @name         Enum Definitions             */
/******************************************************************************/
/*! @name Sensor Register Bank settings */
typedef enum
{
    REG_BANK_0 = 0x00, // Register bank 0 (default)
    REG_BANK_1 = 0x01, // Register bank 1
} sfe_as7343_reg_bank_t;

/*! @name Sensor Channels */
typedef enum
{
    CH_BLUE_FZ_450NM = 0x00, // Blue peak wavelength 450 nanometers (cycle 1)
    CH_GREEN_FY_555NM,       // Green (wide bandwidth) peak wavelength 555 nanometers (cycle 1)
    CH_ORANGE_FXL_600NM,     // Orange peak wavelength 600 nanometers (cycle 1)
    CH_NIR_855NM,            // NIR peak wavelength 855 nanometers (cycle 1)
    CH_VIS_1,                // VIS (cycle 1)
    CH_FD_1,                 // Flicker Detection (cycle 1)
    CH_DARK_BLUE_F2_425NM,   // Dark Blue peak wavelength 425 nanometers (cycle 2)
    CH_LIGHT_BLUE_F3_475NM,  // Light Blue peak wavelength 475 nanometers (cycle 2)
    CH_BLUE_F4_515NM,        // Blue peak wavelength 515 nanometers (cycle 2)
    CH_BROWN_F6_640NM,       // Brown peak wavelength 640 nanometers (cycle 2)
    CH_VIS_2,                // VIS (cycle 2)
    CH_FD_2,                 // Flicker Detection (cycle 2)
    CH_PURPLE_F1_405NM,      // Purple peak wavelength 405 nanometers (cycle 3)
    CH_RED_F7_690NM,         // Red peak wavelength 690 nanometers (cycle 3)
    CH_DARK_RED_F8_745NM,    // Dark Red peak wavelength 745 nanometers (cycle 3)
    CH_GREEN_F5_550NM,       // Green (Narrow bandwidth) peak wavelength 550 nanometers (cycle 3)
    CH_VIS_3,                // VIS (cycle 3)
    CH_FD_3,                 // Flicker Detection (cycle 3)
} sfe_as7343_channel_t;

/*! @name Sensor number of channels constant */
#define ksfAS7343NumChannels 18 // Number of channels in the AS7343 sensor

/*! @name Sensor gain settings */
typedef enum
{
    AGAIN_0_5 = 0x00, // 0.5x gain
    AGAIN_1,
    AGAIN_2,
    AGAIN_4,
    AGAIN_8,
    AGAIN_16,
    AGAIN_32,
    AGAIN_64,
    AGAIN_128,
    AGAIN_256,
    AGAIN_512,
    AGAIN_1024,
    AGAIN_2048,

} sfe_as7343_again_t;

/*! @name Flicker Detection Gain Settings (ADC5) */
typedef enum
{
    FD_GAIN_0_5 = 0x00, // 0.5x gain
    FD_GAIN_1,
    FD_GAIN_2,
    FD_GAIN_4,
    FD_GAIN_8,
    FD_GAIN_16,
    FD_GAIN_32,
    FD_GAIN_64,
    FD_GAIN_128,
    FD_GAIN_256,
    FD_GAIN_512,
    FD_GAIN_1024,
    FD_GAIN_2048,
} sfe_as7343_fd_gain_t;

/*! @name FIFO Threshold settings */
typedef enum
{
    FIFO_THRESHOLD_LVL_1 = 0x00,
    FIFO_THRESHOLD_LVL_4,
    FIFO_THRESHOLD_LVL_8,
    FIFO_THRESHOLD_LVL_16,
} sfe_as7343_fifo_threshold_t;

/*! @name Spectral Threshold Channel */
typedef enum
{
    SPECTRAL_THRESHOLD_CHANNEL_0 = 0x00,
    SPECTRAL_THRESHOLD_CHANNEL_1,
    SPECTRAL_THRESHOLD_CHANNEL_2,
    SPECTRAL_THRESHOLD_CHANNEL_3,
    SPECTRAL_THRESHOLD_CHANNEL_4,
    SPECTRAL_THRESHOLD_CHANNEL_5,
} sfe_as7343_spectral_threshold_channel_t;

/*! @name Automatic Channel Read-out (auto_smux setting) */
typedef enum
{
    AUTOSMUX_6_CHANNELS = 0x00,  // 6 channel readout: FZ, FY, FXL, NIR, 2xVIS, FD
    AUTOSMUX_12_CHANNELS = 0x02, // 12 channel readout
                                 // Cycle 1: FZ, FY, FXL, NIR, 2xVIS, FD
                                 // Cycle 2: FZ, F3, F4, F6, 2xVIS, FD
    AUTOSMUX_18_CHANNELS = 0x03, // 18 channel readout
                                 // Cycle 1: FZ, FY, FXL, NIR, 2xVIS, FD
                                 // Cycle 2: F2, F3, F4, F6, 2xVIS, FD
                                 // Cycle 3: F1, F7, F8, F5, 2xVIS, FD
} sfe_as7343_auto_smux_channel_t;

/*! @name GPIO mode settings */
typedef enum
{
    AS7343_GPIO_MODE_OUTPUT = 0x00, // GPIO set to output mode
    AS7343_GPIO_MODE_INPUT,         // GPIO set to input mode
} sfe_as7343_gpio_mode_t;

/*! @name GPIO output settings */
typedef enum
{
    AS7343_GPIO_OUTPUT_LOW = 0x00, // GPIO set to low
    AS7343_GPIO_OUTPUT_HIGH,       // GPIO set to high
} sfe_as7343_gpio_output_t;

/******************************************************************************/
/*! @name         Register Definitions             */
/******************************************************************************/

/*! @name GPIO output settings */
#define ksfAS7343RegAuxID 0x58 // Register Address
typedef union {
    struct
    {
        uint8_t auxid : 4;
        uint8_t reserved : 4;
    };
    uint8_t byte;
} sfe_as7343_reg_auxid_t;

#define ksfAS7343RegRevID 0x59 // Register Address

typedef union {
    struct
    {
        uint8_t revid : 3;
        uint8_t reserved : 5;
    };
    uint8_t byte;
} sfe_as7343_reg_revid_t;

#define ksfAS7343RegID 0x5A // Register Address, register is a single uint8_t.
typedef uint8_t sfe_as7343_reg_id_t;

#define ksfAS7343RegCfg12 0x66 // Register Address

typedef union {
    struct
    {
        uint8_t reserved : 5;
        uint8_t sp_th_ch : 3;
    };
    uint8_t byte;
} sfe_as7343_reg_cfg12_t;

#define ksfAS7343RegEnable 0x80 // Register Address

typedef union {
    struct
    {
        uint8_t pon : 1;
        uint8_t sp_en : 1;
        uint8_t reserved : 1;
        uint8_t wen : 1;
        uint8_t smuxen : 1;
        uint8_t reserved_one : 1;
        uint8_t fden : 1;
        uint8_t reserved_two : 1;
        // uint8_t enable : 1;
    };
    uint8_t byte;
} sfe_as7343_reg_enable_t;

#define ksfAS7343RegATime 0x81 // Register Address, register is a single uint8_t.
typedef uint8_t sfe_as7343_reg_atime_t;

#define ksfAS7343RegWTime 0x83 // Register Address, register is a single uint8_t.
typedef uint8_t sfe_as7343_reg_wtime_t;

#define ksfAS7343RegSpThL 0x84 // Register Address

typedef union {
    struct
    {
        uint8_t sp_th_l_lsb : 8;
        uint8_t sp_th_l_msb : 8;
    };
    uint16_t word;
} sfe_as7343_reg_sp_th_l_t;

#define ksfAS7343RegSpThH 0x86 // Register Address

typedef union {
    struct
    {
        uint8_t sp_th_h_lsb : 8;
        uint8_t sp_th_h_msb : 8;
    };
    uint16_t word;
} sfe_as7343_reg_sp_th_h_t;

#define ksfAS7343RegStatus 0x93 // Register Address

typedef union {
    struct
    {
        uint8_t sint : 1;
        uint8_t reserved : 1;
        uint8_t fint : 1;
        uint8_t aint : 1;
        uint8_t reserved_one : 3;
        uint8_t asat : 1;
    };
    uint8_t byte;
} sfe_as7343_reg_status_t;

#define ksfAS7343RegAStatus 0x94 // Register Address

typedef union {
    struct
    {
        uint8_t again_status : 4;
        uint8_t reserved : 3;
        uint8_t asat_satatus : 1;
    };
    uint8_t byte;
} sfe_as7343_reg_astatus_t;

#define ksfAS7343RegData0 0x95  // Register Address
#define ksfAS7343RegData1 0x97  // Register Address
#define ksfAS7343RegData2 0x99  // Register Address
#define ksfAS7343RegData3 0x9B  // Register Address
#define ksfAS7343RegData4 0x9D  // Register Address
#define ksfAS7343RegData5 0x9F  // Register Address
#define ksfAS7343RegData6 0xA1  // Register Address
#define ksfAS7343RegData7 0xA3  // Register Address
#define ksfAS7343RegData8 0xA5  // Register Address
#define ksfAS7343RegData9 0xA7  // Register Address
#define ksfAS7343RegData10 0xA9 // Register Address
#define ksfAS7343RegData11 0xAB // Register Address
#define ksfAS7343RegData12 0xAD // Register Address
#define ksfAS7343RegData13 0xAF // Register Address
#define ksfAS7343RegData14 0xB1 // Register Address
#define ksfAS7343RegData15 0xB3 // Register Address
#define ksfAS7343RegData16 0xB5 // Register Address
#define ksfAS7343RegData17 0xB7 // Register Address

typedef union {
    struct
    {
        uint8_t data_l : 8;
        uint8_t data_h : 8;
    };
    uint16_t word;
} sfe_as7343_reg_data_t;

#define ksfAS7343RegStatus2 0x90 // Register Address

typedef union {
    struct
    {
        uint8_t fdsat_dig : 1;
        uint8_t fdsat_ana : 1;
        uint8_t reserved : 1;
        uint8_t asat_ana : 1;
        uint8_t asat_dig : 1;
        uint8_t reserved_one : 1;
        uint8_t avalid : 1;
        uint8_t reserved_two : 1;
    };
    uint8_t byte;
} sfe_as7343_reg_status2_t;

#define ksfAS7343RegStatus3 0x91 // Register Address

typedef union {
    struct
    {
        uint8_t reserved : 4;
        uint8_t int_sp_l : 1;
        uint8_t int_sp_h : 1;
        uint8_t reserved_one : 2;
    };
    uint8_t byte;
} sfe_as7343_reg_status3_t;

#define ksfAS7343RegStatus4 0xBC // Register Address

typedef union {
    struct
    {
        uint8_t int_busy : 1;
        uint8_t sai_act : 1;
        uint8_t sp_trig : 1;
        uint8_t reserved : 1;
        uint8_t fd_trig : 1;
        uint8_t ov_temp : 1;
        uint8_t reserved_one : 1;
        uint8_t fifo_ov : 1;
    };
    uint8_t byte;
} sfe_as7343_reg_status4_t;

#define ksfAS7343RegFdStatus 0xE3 // Register Address

typedef union {
    struct
    {
        uint8_t fd_100hz_det : 1;
        uint8_t fd_120hz_det : 1;
        uint8_t fd_100hz_valid : 1;
        uint8_t fd_120hz_valid : 1;
        uint8_t fd_saturation : 1;
        uint8_t fd_meas_valid : 1;
        uint8_t reserved : 2;
    };
    uint8_t byte;
} sfe_as7343_reg_fd_status_t;

#define ksfAS7343RegStatus5 0x93 // Register Address

typedef union {
    struct
    {
        uint8_t reserved : 2;
        uint8_t sint_smux : 1;
        uint8_t sint_fd : 1;
        uint8_t reserved_one : 4;
    };
    uint8_t byte;
} sfe_as7343_reg_data_status5_t;

#define ksfAS7343RegCfg0 0xBF // Register Address

typedef union {
    struct
    {
        uint8_t reserved : 2;
        uint8_t wlong : 1;
        uint8_t reserved_one : 1;
        uint8_t reg_bank : 1;
        uint8_t low_power : 1;
        uint8_t reserved_two : 2;
    };
    uint8_t byte;
} sfe_as7343_reg_cfg0_t;

#define ksfAS7343RegCfg1 0xC6 // Register Address

typedef union {
    struct
    {
        uint8_t again : 5;
        uint8_t reserved : 3;
    };
    uint8_t byte;
} sfe_as7343_reg_cfg1_t;

#define ksfAS7343RegCfg3 0xC7 // Register Address

typedef union {
    struct
    {
        uint8_t reserved : 4;
        uint8_t sai : 1;
        uint8_t reserved2 : 3;
    };
    uint8_t byte;
} sfe_as7343_reg_cfg3_t;

#define ksfAS7343RegCfg6 0xF5 // Register Address

typedef union {
    struct
    {
        uint8_t reserved : 3;
        uint8_t smux_cmd : 2;
        uint8_t reserved_one : 3;
    };
    uint8_t byte;
} sfe_as7343_reg_cfg6_t;

#define ksfAS7343RegCfg8 0xC9 // Register Address

typedef union {
    struct
    {
        uint8_t reserved : 6;
        uint8_t fifo_th : 2;
    };
    uint8_t byte;
} sfe_as7343_reg_cfg8_t;

#define ksfAS7343RegCfg9 0xCA // Register Address

typedef union {
    struct
    {
        uint8_t reserved : 5;
        uint8_t sienc_smux : 1;
        uint8_t reserved_one : 1;
        uint8_t sienc_fd : 1;
        uint8_t reserved_two : 1;
    };
    uint8_t byte;
} sfe_as7343_reg_cfg9_t;

#define ksfAS7343RegCfg10 0x65 // Register Address

typedef union {
    struct
    {
        uint8_t fd_pers : 3;
        uint8_t reserved : 5;
    };
    uint8_t byte;
} sfe_as7343_reg_cfg10_t;

#define ksfAS7343RegPers 0xCF // Register Address

typedef union {
    struct
    {
        uint8_t apers : 4;
        uint8_t reserved : 4;
    };
    uint8_t byte;
} sfe_as7343_reg_pers_t;

#define ksfAS7343RegGpio 0x6B // Register Address

typedef union {
    struct
    {
        uint8_t gpio_in : 1;
        uint8_t gpio_out : 1;
        uint8_t gpio_in_en : 1;
        uint8_t gpio_inv : 1;
        uint8_t reserved : 4;
    };
    uint8_t byte;
} sfe_as7343_reg_gpio_t;

#define ksfAS7343RegAStep 0xD4 // Register Address

typedef union {
    struct
    {
        uint8_t astep_l : 8;
        uint8_t astep_h : 8;
    };
    uint16_t word;
} sfe_as7343_reg_astep_t;

#define ksfAS7343RegCfg20 0xD6 // Register Address

typedef union {
    struct
    {
        uint8_t reserved : 5;
        uint8_t auto_smux : 2;
        uint8_t fd_fifo_8b : 1;
    };
    uint8_t byte;
} sfe_as7343_reg_cfg20_t;

#define ksfAS7343RegLed 0xCD // Register Address

typedef union {
    struct
    {
        uint8_t led_drive : 7;
        uint8_t led_act : 1;
    };
    uint8_t byte;
} sfe_as7343_reg_led_t;

#define ksfAS7343RegAgcGainMax 0xD7 // Register Address

typedef union {
    struct
    {
        uint8_t reserved : 4;
        uint8_t agc_fd_gain_max : 4;
    };
    uint8_t byte;
} sfe_as7343_reg_agc_gain_max_t;

#define ksfAS7343RegAzConfig 0xDE // Register Address, register is a single uint8_t.
typedef uint8_t sfe_as7343_reg_az_config_t;

#define ksfAS7343RegFdTime1 0xE0 // Register Address, register is a single uint8_t.
typedef uint8_t sfe_as7343_reg_fd_time_1_t;

#define ksfAS7343RegFdTime2 0xE2 // Register Address

typedef union {
    struct
    {
        uint8_t fd_time_h : 3; // TODO: Unkown big order
        uint8_t fd_gain : 5;
    };
    uint8_t byte;
} sfe_as7343_reg_fd_time_2_t;

#define ksfAS7343RegFdTimeCfg0 0xDF // Register Address

typedef union {
    struct
    {
        uint8_t reserved : 7;
        uint8_t fifo_write_fd : 1;
    };
    uint8_t byte;
} sfe_as7343_reg_fd_cfg0_t;

#define ksfAS7343RegIntEnab 0xF9 // Register Address

typedef union {
    struct
    {
        uint8_t sein : 1;
        uint8_t reserved : 1;
        uint8_t fien : 1;
        uint8_t sp_ien : 1;
        uint8_t reserved_one : 3;
        uint8_t asien : 1;
    };
    uint8_t byte;
} sfe_as7343_reg_intenab_t;

#define ksfAS7343RegControl 0xFA // Register Address

typedef union {
    struct
    {
        uint8_t clear_sai_act : 1;
        uint8_t fifo_clr : 1;
        uint8_t sp_man_az : 1;
        uint8_t sw_reset : 1;
        uint8_t reserved : 4;
    };
    uint8_t byte;
} sfe_as7343_reg_control_t;

#define ksfAS7343RegFifoMap 0xFC // Register Address

typedef union {
    struct
    {
        uint8_t fifo_write_astatus : 1;
        uint8_t fifo_write_ch0_data : 1;
        uint8_t fifo_write_ch1_data : 1;
        uint8_t fifo_write_ch2_data : 1;
        uint8_t fifo_write_ch3_data : 1;
        uint8_t fifo_write_ch4_data : 1;
        uint8_t fifo_write_ch5_data : 1;
        uint8_t reserved : 1;
    };
    uint8_t byte;
} sfe_as7343_reg_fifo_map_t;

#define ksfAS7343RegFifoLvl 0xFD // Register Address, register is a single uint8_t.
typedef uint8_t sfe_as7343_reg_fifo_lvl_t;

#define ksfAS7343RegFData 0xFE // Register Address

typedef union {
    struct
    {
        uint8_t fdata_l : 8;
        uint8_t fdata_h : 8;
    };
    uint16_t word;
} sfe_as7343_reg_fifo_data_t;

#endif  // _AS7343_DEFS_H