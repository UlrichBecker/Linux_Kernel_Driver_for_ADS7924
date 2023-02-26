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
 * @file ads7924driver.c
 * @author Ulrich Becker
 * @copyright www.INKATRON.de
 * @date 2017.04.24
 * @brief Linux-driver-initialization for AD-Converter ADS7924
 * @see ads7924driver.h
 * @example ads7924test.c
 */
#include "ads7924driver.h"
#include "ads7924fileIo.h"
#include "ads7924core.h"
#include "ads7924Irq.h"
#include "ads7924_dev_tree_names.h"
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#ifndef VERSION
   #error Macro VERSION for version-string is not defined in Makefile!
#endif
#define __VERSION TS( VERSION )

#define CHIP_NAME "ADS7924"

#if defined( _ADS7924_NO_DEV_TREE ) && !defined(__DOXYGEN__)

#ifdef CONFIG_ADS7924_USE_0A0
  #define _USE_0A0 true
#else
  #define _USE_0A0 false
#endif

#ifdef CONFIG_ADS7924_USE_0A1
  #define _USE_0A1 true
#else
  #define _USE_0A1 false
#endif

#ifdef CONFIG_ADS7924_USE_0A2
  #define _USE_0A2 true
#else
  #define _USE_0A2 false
#endif

#ifdef CONFIG_ADS7924_USE_0A3
  #define _USE_0A3 true
#else
  #define _USE_0A3 false
#endif

#ifdef CONFIG_ADS7924_USE_0B0
  #define _USE_0B0 true
#else
  #define _USE_0B0 false
#endif

#ifdef CONFIG_ADS7924_USE_0B1
  #define _USE_0B1 true
#else
  #define _USE_0B1 false
#endif

#ifdef CONFIG_ADS7924_USE_0B2
  #define _USE_0B2 true
#else
  #define _USE_0B2 false
#endif

#ifdef CONFIG_ADS7924_USE_0B3
  #define _USE_0B3 true
#else
  #define _USE_0B3 false
#endif

#ifdef CONFIG_ADS7924_USE_1A0
  #define _USE_1A0 true
#else
  #define _USE_1A0 false
#endif

#ifdef CONFIG_ADS7924_USE_1A1
  #define _USE_1A1 true
#else
  #define _USE_1A1 false
#endif

#ifdef CONFIG_ADS7924_USE_1A2
  #define _USE_1A2 true
#else
  #define _USE_1A2 false
#endif

#ifdef CONFIG_ADS7924_USE_1A3
  #define _USE_1A3 true
#else
  #define _USE_1A3 false
#endif

#ifdef CONFIG_ADS7924_USE_1B0
  #define _USE_1B0 true
#else
  #define _USE_1B0 false
#endif

#ifdef CONFIG_ADS7924_USE_1B1
  #define _USE_1B1 true
#else
  #define _USE_1B1 false
#endif

#ifdef CONFIG_ADS7924_USE_1B2
  #define _USE_1B2 true
#else
  #define _USE_1B2 false
#endif

#ifdef CONFIG_ADS7924_USE_1B3
  #define _USE_1B3 true
#else
  #define _USE_1B3 false
#endif

#ifdef CONFIG_ADS7924_USE_2A0
  #define _USE_2A0 true
#else
  #define _USE_2A0 false
#endif

#ifdef CONFIG_ADS7924_USE_2A1
  #define _USE_2A1 true
#else
  #define _USE_2A1 false
#endif

#ifdef CONFIG_ADS7924_USE_2A2
  #define _USE_2A2 true
#else
  #define _USE_2A2 false
#endif

#ifdef CONFIG_ADS7924_USE_2A3
  #define _USE_2A3 true
#else
  #define _USE_2A3 false
#endif

#ifdef CONFIG_ADS7924_USE_2B0
  #define _USE_2B0 true
#else
  #define _USE_2B0 false
#endif

#ifdef CONFIG_ADS7924_USE_2B1
  #define _USE_2B1 true
#else
  #define _USE_2B1 false
#endif

#ifdef CONFIG_ADS7924_USE_2B2
  #define _USE_2B2 true
#else
  #define _USE_2B2 false
#endif

#ifdef CONFIG_ADS7924_USE_2B3
  #define _USE_2B3 true
#else
  #define _USE_2B3 false
#endif

#ifdef CONFIG_ADS7924_USE_3A0
  #define _USE_3A0 true
#else
  #define _USE_3A0 false
#endif

#ifdef CONFIG_ADS7924_USE_3A1
  #define _USE_3A1 true
#else
  #define _USE_3A1 false
#endif

#ifdef CONFIG_ADS7924_USE_3A2
  #define _USE_3A2 true
#else
  #define _USE_3A2 false
