/***************************************************************************
 * Copyright 2006-2009 Xavier Guerrin                                      *
 * Copyright 2009 Pablo Daniel Pareja Obregon                              *
 * This file was part of QElectroTech and modified by Pablo Daniel Pareja  *
 * Obregon to be included in Caneda.                                       *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "printdialog.h"

#include "idocument.h"
#include "schematicscene.h"
#include "settings.h"

#include "caneda-tools/global.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QPrintDialog>
#include <QPrinter>
#include <QRadioButton>

#include <cmath>

namespace Caneda
{
    PrintDialog::PrintDialog(IDocument *document, QWidget *parent) :
        QDialog(parent),
        m_printer(0),
        m_document(document)
    {
        ui.setupUi(this);
        ui.widget->setEnabled(false);
        ui.printerChoice->setIcon(QIcon(Caneda::bitmapDirectory() + "printer.png"));
        ui.pdfChoice->setIcon(QIcon(Caneda::bitmapDirectory() + "pdf.png"));
        ui.psChoice->setIcon(QIcon(Caneda::bitmapDirectory() + "eps.png"));
        ui.fitInPageButton->setIcon(QIcon(Caneda::bitmapDirectory() + "viewmagfit.png"));

        if (!m_document->printSupportsFitInPage()) {
            ui.fitInPageButton->setChecked(false);
            ui.fitInPageButton->hide();
        }

        connect(ui.printerChoice, SIGNAL(toggled(bool)), this, SLOT(onChoiceToggled()));
        connect(ui.pdfChoice, SIGNAL(toggled(bool)), this, SLOT(onChoiceToggled()));
        connect(ui.psChoice, SIGNAL(toggled(bool)), this, SLOT(onChoiceToggled()));
        connect(ui.browseButton, SIGNAL(clicked()), this, SLOT(onBrowseButtonClicked()));

        m_printer = new QPrinter;
        m_printer->setOrientation(QPrinter::Landscape);

        const QString fileName = m_document->fileName();
        if (!fileName.isEmpty()) {
            m_printer->setDocName(fileName);
            ui.filePathEdit->setText(fileName + ".pdf");
        }
    }

    //! Destructor
    PrintDialog::~PrintDialog()
    {
        delete m_printer;
    }

    void PrintDialog::done(int r)
    {
        if (r == QDialog::Accepted) {
            if (ui.printerChoice->isChecked()) {
                QPointer<QPrintDialog> d(new QPrintDialog);
                d->setWindowTitle(tr("Print options", "window title"));
                d->setEnabledOptions(QAbstractPrintDialog::PrintShowPageSize);

                int status = d->exec();

                delete d.data();

                if(status == QDialog::Rejected) {
                    return;
                }
            }
            else if (ui.pdfChoice->isChecked()) {
                m_printer->setOutputFormat(QPrinter::PdfFormat);
                m_printer->setOutputFileName(ui.filePathEdit->text());
            }
            else if (ui.psChoice->isChecked()) {
                m_printer->setOutputFormat(QPrinter::PostScriptFormat);
                m_printer->setOutputFileName(ui.filePathEdit->text());
            }

            m_document->print(m_printer, ui.fitInPageButton->isChecked());
        }

        QDialog::done(r);
    }

    void PrintDialog::onChoiceToggled()
    {
        if (ui.printerChoice->isChecked()) {
            ui.widget->setEnabled(false);
        } else {
            QString path = ui.filePathEdit->text();
            if (ui.pdfChoice->isChecked()) {
                if (path.endsWith(".ps")) {
                    path.replace(path.length() - 3, 3, ".pdf");
                }
            } else /* ui.psChoice->isChecked() */{
                if (path.endsWith(".pdf")) {
                    path.replace(path.length() - 4, 4, ".ps");
                }
            }
            ui.filePathEdit->setText(path);
            ui.widget->setEnabled(true);
        }
    }

    //! Allows the user to select a file
    void PrintDialog::onBrowseButtonClicked()
    {
        QString extension;
        QString filter;

        if(ui.printerChoice->isChecked()) {
            return;
        }
        else if(ui.pdfChoice->isChecked()) {
            extension = ".pdf";
            filter    = tr("PDF Files (*.pdf)", "file filter");
        }
        else if(ui.psChoice->isChecked()) {
            extension = ".ps";
            filter    = tr("PostScript Files (*.ps)", "file filter");
        }

        QString filepath = QFileDialog::getSaveFileName(this,
                QString(),
                ui.filePathEdit->text(),
                filter);

        if(!filepath.isEmpty()) {
            if(!filepath.endsWith(extension)) {
                filepath += extension;
            }
            ui.filePathEdit->setText(filepath);
        }
    }

} // namespace Caneda
