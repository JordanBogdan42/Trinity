/**
 * @file ConfigWizardMainWindow.h
 * @date Nov 19, 2013
 * @author sizun
 * 
 * @note SVN tag: $Id: ConfigWizardMainWindow.h 1133 2014-02-11 09:34:39Z psizun $
 * @note Contributor(s): Patrick Sizun
 * @note 
 * @note This file is part of the GetConfigWizard software project.
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

#ifndef get_ConfigWizardMainWindow_h_INCLUDED
#define get_ConfigWizardMainWindow_h_INCLUDED

#include "get/ConfigWizardMainWindow.ui.h"

#include <CCfg/Control/Workspace.h>
#include <CCfg/Path.h>
#include <CCfg/Value.h>
using namespace CCfg;
#include <Utils/Ui/DesignedWidget.hpp>

#include <QMainWindow>
#include <QVariant>
#include <QWidget>

#include <memory>
#include <string>

// forward declarations
namespace CCfg
{
	namespace Model
	{
		class ConfigModel;
	} // namespace Model
	namespace Ui
	{
		class EditorMainWindow;
	} // namespace Ui
} // namespace CCfg
class QAbstractProxyModel;
class QLabel;
class QTableView;

namespace get
{
namespace cfg
{
	class ChannelConfigModel;
}
namespace geo
{
	class GeometryModel;
}
//_________________________________________________________________________________________________
class ConfigWizardMainWindow : public Utils::Ui::DesignedWidget< Ui_ConfigWizardMainWindow, QMainWindow >
{
	Q_OBJECT
public:
	ConfigWizardMainWindow(QWidget* parent = 0);
	static bool createNewConfig(const std::string & templatePath, const std::string & configPath);
protected:
	void createActions();
	void createChannelActions();
	virtual void closeEvent(QCloseEvent *event);
	bool warnOpIsNotUndoable();
private slots:
	void editGeometry();
	void about();
	void createNewConfig();
	void onConfigActivated(Model::ConfigModel* current, Model::ConfigModel* previous);
	void goToCurrentChannel();
	void editChannelParameter(const std::string parameterName, const CCfg::Value & value);
	void editLsbThreshold();
	void editGain();
	void editSelect();
	void editZeroSuppressionThreshold();
	void editReadoutCondition();
	void editTriggerInhibition();
	void editChipParameter(const std::string parameterPath, const CCfg::Value & value);
	void editMsbThreshold();
	void drawThreshold();
	void drawLsbThreshold();
private:
	CCfg::Ui::EditorMainWindow* cfgEditor_;
	QTableView* channelView_;
	cfg::ChannelConfigModel* channelModel_;
	geo::GeometryModel* geoModel_;
	bool warnIfNotUndoable_;
};
//_________________________________________________________________________________________________
} /* namespace get */
#endif /* get_ConfigWizardMainWindow_h_INCLUDED */
