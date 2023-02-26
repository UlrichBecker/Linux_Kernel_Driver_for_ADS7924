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
 * @file ads7924ioctl.h
 * @author Ulrich Becker
 * @copyright www.INKATRON.de
 * @date 2017.05.05
 * @brief Definition of the op-codes for ioctl() for the ADS7924 driver
 */
#ifndef _ADS7924IOCTL_H
#define _ADS7924IOCTL_H

#include <linux/ioctl.h>

#ifdef __KERNEL__
  #include <linux/module.h>
#else
  #include <stdint.h>
  #include <sys/ioctl.h>
#endif

#ifndef STATIC_ASSERT
  #define __STATIC_ASSERT__( condition, line ) \
       extern char static_assertion_on_line_##line[2*((condition)!=0)-1];

  #define STATIC_ASSERT( condition ) __STATIC_ASSERT__( condition, __LINE__)
#endif

typedef struct
{
   uint8_t b0: 1;
   uint8_t b1: 1;
   uint8_t b2: 1;
   uint8_t b3: 1;
   uint8_t b4: 1;
   uint8_t b5: 1;
   uint8_t b6: 1;
   uint8_t b7: 1;
}
#ifndef __DOXYGEN__
__attribute__ ((packed))
#endif
BIT_T;

typedef union
{
   uint8_t byte;
   BIT_T   bit;
}
#ifndef __DOXYGEN__
__attribute__ ((packed))
#endif
BIT_ACCESS_T;

/*!----------------------------------------------------------------------------
 * @brief Access-type for single bit manipulation in the registers of ADC7924.
 *
 * Example:
 * @code
 * int adcEnableIntEdgeTrigger( int fd )
 * {
 *    ADS7924_BIT_EDIT_T access = {{{0}}, {{0}}};
 *
 *    access.m_set.bit.b0 = 1;
 *
 *    return ioctl( fd, ADS7924_IOCTL_EDIT_INTCONFIG, &access );
 * }
 * @endcode
 */
typedef struct
{
   BIT_ACCESS_T m_clear;
   BIT_ACCESS_T m_set;
}
#ifndef __DOXYGEN__
__attribute__ ((packed))
#endif
ADS7924_BIT_EDIT_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( ADS7924_BIT_EDIT_T ) == 2 );
#endif

/*!----------------------------------------------------------------------------
 * @defgroup MIN_MAX_VALUES Minimum and maximum analog values of ADS7924
 * @{
 */
/*!
 * @brief Minimum analog value.
 */
#define ADS7924_MIN_VALUE (unsigned int)0x000

/*!
 * @brief Maximum analog value.
 */
#define ADS7924_MAX_VALUE (unsigned int)0xFFF

/*! @} End of defgroup MIN_MAX_VALUES */

/*!----------------------------------------------------------------------------
 * @defgroup OP_MODES Operation-modes of ADS7924
 * @see ADS7924_IOCTL_SET_MODE
 * @see ADS7924_IOCTL_GET_MODE
 * @{
 */

#define MODE5      (1 << 7)
#define MODE4      (1 << 6)
#define MODE3      (1 << 5)
#define MODE2      (1 << 4)
#define MODE1      (1 << 3)
#define MODE0      (1 << 2)

#define ADS7924_MODE_IDLE                  (0x00                                         )
#define ADS7924_MODE_AWAKE                 (MODE5                                        )
#define ADS7924_MODE_MANUAL_SINGLE         (MODE5 | MODE4                                )
#define ADS7924_MODE_MANUAL_SCAN           (MODE5 | MODE4 |                 MODE1        )
#define ADS7924_MODE_AUTO_SINGLE           (MODE5 | MODE4 |                         MODE0)
#define ADS7924_MODE_AUTO_SCAN             (MODE5 | MODE4 |                 MODE1 | MODE0)
#define ADS7924_MODE_AUTO_SINGLE_SLEEP     (MODE5 | MODE4 | MODE3 |                 MODE0)
#define ADS7924_MODE_AUTO_SCAN_SLEEP       (MODE5 | MODE4 | MODE3 |         MODE1 | MODE0)
#define ADS7924_MODE_AUTO_BURST_SCAN_SLEEP (MODE5 | MODE4 | MODE3 | MODE2 | MODE1 | MODE0)

/*! @} End of defgroup OP_MODES */

/*!----------------------------------------------------------------------------
 * @defgroup INT_CONFIG Flags of the interrupt configuration register: INTCONFIG
 * @see ADS7924_IOCTL_SET_INTCONFIG
 * @see ADS7924_IOCTL_GET_INTCONFIG
 * @see ADS7924_IOCTL_EDIT_INTCONFIG
 * @see INTCONFIG
 * @{
 */