#endif

#ifdef CONFIG_ADS7924_USE_3A3
  #define _USE_3A3 true
#else
  #define _USE_3A3 false
#endif

#ifdef CONFIG_ADS7924_USE_3B0
  #define _USE_3B0 true
#else
  #define _USE_3B0 false
#endif

#ifdef CONFIG_ADS7924_USE_3B1
  #define _USE_3B1 true
#else
  #define _USE_3B1 false
#endif

#ifdef CONFIG_ADS7924_USE_3B2
  #define _USE_3B2 true
#else
  #define _USE_3B2 false
#endif

#ifdef CONFIG_ADS7924_USE_3B3
  #define _USE_3B3 true
#else
  #define _USE_3B3 false
#endif

/*!----------------------------------------------------------------------------
 * @brief Use-list item-type
 *
 * It tells whether a ADS7924-chip is present on the concerning
 * I2C-bus, and which analog-channel  will be used.
 *
 * @note  This structure will only compiled and used, if the device-tree
 *        will not used (CONFIG_ADS7924_NO_DEV_TREE=Y or CONFIG_OF=N).
 */
typedef struct
{
  ADC_CONST int  busNumber;
  ADC_CONST bool isPresent[ADC_CHIPS_PER_BUS][ADC_CHANNELS_PER_CHIP];
} I2C_LIST_ITEM;

/*!----------------------------------------------------------------------------
 * @brief Initializer for use list item in the case that no device tree is
 *        supported.
 * @note  This initialization will only compiled and used, if the device-tree
 *        will not used (CONFIG_ADS7924_NO_DEV_TREE=Y or CONFIG_OF=N).
 */
#define ADC_USE_LIST_INITIALIZER( n ) \
{                                     \
   .busNumber = n,                    \
   .isPresent =                       \
   {                                  \
      {                               \
         _USE_##n##A0,                \
         _USE_##n##A1,                \
         _USE_##n##A2,                \
         _USE_##n##A3                 \
      },                              \
      {                               \
         _USE_##n##B0,                \
         _USE_##n##B1,                \
         _USE_##n##B2,                \
         _USE_##n##B3                 \
      }                               \
   }                                  \
}

/*!----------------------------------------------------------------------------
 * @brief Supposed possible maximum of I2C- buses
 * @note  This initialization will only compiled and used, if the device-tree
 *        will not used (CONFIG_ADS7924_NO_DEV_TREE=Y or CONFIG_OF=N).
 */
#define MAX_I2C_BUSES 4

/*!----------------------------------------------------------------------------
 * @note  This initialization will only compiled and used, if the device-tree
 *        will not used (CONFIG_ADS7924_NO_DEV_TREE=Y or CONFIG_OF=N).
 */
static const __initdata I2C_LIST_ITEM  mg_I2cUseList[MAX_I2C_BUSES] =
{
   ADC_USE_LIST_INITIALIZER( 0 ),
   ADC_USE_LIST_INITIALIZER( 1 ),
   ADC_USE_LIST_INITIALIZER( 2 ),
   ADC_USE_LIST_INITIALIZER( 3 )
};
#endif /* ifdef _ADS7924_NO_DEV_TREE */

/*!----------------------------------------------------------------------------
 * @brief Initializing of the global variables.
 */
GLOBAL_T g_data =
{
   .pName         = DEVICE_BASE_FILE_NAME,
   .maxMinor      = 0,
#if defined( _ADS7924_NO_DEV_TREE ) || defined(__DOXYGEN__)
   .adcInterrupt.gpioPin = CONFIG_ADS7924_INTERRUPT_GPIO,
#endif
   .pI2cBusAncor = NULL,
   .error = 0
};

static struct i2c_device_id mg_ads7924DeviceId[] =
{
   { DEVICE_BASE_FILE_NAME, 0 },
   {}
};

MODULE_DEVICE_TABLE( i2c, mg_ads7924DeviceId );

#if !defined( _ADS7924_NO_DEV_TREE ) || defined(__DOXYGEN__)
/*!----------------------------------------------------------------------------
 * @note  This initialization will only compiled and used, if the device-tree
 *        will used (CONFIG_ADS7924_NO_DEV_TREE=N or CONFIG_OF=N).
 */
static const struct of_device_id mg_ads7924dtIds[] =
{
   { .compatible = ADS7924_DT_COMPATIBLE },
   {}
};

MODULE_DEVICE_TABLE( of, mg_ads7924dtIds );

static int onAds7924probe( struct i2c_client* pI2cClient,
                           const struct i2c_device_id* pId );
static int onAds7924remove( struct i2c_client* pI2cClient );

#endif /* ifndef _ADS7924_NO_DEV_TREE */

