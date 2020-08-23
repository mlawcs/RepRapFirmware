/*
 * Devices.cpp
 *
 *  Created on: 9 Jul 2020
 *      Author: David
 *  License: GNU GPL v3
 */

#include "Devices.h"
#include <RepRapFirmware.h>
#include <AnalogIn.h>
#include <AnalogOut.h>
#include <TaskPriorities.h>

#include <hal_usb_device.h>
#include <peripheral_clk_config.h>

// Analog input support
constexpr size_t AnalogInTaskStackWords = 300;
static Task<AnalogInTaskStackWords> analogInTask;

// Serial device support
Uart serialUart0(Serial0SercomNumber, Sercom0RxPad, 512, 512);
SerialCDC serialUSB(UsbVBusPin, 512, 512);

# if !defined(SERIAL0_ISR0) || !defined(SERIAL0_ISR2) || !defined(SERIAL0_ISR3)
#  error SERIAL0_ISRn not defined
# endif

void SERIAL0_ISR0() noexcept
{
	serialUart0.Interrupt0();
}

void SERIAL0_ISR2() noexcept
{
	serialUart0.Interrupt2();
}

void SERIAL0_ISR3() noexcept
{
	serialUart0.Interrupt3();
}

static void UsbInit() noexcept
{
	// Set up USB clock
	hri_gclk_write_PCHCTRL_reg(GCLK, USB_GCLK_ID, GCLK_PCHCTRL_GEN(GclkNum48MHz) | GCLK_PCHCTRL_CHEN);
	hri_mclk_set_AHBMASK_USB_bit(MCLK);
	hri_mclk_set_APBBMASK_USB_bit(MCLK);

	// Set up USB pins
	// This is the code generated by Atmel Start. I don't know whether it is all necessary.
	gpio_set_pin_direction(PortAPin(24), GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PortAPin(24), false);
	gpio_set_pin_pull_mode(PortAPin(24), GPIO_PULL_OFF);
	gpio_set_pin_function(PortAPin(24), PINMUX_PA24H_USB_DM);

	gpio_set_pin_direction(PortAPin(25), GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PortAPin(25), false);
	gpio_set_pin_pull_mode(PortAPin(25), GPIO_PULL_OFF);
	gpio_set_pin_function(PortAPin(25), PINMUX_PA25H_USB_DP);
}

static void SdhcInit() noexcept
{
	// Set up SDHC clock
	hri_mclk_set_AHBMASK_SDHC1_bit(MCLK);
	hri_gclk_write_PCHCTRL_reg(GCLK, SDHC1_GCLK_ID, GCLK_PCHCTRL_GEN(GclkNum90MHz) | GCLK_PCHCTRL_CHEN);
	hri_gclk_write_PCHCTRL_reg(GCLK, SDHC1_GCLK_ID_SLOW, GCLK_PCHCTRL_GEN(GclkNum31KHz) | GCLK_PCHCTRL_CHEN);

	// Set up SDHC pins
#if 1
	// This is the code generated by Atmel Start. I don't know whether it is all necessary.
	gpio_set_pin_direction(PortAPin(21), GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PortAPin(21), false);
	gpio_set_pin_pull_mode(PortAPin(21), GPIO_PULL_OFF);
	gpio_set_pin_function(PortAPin(21), PINMUX_PA21I_SDHC1_SDCK);

	gpio_set_pin_direction(PortAPin(20), GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PortAPin(20), false);
	gpio_set_pin_pull_mode(PortAPin(20), GPIO_PULL_OFF);
	gpio_set_pin_function(PortAPin(20), PINMUX_PA20I_SDHC1_SDCMD);

	gpio_set_pin_direction(PortBPin(18), GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PortBPin(18), false);
	gpio_set_pin_pull_mode(PortBPin(18), GPIO_PULL_OFF);
	gpio_set_pin_function(PortBPin(18), PINMUX_PB18I_SDHC1_SDDAT0);

	gpio_set_pin_direction(PortBPin(19), GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PortBPin(19), false);
	gpio_set_pin_pull_mode(PortBPin(19), GPIO_PULL_OFF);
	gpio_set_pin_function(PortBPin(19), PINMUX_PB19I_SDHC1_SDDAT1);

	gpio_set_pin_direction(PortBPin(20), GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PortBPin(20), false);
	gpio_set_pin_pull_mode(PortBPin(20), GPIO_PULL_OFF);
	gpio_set_pin_function(PortBPin(20), PINMUX_PB20I_SDHC1_SDDAT2);

	gpio_set_pin_direction(PortBPin(21), GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PortBPin(21), false);
	gpio_set_pin_pull_mode(PortBPin(21), GPIO_PULL_OFF);
	gpio_set_pin_function(PortBPin(21), PINMUX_PB21I_SDHC1_SDDAT3);
#else
	// Setup SD card interface pins
	for (Pin p : SdMciPins)
	{
		SetPinFunction(p, SdMciPinsFunction);
	}
#endif
}

// Serial interface
static void SerialInit()
{
	SetPinFunction(Serial0TxPin, Serial0PinFunction);
	SetPinFunction(Serial0RxPin, Serial0PinFunction);
	// We don't make the init call here, that's done by the GCodes module
}

void DeviceInit()
{
	// Ensure the Ethernet PHY or WiFi module is held reset
	pinMode(EthernetPhyResetPin, OUTPUT_LOW);

	UsbInit();
	SerialInit();
	SdhcInit();

	AnalogIn::Init(FirstAdcDmaChannel, DmacPrioAdcTx, DmacPrioAdcRx);
	AnalogOut::Init();
	analogInTask.Create(AnalogIn::TaskLoop, "AIN", nullptr, TaskPriority::AinPriority);
}

// End
