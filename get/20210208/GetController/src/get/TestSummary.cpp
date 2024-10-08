/**
 * $Id: TestSummary.cpp 426 2012-10-19 08:10:24Z psizun $
 * @file TestSummary.cpp
 * @date 19 avr. 2012
 * @author sizun
 * -----------------------------------------------------------------------------
 * © Commissariat a l'Energie Atomique et aux Energies Alternatives (CEA)
 * -----------------------------------------------------------------------------
 * FREE SOFTWARE LICENCING
 * This software is governed by the CeCILL license under French law and abiding
 * by the rules of distribution of free software. You can use, modify and/or
 * redistribute the software under the terms of the CeCILL license as circulated
 * by CEA, CNRS and INRIA at the following URL: "http://www.cecill.info".
 * As a counterpart to the access to the source code and rights to copy, modify
 * and redistribute granted by the license, users are provided only with a
 * limited warranty and the software's author, the holder of the economic
 * rights, and the successive licensors have only limited liability. In this
 * respect, the user's attention is drawn to the risks associated with loading,
 * using, modifying and/or developing or reproducing the software by the user in
 * light of its specific status of free software, that may mean that it is
 * complicated to manipulate, and that also therefore means that it is reserved
 * for developers and experienced professionals having in-depth computer
 * knowledge. Users are therefore encouraged to load and test the software's
 * suitability as regards their requirements in conditions enabling the security
 * of their systems and/or data to be ensured and, more generally, to use and
 * operate it in the same conditions as regards security.
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 * -----------------------------------------------------------------------------
 * COMMERCIAL SOFTWARE LICENCING
 * You can obtain this software from CEA under other licencing terms for
 * commercial purposes. For this you will need to negotiate a specific contract
 * with a legal representative of CEA.
 * -----------------------------------------------------------------------------
 */

#include "get/TestSummary.h"
#include "get/GetControllerException.h"

#include "CCfg/CConfig.h"
#include "CCfg/Io/Url.h"
#include "CCfg/Io/Document.h"
using namespace CCfg;
using namespace CCfg::Io;

#include <QFileInfo>
#include <QDir>
#include <iostream>