/*!----------------------------------------------------------------------------
 * @note  This function will only compiled and used, if no device-tree
 *        will used (CONFIG_ADS7924_NO_DEV_TREE=Y or CONFIG_OF=n).
 */
static struct i2c_driver mg_ads7924Driver =
{
   .driver =
   {
      .name  = DEVICE_BASE_FILE_NAME,
      .owner = THIS_MODULE
   #ifndef _ADS7924_NO_DEV_TREE
      ,
      .of_match_table = of_match_ptr( mg_ads7924dtIds )
   #endif
   },
   .id_table = mg_ads7924DeviceId
#ifndef _ADS7924_NO_DEV_TREE
   ,
   .probe    = onAds7924probe,
   .remove   = onAds7924remove
#endif
};

#if defined( _ADS7924_NO_DEV_TREE ) || defined(__DOXYGEN__)
/*!----------------------------------------------------------------------------
 * @brief Checks whether at least one of the 4 analog channels will be used.
 *
 * Helper function used in buildObjects()
 *
 * @note  This function will only compiled and used, if no device-tree
 *        will used (CONFIG_ADS7924_NO_DEV_TREE=Y or CONFIG_OF=n).
 *
 * @param busNumber I2C-bus-number
 * @param chipIndex 0 or 1
 * @retval true Chip will used because at least one analog-channels will used.
 * @retval false Chip will not used.
 */
static bool __init isAdcChannelUsed( int busNumber, int chipIndex )
{
   int i;
   BUG_ON( chipIndex >= ADC_CHIPS_PER_BUS );

   for( i = 0; i < ADC_CHANNELS_PER_CHIP; i++ )
   {
      if( mg_I2cUseList[busNumber].isPresent[chipIndex][i] )
         return true;
   }
   return false;
}
#endif /* ifdef  _ADS7924_NO_DEV_TREE */

/*!----------------------------------------------------------------------------
 * @brief Function queries the status-byte of the ADS7924 and checks whether
 *        it is valid or not.
 * @retval ==0 Received status-byte is valid ADS7024 is present.
 * @retval <0 Device ADS7924 not present.
 */
static inline int verifyStatus( struct i2c_client* pI2cChannel )
{
   int i;
   u8 adcStatusByte;

   if( _adcReadStatusByte( pI2cChannel, &adcStatusByte ) < 0 )
   {
      return -ENODEV;
   }
   DEBUG_MESSAGE( ": Status byte: 0x%02X\n", adcStatusByte );

   for( i = 0; i < ARRAY_SIZE( g_ads7924i2cAddrMap ); i++ )
   {
      if( (pI2cChannel->addr == g_ads7924i2cAddrMap[i].addr) &&
          (adcStatusByte     == g_ads7924i2cAddrMap[i].status) )
      {
         return 0;
      }
   }
   ERROR_MESSAGE( ": Status-byte 0x%02X doesn't match!\n", adcStatusByte );
   return -ENODEV;
}

#ifdef _ADS7924_NO_DEV_TREE
 #define ADS7924_KFREE( ptr ) kfree( ptr )
#else
 #define ADS7924_KFREE( ptr )
#endif
/*!----------------------------------------------------------------------------
 * @brief Function gets already allocated memory free.
 */
static void allFree( void )
{
   int chipNumber;
   int channelNumber;
   BUS_T* pI2cBusTmp;
   BUS_T* pI2cBus = g_data.pI2cBusAncor;

   while( pI2cBus != NULL )
   {
      pI2cBusTmp = pI2cBus->pNext;
      for( chipNumber = 0; chipNumber < ARRAY_SIZE( pI2cBus->paChip ); chipNumber++ )
      {
         if( pI2cBus->paChip[chipNumber] == NULL )
            continue;

      #ifdef _ADS7924_NO_DEV_TREE
        // Not necessary will accomplished by unregister I2C-Device.
        // if( pI2cBus->paChip[chipNumber]->pI2cSlave->irq != 0 )
        //    free_irq( pI2cBus->paChip[chipNumber]->pI2cSlave->irq, NULL );
      #endif

         for( channelNumber = 0; channelNumber < ADC_CHANNELS_PER_CHIP; channelNumber++ )
         {
            if( pI2cBus->paChip[chipNumber]->paChannel[channelNumber] == NULL )
               continue;
            DEBUG_MESSAGE( ": ADS7924_KFREE ADC_CHANNEL_T Minor: %d\n",
                           pI2cBus->paChip[chipNumber]->paChannel[channelNumber]->minor );
            device_destroy( g_data.pClass,
                            g_data.deviceNumber | 
                            pI2cBus->paChip[chipNumber]->paChannel[channelNumber]->minor );
            ADS7924_KFREE( pI2cBus->paChip[chipNumber]->paChannel[channelNumber] );
            pI2cBus->paChip[chipNumber]->paChannel[channelNumber] = NULL;
         }
      #ifdef _ADS7924_NO_DEV_TREE
         if( pI2cBus->paChip[chipNumber]->pI2cSlave != NULL )
         {
            DEBUG_MESSAGE( ": Unregister I2C-device \"%s\" by address 0x%02X\n",
                           pI2cBus->paChip[chipNumber]->pI2cSlave->name,
                           pI2cBus->paChip[chipNumber]->pI2cSlave->addr );
            /*
             * Implicitly invoking of onAds7924I2cRemove
             */
            i2c_unregister_device( pI2cBus->paChip[chipNumber]->pI2cSlave );
         }
      #endif
         DEBUG_MESSAGE( ": ADS7924_KFREE ADS7924_T Minor: %d\n",
                        pI2cBus->paChip[chipNumber]->minor );
         device_destroy( g_data.pClass,
                         g_data.deviceNumber | pI2cBus->paChip[chipNumber]->minor );
         ADS7924_KFREE( pI2cBus->paChip[chipNumber] );
         pI2cBus->paChip[chipNumber] = NULL;
      }
      DEBUG_MESSAGE( ": ADS7924_KFREE BUS_T number %d\n", pI2cBus->pI2cAdapter->nr );
      ADS7924_KFREE( pI2cBus );
      pI2cBus = pI2cBusTmp;
   }
   g_data.pI2cBusAncor = NULL;
}

