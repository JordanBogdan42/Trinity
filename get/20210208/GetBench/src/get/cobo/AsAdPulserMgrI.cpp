/**
 * @file AsAdPulserMgrI.cpp
 * @date Dec 19, 2013
 * @author sizun
 * 
 * @note SVN tag: $Id: AsAdPulserMgrI.cpp 1384 2014-12-11 07:54:57Z psizun $
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

#include "AsAdPulserMgrI.h"
#include "CtrlNodeI.h"
#include "mdaq/hw/server/DeviceImpl.h"
#include "utl/BitFieldHelper.hpp"
#include "utl/Logging.h"
#ifdef VXWORKS
#include "logLib.h"
#endif

#include <boost/cstdint.hpp>
#include <cmath>
#include <sstream>

using mdaq::hw::NodePrx;
using mdaq::hw::DevicePrx;
using mdaq::hw::server::DeviceImpl;
using ::mdaq::utl::CmdException;

namespace get
{
namespace cobo
{
//__________________________________________________________________________________________________
const float AsAdPulserMgrI::Vref_mV = 1102.0f;
//__________________________________________________________________________________________________
AsAdPulserMgrI::PeriodicPulser::PeriodicPulser(AsAdPulserMgrI & mgr, long voltage_mV, size_t period_ms, long voltage2_mV, bool randomEnabled)
	: utl::PeriodicTask(period_ms, randomEnabled), mgr_(mgr), voltage_mV_(voltage_mV), voltage2_mV_(voltage2_mV)
{
}
//__________________________________________________________________________________________________
AsAdPulserMgrI::PeriodicPulser::~PeriodicPulser()
{
}
//__________________________________________________________________________________________________
//timespec timespec_diff(const timespec & start, const timespec & end)
//{
//	timespec temp;
//	if ((end.tv_nsec-start.tv_nsec)<0)
//	{
//		temp.tv_sec = end.tv_sec-start.tv_sec-1;
//		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
//	}
//	else {
//		temp.tv_sec = end.tv_sec-start.tv_sec;
//		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
//	}
//	return temp;
//}
void AsAdPulserMgrI::PeriodicPulser::doTask()
{
//	struct timespec tstart, tstop;
//	clock_gettime(CLOCK_REALTIME, &tstart);

	//LOG_DEBUG() << "Timeout of periodic AsAd pulser...";
	mgr_.triggerPulserAndRestore(voltage_mV_);
	if (std::fabs(voltage2_mV_) > 0)
	{
		mgr_.triggerPulserAndRestore(voltage2_mV_);
	}

//	clock_gettime(CLOCK_REALTIME, &tstop);
//	timespec elapsed = timespec_diff(tstart, tstop);
//	LOG_DEBUG() << "doTask took " << elapsed.tv_sec << " s " << elapsed.tv_nsec << " ns";
}
//__________________________________________________________________________________________________
AsAdPulserMgrI::AsAdPulserMgrI::RegisterCache::RegisterCache()
	: isCacheReady_(false), triggerMask_(0), tcm0_(0), tcm1_(0), tcm2_(0)
{

}
//__________________________________________________________________________________________________
void AsAdPulserMgrI::AsAdPulserMgrI::RegisterCache::buildCache(CtrlNodeI & /* node */)
{
	isCacheReady_ = false;

	try
	{
		LOG_INFO() << "Caching reference to registers...";
/*
		// Get reference to CoBo 'ctrl' and AsAd local devices
		DeviceImpl & ctrl = node.findLocalDevice("ctrl");
		DeviceImpl & asad = node.findLocalDevice("asad");

		// Store pointer to registers
		triggerMask_ = & ctrl.findRegister("triggerMask");
		tcm0_ = & asad.findRegister("TCM0");
		tcm1_ = & asad.findRegister("TCM1");
		tcm2_ = & asad.findRegister("TCM2");
*/
		isCacheReady_ = true;
	}
	catch (const CmdException & e)
	{
		LOG_ERROR() << e.reason;
	}
}
//__________________________________________________________________________________________________
AsAdPulserMgrI::AsAdPulserMgrI(CtrlNodeI & node) : node_(node),
		voltage_mV_(-1000), voltage2_mV_(0), period_ms_(1000), poissonMode_(false),
		triggerMask_(0), slowControlMode_(false), slowControl2pMode_(false),
		defaultDacCode_(8089u), externalModeEnabled_(false), externalDacCode_(15430u),
		periodicPulser_(0)
{
}
//__________________________________________________________________________________________________
void AsAdPulserMgrI::disableSlowControlTriggerModes(::mdaq::hw::Device & ctrl, bool check)
{
//	struct timespec tstart, tstop;
//	clock_gettime(CLOCK_REALTIME, &tstart);

	static const uint32_t slowControlTriggerMask = UINT32_C(0x1010);
	if (check)
	{
		triggerMask_ = ctrl.readRegister("triggerMask");
	}
	if ((slowControlMode_ = triggerMask_ & slowControlTriggerMask))
	{
		ctrl.writeRegister("triggerMask", triggerMask_ & ~slowControlTriggerMask);
	}

//	clock_gettime(CLOCK_REALTIME, &tstop);
//	timespec elapsed = timespec_diff(tstart, tstop);
//	LOG_DEBUG() << "disableSlowControlTriggerModes took " << elapsed.tv_sec << " s " << elapsed.tv_nsec << " ns";
}
//__________________________________________________________________________________________________
void AsAdPulserMgrI::restoreSlowControlTriggerModes(::mdaq::hw::Device & ctrl)
{
//	struct timespec tstart, tstop;
//	clock_gettime(CLOCK_REALTIME, &tstart);

	if (slowControlMode_)
	{
		ctrl.writeRegister("triggerMask", triggerMask_);
	}

//	clock_gettime(CLOCK_REALTIME, &tstop);
//	timespec elapsed = timespec_diff(tstart, tstop);
//	LOG_DEBUG() << "restoreSlowControlTriggerModes took " << elapsed.tv_sec << " s " << elapsed.tv_nsec << " ns";
}
//__________________________________________________________________________________________________
/**
 * Returns the 14-bit DAC decimal code corresponding to the given output voltage.
 *
 *@param step_mV Difference Vout - Vref between output voltage and reference voltage Vref (1102 mV), in mV.
 * @return 14-bit DAC code
 *
 * Vout must be between -0.0126 x 2 x Vref and 2 x Vref.
 * @see AsAd Test and Calibration Management (GET-AS-002-0005), section 1.2
 */
