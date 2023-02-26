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
 * @file ads7924driver.h
 * @author Ulrich Becker
 * @copyright www.INKATRON.de
 * @date 2017.04.24
 * @brief Linux-driver-initialization for AD-Converter ADS7924, definition
 *        of the global objects.
 * @see ads7924driver.c
 */
#ifndef _ADS7925DRIVER_H_
#define _ADS7925DRIVER_H_

#include <linux/version.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <asm/uaccess.h>
#include "ads7924ioctl.h"
#ifdef CONFIG_PROC_FS
#include "ads7924procFs.h"
#endif

/*!
 * @brief Device base file-name appearing in
 *        /sys/class/<base-file-name>/<base-file-name>[minor-number]
 *
 */
#define DEVICE_BASE_FILE_NAME KBUILD_MODNAME

#if defined( CONFIG_PROC_FS ) || defined(__DOXYGEN__)
   /*! @brief Definition of the name in the process file system. */
   #define PROC_FS_NAME "driver/"DEVICE_BASE_FILE_NAME
#endif

#if defined( CONFIG_ADS7924_NO_DEV_TREE ) || !defined( CONFIG_OF )
   #define _ADS7924_NO_DEV_TREE
#endif

#if defined( _ADS7924_NO_DEV_TREE ) || defined(__DOXYGEN__)
  #define _ADS7924_INIT __init
  #define _ADS7924_INIT_DATA __initdata
#else
  #define _ADS7924_INIT
  #define _ADS7924_INIT_DATA
#endif
  
/* Begin of message helper macros for "dmesg" *********************************/
/* NOTE for newer systems with "systend" 
 * "dmesg -w" corresponds the old “tail -f /var/log/messages”
 */
