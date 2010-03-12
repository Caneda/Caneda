/***************************************************************************
 * Copyright 2006-2009 Xavier Guerrin                                      *
 * Copyright 2009 Pablo Daniel Pareja Obregon                              *
 * This file was part of QElectroTech and modified by Pablo Daniel Pareja  *
 * Obregon to be included in Qucs.                                         *
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

#include "qucs-tools/global.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QPrintDialog>
#include <QPrinter>
#include <QRadioButton>

#include <cmath>

/*!
 * Constructor
 * @param schemaToPrint  Schematic to print
 * @param parent  parent Widget of the dialogue
 */
PrintDialog::PrintDialog(SchematicScene *schemaToPrint, QWidget *parent) :
    QWidget(parent),
    dialog(0)
{
    schema = schemaToPrint;

    printer = new QPrinter();
    printer->setOrientation(QPrinter::Landscape);

    setDocName(schema->fileName());
    setFileName(schema->fileName());

    if(!docname.isEmpty()) {
        printer->setDocName(docname);
    }

    //First we display a dialog asking the user the kind of impression he wants to make
    buildPrintTypeDialog();
    if(dialog->exec() == QDialog::Rejected) {
        return;
    }

    //Printer settings according to the type of printing chosen
    if(printerChoice->isChecked()) {
        QPrintDialog printDialog(printer, parentWidget());
        printDialog.setWindowTitle(tr("Print options", "window title"));
        printDialog.setEnabledOptions(QAbstractPrintDialog::PrintShowPageSize);
        if(printDialog.exec() == QDialog::Rejected) {
            return;
        }
    }
    else if(pdfChoice->isChecked()) {
        printer->setOutputFormat(QPrinter::PdfFormat);
        printer->setOutputFileName(editFilepath->text());
    }
    else {
        printer->setOutputFormat(QPrinter::PostScriptFormat);
        printer->setOutputFileName(editFilepath->text());
    }

    print(fitInPage->isChecked());
}

//! Destructor
PrintDialog::~PrintDialog()
{
    delete dialog;
    delete printer;
}

//! Defines the file name if the user selects output to a PDF
void PrintDialog::setFileName(const QString &name)
{
    filename = name;
}

//! @return PDF filename
QString PrintDialog::fileName() const
{
    return(filename);
}

//! Defines the document name
void PrintDialog::setDocName(const QString &name)
{
    docname = name;
}

//! @return Document name
QString PrintDialog::docName() const
{
    return(docname);
}

/*!
 * @param fullpage true to use the entire sheet in the calculation
 * @return Number of pages needed to print the schema with the
 * paper used in the current printer.
 */
int PrintDialog::pagesCount(bool fullpage) const
{
    return(horizontalPagesCount(fullpage) * verticalPagesCount(fullpage));
}

/*!
 * @param fullpage true to use the entire sheet in the calculation
 * @return The width in pages to print the schema with the paper
 *         used in the current printer.
 */
int PrintDialog::horizontalPagesCount(bool fullpage) const
{
    // pageRect and paperRect reflect the orientation of the paper
    QRectF printable_area = fullpage ? printer->paperRect() : printer->pageRect();
    QRectF schema_rect = schema->imageBoundingRect();
    QSizeF schema_size = QSizeF(schema_rect.width(), schema_rect.height());

    int h_pages_count =
        int(ceil(schema_size.width() / printable_area.width()));
    return(h_pages_count);
}

/*!
 * @param fullpage true to use the entire sheet in the calculation
 * @return The height in pages to print the schema with the paper
 *         used in the current printer.
 */
int PrintDialog::verticalPagesCount(bool fullpage) const
{
    //pageRect and paperRect reflect the orientation of the paper
    QRectF printable_area = fullpage ? printer->paperRect() : printer->pageRect();
    QRectF schema_rect = schema->imageBoundingRect();
    QSizeF schema_size = QSizeF(schema_rect.width(), schema_rect.height());

    int v_pages_count =
        int(ceil(schema_size.height() / printable_area.height()));
    return(v_pages_count);
}

