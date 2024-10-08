/**
 * @file EditMappingDialog.cpp
 * @date Jan 28, 2014
 * @author sizun
 * 
 * @note SVN tag: $Id: EditMappingDialog.cpp 1105 2014-01-29 08:47:35Z psizun $
 * @note Contributor(s): Patrick Sizun
 * @note 
 * @note This file is part of the ConfigWizard software project.
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

#include "EditMappingDialog.h"
#include "GeometryModel.h"
#include "utl/Logging.h"

#include <QFileDialog>
#include <fstream>
#include <sstream>

namespace get
{
namespace geo
{
//__________________________________________________________________________________________________
EditMappingDialog::EditMappingDialog(QWidget* parentWidget) : Utils::Ui::DesignedWidget<Ui_EditMappingDialog, QDialog>(parentWidget)
{
	QMetaObject::connectSlotsByName(this);
	ui().mappingView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}
//__________________________________________________________________________________________________
QSize EditMappingDialog::sizeHint() const
{
	return QSize(500, 400);
}
//__________________________________________________________________________________________________
void EditMappingDialog::setGeometryModel(GeometryModel* model)
{
	ui().mappingView->setModel(model);
}
//__________________________________________________________________________________________________
GeometryModel* EditMappingDialog::geometryModel() const
{
	return dynamic_cast< GeometryModel* >(ui().mappingView->model());
}
//__________________________________________________________________________________________________
void EditMappingDialog::on_addButton_clicked(bool)
{
	GeometryModel* model = geometryModel();
	if (model) model->addChannel();
}
//__________________________________________________________________________________________________
void EditMappingDialog::on_removeButton_clicked(bool)
{
	GeometryModel* model = geometryModel();
	if (model) model->removeLastChannel();
}
//__________________________________________________________________________________________________
void EditMappingDialog::on_clearButton_clicked(bool)
{
	GeometryModel* model = geometryModel();
	if (model) model->clear();
}
//__________________________________________________________________________________________________
void EditMappingDialog::on_importButton_clicked(bool)
{
	GeometryModel* model = geometryModel();
	if (not model) return;

	// Select file to import from
	QString filter = QString("%1 (*.txt);;%2 (*.*)").arg(tr("Text files")).arg(tr("All files"));
	QString fileName = QFileDialog::getOpenFileName(this, tr("Import mapping"), "", filter);
	if (fileName.isEmpty()) return;

    // Open file
    std::ifstream stream(fileName.toStdString().c_str());
    if (! stream)
    {
        LOG_ERROR() << "Could not open file " << fileName.toStdString();
        return;
    }

    // Clear model
    model->clear();

    // Parse file
    std::string line;
    PadChannel  con;
    while (std::getline(stream, line))
    {
          // Discard comment lines
          if (line.empty() || line[0] == '#') continue;
          // Parse line
          std::istringstream iss (line);
          iss >> con.channel.coboId >> con.channel.asadId >> con.channel.agetId >> con.channel.channelId >> con.pad.first >> con.pad.second;
          // Add to model
          if (iss) model->addChannel(con);
    }
}
//__________________________________________________________________________________________________
void EditMappingDialog::on_exportButton_clicked(bool)
{
	GeometryModel* model = geometryModel();
	if (not model) return;

	// Select file to export to
	QString filter = QString("%1 (*.txt);;%2 (*.*)").arg(tr("Text files")).arg(tr("All files"));
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export mapping"), "mapping.txt", filter);
	if (fileName.isEmpty()) return;

    // Open file
    std::ofstream stream(fileName.toStdString().c_str());
    if (! stream)
    {
        LOG_ERROR() << "Could not create file " << fileName.toStdString();
        return;
    }

    // Write file
    int rowCount = model->rowCount();
    stream << "# cobo\tasad\taget\tchannel\trow\tcolumn\n";
    for (int row=0; row < rowCount; ++row)
    {
    	const PadChannel & con = model->mapping(row);
    	stream << con.channel.coboId << '\t' << con.channel.asadId << '\t' << con.channel.agetId << '\t' << con.channel.channelId
    			<< '\t' << con.pad.first << '\t' << con.pad.second << '\n';
    }
}
//__________________________________________________________________________________________________
void EditMappingDialog::accept()
{
	QDialog::accept();
}
//__________________________________________________________________________________________________
} /* namespace geo */
} /* namespace get */
