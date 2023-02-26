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
 * @file ads7924core.c
 * @author Ulrich Becker
 * @copyright www.INKATRON.de
 * @date 2017.04.21
 * @brief Implementation of in/out base functions of AD-Converter ADS7924
 * @see ads7924core.h
 */
#include "ads7924core.h"

/*!
 * @brief Lock I2C-device
 */
#define LOCK_I2C( pChip )   mutex_lock( &pChip->oI2cMutex )

/*!
 * @brief Unlock I2C-device
 */
#define UNLOCK_I2C( pChip ) mutex_unlock( &pChip->oI2cMutex )


#define READ_CONTINUE  0x80

/*!----------------------------------------------------------------------------
 * @brief Association between index, I2C-address and reset status byte.
 */
const __initdata ADS7924_I2C_ADDRESS_ITEM_T g_ads7924i2cAddrMap[ADC_CHIPS_PER_BUS] =
{
   {
      .addr   = ADS7924_I2C_ADDRESS_A0_TO_GROUND,
      .status = ADS7924_RESET_STATE_A0_TO_GROUND
   },
   {
      .addr   = ADS7924_I2C_ADDRESS_A0_TO_DVDD,
      .status = ADS7924_RESET_STATE_A0_TO_DVDD
   }
};

/*!----------------------------------------------------------------------------
 * @brief Initializer of constant global object g_ads7924InternList
 * @see ADS7924_INTERN_T
 * @see g_ads7924InternList
 */
#define ADS7924_INTERN_INITIALIZER( n ) \
{                                       \
   .dataAddrUpper = DATA##n##_U,        \
   .dataAddrLower = DATA##n##_L,        \
   .upperLimit    = ULR##n,             \
   .lowerLimit    = LLR##n,             \
   .stateMask     = ALARM_ST##n,        \
   .enableMask    = AEN_ST##n           \
}

/*!----------------------------------------------------------------------------
 * @brief Initializing of register-addresses and bit-masks for all four
 *        ADS7924 analog channels.
 *
 * In this way a indexed addressing of those becomes possible.
 * @see ADS7924_INTERN_T
 * @see ADS7924_INTERN_INITIALIZER
 */
const ADS7924_INTERN_T g_ads7924InternList[ADC_CHANNELS_PER_CHIP] =
{
   ADS7924_INTERN_INITIALIZER( 0 ),
   ADS7924_INTERN_INITIALIZER( 1 ),
   ADS7924_INTERN_INITIALIZER( 2 ),
   ADS7924_INTERN_INITIALIZER( 3 )
};


#define CASE_ITEM( item ) case item: return #item

/*!----------------------------------------------------------------------------
 * @see ads7924core.h
 */
const char* getRegisterName( int i )
{
   switch( i )
   {
      CASE_ITEM( MODECNTRL );
      CASE_ITEM( INTCNTRL  );
      CASE_ITEM( DATA0_U   );
      CASE_ITEM( DATA0_L   );
      CASE_ITEM( DATA1_U   );
      CASE_ITEM( DATA1_L   );
      CASE_ITEM( DATA2_U   );
      CASE_ITEM( DATA2_L   );
      CASE_ITEM( DATA3_U   );
      CASE_ITEM( DATA3_L   );
      CASE_ITEM( ULR0      );
      CASE_ITEM( LLR0      );
      CASE_ITEM( ULR1      );
      CASE_ITEM( LLR1      );
      CASE_ITEM( ULR2      );
      CASE_ITEM( LLR2      );
      CASE_ITEM( ULR3      );
      CASE_ITEM( LLR3      );
      CASE_ITEM( INTCONFIG );
      CASE_ITEM( SLPCONFIG );
      CASE_ITEM( ACQCONFIG );
      CASE_ITEM( PWRCONFIG );
      CASE_ITEM( RESET     );
   }
   return "unknown";
}

/*!----------------------------------------------------------------------------
 * @see ads7924core.h
 */
