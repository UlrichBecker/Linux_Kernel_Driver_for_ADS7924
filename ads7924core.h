/******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
/*!
 * @file ads7924core.h
 * @author Ulrich Becker
 * @copyright www.INKATRON.de
 * @date 2017.04.21
 * @brief Declaration of in/out base functions of AD-Converter ADS7924
 * @see ads7924core.c
 */
#ifndef _ADS7924CORE_H_
#define _ADS7924CORE_H_

#include "ads7924driver.h"

//=============================================================================
// Defines
//=============================================================================

/*!
 * @defgroup I2C_ADDR I2C-slave address of ADS7924
 * See also file "ADS7924.pdf" page 27 "I2C ADDRESS SELECTION"
 * @{
 */
/*!
 * @brief I2C-slave-address when pin A0 is connected to ground
 */
#define ADS7924_I2C_ADDRESS_A0_TO_GROUND  0x48

/*!
 * @brief I2C-slave-address when pin A0 is connected ti DVDD
 */
#define ADS7924_I2C_ADDRESS_A0_TO_DVDD    0x49

/*!
 * @brief Status-value when pin A0 is connected to ground
 */
#define ADS7924_RESET_STATE_A0_TO_GROUND  0x18

/*!
 * @brief Status-value when pin A0 is connected to DVDD
 */
#define ADS7924_RESET_STATE_A0_TO_DVDD    0x19
/*! @} defgroup I2C_ADDR */


/*!----------------------------------------------------------------------------
 * @defgroup REG_ADDRS Register addresses of ADS7924
 * See also file "ADS7924.pdf" page 20 "Table 3. Register Map"
 * @{
 */

/*!
 * @brief Register address of ADC Mode Control Register
 */
#define MODECNTRL 0x00
   #define SEL_ID1    (1 << 1)
   #define SEL_ID0    (1 << 0)

/*!
 * @brief Register address of Interrupt Control Register
 */
#define INTCNTRL  0x01
   #define ALARM_ST3  (1 << 7)
   #define ALARM_ST2  (1 << 6)
   #define ALARM_ST1  (1 << 5)
   #define ALARM_ST0  (1 << 4)
   #define AEN_ST3    (1 << 3)
   #define AEN_ST2    (1 << 2)
   #define AEN_ST1    (1 << 1)
   #define AEN_ST0    (1 << 0)

//!@brief Address of Conversion Data for Channel 0, Upper Bits Register
#define DATA0_U   0x02
//!@brief Address of Conversion Data for Channel 0, Lower Bits Register
#define DATA0_L   0x03

//!@brief Address of Conversion Data for Channel 1, Upper Bits Register
#define DATA1_U   0x04
//!@brief Address of Conversion Data for Channel 1, Lower Bits Register
#define DATA1_L   0x05

//!@brief Address of Conversion Data for Channel 2, Upper Bits Register
#define DATA2_U   0x06
//!@brief Address of Conversion Data for Channel 2, Lower Bits Register
#define DATA2_L   0x07

//!@brief Address of Conversion Data for Channel 3, Upper Bits Register
#define DATA3_U   0x08
//!@brief Address of Conversion Data for Channel 3, Lower Bits Register
#define DATA3_L   0x09

//!@brief Address of Upper Limit Threshold for Channel 0 Comparator Register
#define ULR0      0x0A
//!@brief Address of Lower Limit Threshold for Channel 0 Comparator Register
#define LLR0      0x0B

//!@brief Address of Upper Limit Threshold for Channel 1 Comparator Register
#define ULR1      0x0C
//!@brief Address of Lower Limit Threshold for Channel 1 Comparator Register
#define LLR1      0x0D

//!@brief Address of Upper Limit Threshold for Channel 2 Comparator Register
#define ULR2      0x0E
//!@brief Address of Lower Limit Threshold for Channel 2 Comparator Register
#define LLR2      0x0F

//!@brief Address of Upper Limit Threshold for Channel 3 Comparator Register
#define ULR3      0x10
//!@brief Address of Lower Limit Threshold for Channel 3 Comparator Register
#define LLR3      0x11

/*!
 * @brief Register address of Interrupt Configuration Register
 * @see INT_CONFIG
 */
#define INTCONFIG 0x12

/*!
 * @brief Register address of Sleep Configuration Register
 * @see SLEEP_CONF
 */
#define SLPCONFIG 0x13

/*!
 * @brief Register address of 
 * @see ACQ_CONFIG
 */
#define ACQCONFIG 0x14

/*!
 * @brief Register address of Acquire Configuration Register
 * @see PWR_CONFIG
 */
#define PWRCONFIG 0x15

/*!
 * @brief Register address of Reset Register
 * @see ADS7924_IOCTL_RESET
 */
#define RESET     0x16

