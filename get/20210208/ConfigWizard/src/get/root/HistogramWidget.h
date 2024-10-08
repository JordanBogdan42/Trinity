/**
 * @file HistogramWidget.h
 * @date Jan 28, 2014
 * @author sizun
 * 
 * @note SVN tag: $Id: HistogramWidget.h 1105 2014-01-29 08:47:35Z psizun $
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

#ifndef get_root_HistogramWidget_h_INCLUDED
#define get_root_HistogramWidget_h_INCLUDED

#include <QDialog>
#ifdef HAVE_QTROOT
class TQtWidget;
class TVirtualPad;
class TH1;
#include <QDialogButtonBox>
class QAbstractButton;
class QDialogButtonBox;
class QPushButton;
class QCheckBox;
#endif // HAVE_QTROOT

namespace get
{
namespace root
{
//_________________________________________________________________________________________________
#ifndef HAVE_QTROOT
/**
 * A widget to display a ROOT histogram,
 * with buttons to save it, export its contents or set its range.
 */
class HistogramWidget : public QObject
{
	Q_OBJECT
};
//_________________________________________________________________________________________________
#else // HAVE_QTROOT
class HistogramWidget : public QWidget
{
	Q_OBJECT
public:
	HistogramWidget(QWidget* parentWidget=0);
	~HistogramWidget();
	QSize	sizeHint () const { return QSize(800, 600); }
	TVirtualPad* canvas() const;
	bool isTH1F() const;
	bool isTH2F() const;
	void setStandardButtons(QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Save | QDialogButtonBox::Close);
	void saveHist();
	void setHistRange(double const & minimum, double const & maximum);
	TH1* hist() const;
	void setHist(TH1* his, const char* title="");
protected slots:
	void setLogMode(bool enabled);
private slots:
	void on_buttonBox_clicked(QAbstractButton* button);
	void setHistRange();
	void exportHist();
private:
	TH1* hist_;
	TQtWidget* qtWidget_;
	QDialogButtonBox* buttonBox_;
	QCheckBox* logButton;
	QPushButton* rangeButton;
	QPushButton* exportButton;
};
#endif // HAVE_QTROOT
//_________________________________________________________________________________________________
} /* namespace root */
} /* namespace get */
#endif /* get_root_HistogramWidget_h_INCLUDED */