uint16_t AsAdPulserMgrI::dacDecimalCode(const float & step_mV)
{
	static const float two_exp_13 = static_cast<float>(0x2000);
	static const float mult = (31.6f / 32.0f) * two_exp_13;

	uint16_t code = 0;
	// If voltage already used, return previously computed decimal code
	DacCodeMap::const_iterator dacIt = dacCodes_.find(step_mV);
	if (dacCodes_.end() != dacIt)
	{
		code = dacIt->second;
	}
	else
	{
		// Check input voltage is valid
		if (std::fabs(step_mV) > 1000.0f)
		{
			std::ostringstream stream;
			stream << (Vref_mV + step_mV) << " mV is outside the valid range for the AsAd pulser output voltage.";
			throw CmdException(stream.str());
		}

		// Compute DAC decimal code from voltage
		// Vout = 2 x Vref x (1 - (2**-9)/31.6 x code) with Vref = 1.102 V +/- 0.6%
		// code = 31.6 x (2**9) x (1 - (Vout/2)/Vref)
		// code = 31.6 / 32 x (2**13) x (1 - (Vout-Vref)/Vref)
		dacCodes_[step_mV] = (uint16_t) (mult * (1 - float(step_mV)/Vref_mV));
		code = dacCodes_[step_mV];
	}
	return code;
}
//__________________________________________________________________________________________________
/**
 *
 * @param defaultVoltage_mV Difference Vout - Vref between default output voltage and reference voltage Vref (1102 mV), in mV.
 */