#if 0
//void (* release) (struct device *dev);
static void onDeviceRelease( struct device* pDev )
{
   DEBUG_MESSAGE( ": *******\n" );
}
#endif

/*!----------------------------------------------------------------------------
 * @brief Function builds the channel-objects of the given chip-object .
 * @param poChip Pointer of chip-object
 * @retval <0: Error
 * @retval >0: Actual minor number (add on the minor of poChip).
 */
static int _ADS7924_INIT buildChannelObjects( ADS7924_T* poChip )
{
   int i;
#ifndef _ADS7924_NO_DEV_TREE
   char textBuffer[16];
   size_t len;
#endif

   BUG_ON( poChip == NULL );

   for( i = 0; i < ADC_CHANNELS_PER_CHIP; i++ )
   {
      BUG_ON( poChip->paChannel[i] != NULL );
#ifdef _ADS7924_NO_DEV_TREE
      if( !mg_I2cUseList[poChip->pParent->pI2cAdapter->nr].isPresent[poChip->number][i] )
      {
         DEBUG_MESSAGE( ": Channel %s%d%c%d not used!\n",
                        g_data.pName,
                        poChip->pParent->pI2cAdapter->nr,
                        'A' + poChip->number,
                        i );
         continue;
      }
#else
      snprintf( textBuffer, sizeof(textBuffer), TS(ADS7924_DT_CHANNEL_PROPERTY)"%d", i );
      if( of_get_property( poChip->pI2cSlave->dev.of_node, textBuffer, &len ) == NULL )
      {
         DEBUG_MESSAGE( ": Property %s not present -> Channel %s%d%c%d not used!\n",
                         textBuffer,
                         g_data.pName,
                         poChip->pParent->pI2cAdapter->nr,
                         'A' + poChip->number,
                         i );
         continue;
      }
#endif
      g_data.maxMinor++;
      DEBUG_MESSAGE( ": Create object for %s%d%c%d Minor: %d\n",
                      g_data.pName,
                      poChip->pParent->pI2cAdapter->nr,
                      'A' + poChip->number, 
                      i,
                      g_data.maxMinor );

#ifdef _ADS7924_NO_DEV_TREE
      poChip->paChannel[i] = kzalloc( sizeof( ADC_CHANNEL_T ), GFP_KERNEL );
#else
      poChip->paChannel[i] = devm_kzalloc( &poChip->pI2cSlave->dev, 
                                           sizeof( ADC_CHANNEL_T ),
                                           GFP_KERNEL );
#endif
      if( poChip->paChannel[i] == NULL )
      {
         ERROR_MESSAGE( ": Can not allocate ADC_CHANNEL_T for channel %d\n", i );
         allFree();
         return -ENOMEM;
      }

     // poChip->pI2cSlave->dev.release = onDeviceRelease;

      poChip->paChannel[i]->pParent = poChip;
      poChip->paChannel[i]->minor = g_data.maxMinor;
      poChip->paChannel[i]->cannelNumber = i;
      atomic_set( &poChip->paChannel[i]->openCounter, 0 );
      poChip->paChannel[i]->outputFormat = CONFIG_ADS7924_DEFAULT_OUTPUT_FORMAT;
      initWaitQueue( &poChip->paChannel[i]->waitQueue );
      mutex_init( &poChip->paChannel[i]->result.oMutex );
      mutex_init( &poChip->paChannel[i]->oMutex );
      poChip->paChannel[i]->result.isValid = false;
   }
   return 0;
}