/*!
 * Construct a non-standard dialog asking the user what type of print
 * to perform: PDF, PS or physical printer
 */
void PrintDialog::buildPrintTypeDialog()
{
    dialog = new QDialog(parentWidget());
    dialog->setWindowTitle(tr("Printing type choice"));
    dialog->setMinimumWidth(400);

    QLabel *labelPrintType  = new QLabel(tr("What kind of printing do you wish?"));
    QLabel *iconPrinter = new QLabel();
    QLabel *iconPdf = new QLabel();
    QLabel *iconPs = new QLabel();

    iconPrinter->setPixmap(QPixmap(Qucs::bitmapDirectory() + "printer.png"));
    iconPdf->setPixmap(QPixmap(Qucs::bitmapDirectory() + "pdf.png"));
    iconPs->setPixmap(QPixmap(Qucs::bitmapDirectory() + "eps.png"));

    QButtonGroup *printtypeChoice = new QButtonGroup();
    printerChoice = new QRadioButton(tr("Print to a physical printer",
                "Print type choice"));
    pdfChoice = new QRadioButton(tr("Print to a PDF file",
                "Print type choice"));
    psChoice = new QRadioButton(tr("Print to a PostScript file (PS)", "Print type choice"));

    printtypeChoice->addButton(printerChoice);
    printtypeChoice->addButton(pdfChoice);
    printtypeChoice->addButton(psChoice);
    printerChoice->setChecked(true);

    editFilepath = new QLineEdit();
    browseButton = new QPushButton("...");

    QLabel *iconFitInPage = new QLabel();
    iconFitInPage->setPixmap(QPixmap(Qucs::bitmapDirectory() + "viewmagfit.png"));
    fitInPage = new QCheckBox("Fit result into page");
    fitInPage->setChecked(true);

    QDialogButtonBox *buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    if(!filename.isEmpty()) {
        editFilepath->setText(filename + ".pdf");
    }

    connect(printerChoice, SIGNAL(toggled(bool)), SLOT(updatePrintTypeDialog()));
    connect(pdfChoice, SIGNAL(toggled(bool)), SLOT(updatePrintTypeDialog()));
    connect(psChoice, SIGNAL(toggled(bool)), SLOT(updatePrintTypeDialog()));
    connect(browseButton, SIGNAL(clicked(bool)), SLOT(browseFilePrintTypeDialog()));
    connect(buttons, SIGNAL(accepted()), SLOT(acceptPrintTypeDialog()));
    connect(buttons, SIGNAL(rejected()), dialog, SLOT(reject()));

    //Organize layout
    QGridLayout *glayout = new QGridLayout();
    QHBoxLayout *hlayout = new QHBoxLayout();
    QVBoxLayout *vlayout = new QVBoxLayout();

    hlayout->addWidget(editFilepath);
    hlayout->addWidget(browseButton);

    glayout->addWidget(iconPrinter, 0, 0);
    glayout->addWidget(printerChoice, 0, 1);
    glayout->addWidget(iconPdf, 1, 0);
    glayout->addWidget(pdfChoice, 1, 1);
    glayout->addWidget(iconPs, 2, 0);
    glayout->addWidget(psChoice, 2, 1);
    glayout->addLayout(hlayout, 3, 1);
    glayout->addWidget(iconFitInPage, 4, 0);
    glayout->addWidget(fitInPage, 4, 1);

    vlayout->addWidget(labelPrintType);
    vlayout->addLayout(glayout);
    vlayout->addWidget(buttons);

    dialog->setLayout(vlayout);

    updatePrintTypeDialog();
}

//! Ensures coherence of the type of printing dialogue
void PrintDialog::updatePrintTypeDialog()
{
    bool file_print = !(printerChoice->isChecked());

    editFilepath->setEnabled(file_print);
    browseButton->setEnabled(file_print);

    if(file_print) {
        QString filepath = editFilepath->text();
        if(!filepath.isEmpty()) {
            if(pdfChoice->isChecked() && filepath.endsWith(".ps")) {
                QRegExp re("\\.ps$", Qt::CaseInsensitive);
                filepath.replace(re, ".pdf");
                editFilepath->setText(filepath);
            }
            else if(psChoice->isChecked() && filepath.endsWith(".pdf")) {
                QRegExp re("\\.pdf$", Qt::CaseInsensitive);
                filepath.replace(re, ".ps");
                editFilepath->setText(filepath);
            }
        }
    }
}