void AsAdPulserMgrI::setDefaultVoltage(Ice::Long defaultVoltage_mV)
{
	// Store default code
	defaultDacCode_ = dacDecimalCode(defaultVoltage_mV);
	//LOG_DEBUG() << "TCM: default code=" << (defaultDacCode_ & 0x3FFF);

	// Set output voltage
	triggerPulser(defaultVoltage_mV);
}
//__________________________________________________________________________________________________
/**
 * Triggers a voltage step in the AsAd voltage generator.
 * @param voltage_mV Difference Vout - Vref between output voltage and reference voltage Vref (1102 mV), in mV.
 *
 * The AsAd pulser is controlled via the Test and Calibration Manager (TCM) device.
 * It is configured through three read-write 8-bit registers.
 * TCM0 is the configuration register.
 * TCM1 and TCM2 define the step voltage amplitude.
 *
 * The selection of the AsAd boards the command should be sent to must be done beforehand,
 *  by setting the CoBo  mask for AsAd slow control commands.
 *
 * @see AsAd Test and Calibration Management (GET-AS-002-0005)
 * @see AsAd Slow Control Manager: Protocols and Registers mapping (GET-AS-002-0010)
 */
void AsAdPulserMgrI::triggerPulser(Ice::Long voltage_mV)
{
//	struct timespec tstart, tstop;
//	clock_gettime(CLOCK_REALTIME, &tstart);

	// Check validity of input voltage and compute corresponding DAC code
	const uint16_t dacCode = dacDecimalCode(voltage_mV);
	//LOG_DEBUG() << "TCM: code=" << (dacCode & 0x3FFF);

	// Get reference to CoBo 'ctrl' and AsAd local devices
	DeviceImpl & ctrl = node_.findLocalDevice("ctrl");
	DeviceImpl & asad = node_.findLocalDevice("asad");

	// Remember whether slow control trigger mode is enabled
	// Disable slow control trigger mode to avoid triggering when writing to AsAd TCM1 register
	// We only want to trigger when writing to AsAd TCM2 register
	disableSlowControlTriggerModes(ctrl);

	// Set MSB of generator output voltage
	// Write to AsAd TCM1 register: TCM1<5..0> = DATA<13..8>
	asad.writeRegister("TCM1", ::utl::BitFieldHelper< uint16_t >::getField(dacCode, 8, 6));

	// Restore slow control trigger mode unless we are setting generator back to its default output voltage
	if (dacCode != defaultDacCode_) restoreSlowControlTriggerModes(ctrl);

	// Set LSB of generator output voltage
	// Write to AsAd TCM2 register: TCM2<7..0> = DATA<7..0>
	asad.writeRegister("TCM2", ::utl::BitFieldHelper< uint16_t >::getField(dacCode, 0, 8));

	// Restore slow control trigger mode if not already done
	if (dacCode == defaultDacCode_) restoreSlowControlTriggerModes(ctrl);

//	clock_gettime(CLOCK_REALTIME, &tstop);
//	timespec elapsed = timespec_diff(tstart, tstop);
//	LOG_DEBUG() << "triggerPulser took " << elapsed.tv_sec << " s " << elapsed.tv_nsec << " ns";
}
//__________________________________________________________________________________________________
/**
 * Sets generator output voltage to given value and restores default output voltage.
 *
 * Pulses result from changes in the generator output voltage.
 *
 * @param voltage_mV Difference Vout - Vref between output voltage and reference voltage Vref (1102 mV), in mV.
 * @see AsAd Test and Calibration Management (GET-AS-002-0005)
 * @see AsAd Slow Control Manager: Protocols and Registers mapping (GET-AS-002-0010)
 */
void AsAdPulserMgrI::triggerPulserAndRestore(Ice::Long voltage_mV)
{
	// Check validity of input voltage and compute corresponding DAC code
	const uint16_t dacCode = dacDecimalCode(voltage_mV);

	// Actually set generator
	triggerPulserAndRestore_byDacCode(dacCode);
}
//__________________________________________________________________________________________________
/**
 * Sets generator output voltage to given value and restores default output voltage.
 *
 * Pulses result from changes in the generator output voltage.
 *
 * @param dacCode DAC code (14 bits).
 * @see AsAd Test and Calibration Management (GET-AS-002-0005)
 * @see AsAd Slow Control Manager: Protocols and Registers mapping (GET-AS-002-0010)
 */