#define RST_ID7    (1 << 7)
#define RST_ID6    (1 << 6)
#define RST_ID5    (1 << 5)
#define RST_ID4    (1 << 4)
#define RST_ID3    (1 << 3)
#define RST_ID2    (1 << 2)
#define RST_ID1    (1 << 1)
#define RST_ID0    (1 << 0)

#define MAX_ADC_ADDRESS RESET

/*! @} End of defgroup REG_ADDRS */

/*!----------------------------------------------------------------------------
 * @brief I2C-address status-byte mapping.
 * @see I2C_ADDR
 */
typedef struct
{
   u8 addr;   //!<@brief I2C-address
   u8 status; //!<@brief Reset response state
} ADS7924_I2C_ADDRESS_ITEM_T;

extern const ADS7924_I2C_ADDRESS_ITEM_T g_ads7924i2cAddrMap[ADC_CHIPS_PER_BUS];

/*!---------------------------------------------------------------------------
 * @brief Constant object contains the ADS7924 intern register-addresses and
 *        bit-masks which belongs to each of the four analog-channels.
 * @see g_ads7924InternList
 * @see ADS7924_INTERN_INITIALIZER
 */
typedef struct
{
   u8 dataAddrUpper; //!<@brief Address of DATA[0-3]_U
   u8 dataAddrLower; //!<@brief Address of DATA[0-3]_L
   u8 upperLimit;    //!<@brief Address of ULR[0-3]
   u8 lowerLimit;    //!<@brief Address of LLR[0-3]
   u8 stateMask;     //!<@brief Bit-mask of ALARM_ST[0-3]
   u8 enableMask;    //!<@brief Bit-mask of AEN_ST[0-3]
} ADS7924_INTERN_T;

extern const ADS7924_INTERN_T g_ads7924InternList[ADC_CHANNELS_PER_CHIP];

/*!----------------------------------------------------------------------------
 * @brief Returns the register name as ASCII-string by the given
 *        register-address.
 * 
 * Helper function for the process-file-system and debug purposes.
 * 
 * @param i Register-address
 * @retval Namestring of the given address.
 */
const char* getRegisterName( int i );

/*!----------------------------------------------------------------------------
 * @brief Returns the mode-name as ASCII-string by the given value.
 *
 * Helper function for the process-file-system and debug purposes.
 *
 * @param mode Binary value of mode.
 * @retval Namestring of the given mode
 */
const char* getModeName( int mode );

/*!----------------------------------------------------------------------------
 * @brief Read the internal registers of the ADC ADS7924 and copy it in pData.
 * @param address Start address of the registers.
 * @param pData Start address of target memory.
 * @param size Number of bytes to read.
 * @retval >=0 Number of successful received bytes.
 * @retval <0  Error
 */
ssize_t _readAdcRegister( struct i2c_client* poI2cClient, u8 address, void* pData, size_t size );

/*!----------------------------------------------------------------------------
 * @brief Writes the internal registers of the ADC ADS7924.
 * @param address Start address of the registers.
 * @param pData Start address of memory to send to ADS7924.
 * @param size Number of bytes to send.
 * @retval >=0 Number of successful sent bytes.
 * @retval <0  Error
 */
ssize_t _writeAdcRegister( struct i2c_client* poI2cClient, u8 address, void* const pData, size_t size );

/*!----------------------------------------------------------------------------
 */
int _adcChipReset( struct i2c_client* poI2cClient );

/*!----------------------------------------------------------------------------
 */
int adcChipReset( ADS7924_T* pChip );

/*!----------------------------------------------------------------------------
 */
int _adcReadStatusByte( struct i2c_client* poI2cClient, u8* pStatus );

/*!----------------------------------------------------------------------------
 */
int adcReadStatusByte( ADS7924_T* pChip, u8* pStatus );

/*!----------------------------------------------------------------------------
 */
int _adcReadModeByte( struct i2c_client* poI2cClient, u8* pMode );

/*!----------------------------------------------------------------------------
 */
int adcReadModeByte( ADS7924_T* pChip, u8* pMode );

/*!----------------------------------------------------------------------------
 */
int _adcWriteModeByte( struct i2c_client* poI2cClient, u8 mode );

/*!----------------------------------------------------------------------------
 */
int adcWriteModeByte( ADS7924_T* pChip, u8 mode );

/*!----------------------------------------------------------------------------
 */
int _adcReadIntCtrl( struct i2c_client* poI2cClient, u8* pIntctrl );

/*!----------------------------------------------------------------------------
 */
int adcReadIntCtrl( ADS7924_T* pChip, u8* pIntctrl );

/*!----------------------------------------------------------------------------
 */
int _adcWriteIntCtrl( struct i2c_client* poI2cClient, u8 intctrl );

/*!----------------------------------------------------------------------------
 */
int adcWriteIntCtrl( ADS7924_T* pChip, u8 intctrl );

/*!----------------------------------------------------------------------------
 */
int _adcEditIntCtrl( struct i2c_client* poI2cClient, u8 set, u8 clear );

