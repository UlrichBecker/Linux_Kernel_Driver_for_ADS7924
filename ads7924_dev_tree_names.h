/*!
 ******************************************************************************
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
 * @file ads7924_dev_tree_names.h
 * @brief Names of the device-tree-part for AD-Converter ADS7924
 * @author Ulrich Becker
 * @copyright www.INKATRON.de
 * @date 2017.04.24
 *
 * \n
 * Example of code-section in the device-tree for using the driver of
 * ADS7924:
 * @code
 * #include <ads7924_dev_tree_names.h>
 * //...
 * &i2c1 {
 *  #address-cells = <0x1>;
 *  #size-cells = <0x0>;
 *
 *  status = "okay";
 *
 *   ads7924: ads7924@48 {
 *     compatible = ADS7924_DT_COMPATIBLE;
 *     reg = <0x48>;
 *     interrupt-parent = <&gpio3>;
 *     interrupts = <18 IRQ_TYPE_EDGE_FALLING>;
 *     ADS7924_CHANNEL( 0 );
 *     ADS7924_CHANNEL( 1 );
 *     status = "okay";
 *   };
 * };
 * @endcode
 *
 * In this example the ADS7924 is connected to I2C-bus-number 1 and
 * only adc-input-channel 0 and 1 will be used.\n
 * The I2C-bus number 1 appears in the linux-kernel as number 0.\n
 * The interrupt-line is connected to GPIO-pin 18 \n
 * The preprocessor "cpp" will translate the above code-section as follow:
 * @code
 * &i2c1 {
 *    #address-cells = <0x1>;
 *    #size-cells = <0x0>;
 *    status = "okay";
 *    ads7924: ads7924@48 {
 *       compatible = "ti,ads7924";
 *       reg = <0x48>;
 *       interrupt-parent = <&gpio3>;
 *       interrupts = <18 2>;
 *       channel0;
 *       channel1;
 *       status = "okay";
 *    };
 * };
 * @endcode
 * 
 * Based on the above example, the driver will create the following
 * device-files in the folder /dev/:
 * @code
 * /dev/adc0A
 * /dev/adc0A0
 * /dev/adc0A1
 * @endcode
 */
#ifndef _ADS7924_DEV_TREE_NAMES_H_
#define _ADS7924_DEV_TREE_NAMES_H_

/*!

 */

#ifndef _TS
   /*!
    * @brief Helper macro for TS
    */
   #define _TS( s ) # s
#endif
#ifndef TS
   /*!
    * @brief Converts the parameter into a in quotation marks
    *        embedded text-string. (ToString)
    */
   #define TS( s ) _TS( s )
#endif

#ifndef _MERGE
   #define _MERGE( a, b ) a ## b
#endif
#ifndef MERGE
   /*!
    * @brief Merges both parameters into a single expression.
    */
   #define MERGE( a, b ) _MERGE( a, b )
#endif

#define ADS7924_DT_COMPATIBLE            "ti,ads7924"
#define ADS7924_DT_BASE_NODE_NAME        ads7924
#define ADS7924_DT_GPIO_INTERRUPT_INPUT  alarm_input_gpio
#define ADS7924_DT_CHANNEL_PROPERTY      channel

/*!
 * @brief Optional template for the used ADC-channel properties.
 */
#define ADS7924_CHANNEL( n ) MERGE( ADS7924_DT_CHANNEL_PROPERTY, n )

#endif /* ifndef _ADS7924_DEV_TREE_NAMES_H_ */
/*================================== EOF ====================================*/