void AsAdPulserMgrI::triggerPulserAndRestore_byDacCode(uint16_t dacCode)
{
	// See email from JL Pedroza on December 3rd, 2014.
	//if (dacCode == defaultDacCode_) return;

	try
	{
		// Get reference to CoBo 'ctrl' and AsAd local devices
		DeviceImpl & ctrl = node_.findLocalDevice("ctrl");
		DeviceImpl & asad = node_.findLocalDevice("asad");

		// Disable slow control trigger mode to avoid triggering CoBo when writing to AsAd TCM1 register or reading TCM0 register
		disableSlowControlTriggerModes(ctrl);

		// Check whether pulser is triggered by slow control or by ISP1 external signal
		const bool pulserExternalMode = asad.readField("TCM0", "Trg");

		// Write voltage to TCM registers, triggering AsAd pulser and possibly CoBo on last step
		if (not pulserExternalMode)
		{
			//LOG_DEBUG() << "TCM: code=" << (dacCode & 0x3FFF);
			// Set MSB of step voltage
			// Write to AsAd TCM1 register: TCM1<5..0> = DATA<13..8>
			asad.writeRegister("TCM1", ::utl::BitFieldHelper< uint16_t >::getField(dacCode, 8, 6));
			// Restore slow control trigger mode
			restoreSlowControlTriggerModes(ctrl);
			// Set LSB of step voltage
			// Write to AsAd TCM2 register: TCM2<7..0> = DATA<7..0>
			// Since AsAd pulser is in slow control triggering mode, it changes the DAC output voltage
			asad.writeRegister("TCM2", ::utl::BitFieldHelper< uint16_t >::getField(dacCode, 0, 8));

			// Disable slow control trigger mode to avoid triggering when writing to AsAd TCM1 register
			disableSlowControlTriggerModes(ctrl, false);
		}
		// For external mode, enable AsAd slow control mode just to switch output voltage back to its default value
		else
		{
			asad.writeField("TCM0", "Trg", 0);
		}

		// Reset TCM registers without triggering
		//LOG_DEBUG() << "TCM: code=" << (defaultDacCode_ & 0x3FFF);
		// Set MSB of step voltage
		// Write to AsAd TCM1 register: TCM1<5..0> = DATA<13..8>
		asad.writeRegister("TCM1", ::utl::BitFieldHelper< uint16_t >::getField(defaultDacCode_, 8, 6));
		// Set LSB of step voltage
		// Write to AsAd TCM2 register: TCM2<7..0> = DATA<7..0>
		asad.writeRegister("TCM2", ::utl::BitFieldHelper< uint16_t >::getField(defaultDacCode_, 0, 8));

		// Restore pulser external mode and prepare DAC for next external signal
		if (pulserExternalMode)
		{
			asad.writeField("TCM0", "Trg", 1u);
			//LOG_DEBUG() << "TCM: code=" << (dacCode & 0x3FFF);
			// Set MSB of step voltage
			// Write to AsAd TCM1 register: TCM1<5..0> = DATA<13..8>
			asad.writeRegister("TCM1", ::utl::BitFieldHelper< uint16_t >::getField(dacCode, 8, 6));
			// Set LSB of step voltage
			// Write to AsAd TCM2 register: TCM2<7..0> = DATA<7..0>
			asad.writeRegister("TCM2", ::utl::BitFieldHelper< uint16_t >::getField(dacCode, 0, 8));
		}

		// Restore slow control trigger mode
		restoreSlowControlTriggerModes(ctrl);
	}
	catch (const mdaq::utl::CmdException & e)
	{
		LOG_ERROR() << e.reason;
	}
}
//__________________________________________________________________________________________________
/**
 * Stores info pertaining to external triggering mode.
 */