#define ERROR_MESSAGE( constStr, n... ) \
   printk( KERN_ERR DEVICE_BASE_FILE_NAME "-systemerror %d: %s" constStr, __LINE__, __func__, ## n )

#if defined( CONFIG_DEBUG_ADS7924 ) || defined(__DOXYGEN__)
   #define DEBUG_MESSAGE( constStr, n... ) \
      printk( KERN_DEBUG DEVICE_BASE_FILE_NAME "-dbg %d: %s" constStr, __LINE__, __func__, ## n )

   #define DEBUG_ACCESSMODE( pFile ) \
      DEBUG_MESSAGE( ": access: %s\n", \
                     (pFile->f_flags & O_NONBLOCK)? "non blocking":"blocking" )
#else
   #define DEBUG_MESSAGE( constStr, n... )
   #define DEBUG_ACCESSMODE( pFile )
#endif

#define INFO_MESSAGE( constStr, n... ) \
   printk( KERN_INFO DEVICE_BASE_FILE_NAME ": " constStr, ## n )

/* End of message helper macros for "dmesg" **********************************/

#define ADC_CHIPS_PER_BUS     2 /*!<@brief Maximum ADS7924 per I2C-bus */
#define ADC_CHANNELS_PER_CHIP 4 /*!<@brief Maximum analog channels per ADS7924 */

#ifdef _ADS7924_NO_DEV_TREE
  #define ADC_CONST const
#else
  #define ADC_CONST
#endif

typedef u16 VALUE_T; /*!<@brief Type to storing analog values */
typedef u8  LIMIT_T; /*!<@brief Type for ADS7924 threshold-registers */

/*!
 * @brief Object keeps the last analog value read via the I2C bus.
 */
typedef struct
{
   struct mutex     oMutex;  /*!<@brief Race condition guard. */
   volatile VALUE_T value;   /*!<@brief Analog value. */
   volatile bool    isValid; /*!<@brief Becomes true if analog-value valid. */
} ANALOG_T;

STATIC_ASSERT( sizeof( VALUE_T ) == 2 );

/*!
 * @brief Possible output formats for the user space function "read"
 */
typedef enum
{
   OUT_BIN = 0, //!<@brief Analog value in binary-format.
   OUT_DEC = 1, //!<@brief Analog value in ASCII-decimal-format
   OUT_HEX = 2  //!<@brief Analog value in ASCII-hexadecimal-format
} OUTPUT_FORMAT_T;

/*!----------------------------------------------------------------------------
 * @brief Specialization of the Linux wait-queue for this module.
 */
typedef struct
{
   volatile bool     waiting;
   volatile bool     awoken;
   wait_queue_head_t queue;
} WAIT_QUEUE_T;

/*!---------------------------------------------------------------------------
 * @brief Sets the current process in sleeping-mode.
 */
static inline int sleepIfNecessary( WAIT_QUEUE_T* poQueue )
{
   return wait_event_interruptible( poQueue->queue, !poQueue->waiting );
}

/*!---------------------------------------------------------------------------
 * @brief Wakes a sleeping process.
 */
static inline void wakeUp( WAIT_QUEUE_T* poQueue )
{
   poQueue->waiting = false;
   poQueue->awoken  = true;
   wake_up_interruptible( &poQueue->queue );
}

/*!---------------------------------------------------------------------------
 * @brief Checks whether a process was fresh awoken.
 */
static inline bool isAwoken( WAIT_QUEUE_T* poQueue )
{
   bool ret = poQueue->awoken;
   poQueue->awoken = false;
   return ret;
}

/*!----------------------------------------------------------------------------
 * @brief Initializing of specialized wait queue.
 */
static inline void initWaitQueue( WAIT_QUEUE_T* poQueue )
{
   poQueue->waiting = false;
   poQueue->awoken  = false;
   init_waitqueue_head( &poQueue->queue );
}

struct _ADS7924_T; // Resolves the chicken egg problem...

/*!----------------------------------------------------------------------------
 * @brief Object for one of the four analog-channels of ADS7924
 */
typedef struct
{
   struct _ADS7924_T* pParent;
   int                minor;
   atomic_t           openCounter;
   int                cannelNumber;
   ANALOG_T           result;
   OUTPUT_FORMAT_T    outputFormat;
   WAIT_QUEUE_T       waitQueue;
   struct mutex       oMutex;
} ADC_CHANNEL_T;

/*!----------------------------------------------------------------------------
 * @brief Stores the analog value thread-save.
 */
static inline void setResult( ADC_CHANNEL_T* pChannel, VALUE_T value )
{
   mutex_lock( &pChannel->result.oMutex );
   pChannel->result.value = value;
   mutex_unlock( &pChannel->result.oMutex );
}

/*!----------------------------------------------------------------------------
 * @brief Get the stored analog value thread-save back.
 */
static inline VALUE_T getResult( ADC_CHANNEL_T* pChannel )
{
   VALUE_T ret;

   mutex_lock( &pChannel->result.oMutex );
   ret = pChannel->result.value;
   mutex_unlock( &pChannel->result.oMutex );
   return ret;
}

/*!---------------------------------------------------------------------------
 * @brief Put the process sleeping to which belongs the given channel-object,
 *        until he becomes awake by wakeUpChannel.
 * @see wakeUpChannel
 */
static inline int sleepChannelIfNecessary( ADC_CHANNEL_T* pChannel )
{
#ifdef CONFIG_DEBUG_ADS7924
   if( pChannel->waitQueue.waiting )
      DEBUG_MESSAGE( ": Channel %d is waiting...\n",
                     pChannel->cannelNumber );
#endif
   return sleepIfNecessary( &pChannel->waitQueue );
}

/*!----------------------------------------------------------------------------
 * @brief Wakes a sleeping process to which belongs the given channel-object.
 */
static inline void wakeUpChannel( ADC_CHANNEL_T* pChannel )
{
   wakeUp( &pChannel->waitQueue );
}

#ifdef _ADS7924_NO_DEV_TREE
/*!----------------------------------------------------------------------------
 * @brief Object handles the interrupt of possible ADS7924 alarm-events.
 * @note This type will only compiled and used, when the device-tree
 *       will not used. (CONFIG_ADS7924_NO_DEV_TREE=Y or CONFIG_OF=N)
 */
typedef struct
{
   ADC_CONST int gpioPin; //!<@brief The GPIO-input line of the ADS7924 alarms.
   int           irq;     //!<@brief From the gpioPin generated interrupt-number.
} GPIO_INTERRUPT_T;
#endif /* ifdef _ADS7924_NO_DEV_TREE */

struct _BUS_T; // Resolves the chicken egg problem...

/*!----------------------------------------------------------------------------
 * @brief Object represents a single chip of analog to
 *        digital converter ADS7924
 */
typedef struct _ADS7924_T
{
   int                   minor;
   int                   number;
   atomic_t              openCounter;
   u8                    shadowAlarmStatus;
   bool                  afterReset;
   struct _BUS_T*        pParent;
   struct i2c_board_info i2cBoardInfo;
   struct i2c_client*    pI2cSlave;
   struct mutex          oI2cMutex;
   ADC_CHANNEL_T*        paChannel[ADC_CHANNELS_PER_CHIP];
} ADS7924_T;


/*!----------------------------------------------------------------------------
 * @brief Object represents a I2C-driver
 *
 * Specialization of a Linux I2C-Driver for this module.
 * Forward chained list of hardware-I2C-buses in which
 * exist at least one analog to digital converter ADS7924.
 */
typedef struct _BUS_T
{
   struct _BUS_T*      pNext;
#ifndef _ADS7924_NO_DEV_TREE
   struct device_node* pDtNode;
#endif
   struct i2c_adapter* pI2cAdapter;
   ADS7924_T*          paChip[ADC_CHIPS_PER_BUS];
} BUS_T;

/*!----------------------------------------------------------------------------
 * @brief Browser-macro for the forward chained I2C-bus list.
 * @see BUS_T
 */
#define FOR_EACH_I2C_BUS( poBus ) \
  for( poBus = g_data.pI2cBusAncor; poBus != NULL; poBus = poBus->pNext )

/*!----------------------------------------------------------------------------
 * @brief Collection of the driver global variables.
 */
typedef struct
{
   const char*              pName; //!<@brief name-prefix appearing in /dev/...
   dev_t                    deviceNumber;
   struct cdev*             pObject;
   struct class*            pClass;
#ifdef _ADS7924_NO_DEV_TREE
   GPIO_INTERRUPT_T         adcInterrupt;
#endif
   int                      maxMinor;
   BUS_T*                   pI2cBusAncor; //!<@brief Anchor of chained list.
   volatile int             error;
#ifdef CONFIG_PROC_FS
   struct proc_dir_entry*   poProcFile;
#endif
} GLOBAL_T;

extern GLOBAL_T g_data;

#endif /* ifndef _ADS7925DRIVER_H_ */
/*================================== EOF ====================================*/
