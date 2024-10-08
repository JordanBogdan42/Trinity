/**
 * @file LedManagerI.cpp
 * @date Mar 6, 2014
 * @author sizun
 * 
 * @note SVN tag: $Id: LedManagerI.cpp 1724 2017-11-14 08:45:16Z psizun $
 * @note Contributor(s): Patrick Sizun
 * @note 
 * @note This file is part of the GetBench software project.
 *
 * @copyright © Commissariat a l'Energie Atomique et aux Energies Alternatives (CEA)
 *
 * @par FREE SOFTWARE LICENCING
 * This software is governed by the CeCILL license under French law and abiding  * by the rules of distribution of free
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated by
 * CEA, CNRS and INRIA at the following URL: "http://www.cecill.info". As a counterpart to the access to the source code
 * and rights to copy, modify and redistribute granted by the license, users are provided only with a limited warranty
 * and the software's author, the holder of the economic rights, and the successive licensors have only limited
 * liability. In this respect, the user's attention is drawn to the risks associated with loading, using, modifying
 * and/or developing or reproducing the software by the user in light of its specific status of free software, that may
 * mean that it is complicated to manipulate, and that also therefore means that it is reserved for developers and
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or data
 * to be ensured and, more generally, to use and operate it in the same conditions as regards security. The fact that
 * you are presently reading this means that you have had knowledge of the CeCILL license and that you accept its terms.
 *
 * @par COMMERCIAL SOFTWARE LICENCING
 * You can obtain this software from CEA under other licencing terms for commercial purposes. For this you will need to
 * negotiate a specific contract with a legal representative of CEA.
 *
 */

#include "LedManagerI.h"
#include "utl/Logging.h"
#include "utl/BitFieldHelper.hpp"
#include "mdaq/hw/server/HwNodeImpl.h"
#include "mdaq/hw/server/DeviceImpl.h"
#ifdef HAVE_XGPIO
#include "xparameters.h"
#endif
#ifdef VXWORKS
#include <taskLib.h> // taskDelay
#include <sysLib.h> // sysClkRateGet
#endif

using mdaq::utl::CmdException;