#define AIMCNT2    (1 << 7)
#define AIMCNT1    (1 << 6)
#define AIMCNT0    (1 << 5)
#define INTCNFG1   (1 << 4)
#define INTCNFG0   (1 << 3)
#define BUSY_nINT  (1 << 2)
#define INTPOL     (1 << 1)
#define INTTRIG    (1 << 0)

/*
 * Perhaps at this point there is a misprint in the specification
 * ADS7924.pdf page 24 chapter "Interrupt Configuration Register"!
 * Therefore be careful with the following macros.
 * ADS7924_THRESHOLD_ALARM_4 and ADS7924_THRESHOLD_ALARM_7 are
 * identical - that couldn't be so! :-/
 */
#define ADS7924_AIMCNT_MASK        (AIMCNT2 | AIMCNT1 | AIMCNT0)
#if 0
#define ADS7924_ALWAYS_ALARM        0
#define ADS7924_THRESHOLD_ALARM_1  (          AIMCNT1          )
#define ADS7924_THRESHOLD_ALARM_2  (AIMCNT2                    )
#define ADS7924_THRESHOLD_ALARM_3  (AIMCNT2 | AIMCNT1          )
#define ADS7924_THRESHOLD_ALARM_4  (AIMCNT2 | AIMCNT1 | AIMCNT0)
#define ADS7924_THRESHOLD_ALARM_5  (AIMCNT2 |           AIMCNT0)
#define ADS7924_THRESHOLD_ALARM_6  (AIMCNT2 | AIMCNT1          )
#define ADS7924_THRESHOLD_ALARM_7  (AIMCNT2 | AIMCNT1 | AIMCNT0)
#else
/*
 * That seems to be correct! :-)
 */
#define ADS7924_ALWAYS_ALARM        0
#define ADS7924_THRESHOLD_ALARM_1  (                    AIMCNT0)
#define ADS7924_THRESHOLD_ALARM_2  (          AIMCNT1          )
#define ADS7924_THRESHOLD_ALARM_3  (          AIMCNT1 | AIMCNT0)
#define ADS7924_THRESHOLD_ALARM_4  (AIMCNT2                    )
#define ADS7924_THRESHOLD_ALARM_5  (AIMCNT2 |           AIMCNT0)
#define ADS7924_THRESHOLD_ALARM_6  (AIMCNT2 | AIMCNT1          )
#define ADS7924_THRESHOLD_ALARM_7  (AIMCNT2 | AIMCNT1 | AIMCNT0)
#endif

/*! @} endi defgroup INT_CONFIG */

/*!----------------------------------------------------------------------------
 * @defgroup SLEEP_CONF Flags of the sleep configuration register: SLPCONFIG
 * @see ADS7924_IOCTL_SET_SLPCONFIG
 * @see ADS7924_IOCTL_GET_SLPCONFIG
 * @see ADS7924_IOCTL_EDIT_SLPCONFIG
 * @{
 */
#define CONVCTRL   (1 << 6)
#define SLPDIV4    (1 << 5)
#define SLPMULT8   (1 << 4)
#define SLPTIME2   (1 << 2)
#define SLPTIME1   (1 << 1)
#define SLPTIME0   (1 << 0)
/*! @} End of defgroup SLEEP_CONF */

/*!
 * @defgroup ACQ_CONFIG Flags of ACQCONFIG: Acquire Configuration Register
 * @see ADS7924_IOCTL_SET_ACQCONFIG
 * @see ADS7924_IOCTL_GET_ACQCONFIG
 * @see ADS7924_IOCTL_EDIT_ACQCONFIG
 * @see ACQCONFIG
 * @{
 */
#define ACQTIME4   (1 << 4)
#define ACQTIME3   (1 << 3)
#define ACQTIME2   (1 << 2)
#define ACQTIME1   (1 << 1)
#define ACQTIME0   (1 << 0)
/*! @} End of defgroup ACQ_CONFIG */

/*!
 * @defgroup PWR_CONFIG
 * @see PWRCONFIG
 * @see ADS7924_IOCTL_SET_PWRCONFIG
 * @see ADS7924_IOCTL_GET_PWRCONFIG
 * @see ADS7924_IOCTL_EDIT_PWRCONFIG
 * @{
 */
#define CALCNTL    (1 << 7)
#define PWRCONPOL  (1 << 6)
#define PWRCONEN   (1 << 5)
#define PWRUPTIME4 (1 << 4)
#define PWRUPTIME3 (1 << 3)
#define PWRUPTIME2 (1 << 2)
#define PWRUPTIME1 (1 << 1)
#define PWRUPTIME0 (1 << 0)
/*! @} End of defgroup PWR_CONFIG*/