const char* getModeName( int mode )
{
   switch( mode & ADS7924_MODE_AUTO_BURST_SCAN_SLEEP )
   {
      CASE_ITEM( ADS7924_MODE_IDLE );
      CASE_ITEM( ADS7924_MODE_AWAKE );
      CASE_ITEM( ADS7924_MODE_MANUAL_SINGLE );
      CASE_ITEM( ADS7924_MODE_MANUAL_SCAN );
      CASE_ITEM( ADS7924_MODE_AUTO_SINGLE );
      CASE_ITEM( ADS7924_MODE_AUTO_SCAN );
      CASE_ITEM( ADS7924_MODE_AUTO_SINGLE_SLEEP );
      CASE_ITEM( ADS7924_MODE_AUTO_SCAN_SLEEP );
      CASE_ITEM( ADS7924_MODE_AUTO_BURST_SCAN_SLEEP );
   }
   return "unknown";
}

/*!----------------------------------------------------------------------------
 * @see ads7924core.h
 */
ssize_t _readAdcRegister( struct i2c_client* poI2cClient, u8 address, void* pData, size_t size )
{
   ssize_t ret;

   BUG_ON( poI2cClient == NULL );
   BUG_ON( size == 0 );
   BUG_ON( address + size > MAX_ADC_ADDRESS+1 );

   if( size > 1 )
      address |= READ_CONTINUE;

   ret = i2c_master_send( poI2cClient, &address, sizeof( address ) );
   if( ret < 0 )
   {
      ERROR_MESSAGE( ": Unable to send address 0x%02X %s to ADS7924\n",
                     address, getRegisterName( address ) );
      return ret;
   }
   ret = i2c_master_recv( poI2cClient, pData, size );
   if( ret < 0 )
      ERROR_MESSAGE( ": Unable to receive %d bytes from ADS7924 register %s\n",
                     size, getRegisterName( address ) );

   return ret;
}


/*!----------------------------------------------------------------------------
 * @see ads7924core.h
 */
