/**
 * @file FileSend.cpp
 * @date Apr 3, 2014
 * @author sizun
 * 
 * @note SVN tag: $Id: FileSend.cpp 1569 2014-04-03 12:25:14Z psizun $
 * @note Contributor(s): Patrick Sizun
 * @note 
 * @note This file is part of the MDaq software project.
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

#include "mdaq/daq/TcpDataSender.hpp"
#include "mdaq/daq/IceDataSender.h"
#include "mdaq/utl/CmdLineArgs.h"
#include "mdaq/utl/ConsoleLoggingBackend.h"
#include "mdaq/mt/Timer.h"
#include "mdaq/DefaultPortNums.h"
#include "utl/Logging.h"
#ifdef VXWORKS
#include "mdaq/utl/net/ZBufConnection.h"
#endif

#include ICE_HEADER(ICE_EDITION.h)
#include ICE_HEADER(Communicator.h)
#include ICE_HEADER(Config.h)
#ifdef ICEE_INT_VERSION
#include ICE_HEADER(Time.h)
#else
#include <IceUtil/Time.h>
#endif

#include <boost/lexical_cast.hpp>
#include <cstdlib>
#include <fstream>
#include <vector>

using ::utl::net::SocketAddress;
using ::mdaq::utl::CmdLineArgs;
using ::mdaq::utl::ConsoleLoggingBackend;
using ::mdaq::utl::net::TcpConnection;
using ::mdaq::daq::DataSender;
using ::mdaq::daq::TcpDataSender;
using ::mdaq::daq::IceDataSender;
using ::mdaq::mt::Timer;

#ifdef VXWORKS
using ::mdaq::utl::net::ZBufConnection;
#endif

static Ice::CommunicatorPtr ic;

#ifdef VXWORKS
int fileSend(const char* argvargs)
{
	std::ostringstream usage;
	usage << "Usage: fileSend(\"ipAddr:[port] Ice|Tcp|ZBuf [data-file-path] [block-size_B]\")";
	CmdLineArgs args = CmdLineArgs::build("perfsend", argvargs);
#else
int main(int argc, char*argv[])
{
	std::ostringstream usage;
	usage << "Usage: " << argv[0] << " <[ipAddr]:[port]> <Ice|Tcp> [data-file-path] [block-size_B]";
	CmdLineArgs args = CmdLineArgs::build(argc, argv);
#endif
	try
	{
		::utl::BackendLogger::setBackend(::utl::LoggingBackendPtr(new ConsoleLoggingBackend));

		if (args.size() < 4) throw usage.str();

		// Setting flow endpoint
		SocketAddress flowEndpoint(args[1]);
		if (flowEndpoint.port() == 0) flowEndpoint.port() = ::mdaq::Default::dataRouterFlowPortNum;
		if (not flowEndpoint.isValid()) flowEndpoint.ipAddress().fromOctets(127,0,0,1);

		// Setting data flow type
		const std::string flowType = args[2];

		// Creating data sender
		std::auto_ptr < DataSender > dataSender;
		if (flowType == "Tcp")
		{
			LOG_INFO() << "Allocating TcpDataSender < TcpConnection >";
			dataSender.reset(new TcpDataSender < TcpConnection > (flowEndpoint));
		}
#ifdef VXWORKS
		else if (flowType == "ZBuf")
		{
			LOG_INFO() << "Allocating TcpDataSender < ZBufConnection >";
			dataSender.reset(new TcpDataSender < ZBufConnection > (flowEndpoint));
		}
#endif
		else if (flowType == "Ice")
		{
			LOG_INFO() << "Initializing Ice communicator for client access";
			ic = Ice::initialize(args);
			dataSender.reset(new IceDataSender(flowEndpoint, ic));
		}
		else throw "Unknown flow type: " + flowType;
		dataSender->setAlwaysFlushData(true);

		// Prepping data
		std::string filePath = args[3];
		std::ifstream file;
		LOG_INFO() << "Opening file " << filePath;
		file.open(filePath.c_str(), std::ios::in | std::ios::binary);
		if (not file)
		{
			LOG_ERROR() << "Could not open file '" << filePath << "': " << strerror(errno);
			return EXIT_FAILURE;
		}

		// Check file size
		file.seekg(0, file.end);
		size_t fileSize_B = file.tellg();
		file.seekg(0, file.beg);
		LOG_INFO() << "Found " << fileSize_B << " B to send";

		size_t dataSize_B = 64u;
		if (args.size() > 4) dataSize_B = boost::lexical_cast< size_t >(args[4]);
		std::vector<char> dataVect(dataSize_B+1); // allocate data
		char* data = &dataVect[0];
		char* dataEnd = data + dataSize_B + 1;

		// Create circular buffer
		uint64_t bufferStartAddr = (uint64_t) &data;
		uint64_t bufferEndAddr = (uint64_t) &dataEnd;
		mdaq::daq::CircularBuffer circularBuffer(bufferStartAddr, bufferEndAddr);

		// Sending data
		LOG_INFO() << "Sending data by chunks of " << dataSize_B << " B...";
		size_t nBytesSent = 0u;
		do
		{
			//LOG_INFO() << " Sending chunk...";
			circularBuffer.reset();
			file.read(data, dataSize_B);
			size_t bytesRead = file.gcount();
			circularBuffer.writePtr.incr(bytesRead);
			dataSender->sendData(circularBuffer);
			nBytesSent += bytesRead;
		}
		while (file.good());

		size_t nBytes_MB = nBytesSent/(1000000);
		LOG_INFO() << "Total data sent: " << nBytes_MB << " MB out of " << (fileSize_B/1000000) << " MB";
		return EXIT_SUCCESS;
	}
	catch (const std::exception& e) { LOG_FATAL() << e.what(); }
	catch (const std::string&  msg) { LOG_FATAL() << msg; }
	catch (const char*         msg) { LOG_FATAL() << msg; }
	catch (...)                     { LOG_FATAL() << "Unknown exception!"; }
	return EXIT_FAILURE;

}