/*!----------------------------------------------------------------------------
 * Begin of ioctl-commands
 */

/*!
 * @brief Magic-number of all ADS7924-ioctl commands.
 */
#define ADS7924_IOCTL_MAGIC 'a' //???

/*!----------------------------------------------------------------------------
 * @defgroup IOCTL_CHIP Ioctl-commands for the entire chip-access:
 *                      /dev/adc[0-n][A-B]
 * @{
 */
/*!
 * @brief Performs a hardware chip-reset of the entire ADS7924.
 * @see RESET
 */
#define ADS7924_IOCTL_RESET            _IO( ADS7924_IOCTL_MAGIC,   0 )

/*!
 * @brief Adjusting the operation mode of the entire ADS7924.
 * @see OP_MODES
 * @see ADS7924_IOCTL_GET_MODE
 */
#define ADS7924_IOCTL_SET_MODE         _IOW( ADS7924_IOCTL_MAGIC,  1, uint8_t )

/*!
 * @brief Returns the current operation-mode of the entire ADS7924
 * @see OP_MODES
 * @see ADS7924_IOCTL_SET_MODE
 */
#define ADS7924_IOCTL_GET_MODE         _IOR( ADS7924_IOCTL_MAGIC,  2, uint8_t )

/*!
 * @brief Setting the value of the interrupt configuration register:
 *        INTCONFIG
 * @see INT_CONFIG
 */
#define ADS7924_IOCTL_SET_INTCONFIG    _IOW( ADS7924_IOCTL_MAGIC,  3, uint8_t )

/*!
 * @brief Returns the current value of the interrupt configuration register:
 *        INTCONFIG
 * @see INT_CONFIG
 */
#define ADS7924_IOCTL_GET_INTCONFIG    _IOR( ADS7924_IOCTL_MAGIC,  4, uint8_t )

/*!
 * @brief Set and/or reset of single bits in the interrupt configuration register INTCONFIG
 * @see INT_CONFIG
 * @see ADS7924_BIT_EDIT_T
 */
#define ADS7924_IOCTL_EDIT_INTCONFIG   _IOW( ADS7924_IOCTL_MAGIC,  5, ADS7924_BIT_EDIT_T )

/*!
 * @brief Set the value of the sleep configuration register SLPCONFIG
 * @see SLEEP_CONF
 * @see ADS7924_IOCTL_GET_SLPCONFIG
 * @see ADS7924_IOCTL_EDIT_SLPCONFIG
 */
#define ADS7924_IOCTL_SET_SLPCONFIG    _IOW( ADS7924_IOCTL_MAGIC,  6, uint8_t )

/*!
 * @brief Returns the value of the sleep configuration register SLPCONFIG
 * @see SLEEP_CONF
 * @see ADS7924_IOCTL_SET_SLPCONFIG
 * @see ADS7924_IOCTL_EDIT_SLPCONFIG
 */
#define ADS7924_IOCTL_GET_SLPCONFIG    _IOR( ADS7924_IOCTL_MAGIC,  7, uint8_t )

/*!
 * @brief Set and/or reset of single bits in the sleep configuration register SLPCONFIG
 * @see SLEEP_CONF
 * @see ADS7924_IOCTL_SET_SLPCONFIG
 * @see ADS7924_IOCTL_GET_SLPCONFIG
 * @see ADS7924_BIT_EDIT_T
 */
#define ADS7924_IOCTL_EDIT_SLPCONFIG   _IOW( ADS7924_IOCTL_MAGIC,  8, ADS7924_BIT_EDIT_T )

/*!
 * @brief Sets the value in the acquire configuration register
 * @see ACQ_CONFIG
 * @see ADS7924_IOCTL_GET_ACQCONFIG
 * @see ADS7924_IOCTL_EDIT_ACQCONFIG
 * @see ACQCONFIG
 */
#define ADS7924_IOCTL_SET_ACQCONFIG    _IOW( ADS7924_IOCTL_MAGIC,  9, uint8_t )

/*!
 * @brief Returns the value of the acquire configuration register
 * @see ACQ_CONFIG
 * @see ADS7924_IOCTL_SET_ACQCONFIG
 * @see ADS7924_IOCTL_EDIT_ACQCONFIG
 * @see ACQCONFIG
 */
#define ADS7924_IOCTL_GET_ACQCONFIG    _IOR( ADS7924_IOCTL_MAGIC, 10, uint8_t )