//! Allows the user to select a file
void PrintDialog::browseFilePrintTypeDialog()
{
    QString extension;
    QString filter;

    if(printerChoice->isChecked()) {
        return;
    }
    else if(pdfChoice->isChecked()) {
        extension = ".pdf";
        filter    = tr("PDF Files (*.pdf)", "file filter");
    }
    else if(psChoice->isChecked()) {
        extension = ".ps";
        filter    = tr("PostScript Files (*.ps)", "file filter");
    }

    QString filepath = QFileDialog::getSaveFileName(parentWidget(),
            QString(),
            editFilepath->text(),
            filter);

    if(!filepath.isEmpty()) {
        if(!filepath.endsWith(extension)) {
            filepath += extension;
        }
        editFilepath->setText(filepath);
    }
}

//! Checks the status of the print type dialogue.
void PrintDialog::acceptPrintTypeDialog()
{
    bool file_print = !(printerChoice->isChecked());

    if(file_print) {
        if(editFilepath->text().isEmpty()) {
            QMessageBox::information(parentWidget(),
                    tr("File missing", "message box title"),
                    tr("You must enter the path of the PDF/PS file to create.",
                        "message box content"));
        }
        else {
            dialog->accept();
        }
    }
    else {
        dialog->accept();
    }
}

/*!
 * Print a schematic
 * @param fit_page True to adapt the schematic to page, false otherwise
 */
void PrintDialog::print(bool fit_page)
{
    QPainter qp(printer);

    if(!schema) {
        qp.end();
        return;
    }

    bool full_page = printer->fullPage();

    bool viewGridStatus = schema->isGridVisible();
    schema->setGridVisible(false);

    if(fit_page) {
        schema->render(&qp, QRectF(), schema->imageBoundingRect(), Qt::KeepAspectRatio);
    }
    else {
        //Printing on one or more pages
        QRectF diagram_rect = schema->imageBoundingRect();
        QRectF printed_area = full_page ? printer->paperRect() : printer->pageRect();

        qreal used_width  = printed_area.width();
        qreal used_height = printed_area.height();
        int h_pages_count = horizontalPagesCount(full_page);
        int v_pages_count = verticalPagesCount(full_page);

        QVector< QVector< QRectF > > pages_grid;

        //The schematic is printed on a grid of sheets
        qreal y_offset = 0;
        for(int i = 0; i < v_pages_count; ++i) {
            pages_grid << QVector< QRectF >();

            //Runs through the sheets of the line
            qreal x_offset = 0;
            for(int j = 0; j < h_pages_count; ++j) {
                pages_grid.last() << QRectF(QPointF(x_offset, y_offset),
                        QSizeF(qMin(used_width, diagram_rect.width()  - x_offset),
                            qMin(used_height, diagram_rect.height() - y_offset)));
                x_offset += used_width;
            }
            y_offset += used_height;
        }

        //Retains only the pages for printing
        QVector<QRectF> pages_to_print;
        for(int i = 0; i < v_pages_count; ++i) {
            for(int j = 0; j < h_pages_count; ++j) {
                pages_to_print << pages_grid.at(i).at(j);
            }
        }

        //Scan the pages for printing
        for(int i = 0; i < pages_to_print.count(); ++i) {
            QRectF current_rect(pages_to_print.at(i));
            schema->render(&qp, QRectF(QPointF(0,0), current_rect.size()),
                    current_rect.translated(diagram_rect.topLeft()), Qt::KeepAspectRatio);
            if(i != pages_to_print.count()-1) {
                printer->newPage();
            }
        }
    }

    schema->setGridVisible(viewGridStatus);
}
