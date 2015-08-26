/***************************************************************************
 * Copyright (C) 2010-2013 by Pablo Daniel Pareja Obregon                  *
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

#include "exportdialog.h"

#include "global.h"
#include "idocument.h"
#include "settings.h"

#include <QCompleter>
#include <QDialogButtonBox>
#include <QDirModel>
#include <QFileDialog>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMessageBox>
#include <QSvgGenerator>

#include <math.h>

namespace Caneda
{
    //! \brief Constructor.
    ExportDialog::ExportDialog(IDocument *document, QWidget *parent) :
            QDialog(parent),
            m_document(document)
    {
        ui.setupUi(this);

        // Title and file name of the scene
        QString diagramFilename = document->fileName();
        if(diagramFilename.isEmpty()) {
            diagramFilename = QObject::tr("untitled");
        }

        ui.labelSchematicName->setText(diagramFilename);
        ui.editFilename->setClearButtonEnabled(true);
        ui.editFilename->setText(diagramFilename);

        // Directory to save into
        if(document->fileName().isEmpty()) {
            ui.editPath->setText(QDir::toNativeSeparators(QDir::homePath()));
        }
        else {
            QFileInfo info(document->fileName());
            QString diagramPath = info.path();
            ui.editPath->setText(QDir::toNativeSeparators(diagramPath));
        }

        QCompleter *completer = new QCompleter(this);
        completer->setModel(new QDirModel(completer));
        ui.editPath->setCompleter(completer);

        ui.btnBrowse->setIcon(Caneda::icon("document-open"));

        // Geometry settings
        slotResetSize();

        ui.btnLock->setIcon(Caneda::icon("object-locked"));
        ui.btnLock->setToolTip(QObject::tr("Keep proportions"));

        ui.btnReset->setIcon(Caneda::icon("edit-clear-locationbar-rtl"));
        ui.btnReset->setToolTip(QObject::tr("Restore dimensions"));

        ui.comboFormat->addItem(tr("PNG (*.png)"), "PNG");
        ui.comboFormat->addItem(tr("JPEG (*.jpg)"), "JPG");
        ui.comboFormat->addItem(tr("Bitmap (*.bmp)"), "BMP");
        ui.comboFormat->addItem(tr("SVG (*.svg)"), "SVG");
        slotChangeFilesExtension();

        connect(ui.btnBrowse, SIGNAL(clicked()), SLOT(slotChooseDirectory()));
        connect(ui.spinWidth, SIGNAL(valueChanged(int)), SLOT(slotCorrectHeight()));
        connect(ui.spinHeight, SIGNAL(valueChanged(int)), SLOT(slotCorrectWidth()));
        connect(ui.btnLock, SIGNAL(toggled(bool)), SLOT(slotLockRatioChanged()));
        connect(ui.btnReset, SIGNAL(clicked()), this, SLOT(slotResetSize()));
        connect(ui.btnPreview, SIGNAL(clicked()), SLOT(slotPreview()));
        connect(ui.comboFormat, SIGNAL(currentIndexChanged(int)), SLOT(slotChangeFilesExtension()));

        connect(this, SIGNAL(accepted()), SLOT(slotExport()));
    }

    //! Ask the user for a folder destination
    void ExportDialog::slotChooseDirectory()
    {
        QString dir = QFileDialog::getExistingDirectory(this,
                tr("Export to folder"),
                QDir::homePath());

        if(!dir.isEmpty()) {
            ui.editPath->setText(dir);
        }
    }

    /*!
     * Method to adjust the width of the schematic if
     * "Lock Proportions" is selected.
     */
    void ExportDialog::slotCorrectWidth()
    {
        if(!ui.btnLock->isChecked()) {
            return;
        }

        qreal ratio = diagramRatio();

        ui.spinWidth->blockSignals(true);
        ui.spinWidth->setValue(ui.spinHeight->value() * ratio);
        ui.spinWidth->blockSignals(false);
    }

    /*!
     * Method to adjust the height of the schematic if
     * "Lock Proportions" is selected.
     */
    void ExportDialog::slotCorrectHeight()
    {
        if(!ui.btnLock->isChecked()) {
            return;
        }

        qreal ratio = diagramRatio();

        ui.spinHeight->blockSignals(true);
        ui.spinHeight->setValue(ui.spinWidth->value() / ratio);
        ui.spinHeight->blockSignals(false);
    }

    //! Update lock ratio button and correct geometry
    void ExportDialog::slotLockRatioChanged()
    {
        if(ui.btnLock->isChecked()) {
            ui.btnLock->setIcon(Caneda::icon("object-locked"));
        }
        else {
            ui.btnLock->setIcon(Caneda::icon("object-unlocked"));
        }

        if(ui.btnLock->isChecked()) {
            slotCorrectHeight();
        }
    }

    //! Reset the size of a schematic
    void ExportDialog::slotResetSize()
    {
        QSizeF size = m_document->documentSize();

        ui.spinWidth->blockSignals(true);
        ui.spinHeight->blockSignals(true);

        ui.spinWidth->setValue(size.width());
        ui.spinHeight->setValue(size.height());

        ui.spinWidth->blockSignals(false);
        ui.spinHeight->blockSignals(false);
    }

    //! Shows a preview dialog
    void ExportDialog::slotPreview()
    {
        QDialog preview_dialog;
        preview_dialog.setWindowTitle(tr("Preview"));
        preview_dialog.setWindowState(preview_dialog.windowState() | Qt::WindowMaximized);

        QGraphicsScene *preview_scene = new QGraphicsScene();
        preview_scene->setBackgroundBrush(Qt::lightGray);
        QGraphicsView *preview_view = new QGraphicsView(preview_scene);
        preview_view->setDragMode(QGraphicsView::ScrollHandDrag);
        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
        connect(buttons, SIGNAL(accepted()), &preview_dialog, SLOT(accept()));

        QVBoxLayout *vboxlayout1 = new QVBoxLayout();
        vboxlayout1->addWidget(preview_view);
        vboxlayout1->addWidget(buttons);
        preview_dialog.setLayout(vboxlayout1);

        QImage preview_image = generateImage();

        QGraphicsPixmapItem *qgpi = new QGraphicsPixmapItem(QPixmap::fromImage(preview_image));
        preview_scene->addItem(qgpi);
        preview_scene->setSceneRect(QRectF(0.0, 0.0, preview_image.width(), preview_image.height()));

        preview_dialog.exec();
    }

    //! Modify the filename extension according to the selected format
    void ExportDialog::slotChangeFilesExtension()
    {
        // Retrieve the format to use (short and extended)
        QString acronym = ui.comboFormat->itemData(ui.comboFormat->currentIndex()).toString();
        QString extension = "." + acronym.toLower();

        QString filename = ui.labelSchematicName->text();
        QFileInfo info(filename);
        filename = info.completeBaseName() + extension;

        ui.editFilename->setText(filename);
    }

    //! Export the scene to an image
    void ExportDialog::slotExport()
    {
        // Determine the destination folder
        QDir dir(ui.editPath->text());
        if(!dir.exists()) {
            QMessageBox::warning(this, tr("Folder not specified"),
                    tr("You must specify the path to the folder where the files will be exported."),
                    QMessageBox::Ok);
            return;
        }

        // Determine the filename to use
        QString filename = ui.editFilename->text();
        filename = dir.absoluteFilePath(filename);

        QFileInfo info(filename);
        if(info.exists() && !info.isWritable()) {
            QMessageBox::critical(this, tr("Could not write into file"),
                                  QString(tr("It looks like you do not have the necessary permissions to write in the file %1."))
                                  .arg(filename),
                                  QMessageBox::Ok);
            return;
        }

        // Save the image
        QFile file(filename);
        QString acronym = ui.comboFormat->itemData(ui.comboFormat->currentIndex()).toString();

        if(acronym == "SVG") {
            generateSvg(file);
        }
        else {
            QImage image = generateImage();
            image.save(&file, acronym.toUtf8().data());
        }

        file.close();
    }

    //! \return The aspect ratio of the schematic
    qreal ExportDialog::diagramRatio()
    {
        QSizeF size = m_document->documentSize();
        qreal ratio = size.width() / size.height();
        return(ratio);
    }

    /*!
     * \brief Generate an image to export
     *
     * \return Exported image
     */
    QImage ExportDialog::generateImage()
    {
        int width = ui.spinWidth->value();
        int height = ui.spinHeight->value();

        saveReloadDiagramParameters(true);

        QImage image(width, height, QImage::Format_RGB32);
        image.fill(qRgb(255, 255, 255));
        m_document->exportImage(image);

        saveReloadDiagramParameters(false);

        return(image);
    }

    /*!
     * \brief Exports the scene in SVG
     *
     * \param file File where SVG is being exported
     */
    void ExportDialog::generateSvg(QFile &file)
    {
        int width = ui.spinWidth->value();
        int height = ui.spinHeight->value();

        saveReloadDiagramParameters(true);

        QSvgGenerator svg_engine;
        svg_engine.setOutputDevice(&file);
        svg_engine.setSize(QSize(width, height));
        m_document->exportImage(svg_engine);

        saveReloadDiagramParameters(false);
    }

    /*!
     * \brief Saves or restores the parameters of the scene
     *
     * \param save True to save the settings of the scene and apply those
     * defined by the form, false to restore the settings
     */
    void ExportDialog::saveReloadDiagramParameters(bool save)
    {
        static bool state_drawGrid;

        if(save) {
            // save the parameters
            state_drawGrid = Settings::instance()->currentValue("gui/gridVisible").value<bool>();

            Settings::instance()->setCurrentValue("gui/gridVisible", ui.checkDrawGrid->isChecked());
        }
        else {
            // restore the parameters
            Settings::instance()->setCurrentValue("gui/gridVisible", state_drawGrid);
        }
    }

} // namespace Caneda
