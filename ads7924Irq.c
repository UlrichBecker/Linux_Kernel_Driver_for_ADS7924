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
 * @file ads7924Irq.c
 * @author Ulrich Becker
 * @copyright www.INKATRON.de
 * @brief Handling of the GPIO-interrupt for possible alarms from ADS7924
 * @date 2017.05.12
 * @see ads7924Irq.h
 */
#include "ads7924core.h"
#include "ads7924Irq.h"

/*!----------------------------------------------------------------------------
 * @brief Thread-function becomes indirectly invoked from the ADC-interrupt.
 */
#ifdef _ADS7924_NO_DEV_TREE
inline
#endif
static irqreturn_t onIrqBottomHalf( int irq, void* pData )
{
   int            adcChannelIndex;
   u8             alarmStatus;
   ADC_CHANNEL_T* pChannel;
   ADS7924_T*     pAds7924 = pData;

   if( adcReadIntCtrl( pAds7924, &alarmStatus ) < 0 )
   {
      ERROR_MESSAGE( ": adcReadIntCtrl() failed!\n" );
      return IRQ_HANDLED;
   }

   if( pAds7924->afterReset )
   {
      /* We discard the first interrupt after chip-reset */
      pAds7924->afterReset = false;
      return IRQ_HANDLED;
   }

   for( adcChannelIndex = 0; adcChannelIndex < ADC_CHANNELS_PER_CHIP; adcChannelIndex++ )
   {
      pChannel = pAds7924->paChannel[adcChannelIndex];
      if( pChannel == NULL )
         continue; /* Channel not present */

      if( atomic_read( &pChannel->openCounter ) == 0 )
         continue; /* Channel currently not open respectively not used by any application. */

      if( (g_ads7924InternList[pChannel->cannelNumber].enableMask & alarmStatus) == 0 )
         continue; /* Alarm isn't for this channel. */

      if( readAnalogValue( pChannel ) < 0 )
      {
         ERROR_MESSAGE( ": readAnalogValue() failed!\n" );
         continue;
      }

      /* Triggering select() of user-space application. */
      wakeUpChannel( pChannel ); 
   }
   return IRQ_HANDLED;
}

#ifdef _ADS7924_NO_DEV_TREE
/*!----------------------------------------------------------------------------
 * @brief Thread-function becomes indirectly invoked from the ADC-interrupt.
 */
static irqreturn_t onIrqBottomHalfLoop( int irq, void* pData )
{
   int            chipIndex;
   ADS7924_T*     pAds7924;
   BUS_T*         pI2cBus;

   DEBUG_MESSAGE( "\n" );

   FOR_EACH_I2C_BUS( pI2cBus )
   {
      for( chipIndex = 0; chipIndex < ADC_CHIPS_PER_BUS; chipIndex++ )
      {
         pAds7924 = pI2cBus->paChip[chipIndex];
         if( pAds7924 == NULL )
            continue; /* Chip is not present. */

         onIrqBottomHalf( irq, pAds7924 );
      }
   }
   return IRQ_HANDLED;
}
#endif

#ifdef _ADS7924_NO_DEV_TREE
/*-----------------------------------------------------------------------------
 */
int _ADS7924_INIT initGpioInterrupt( void )
{
   if( gpio_request( g_data.adcInterrupt.gpioPin, g_data.pName ) )
   {
      ERROR_MESSAGE( ": Unable to request GPIO-pin %d\n",
                     g_data.adcInterrupt.gpioPin );
      return -1;
   }

   if( gpio_direction_input( g_data.adcInterrupt.gpioPin ) )
   {
      ERROR_MESSAGE( ": Unable to use GPIO-pin %d as input!\n",
                      g_data.adcInterrupt.gpioPin );
      goto L_FREE_GPIO;
   }
   DEBUG_MESSAGE( ": Using GPIO-pin: %d\n", g_data.adcInterrupt.gpioPin );

   g_data.adcInterrupt.irq = gpio_to_irq( g_data.adcInterrupt.gpioPin );
   if( g_data.adcInterrupt.irq < 0 )
   {
      ERROR_MESSAGE( ": Unable to map interrupt with GPIO-pin %d!\n",
                     g_data.adcInterrupt.gpioPin );
      goto L_FREE_GPIO;
   }

   if( request_threaded_irq( g_data.adcInterrupt.irq,
                             NULL,
                             onIrqBottomHalfLoop,
                             IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
                             g_data.pName,
                             NULL ) != 0
     )
   {
      ERROR_MESSAGE( ": Unable to request interrupt number %d\n",
                     g_data.adcInterrupt.irq );
      goto L_FREE_GPIO;
   }
   DEBUG_MESSAGE( ": Using interrupt number: %d\n", g_data.adcInterrupt.irq );

   return 0;

L_FREE_GPIO:
   gpio_free( g_data.adcInterrupt.gpioPin );
   return -1;
}
#else
int _ADS7924_INIT initGpioInterrupt( ADS7924_T* pAds7924 )
{
   char name[16];

   snprintf( name, sizeof( name ), "%s%d%c",
             g_data.pName,
             pAds7924->pI2cSlave->adapter->nr,
             'A' + pAds7924->number );

   DEBUG_MESSAGE( ": Using interrupt number: %d name: %s\n",
                  pAds7924->pI2cSlave->irq, name );

   if( devm_request_threaded_irq( &pAds7924->pI2cSlave->dev,
                                  pAds7924->pI2cSlave->irq,
                                  NULL,
                                  onIrqBottomHalf,
                                  IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
                                  name,
                                  pAds7924 ) != 0
     )
   {
      ERROR_MESSAGE( ": Unable to request interrupt number %d\n",
                     pAds7924->pI2cSlave->irq );
      return -1;
   }
   return 0;
}
#endif /* #else ifdef _ADS7924_NO_DEV_TREE */

/*================================== EOF ====================================*/