void AsAdPulserMgrI::configureExternalPulser(bool enabled, Ice::Long voltage_mV)
{
	externalModeEnabled_ = enabled;
	if (externalModeEnabled_)
	{
		externalDacCode_ = dacDecimalCode(voltage_mV);
	}

	// Build cache for faster operations
	//registers_.buildCache(node_);
}
//__________________________________________________________________________________________________
/**
 * Prepares AsAd voltage generator for the next external signal.
 */
void AsAdPulserMgrI::resetExternalPulser()
{
	if (not externalModeEnabled_ or periodicPulser_) return;

	try
	{
//#ifdef VXWORKS
//		logMsg("Resetting AsAd voltage generator for next external signal...\n", 0, 0, 0, 0, 0, 0);
//#endif

		// Change DAC output voltage to default voltage and back to current voltage
		triggerPulserAndRestore_byDacCode(externalDacCode_);
	}
	catch (const mdaq::utl::CmdException & e)
	{
		LOG_ERROR() << e.reason;
	}
}
//__________________________________________________________________________________________________
/**
 * Configures period and amplitude of periodic pulses.
 * @param mV Voltage step amplitude [mV]
 * @param ms Period [ms] of pulses.
 */
void AsAdPulserMgrI::configurePeriodicPulser(Ice::Long mV, Ice::Long ms)
{
	if (std::fabs(mV) <= 1000 and ms > 0)
	{
		period_ms_ = ms;
		voltage_mV_ = mV;
		LOG_DEBUG() << "Configuring AsAd pulser with period of " << period_ms_ << " ms and a voltage of " << (Vref_mV + voltage_mV_) << " mV";
	}
	else
	{
		std::ostringstream msg;
		msg << "Parameters for periodic AsAd pulser are not valid (period of " << ms << " ms and voltage of " << (Vref_mV + voltage_mV_) << " mV)!";
		throw ::mdaq::utl::CmdException(msg.str());
	}
}
//__________________________________________________________________________________________________
/**
 * Enables or disables random Poisson mode.
 * @param poissonMode Enables Poisson process instead of the periodic process.
 */
void AsAdPulserMgrI::setRandomPulserEnabled(bool poissonMode)
{
	poissonMode_ = poissonMode;
	LOG_DEBUG() << (poissonMode_?"Enabled":"Disabled") << " random pulser mode";
}
//__________________________________________________________________________________________________
/**
 * Sets amplitude of second impulsion. If this amplitude is zero, there will be no second impulsion.
 * @param mV  Amplitude [mV] of second voltage step.
 */
void AsAdPulserMgrI::configureDoublePulserMode(Ice::Long mV)
{
	if (std::fabs(mV) <= 1000)
	{
		voltage2_mV_ = mV;
		LOG_DEBUG() << "Setting voltage of second AsAd pulse to " << (Vref_mV + voltage2_mV_) << " mV";
	}
	else
	{
		std::ostringstream msg;
		msg << mV << " mV is not a valid AsAd pulser amplitude!";
		throw ::mdaq::utl::CmdException(msg.str());
	}
}
//__________________________________________________________________________________________________
/**
 * Starts (already configured) timer for periodic pulses.
 */
void AsAdPulserMgrI::startPeriodicPulser()
{
	periodicPulser_ = 0;
	if (period_ms_ > 0)
	{
		periodicPulser_ = new PeriodicPulser(*this, voltage_mV_, period_ms_, voltage2_mV_, poissonMode_);
	}
}
//__________________________________________________________________________________________________
/**
 * Configures and starts periodic pulser.
 */
void AsAdPulserMgrI::startPeriodicPulser(Ice::Long mV, Ice::Long ms)
{
	configurePeriodicPulser(mV, ms);
	configureDoublePulserMode(0);
	startPeriodicPulser();
}
//__________________________________________________________________________________________________
void AsAdPulserMgrI::stopPeriodicPulser()
{
	bool isStarted = periodicPulser_;
	periodicPulser_ = 0;
	if (isStarted)
	{
		LOG_DEBUG() << "Stopped periodic trigger of the AsAd voltage generator";
	}
}
//__________________________________________________________________________________________________
} /* namespace cobo */
} /* namespace get */