#if defined( _ADS7924_NO_DEV_TREE ) || defined(__DOXYGEN__)
/*!----------------------------------------------------------------------------
 * @brief Builds the chip objects ADS7924 (at most 2) for the
 *        given I2C-bus-object.
 *
 * @note  This function will only compiled and used, if no device-tree
 *        will used (CONFIG_ADS7924_NO_DEV_TREE=Y or CONFIG_OF=n).
 *
 * @param poI2cBus Pointer I2C-Bus-object
 * @param pStartMinor Pointer to the current minor-number.
 * @retval ==0: OK
 * @retval <=0: Error
 */
static int __init buildChipObjects( BUS_T* poI2cBus )
{
   int i;

   BUG_ON( poI2cBus == NULL );

   for( i = 0; i < ADC_CHIPS_PER_BUS; i++ )
   {
      if( !isAdcChannelUsed( poI2cBus->pI2cAdapter->nr, i ) )
      {
         DEBUG_MESSAGE( ": Chip %s%d%c not present or not used.\n",
                        g_data.pName,
                        poI2cBus->pI2cAdapter->nr,
                        'A' + i );
         continue;
      }
      DEBUG_MESSAGE( ": Create %s%d%c Minor: %d\n",
                     g_data.pName,
                     poI2cBus->pI2cAdapter->nr,
                     'A' + i,
                     g_data.maxMinor
                   );

      poI2cBus->paChip[i] = kzalloc( sizeof( ADS7924_T ), GFP_KERNEL );
      if( poI2cBus->paChip[i] == NULL )
      {
         ERROR_MESSAGE( ": Can not allocate ADS7924_T for i2c-bus number %d\n", i );
         allFree();
         return -ENOMEM;
      }
      poI2cBus->paChip[i]->minor   = g_data.maxMinor;
      poI2cBus->paChip[i]->number  = i;
      atomic_set( &poI2cBus->paChip[i]->openCounter, 0 );
      poI2cBus->paChip[i]->pParent = poI2cBus;
      mutex_init( &poI2cBus->paChip[i]->oI2cMutex );
      strncpy( poI2cBus->paChip[i]->i2cBoardInfo.type, g_data.pName, I2C_NAME_SIZE );

      BUG_ON( i >= ARRAY_SIZE( g_ads7924i2cAddrMap ) );
      poI2cBus->paChip[i]->i2cBoardInfo.addr = g_ads7924i2cAddrMap[i].addr; //TODO

      DEBUG_MESSAGE( ": Creating I2C-device \"%s\" by address 0x%02X in bus: %d\n",
                      poI2cBus->paChip[i]->i2cBoardInfo.type,
                      poI2cBus->paChip[i]->i2cBoardInfo.addr,
                      poI2cBus->pI2cAdapter->nr
                   );

      /*
       * Implicitly invoking of function "onAds7924I2cProbe()"...
       */
      poI2cBus->paChip[i]->pI2cSlave = i2c_new_device( poI2cBus->pI2cAdapter, &poI2cBus->paChip[i]->i2cBoardInfo );
      if( poI2cBus->paChip[i]->pI2cSlave == NULL )
      {
         ERROR_MESSAGE( ": i2c_new_device() failed on bus %d I2C-address: 0x%02X\n",
                        poI2cBus->pI2cAdapter->nr,
                        poI2cBus->paChip[i]->i2cBoardInfo.addr );
         allFree();
         return -EIO;
      }
      if( g_data.error != 0 )
      {
         allFree();
         return g_data.error;
      }

      if( verifyStatus( poI2cBus->paChip[i]->pI2cSlave ) < 0 )
      {
         allFree();
         return -ENODEV;
      }

      if( _adcChipReset( poI2cBus->paChip[i]->pI2cSlave ) < 0 )
      {
         allFree();
         return -EIO;
      }

      i2c_set_clientdata( poI2cBus->paChip[i]->pI2cSlave, poI2cBus->paChip[i] );

      g_data.error = buildChannelObjects( poI2cBus->paChip[i] );
      if( g_data.error < 0 )
         return g_data.error;

      g_data.maxMinor++;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @brief Function creates all device-objects related to the appropriate
 *        device-files.
 * @note  This function will only compiled and used, if no device-tree
 *        will used (CONFIG_ADS7924_NO_DEV_TREE=Y or CONFIG_OF=n).
 *
 * @see mg_I2cUseList
 */
static int __init buildObjects( void )
{
   int i;
   BUS_T* pI2cBus;
   BUS_T* pI2cBusLast = NULL;

   /*
    * Creating the I2C-bus objects.
    */
   for( i = 0; i < MAX_I2C_BUSES; i++ )
   {
      if( !(isAdcChannelUsed( i, 0 ) || isAdcChannelUsed( i, 1 )) )
      {
         DEBUG_MESSAGE( ": No ADC- Device on I2C %d\n", i );
         continue;
      }
      pI2cBus = g_data.pI2cBusAncor;
      while( pI2cBus != NULL )
      {
         pI2cBusLast = pI2cBus;
         pI2cBus = pI2cBus->pNext;
      }

      pI2cBus = kzalloc( sizeof( BUS_T ), GFP_KERNEL );
      if( pI2cBus == NULL )
      {
         ERROR_MESSAGE( ": Can not allocate BUS_T \n" );
         allFree();
         return -ENOMEM;
      }

      if( g_data.pI2cBusAncor == NULL )
         g_data.pI2cBusAncor = pI2cBus;
      pI2cBus->pNext = NULL;
      if( pI2cBusLast != NULL )
         pI2cBusLast->pNext = pI2cBus;

      pI2cBus->pI2cAdapter = i2c_get_adapter( i );
      if( pI2cBus->pI2cAdapter == NULL )
      {
         ERROR_MESSAGE( ": i2c_get_adapter( %d ) failed!\n", i );
         allFree();
         return -ENODEV;
      }

      /*
       * Creating the chip object ADS7924 (at most 2) for the actual I2C-bus.
       */
      g_data.error = buildChipObjects( pI2cBus ); //, &minor );
      if( g_data.error < 0 )
         return g_data.error;
   } /* End for( i = 0; i < ARRAY_SIZE( mg_I2cUseList ); i++ ) */
   return 0;
}
#endif /* ifdef _ADS7924_NO_DEV_TREE */

/*!----------------------------------------------------------------------------
 * @brief Builds all device-files appearing in /dev/
 */
static int _ADS7924_INIT buildDeviceFiles( void )
{
   int            chipIndex;
   int            adcChannelIndex;
   BUS_T*         pI2cBus;
   ADS7924_T*     pAds7924;
   ADC_CHANNEL_T* pChannel;

   BUG_ON( g_data.pClass == NULL );
   BUG_ON( (g_data.deviceNumber & 0xFF) != 0 );

   FOR_EACH_I2C_BUS( pI2cBus )
   {
      for( chipIndex = 0; chipIndex < ADC_CHIPS_PER_BUS; chipIndex++ )
      {
         pAds7924 = pI2cBus->paChip[chipIndex];
         if( pAds7924 == NULL )
            continue;

         if( device_create( g_data.pClass,
                            NULL,
                            g_data.deviceNumber | pAds7924->minor,
                            NULL,
                            "%s%d%c",
                            g_data.pName,
                            pI2cBus->pI2cAdapter->nr,
                            'A' + chipIndex
                          )
            == NULL )
         {
            ERROR_MESSAGE( ": Can not create device-file %s%d%c\n",
                           g_data.pName,
                           pI2cBus->pI2cAdapter->nr,
                           'A' + chipIndex );
            allFree();
            return -EIO;
         }

         for( adcChannelIndex = 0; adcChannelIndex < ADC_CHANNELS_PER_CHIP; adcChannelIndex++ )
         {
            pChannel = pAds7924->paChannel[adcChannelIndex];
            if( pChannel == NULL )
               continue;

            if( device_create( g_data.pClass,
                               NULL,
                               g_data.deviceNumber | pChannel->minor,
                               NULL,
                               "%s%d%c%d",
                               g_data.pName,
                               pI2cBus->pI2cAdapter->nr,
                               'A' + chipIndex,
                               pChannel->cannelNumber
                             )
                == NULL )
            {
               ERROR_MESSAGE( ": Can not create device-file %s%d%c%d\n",
                              g_data.pName,
                              pI2cBus->pI2cAdapter->nr,
                              'A' + chipIndex,
                              pChannel->cannelNumber );
               allFree();
               return -EIO;
            }
         } /* End channel-loop */
      } /* End chip-loop */
   } /* End bus-loop */
   return 0;
}

#if !defined( _ADS7924_NO_DEV_TREE ) || defined(__DOXYGEN__)
/*!----------------------------------------------------------------------------
 * @note  This function will only compiled and used, if the device-tree
 *        will used (CONFIG_ADS7924_NO_DEV_TREE=N and CONFIG_OF=Y).
 */
static inline ADS7924_T* allocateAds7924Object( struct i2c_client* pI2cChannel )
{
   BUS_T* pI2cBusLast;
   BUS_T* pI2cBus;
   int    number;

   for( number = 0; number < ARRAY_SIZE( g_ads7924i2cAddrMap ); number++ )
   {
      if( g_ads7924i2cAddrMap[number].addr == pI2cChannel->addr )
         break;
   }
   if( number == ARRAY_SIZE( g_ads7924i2cAddrMap ) )
   {
      ERROR_MESSAGE( ": I2C-address 0x%02X doesn't match!\n", pI2cChannel->addr );
      return NULL;
   }

   pI2cBusLast = NULL;
   pI2cBus = g_data.pI2cBusAncor;
   while( pI2cBus != NULL )
   {
      pI2cBusLast = pI2cBus;
      BUG_ON( pI2cBus->pI2cAdapter == NULL );
      if( pI2cBus->pI2cAdapter->nr == pI2cChannel->adapter->nr )
      {
         if( pI2cBus->paChip[number] != NULL )
         {
            ERROR_MESSAGE( ": I2C-client with address 0x%02X of bus %d already used!\n",
                           pI2cChannel->addr, pI2cBus->pI2cAdapter->nr );
            return NULL;
         }
         break;
      }
      pI2cBus = pI2cBus->pNext;
   }

   if( pI2cBus == NULL )
   {
      DEBUG_MESSAGE( ": Create I2C-object for bus %d\n",
                     pI2cChannel->adapter->nr );
      pI2cBus = devm_kzalloc( &pI2cChannel->adapter->dev, sizeof( BUS_T ), GFP_KERNEL );
      if( pI2cBus == NULL )
      {
         ERROR_MESSAGE( ": Unable to allocate memory for I2C-bus %d Address: 0x%02X\n",
                        pI2cChannel->adapter->nr, pI2cChannel->addr );
         g_data.error = -ENOMEM;
         return NULL;
      }
      pI2cBus->pI2cAdapter = pI2cChannel->adapter;
      pI2cBus->pNext = NULL;
      if( g_data.pI2cBusAncor == NULL )
         g_data.pI2cBusAncor = pI2cBus;
      if( pI2cBusLast != NULL )
         pI2cBusLast->pNext = pI2cBus;
   }

   DEBUG_MESSAGE( ": Create I2C-channel for address 0x%02X in bus %d minor: %d\n",
                   pI2cChannel->addr, pI2cChannel->adapter->nr, g_data.maxMinor );
   pI2cBus->paChip[number] = devm_kzalloc( &pI2cChannel->dev, sizeof( ADS7924_T ), GFP_KERNEL );
   if( pI2cBus->paChip[number] == NULL )
   {
      ERROR_MESSAGE( ": Unable to allocate memory for I2C-address 0x%02X of bus %d\n",
                      pI2cChannel->addr, pI2cChannel->adapter->nr );
      g_data.error = -ENOMEM;
      return NULL;
   }

   pI2cBus->paChip[number]->minor = g_data.maxMinor;
   pI2cBus->paChip[number]->number = number;
   atomic_set( &pI2cBus->paChip[number]->openCounter, 0 );
   pI2cBus->paChip[number]->pParent = pI2cBus;
   mutex_init( &pI2cBus->paChip[number]->oI2cMutex );
   pI2cBus->paChip[number]->pI2cSlave = pI2cChannel;
   i2c_set_clientdata( pI2cChannel, pI2cBus->paChip[number] );

   g_data.error = buildChannelObjects( pI2cBus->paChip[number] );
   if( g_data.error != 0 )
      return NULL;

   g_data.maxMinor++;

   return pI2cBus->paChip[number];
}

/*!----------------------------------------------------------------------------
 * @brief Callback-function becomes invoked if the appropriate
 *        compatible-property match by the related device-tree.
 * @note  This function will only compiled and used, if the device-tree
 *        will used (CONFIG_ADS7924_NO_DEV_TREE=N and CONFIG_OF=Y).
 */
static int onAds7924probe( struct i2c_client* pI2cClient,
                         const struct i2c_device_id* pId )
{
   ADS7924_T* poChip;

   DEBUG_MESSAGE( "\n" );

   g_data.error = verifyStatus( pI2cClient );
   if( g_data.error != 0 )
      return g_data.error;

   if( _adcChipReset( pI2cClient ) < 0 )
   {
      g_data.error = -EIO;
      return g_data.error;
   }

   poChip = allocateAds7924Object( pI2cClient );
   if( poChip == NULL )
      return g_data.error;

   g_data.error = initGpioInterrupt( poChip );
   if( g_data.error != 0 )
      return g_data.error;

   return 0;
}

/*!----------------------------------------------------------------------------
 * @note  This function will only compiled and used, if the device-tree
 *        will used (CONFIG_ADS7924_NO_DEV_TREE=N and CONFIG_OF=Y).
 */
static int onAds7924remove( struct i2c_client* pI2cClient )
{
   DEBUG_MESSAGE( "\n" );
   allFree();
   return 0;
}
#endif /* ifndef _ADS7924_NO_DEV_TREE */

/*===========================================================================*/
/*!----------------------------------------------------------------------------
 * @brief Driver constructor
 */
static int __init driverInit( void )
{
   INFO_MESSAGE( "Initializing %s Version " __VERSION "\n", g_data.pName );

#ifdef _ADS7924_NO_DEV_TREE
   g_data.error = buildObjects();
   if( g_data.error != 0 )
   {
      return g_data.error;
   }

   if( initGpioInterrupt() < 0 )
   {
      allFree();
      return -EIO;
   }
#endif

   g_data.error = i2c_add_driver( &mg_ads7924Driver );
   if( g_data.error != 0 )
   {
      return g_data.error;
   }

   DEBUG_MESSAGE( ": Used device files: %d\n", g_data.maxMinor );

   if( g_data.maxMinor == 0 )
   {
      ERROR_MESSAGE( ": No instances used!\n" );
      i2c_del_driver( &mg_ads7924Driver );
      return -EIO;
   }

   g_data.pObject = cdev_alloc();
   if( IS_ERR( g_data.pObject ) )
   {
      ERROR_MESSAGE( "cdev_alloc\n" );
      return -EIO;
   }

   cdev_init( g_data.pObject, &mg_fops );

   if( alloc_chrdev_region( &g_data.deviceNumber, 0, g_data.maxMinor, g_data.pName ) < 0 )
   {
      g_data.error = -EIO;
      ERROR_MESSAGE( "alloc_chrdev_region\n" );
      goto L_REMOVE_DEV;
   }

   if( cdev_add( g_data.pObject, g_data.deviceNumber, g_data.maxMinor ) )
   {
      g_data.error = -EIO;
      ERROR_MESSAGE( "cdev_add\n" );
      goto L_REMOVE_CHRDEV;
   }

  /*!
   * Register of the driver-instances visible in /sys/class/DEVICE_BASE_FILE_NAME
   */
   g_data.pClass = class_create( THIS_MODULE, g_data.pName );
   if( IS_ERR( g_data.pClass ) )
   {
      g_data.error = -EIO;
      ERROR_MESSAGE( "class_create: No udev support\n" );
      goto L_REMOVE_CHRDEV;
   }

   g_data.error = buildDeviceFiles();
   if( g_data.error != 0 )
   {
      goto L_REMOVE_CLASS;
   }

#ifdef CONFIG_PROC_FS
   g_data.poProcFile = proc_create( PROC_FS_NAME,
                                    S_IRUGO | S_IWUGO,
                                    NULL,
                                    &mg_procFileOps );
   if( IS_ERR( g_data.poProcFile ) )
   {
      ERROR_MESSAGE( ": Unable to create proc entry: /cdev_allocproc/"PROC_FS_NAME" !\n" );
      goto L_REMOVE_FILES;
   }
#endif

   return 0;

#ifdef CONFIG_PROC_FS
L_REMOVE_FILES:
#endif
   allFree();

#ifdef CONFIG_PROC_FS
   remove_proc_entry( PROC_FS_NAME, NULL );
#endif

L_REMOVE_CLASS:
   class_destroy( g_data.pClass );

L_REMOVE_DEV:
   cdev_del( g_data.pObject );

L_REMOVE_CHRDEV:
   if( g_data.maxMinor > 0 )
      unregister_chrdev_region( g_data.deviceNumber, g_data.maxMinor );

   return g_data.error;
}

/*!----------------------------------------------------------------------------
 * @brief Driver destructor
 */
static void __exit driverExit( void )
{
   INFO_MESSAGE( "Removing\n" );
#ifdef _ADS7924_NO_DEV_TREE
   if( g_data.adcInterrupt.irq > 0 )
      free_irq( g_data.adcInterrupt.irq, NULL );
   gpio_free( g_data.adcInterrupt.gpioPin );
   allFree();
#endif
#ifdef CONFIG_PROC_FS
   remove_proc_entry( PROC_FS_NAME, NULL );
#endif
   i2c_del_driver( &mg_ads7924Driver );
   class_destroy( g_data.pClass );
   cdev_del( g_data.pObject );
   unregister_chrdev_region( g_data.deviceNumber, g_data.maxMinor );
}

/*-----------------------------------------------------------------------------
 */
module_init( driverInit );
module_exit( driverExit );

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Ulrich Becker www.INKATRON.de" );
MODULE_DESCRIPTION( "Linux-driver for AD-Converter ADS7924" );
MODULE_VERSION( __VERSION );
/*================================== EOF ====================================*/
