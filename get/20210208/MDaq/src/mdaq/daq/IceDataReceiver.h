/**
 * @file IceDataReceiver.h
 * @created Sep 12, 2012
 * @author anvar
 * ---------------------------------------------------------------------------------------------------------------------
 * class mdaq::daq::IceDataReceiver
 * Created on: Sep 12, 2012 at Irfu/Sedi/Lilas, CEA Saclay, F-91191, France
 * ---------------------------------------------------------------------------------------------------------------------
 * © Commissariat a l'Energie Atomique et aux Energies Alternatives (CEA)
 * ---------------------------------------------------------------------------------------------------------------------
 * Contributors: Shebli Anvar (shebli.anvar@cea.fr)
 * ---------------------------------------------------------------------------------------------------------------------
 * This software is part of the MDaq framework.
 * ---------------------------------------------------------------------------------------------------------------------
 * FREE SOFTWARE LICENCING
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
 * ---------------------------------------------------------------------------------------------------------------------
 * COMMERCIAL SOFTWARE LICENCING
 * You can obtain this software from CEA under other licencing terms for commercial purposes. For this you will need to
 * negotiate a specific contract with a legal representative of CEA.
 * =====================================================================================================================
 */

#ifndef mdaq_daq_IceDataReceiver_h_INCLUDED
#define mdaq_daq_IceDataReceiver_h_INCLUDED

#include "DataReceiver.h"
#include "mdaq/utl/Server.h"
#include "mdaq/daq/DataProcessor.h"

namespace mdaq
{
namespace daq
{

/**
 * This abstract class implements the main loop of the AbstractDataReceiver interface according to an Ice-based data
 * transfer mechanism. After receiving the data, it calls method processData on its associated data processor.
 */
class IceDataReceiver : public DataReceiver
{
	friend class DataProcessorImpl;

private:
	typedef std::pair<const Ice::Byte*, const Ice::Byte*> BytePtrPair;
	struct DataProcessorImpl : public DataProcessor
	{
		DataProcessorImpl(IceDataReceiver& dataReceiver) : dataReceiver(dataReceiver) {}
	    virtual void processData(const BytePtrPair& bytePtrPair, const Ice::Current&); ///< @override Ice

	    IceDataReceiver& dataReceiver;
	};

public:
	IceDataReceiver(const ::utl::net::SocketAddress& sockAddr);
	virtual ~IceDataReceiver();
	virtual void start(); ///< @override
	virtual void askForStop(); ///< @override
	virtual void waitForStop(); ///< @override

protected:
	mdaq::utl::Server& dataProcessorServer;
};

} // namespace daq
} // namespace mdaq

#endif /* mdaq_daq_IceDataReceiver_h_INCLUDED */