ssize_t _writeAdcRegister( struct i2c_client* poI2cClient, u8 address, void* const pData, size_t size )
{
   ssize_t ret;
   u8 buffer[size + sizeof( address )];

   BUG_ON( poI2cClient == NULL );
   BUG_ON( size == 0 );
   BUG_ON( address + size > MAX_ADC_ADDRESS+1 );

   if( size > 1 )
      address |= READ_CONTINUE;

   buffer[0] = address;
   memcpy( &buffer[1], pData, size );

   ret = i2c_master_send( poI2cClient, buffer, size + sizeof( address ) );
   if( ret < 0 )
      ERROR_MESSAGE( "Unable to send %d bytes to ADS7924 register %s\n",
                     size, getRegisterName( address ) );

   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcWriteByteRegister( struct i2c_client* poI2cClient, u8 address, u8 adcReg )
{
   return _writeAdcRegister( poI2cClient, address, &adcReg, sizeof( adcReg ) );
}

/*!----------------------------------------------------------------------------
 */
static int _adcEditRegister( struct i2c_client* poI2cClient, u8 addr, u8 set, u8 clear )
{
   int ret;
   u8  reg;

   ret =  _readAdcRegister( poI2cClient, addr, &reg, sizeof( reg ) );
   if( ret < 0 )
      return ret;

   DEBUG_MESSAGE( ": %s -> 0x%02X\n", getRegisterName( addr ), reg );

   reg &= ~clear;
   reg |= set;

   DEBUG_MESSAGE( ": %s -> 0x%02X\n", getRegisterName( addr ), reg );

   return _adcWriteByteRegister( poI2cClient, addr, reg );
}

/*!----------------------------------------------------------------------------
 */
int _adcChipReset( struct i2c_client* poI2cClient )
{
   DEBUG_MESSAGE( ": Chip-reset: %s, address: 0x%02X\n", 
                  poI2cClient->name, poI2cClient->addr );
   return _adcWriteByteRegister( poI2cClient, RESET, 0xAA );
}

/*!----------------------------------------------------------------------------
 */
int adcChipReset( ADS7924_T* pChip )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcChipReset( pChip->pI2cSlave );
   if( ret >= 0 )
   {
      pChip->shadowAlarmStatus = 0;
      pChip->afterReset = true; /* Discard the first interrupt */
   }
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcReadStatusByte( struct i2c_client* poI2cClient, u8* pStatus )
{
   return _readAdcRegister( poI2cClient, RESET, pStatus, sizeof( u8 ) );
}

/*!----------------------------------------------------------------------------
 */
int adcReadStatusByte( ADS7924_T* pChip, u8* pStatus )
{
   int ret;
   LOCK_I2C( pChip );
   ret = _adcReadStatusByte( pChip->pI2cSlave, pStatus );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcReadModeByte( struct i2c_client* poI2cClient, u8* pMode )
{
   return _readAdcRegister( poI2cClient, MODECNTRL, pMode, sizeof( u8 ) );
}

/*!----------------------------------------------------------------------------
 */
int adcReadModeByte( ADS7924_T* pChip, u8* pMode )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcReadModeByte( pChip->pI2cSlave, pMode );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcWriteModeByte( struct i2c_client* poI2cClient, u8 mode )
{
   return _adcWriteByteRegister( poI2cClient, MODECNTRL, mode );
}

/*!----------------------------------------------------------------------------
 */
int adcWriteModeByte( ADS7924_T* pChip, u8 mode )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcWriteModeByte( pChip->pI2cSlave, mode );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcReadIntCtrl( struct i2c_client* poI2cClient, u8* pIntctrl )
{
   return _readAdcRegister( poI2cClient, INTCNTRL, pIntctrl, sizeof( u8 ) );
}

/*!----------------------------------------------------------------------------
 */
int adcReadIntCtrl( ADS7924_T* pChip, u8* pIntctrl )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcReadIntCtrl( pChip->pI2cSlave, pIntctrl );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcWriteIntCtrl( struct i2c_client* poI2cClient, u8 intctrl )
{
   return _adcWriteByteRegister( poI2cClient, INTCNTRL, intctrl );
}

/*!----------------------------------------------------------------------------
 */
int adcWriteIntCtrl( ADS7924_T* pChip, u8 intctrl )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcWriteIntCtrl( pChip->pI2cSlave, intctrl );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcEditIntCtrl( struct i2c_client* poI2cClient, u8 set, u8 clear )
{
   return _adcEditRegister( poI2cClient, INTCNTRL, set, clear );
}

/*!----------------------------------------------------------------------------
 */
int adcEditIntCtrl( ADS7924_T* pChip, u8 set, u8 clear )
{
   int ret;
   u8 tmpAlarmCtrl;

   LOCK_I2C( pChip );
   tmpAlarmCtrl = pChip->shadowAlarmStatus;
   tmpAlarmCtrl &= ~clear;
   tmpAlarmCtrl |= set;
   ret = _adcWriteIntCtrl( pChip->pI2cSlave, tmpAlarmCtrl );
   if( ret >= 0 )
      pChip->shadowAlarmStatus = tmpAlarmCtrl;
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcReadIntConfig( struct i2c_client* poI2cClient, u8* pIntconfig )
{
   return _readAdcRegister( poI2cClient, INTCONFIG, pIntconfig, sizeof( u8 ) );
}

/*!----------------------------------------------------------------------------
 */
int adcReadIntConfig( ADS7924_T* pChip, u8* pIntconfig )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcReadIntConfig( pChip->pI2cSlave, pIntconfig );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcWriteIntConfig( struct i2c_client* poI2cClient, u8 intconfig )
{
   return _adcWriteByteRegister( poI2cClient, INTCONFIG, intconfig );
}

/*!----------------------------------------------------------------------------
 */
int adcWriteIntConfig( ADS7924_T* pChip, u8 intconfig )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcWriteIntConfig( pChip->pI2cSlave, intconfig );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcEditIntConfig( struct i2c_client* poI2cClient, u8 set, u8 clear )
{
   return _adcEditRegister( poI2cClient, INTCONFIG, set, clear );
}

/*!----------------------------------------------------------------------------
 */
int adcEditIntConfig( ADS7924_T* pChip, u8 set, u8 clear )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcEditIntConfig( pChip->pI2cSlave, set, clear );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcReadSlpConfig( struct i2c_client* poI2cClient, u8* pSlpConfig )
{
   return _readAdcRegister( poI2cClient, SLPCONFIG, pSlpConfig, sizeof( u8 ) );
}

/*!----------------------------------------------------------------------------
 */
int adcReadSlpConfig( ADS7924_T* pChip, u8* pSlpConfig )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcReadSlpConfig( pChip->pI2cSlave, pSlpConfig );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcWriteSlpConfig( struct i2c_client* poI2cClient, u8 slpConfig )
{
   return _adcWriteByteRegister( poI2cClient, SLPCONFIG, slpConfig );
}

/*!----------------------------------------------------------------------------
 */
int adcWriteSlpConfig( ADS7924_T* pChip, u8 slpConfig )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcWriteSlpConfig( pChip->pI2cSlave, slpConfig );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcEditSlpConfig( struct i2c_client* poI2cClient, u8 set, u8 clear )
{
   return _adcEditRegister( poI2cClient, SLPCONFIG, set, clear );
}

/*!----------------------------------------------------------------------------
 */
int adcEditSlpConfig( ADS7924_T* pChip, u8 set, u8 clear )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcEditSlpConfig( pChip->pI2cSlave, set, clear );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcReadAcqConfig( struct i2c_client* poI2cClient, u8* pAcqConfig )
{
   return _readAdcRegister( poI2cClient, ACQCONFIG, pAcqConfig, sizeof( u8 ) );
}

/*!----------------------------------------------------------------------------
 */
int adcReadAcqConfig( ADS7924_T* pChip, u8* pAcqConfig )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcReadAcqConfig( pChip->pI2cSlave, pAcqConfig );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcWriteAcqConfig( struct i2c_client* poI2cClient, u8 acqConfig )
{
   return _adcWriteByteRegister( poI2cClient, ACQCONFIG, acqConfig );
}

/*!----------------------------------------------------------------------------
 */
int adcWriteAcqConfig( ADS7924_T* pChip, u8 acqConfig )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcWriteAcqConfig( pChip->pI2cSlave, acqConfig );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcEditAcqConfig( struct i2c_client* poI2cClient, u8 set, u8 clear )
{
   return _adcEditRegister( poI2cClient, ACQCONFIG, set, clear );
}

/*!----------------------------------------------------------------------------
 */
int adcEditAcqConfig( ADS7924_T* pChip, u8 set, u8 clear )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcEditAcqConfig( pChip->pI2cSlave, set, clear );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcReadPwrConfig( struct i2c_client* poI2cClient, u8* pPwrConfig )
{
   return _readAdcRegister( poI2cClient, PWRCONFIG, pPwrConfig, sizeof( u8 ));
}

/*!----------------------------------------------------------------------------
 */
int adcReadPwrConfig( ADS7924_T* pChip, u8* pPwrConfig )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcReadPwrConfig( pChip->pI2cSlave, pPwrConfig );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int _adcWritePwrConfig( struct i2c_client* poI2cClient, u8 pwrConfig )
{
   return _adcWriteByteRegister( poI2cClient, PWRCONFIG, pwrConfig );
}

/*-----------------------------------------------------------------------------
 */
int adcWritePwrConfig( ADS7924_T* pChip, u8 pwrConfig )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcWritePwrConfig( pChip->pI2cSlave, pwrConfig );
   UNLOCK_I2C( pChip );
   return ret;
}

/*-----------------------------------------------------------------------------
 */
int _adcEditPwrConfig( struct i2c_client* poI2cClient, u8 set, u8 clear )
{
   return _adcEditRegister( poI2cClient, PWRCONFIG, set, clear );
}

/*-----------------------------------------------------------------------------
 */
int adcEditPwrConfig( ADS7924_T* pChip, u8 set, u8 clear )
{
   int ret;

   LOCK_I2C( pChip );
   ret = _adcEditPwrConfig( pChip->pI2cSlave, set, clear );
   UNLOCK_I2C( pChip );
   return ret;
}

/*!----------------------------------------------------------------------------
 * @see ads7924core.h
 */
int readAnalogValue( ADC_CHANNEL_T* poCannel )
{
   u8 analog[2];
   ssize_t ret;

   //STATIC_ASSERT( sizeof( poCannel->result.value ) == sizeof( analog ) );

   BUG_ON( poCannel->cannelNumber < 0 );
   BUG_ON( poCannel->cannelNumber >= ARRAY_SIZE( g_ads7924InternList ) );

   LOCK_I2C( poCannel->pParent );
   ret = _readAdcRegister( poCannel->pParent->pI2cSlave,
                           g_ads7924InternList[poCannel->cannelNumber].dataAddrUpper,
                           analog,
                           sizeof( analog ));
   UNLOCK_I2C( poCannel->pParent );
   mutex_lock( &poCannel->result.oMutex );
   poCannel->result.isValid = (ret == sizeof( analog ));
   if( poCannel->result.isValid )
   {
   #if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || (__BYTE_ORDER__ == __ORDER_PDP_ENDIAN__)
     /*
      * If the bit size doesn't exceed 16 bit so we can handle the byte order
      * "PDP_ENDIAN" like "LITTLE_ENDIAN".
      */
     ((u8*)&poCannel->result.value)[0] = analog[1];
     ((u8*)&poCannel->result.value)[1] = analog[0];
   #elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
     ((u8*)&poCannel->result.value)[0] = analog[0];
     ((u8*)&poCannel->result.value)[1] = analog[1];
   #else
     #error "Extremely fatal: Byte order (little or big endian) is unclear!"
   #endif
      poCannel->result.value >>= 4;
      DEBUG_MESSAGE( ": Analog-value of channel %d: 0x%02X%02X -> 0x%04X\n",
                     poCannel->cannelNumber,
                     analog[0], analog[1],
                     poCannel->result.value
                   );
   }
   mutex_unlock( &poCannel->result.oMutex );

   return (ret < 0)? -1: 0;
}

/*!----------------------------------------------------------------------------
 */
int adcWriteUpperLimitThreshold( ADC_CHANNEL_T* poCannel, u8 threshold )
{
   int ret;

   BUG_ON( poCannel->cannelNumber < 0 );
   BUG_ON( poCannel->cannelNumber >= ARRAY_SIZE( g_ads7924InternList ) );

   LOCK_I2C( poCannel->pParent );
   ret = _writeAdcRegister( poCannel->pParent->pI2cSlave,
                            g_ads7924InternList[poCannel->cannelNumber].upperLimit,
                            &threshold,
                            sizeof( threshold ));
   UNLOCK_I2C( poCannel->pParent );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int adcReadUpperLimitThreshold( ADC_CHANNEL_T* poCannel, u8* pThreshold )
{
   int ret;

   BUG_ON( poCannel->cannelNumber < 0 );
   BUG_ON( poCannel->cannelNumber >= ARRAY_SIZE( g_ads7924InternList ) );

   LOCK_I2C( poCannel->pParent );
   ret = _readAdcRegister( poCannel->pParent->pI2cSlave,
                           g_ads7924InternList[poCannel->cannelNumber].upperLimit,
                           pThreshold,
                           sizeof( u8 ) );
   UNLOCK_I2C( poCannel->pParent );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int adcWriteLowerLimitThreshold( ADC_CHANNEL_T* poCannel, u8 threshold )
{
   int ret;

   BUG_ON( poCannel->cannelNumber < 0 );
   BUG_ON( poCannel->cannelNumber >= ARRAY_SIZE( g_ads7924InternList ) );

   LOCK_I2C( poCannel->pParent );
   ret = _writeAdcRegister( poCannel->pParent->pI2cSlave,
                            g_ads7924InternList[poCannel->cannelNumber].lowerLimit,
                            &threshold,
                            sizeof( threshold ));
   UNLOCK_I2C( poCannel->pParent );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int adcReadLowerLimitThreshold( ADC_CHANNEL_T* poCannel, u8* pThreshold )
{
   int ret;

   BUG_ON( poCannel->cannelNumber < 0 );
   BUG_ON( poCannel->cannelNumber >= ARRAY_SIZE( g_ads7924InternList ) );

   LOCK_I2C( poCannel->pParent );
   ret = _readAdcRegister( poCannel->pParent->pI2cSlave,
                           g_ads7924InternList[poCannel->cannelNumber].lowerLimit,
                           pThreshold,
                           sizeof( u8 ) );
   UNLOCK_I2C( poCannel->pParent );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int adcAlarmEnable( ADC_CHANNEL_T* poCannel )
{
   int ret;

   BUG_ON( poCannel->cannelNumber < 0 );
   BUG_ON( poCannel->cannelNumber >= ARRAY_SIZE( g_ads7924InternList ) );

   ret = adcEditIntCtrl( poCannel->pParent,
                         g_ads7924InternList[poCannel->cannelNumber].enableMask,
                         0 );
   return ret;
}

/*!----------------------------------------------------------------------------
 */
int adcAlarmDisable( ADC_CHANNEL_T* poCannel )
{
   int ret;

   BUG_ON( poCannel->cannelNumber < 0 );
   BUG_ON( poCannel->cannelNumber >= ARRAY_SIZE( g_ads7924InternList ) );

   ret = adcEditIntCtrl( poCannel->pParent,
                         0,
                         g_ads7924InternList[poCannel->cannelNumber].enableMask );
   return ret;
}


//================================== EOF ======================================