/*!----------------------------------------------------------------------------
 */
int adcEditIntCtrl( ADS7924_T* pChip, u8 set, u8 clear );

/*!----------------------------------------------------------------------------
 */
int _adcReadIntConfig( struct i2c_client* poI2cClient, u8* pIntconfig );

/*!----------------------------------------------------------------------------
 */
int adcReadIntConfig( ADS7924_T* pChip, u8* pIntconfig );

/*!----------------------------------------------------------------------------
 */
int _adcWriteIntConfig( struct i2c_client* poI2cClient, u8 intconfig );

/*!----------------------------------------------------------------------------
 */
int adcWriteIntConfig( ADS7924_T* pChip, u8 intconfig );

/*!--------------------------\t%s:--------------------------------------------------
 */
int _adcEditIntConfig( struct i2c_client*, u8 set, u8 clear );

/*!----------------------------------------------------------------------------
 */
int adcEditIntConfig( ADS7924_T* pChip, u8 set, u8 clear );

/*!----------------------------------------------------------------------------
 */
int _adcReadSlpConfig( struct i2c_client* poI2cClient, u8* pSlpConfig );

/*!----------------------------------------------------------------------------
 */
int adcReadSlpConfig( ADS7924_T* pChip, u8* pSlpConfig );

/*!----------------------------------------------------------------------------
 */
int _adcWriteSlpConfig( struct i2c_client* poI2cClient, u8 slpConfig );

/*!----------------------------------------------------------------------------
 */
int adcWriteSlpConfig( ADS7924_T* pChip, u8 slpConfig );

/*!----------------------------------------------------------------------------
 */
int _adcEditSlpConfig( struct i2c_client* poI2cClient, u8 set, u8 clear );

/*!----------------------------------------------------------------------------
 */
int adcEditSlpConfig( ADS7924_T* pChip, u8 set, u8 clear );

/*!----------------------------------------------------------------------------
 */
int _adcReadAcqConfig( struct i2c_client* poI2cClient, u8* pAcqConfig );

/*!----------------------------------------------------------------------------
 */
int adcReadAcqConfig( ADS7924_T* pChip, u8* pAcqConfig );

/*!----------------------------------------------------------------------------
 */
int readAnalogValue( ADC_CHANNEL_T* poCannel );

/*!----------------------------------------------------------------------------
 */
int _adcWriteAcqConfig( struct i2c_client* poI2cClient, u8 acqConfig );

/*!----------------------------------------------------------------------------
 */
int adcWriteAcqConfig( ADS7924_T* pChip, u8 acqConfig );

/*!----------------------------------------------------------------------------
 */
int _adcEditAcqConfig( struct i2c_client* poI2cClient, u8 set, u8 clear );

/*!----------------------------------------------------------------------------
 */
int adcEditAcqConfig( ADS7924_T* pChip, u8 set, u8 clear );

/*!----------------------------------------------------------------------------
 */
int _adcReadPwrConfig( struct i2c_client* poI2cClient, u8* pPwrConfig );

/*!----------------------------------------------------------------------------
 */
int adcReadPwrConfig( ADS7924_T* pChip, u8* pPwrConfig );

/*!----------------------------------------------------------------------------
 */
int _adcWritePwrConfig( struct i2c_client* poI2cClient, u8 pwrConfig );

/*!----------------------------------------------------------------------------
 */
int adcWritePwrConfig( ADS7924_T* pChip, u8 pwrConfig );

/*!----------------------------------------------------------------------------
 */
int _adcEditPwrConfig( struct i2c_client* poI2cClient, u8 set, u8 clear );

/*!----------------------------------------------------------------------------
 */
int adcEditPwrConfig( ADS7924_T* pChip, u8 set, u8 clear );

/*!----------------------------------------------------------------------------
 */
int readAnalogValue( ADC_CHANNEL_T* poCannel );

/*!----------------------------------------------------------------------------
 */
int adcWriteUpperLimitThreshold( ADC_CHANNEL_T* poCannel, u8 threshold );

/*!----------------------------------------------------------------------------
 */
int adcReadUpperLimitThreshold( ADC_CHANNEL_T* poCannel, u8* pThreshold );

/*!----------------------------------------------------------------------------
 */
int adcWriteLowerLimitThreshold( ADC_CHANNEL_T* poCannel, u8 threshold );

/*!----------------------------------------------------------------------------
 */
int adcReadLowerLimitThreshold( ADC_CHANNEL_T* poCannel, u8* pThreshold );

/*!----------------------------------------------------------------------------
 */
int adcAlarmEnable( ADC_CHANNEL_T* poCannel );

/*!----------------------------------------------------------------------------
 */
int adcAlarmDisable( ADC_CHANNEL_T* poCannel );

#endif /* ifndef _ADS7924CORE_H_ */
/*================================== EOF ====================================*/