/*!
 * @brief Sets and/or resets single bits in the acquire configuration register
 * @see ACQ_CONFIG
 * @see ADS7924_IOCTL_SET_ACQCONFIG
 * @see ADS7924_IOCTL_GET_ACQCONFIG
 * @see ACQCONFIG
 */
#define ADS7924_IOCTL_EDIT_ACQCONFIG   _IOW( ADS7924_IOCTL_MAGIC, 11, ADS7924_BIT_EDIT_T )

/*!
 * @brief Set a value to the Power Configuration Register
 * @see ADS7924_IOCTL_GET_PWRCONFIG
 * @see ADS7924_IOCTL_EDIT_PWRCONFIG
 * @see PWR_CONFIG
 */
#define ADS7924_IOCTL_SET_PWRCONFIG    _IOW( ADS7924_IOCTL_MAGIC, 12, uint8_t )

/*!
 * @brief Returns a the value of the Power Configuration Register
 * @see ADS7924_IOCTL_SET_PWRCONFIG
 * @see ADS7924_IOCTL_EDIT_PWRCONFIG
 * @see PWR_CONFIG
 */
#define ADS7924_IOCTL_GET_PWRCONFIG    _IOR( ADS7924_IOCTL_MAGIC, 13, uint8_t )

/*!
 * @brief Set and/or resets single bits in the Power Configuration Register
 * @see ADS7924_IOCTL_SET_PWRCONFIG
 * @see ADS7924_IOCTL_GET_PWRCONFIG
 * @see PWR_CONFIG
 */
#define ADS7924_IOCTL_EDIT_PWRCONFIG   _IOW( ADS7924_IOCTL_MAGIC, 14, ADS7924_BIT_EDIT_T )

/*! @} End of defgroup IOCTL_CHIP */

/*!----------------------------------------------------------------------------
 * @defgroup IOCTL_CHANNEL Ioctl-commands for single channel-access:
 *                         /dev/adc[0-n][A-B][0-3]
 * @{
 */

/*!
 * @brief Sets the read-output mode in the binary-format
 * @see ADS7924_IOCTL_READMODE_DEC
 * @see ADS7924_IOCTL_READMODE_HEX
 */
#define ADS7924_IOCTL_READMODE_BIN     _IO( ADS7924_IOCTL_MAGIC, 30 )

/*!
 * @brief Sets the read-output mode in the ASCII-decimal format
 * @see ADS7924_IOCTL_READMODE_BIN
 * @see ADS7924_IOCTL_READMODE_HEX
 */
#define ADS7924_IOCTL_READMODE_DEC     _IO( ADS7924_IOCTL_MAGIC, 31 )

/*!
 * @brief Sets the read-output mode in the ASCII-hexadecimal format
 * @see ADS7924_IOCTL_READMODE_BIN
 * @see ADS7924_IOCTL_READMODE_DEC
 */
#define ADS7924_IOCTL_READMODE_HEX     _IO( ADS7924_IOCTL_MAGIC, 32 )

/*!
 * @brief Set value of Upper Limit Threshold register
 * @see ADS7924_IOCTL_SET_LLR
 */
#define ADS7924_IOCTL_SET_ULR          _IOW( ADS7924_IOCTL_MAGIC, 33, uint8_t )

/*!
 * @brief Set value Lower Limit Threshold register
 * @see ADS7924_IOCTL_SET_ULR
 */
#define ADS7924_IOCTL_SET_LLR          _IOW( ADS7924_IOCTL_MAGIC, 34, uint8_t )

/*!
 * @brief Returns the value of the Upper Limit Threshold register
 * @see ADS7924_IOCTL_GET_LLR
 */
#define ADS7924_IOCTL_GET_ULR          _IOR( ADS7924_IOCTL_MAGIC, 35, uint8_t )

/*!
 * @brief Returns the value of Lower Limit Threshold register
 * @see ADS7924_IOCTL_GET_ULR
 */
#define ADS7924_IOCTL_GET_LLR          _IOR( ADS7924_IOCTL_MAGIC, 36, uint8_t )

/*!
 * @brief Enables the alarm (interrupt) of this channel.
 * @see ADS7924_IOCTL_ALARM_DISABLE
 */
#define ADS7924_IOCTL_ALARM_ENABLE     _IO( ADS7924_IOCTL_MAGIC, 37 )

/*!
 * @brief Disables the alarm (interrupt) of this channel.
 * @see ADS7924_IOCTL_ALARM_ENABLE
 */
#define ADS7924_IOCTL_ALARM_DISABLE    _IO( ADS7924_IOCTL_MAGIC, 38 )

/*! @} End of defgroup IOCTL_CHANNEL */

#endif /* ifndef _ADS7924IOCTL_H */
/*================================== EOF ====================================*/