namespace get {

TestSummary::TestSummary()
{
}

TestSummary::TestSummary(const CCfg::CConfig & cfg)
{
	fromCConfig(cfg);
}

TestSummary::~TestSummary()
{
}

void TestSummary::fromCConfig(const CCfg::CConfig & cfg)
{
	try
	{
		CCfg::CConfig testCfg(cfg);
		setName(testCfg.getIndex());
		setType(testCfg("Type").getStringValue());

		// Hardware path
		const Url workspaceUrl = Document::computeQualifiedIncludeUrl(*cfg.find().toObject().getIncludeRoot());
		const QDir workspaceDir = QFileInfo(workspaceUrl.toPath().c_str()).absoluteDir();
		std::string hardwarePath = testCfg("Setup", "Hardware")("Path").getStringValue();
		if (QFileInfo(QString::fromStdString(hardwarePath)).isRelative())
		{
			setHwConfigPath(workspaceDir.absoluteFilePath(QString::fromStdString(hardwarePath)).toStdString());
		}
		else
		{
			setHwConfigPath(hardwarePath);
		}

		// Conditions path
		std::string conditionsPath = testCfg("Setup", "Conditions")("Path").getStringValue();
		if (QFileInfo(QString::fromStdString(conditionsPath)).isRelative())
		{
			setParamConfigPath(workspaceDir.absoluteFilePath(QString::fromStdString(conditionsPath)).toStdString());
		}
		else
		{
			setParamConfigPath(conditionsPath);
		}

		// Data path
		setDataPath(testCfg("Setup", "Data")("Path").getStringValue());

		// Network
		eccEndpoint().fromCConfig(testCfg("Setup", "Network")("IpNode", "ECC"));
		targetEndpoint().fromCConfig(testCfg("Setup", "Network")("IpNode", "TARGET"));
	}
	catch (CCfg::Exception const & e)
	{
		throw GetControllerException(e.what());
	}
}

void TestSummary::toCConfig(const CCfg::CConfig & cfg) const
{
	try
	{
		CCfg::CConfig workspaceCfg(cfg);
		workspaceCfg.find().requireObject("Test", name());
		CCfg::CConfig testCfg(workspaceCfg("Test", name()));

		testCfg("Type").setStringValue(type());

		// Workspace path
		const Url workspaceUrl = Document::computeQualifiedIncludeUrl(*cfg.find().toObject().getIncludeRoot());
		const QDir workspaceDir = QFileInfo(workspaceUrl.toPath().c_str()).absoluteDir();

		// Hardware
		if (QFileInfo(QString::fromStdString(hwConfigPath())).absolutePath() != workspaceDir.absolutePath())
		{
			testCfg("Setup", "Hardware")("Path").setStringValue(hwConfigPath());
		}
		else
		{
			const QString relativePath = workspaceDir.relativeFilePath(QString::fromStdString(hwConfigPath()));
			testCfg("Setup", "Hardware")("Path").setStringValue(relativePath.toStdString());
		}

		// Conditions
		if (QFileInfo(QString::fromStdString(paramConfigPath())).absolutePath() != workspaceDir.absolutePath())
		{
			testCfg("Setup", "Conditions")("Path").setStringValue(paramConfigPath());
		}
		else
		{
			const QString relativePath = workspaceDir.relativeFilePath(QString::fromStdString(paramConfigPath()));
			testCfg("Setup", "Conditions")("Path").setStringValue(relativePath.toStdString());
		}

		// Data path
		testCfg("Setup", "Data")("Path").setStringValue(dataPath());

		// Network
		eccEndpoint().toCConfig(testCfg("Setup", "Network")("IpNode", "ECC"));
		targetEndpoint().toCConfig(testCfg("Setup", "Network")("IpNode", "TARGET"));
	}
	catch (CCfg::Exception const & e)
	{
		throw GetControllerException(e.what());
	}
}

void TestSummary::print(std::ostream & stream)
{
	stream << "[Test name=\"" << name_ << '"'
			<< "\n\tType: \"" << type_ << '"'
			<< "\n\tHardware: \"" << hwConfigPath_ << '"'
			<< "\n\tConditions: \"" << conditionsConfigPath_ << '"'
			<< "\n\tData: \"" << dataPath_ << '"'
			<< "\n\tECC: \"" << eccEndpoint_ << '"'
			<< "\tTARGET: \"" << targetEndpoint_ << '"'
			<< "]" << std::endl;
}

std::string TestSummary::dataPath() const
{
	return dataPath_;
}

void TestSummary::setDataPath(std::string dataPath)
{
	dataPath_ = dataPath;
}

std::string TestSummary::hwConfigPath() const
{
	return hwConfigPath_;
}

void TestSummary::setHwConfigPath(std::string hwConfigPath)
{
	this->hwConfigPath_ = hwConfigPath;
}

std::string TestSummary::name() const
{
	return name_;
}

void TestSummary::setName(std::string name)
{
	name_ = name;
}

std::string TestSummary::paramConfigPath() const
{
	return conditionsConfigPath_;
}

void TestSummary::setParamConfigPath(std::string paramConfigPath)
{
	conditionsConfigPath_ = paramConfigPath;
}

std::string TestSummary::type() const
{
	return type_;
}

void TestSummary::setType(std::string type)
{
	type_ = type;
}

const mdaq::utl::net::SocketAddress & TestSummary::eccEndpoint() const
{
	return eccEndpoint_;
}

const mdaq::utl::net::SocketAddress & TestSummary::targetEndpoint() const
{
	return targetEndpoint_;
}

mdaq::utl::net::SocketAddress & TestSummary::eccEndpoint()
{
	return eccEndpoint_;
}

mdaq::utl::net::SocketAddress & TestSummary::targetEndpoint()
{
	return targetEndpoint_;
}

} /* namespace get */