namespace get
{
namespace cobo
{
//__________________________________________________________________________________________________
LedManagerI::LedManagerI(mdaq::hw::server::HwNodeImpl & node) : node_(node)
{
	LOG_DEBUG() << "Creating LED manager";
}
//__________________________________________________________________________________________________
/**
 * Ensures hardware is a CoBo board (and not an ML507 board).
 */
bool LedManagerI::checkHardwareType()
{
	static bool hardwareChecked = false;
	static bool isCoBoBoard = false;
	if (not hardwareChecked)
	{
		try
		{
			const size_t hardwareType = node_.findLocalDevice("ctrl").readRegister("hardwareVersion");
			// Between CoBo firmware versions 216 and 224, the hardwareVersion register was increased from 2 to 5.
			isCoBoBoard = (2 == hardwareType or 5 == hardwareType);
			hardwareChecked = true;
		}
		catch (CmdException &)
		{
		}
	}
	return isCoBoBoard;
}
//__________________________________________________________________________________________________
/**
 * Initializes the driver of the LED GPIO device.
 */
bool LedManagerI::initializeXgpioDevice()
{
	static bool gpioInitOk = false;
#ifdef HAVE_XGPIO
	if (not gpioInitOk)
	{
		LOG_DEBUG() << "Initializing LED GPIO device driver";
		if (XST_SUCCESS != XGpio_Initialize(&GpioOutput, XPAR_LEDS_DEVICE_ID))
		{
			LOG_ERROR() << "Failed to initialize LED GPIO device.";
		}
		else
		{
			// Set the direction for all signals to be outputs
			XGpio_SetDataDirection(&GpioOutput, LED_CHANNEL, 0x0);
			gpioInitOk = true;
		}
	}
#endif
	return gpioInitOk;
}
//__________________________________________________________________________________________________
size_t LedManagerI::ledBit(const LedType & type, const size_t asadId)
{
	return NumLedsPerAsAd*asadId + type;
}
//__________________________________________________________________________________________________
/**
 * Switches all LED's on or off.
 */
#ifdef HAVE_XGPIO
void LedManagerI::setAllLedsEnabled(bool enabled)
{
	if (not isInitOk()) return;
	stopAllLedPulsers();
	const Xuint32 data = enabled?0xFFFFu:0;
	XGpio_DiscreteWrite(&GpioOutput, LED_CHANNEL, data);
}
#else // HAVE_XGPIO
void LedManagerI::setAllLedsEnabled(bool) {}
#endif // HAVE_XGPIO
//__________________________________________________________________________________________________
/**
 * Switches a given LED on or off.
 * @param ledId Bit of chosen LED in GPIO device register
 * @param enabled Whether to switch the LED on or off.
 */
#ifdef HAVE_XGPIO
void LedManagerI::setLedEnabled(const size_t & ledId, bool enabled)
{
	if (not isInitOk()) return;
	// Stop LED pulser if any
	pulseLed(ledId, 0);

	// Create mask where only the bit of the selected LED is set
	const Xuint32 mask = 1u << ledId;
	if (enabled)
	{
		// Set bit of selected LED to 1, leaving other bits in the register unaffected
		XGpio_DiscreteSet(&GpioOutput, LED_CHANNEL, mask);
	}
	else
	{
		// Set bit of selected LED to 0, leaving other bits in the register unaffected
		XGpio_DiscreteClear(&GpioOutput, LED_CHANNEL, mask);
	}
}
#else // HAVE_XGPIO
void LedManagerI::setLedEnabled(const size_t &, bool) {}
#endif // HAVE_XGPIO
//__________________________________________________________________________________________________
/**
 * Switches a given LED on or off.
 * @param type Type of LED.
 * @param asadId AsAd identifier.
 * @param enabled Whether to switch the LED on or off.
 */
void LedManagerI::setLedEnabled(const LedType & type, const size_t asadId, bool enabled)
{
	setLedEnabled(ledBit(type, asadId), enabled);
}
//__________________________________________________________________________________________________
/**
 * Inverts state of LED with given identifier.
 * @param ledId Bit of chosen LED in GPIO device register
 */
#ifdef HAVE_XGPIO
void LedManagerI::flipLed(const size_t & ledId)
{
	if (not isInitOk()) return;

	// Check current state of LED
	bool newState = not ::utl::BitFieldHelper< Xuint32 >::getBit(XGpio_DiscreteRead(&GpioOutput, LED_CHANNEL), ledId);
	// Set new state
	// Create mask where only the bit of the selected LED is set
	const Xuint32 mask = 1u << ledId;
	if (newState)
	{
		// Set bit of selected LED to 1, leaving other bits in the register unaffected
		XGpio_DiscreteSet(&GpioOutput, LED_CHANNEL, mask);
	}
	else
	{
		// Set bit of selected LED to 0, leaving other bits in the register unaffected
		XGpio_DiscreteClear(&GpioOutput, LED_CHANNEL, mask);
	}
}
#else // HAVE_XGPIO
void LedManagerI::flipLed(const size_t &) {}
#endif // HAVE_XGPIO
//__________________________________________________________________________________________________
/**
 * Inverts state of given LED.
 * @param type Type of LED.
 * @param asadId AsAd identifier.
 */
void LedManagerI::flipLed(const LedType & type, const size_t asadId)
{
	flipLed(ledBit(type, asadId));
}
//__________________________________________________________________________________________________
/**
 * Inverts state of all LEDs simultaneously.
 */
void LedManagerI::flipAllLeds()
{
	if (not isInitOk()) return;
	stopAllLedPulsers();
#ifdef HAVE_XGPIO
	// Check current state of LEDs
	const Xuint32 oldState = XGpio_DiscreteRead(&GpioOutput, LED_CHANNEL);
	// Set new state
	XGpio_DiscreteWrite(&GpioOutput, LED_CHANNEL, ~oldState);
#endif
}
//__________________________________________________________________________________________________
#ifdef HAVE_XGPIO
void LedManagerI::blinkLed(const size_t & ledId)
{
	if (not isInitOk()) return;

#ifdef VXWORKS

	if (ledId >= GPIO_BITWIDTH) return;

	// Create mask where only the bit of the selected LED is set
	const Xuint32 mask = 1u << ledId;

	for (size_t ledLoop = 0; ledLoop < LED_MAX_BLINK; ledLoop++)
	{
		// Set bit of selected LED to 1, leaving other bits in the register unaffected
		XGpio_DiscreteSet(&GpioOutput, LED_CHANNEL, mask);

		// Wait a small amount of time so the LED is visible
		taskDelay(sysClkRateGet() * LED_DELAY_s);

		// Set bit of selected LED to 0, leaving other bits in the register unaffected
		XGpio_DiscreteClear(&GpioOutput, LED_CHANNEL, mask);

		// Wait a small amount of time so the LED is visible
		taskDelay(sysClkRateGet() * LED_DELAY_s);
	}
#endif // VXWORKS
}
#else // HAVE_XGPIO
void LedManagerI::blinkLed(const size_t &) {}
#endif // HAVE_XGPIO
//__________________________________________________________________________________________________
void LedManagerI::blinkAllLeds()
{
	if (not isInitOk()) return;
	stopAllLedPulsers();
	clearAllLeds();

	// Blink each LED successively
	for (size_t ledBit = 0x0; ledBit < GPIO_BITWIDTH; ledBit++)
	{
		blinkLed(ledBit);
	}
}
//__________________________________________________________________________________________________
/**
 * Flips state of LED periodically.
 * @param ledId Bit of LED.
 * @param period_ms Blinking half-period in ms.
 */
void LedManagerI::pulseLed(const size_t & ledId, const size_t period_ms)
{
	pulsers.erase(ledId);
	if (period_ms > 0)
	{
		pulsers[ledId] = new LedPulser(*this, ledId, period_ms);
	}
}
//__________________________________________________________________________________________________
/**
 * Flips state of LED periodically.
 * @param type Type of LED (P, B, U, or D).
 * @param asadId Id of AsAd board the LEDs are associated with.
 * @param period_ms Blinking half-period in ms.
 */
void LedManagerI::pulseLed(const LedType & type, const size_t asadId, const size_t period_ms)
{
	pulseLed(ledBit(type, asadId), period_ms);
}
//__________________________________________________________________________________________________
void LedManagerI::stopAllLedPulsers()
{
	pulsers.clear();
}
//__________________________________________________________________________________________________
/**
 * Sets a LED into one of 4 possible states.
 * @param ledType Type of LED (P, B, U, or D).
 * @param asadId  Id of AsAd board the LEDs are associated with.
 * @param newState State to set the LED into (off, on, pulsing slowly or pulsing fast).
 */
void LedManagerI::modifyLED(const LedType ledType, const size_t & asadId, const LedState newState)
{
	switch (newState)
	{
	case LedOff:
		setLedEnabled(ledType, asadId, false);
		break;
	case LedOn:
		setLedEnabled(ledType, asadId, true);
		break;
	case LedSlowPulse:
		pulseLed(ledType, asadId, 1000u);
		break;
	case LedFastPulse:
		pulseLed(ledType, asadId, 500u);
		break;
	default:
		break;
	}
}
//__________________________________________________________________________________________________
LedManagerI::LedPulser::LedPulser(LedManagerI& mgr, const size_t & ledId, size_t period_ms)
	: utl::PeriodicTask(period_ms, false), mgr_(mgr), ledId_(ledId)
{
}
//__________________________________________________________________________________________________
LedManagerI::LedPulser::~LedPulser()
{
}
//__________________________________________________________________________________________________
void LedManagerI::LedPulser::doTask()
{
	if (ledId_ >= GPIO_BITWIDTH)
		mgr_.flipAllLeds();
	else
		mgr_.flipLed(ledId_);
}
//__________________________________________________________________________________________________
} /* namespace cobo */
} /* namespace get */
