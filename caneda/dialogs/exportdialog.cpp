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

#include "exportdialog.h"

#include "qucs-tools/global.h"

#include <math.h>

#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QDirModel>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QSignalMapper>
#include <QSpinBox>
#include <QSvgGenerator>
#include <QVBoxLayout>

/*!
 * Constructor
 * @param QWidget *parent The parent of the dialog.
 */
ExportDialog::ExportDialog(QList<SchematicScene *> schemasToExport,
        QWidget *parent) : QDialog(parent)
{
    schemas = schemasToExport;

    // The minimum size of the dialogue is fixed
    setMinimumSize(800, 390);
    resize(minimumSize());
    setWindowTitle(tr("Export schematics", "window title"));

    // The dialog has two buttons
    buttons = new QDialogButtonBox(this);
    buttons->setOrientation(Qt::Horizontal);
    buttons->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);
    buttons->button(QDialogButtonBox::Save)->setText(tr("Export"));

    // Layout of elements
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(
            new QLabel(tr("Select the diagrams you want to export and their dimensions:")));
    layout->addWidget(diagramsListPart(), 1);
    layout->addWidget(lastPart());
    layout->addWidget(buttons);
    slot_changeFilesExtension(true);

    // Conections signal/slots
    connect(format, SIGNAL(currentIndexChanged(int)), SLOT(slot_changeFilesExtension()));
    connect(buttons, SIGNAL(accepted()), SLOT(slot_export()));
    connect(buttons, SIGNAL(rejected()), SLOT(reject()));
}

ExportDialog::~ExportDialog()
{
}

/*!
 * Sets up the list of schemes
 * @return The widget representing the list of schematics
 */
QWidget *ExportDialog::diagramsListPart()
{
    preview_mapper_ = new QSignalMapper(this);
    width_mapper_ = new QSignalMapper(this);
    height_mapper_ = new QSignalMapper(this);
    ratio_mapper_ = new QSignalMapper(this);
    reset_mapper_ = new QSignalMapper(this);

    connect(preview_mapper_, SIGNAL(mapped(int)), SLOT(slot_previewDiagram(int)));
    connect(width_mapper_, SIGNAL(mapped(int)), SLOT(slot_correctHeight(int)));
    connect(height_mapper_, SIGNAL(mapped(int)), SLOT(slot_correctWidth(int)));
    connect(ratio_mapper_, SIGNAL(mapped(int)), SLOT(slot_keepRatioChanged(int)));
    connect(reset_mapper_, SIGNAL(mapped(int)), this, SLOT(slot_resetSize(int)));

    QGridLayout *layoutDiagramsList = new QGridLayout();

    int line_count = 0;
    layoutDiagramsList->addWidget(new QLabel(tr("Schematic")),
            line_count, 1, Qt::AlignHCenter | Qt::AlignVCenter);
    layoutDiagramsList->addWidget(new QLabel(tr("File name")),
            line_count, 2, Qt::AlignHCenter | Qt::AlignVCenter);
    layoutDiagramsList->addWidget(new QLabel(tr("Dimensions")),
            line_count, 3, Qt::AlignHCenter | Qt::AlignVCenter);

    // fill the list
    foreach(SchematicScene *sch, schemas) {
        ++line_count;
        ExportDiagramLine *diagram = new ExportDiagramLine(sch);
        diagramsList.insert(line_count, diagram);
        layoutDiagramsList->addWidget(diagram->must_export, line_count, 0);
        layoutDiagramsList->addWidget(diagram->title_label, line_count, 1);
        layoutDiagramsList->addWidget(diagram->file_name, line_count, 2);
        layoutDiagramsList->addLayout(diagram->sizeLayout(), line_count, 3);

        // ifyou uncheck all the schemas, the button "Export" is disabled
        connect(diagram->must_export, SIGNAL(toggled(bool)),
                SLOT(slot_checkDiagramsCount()));

        // mappings and signals for the dimension management of schematics
        width_mapper_ ->setMapping(diagram->width, line_count);
        height_mapper_->setMapping(diagram->height, line_count);
        ratio_mapper_ ->setMapping(diagram->keep_ratio, line_count);
        reset_mapper_ ->setMapping(diagram->reset_size, line_count);
        connect(diagram->width, SIGNAL(valueChanged(int)), width_mapper_, SLOT(map()));
        connect(diagram->height, SIGNAL(valueChanged(int)), height_mapper_, SLOT(map()));
        connect(diagram->keep_ratio, SIGNAL(toggled(bool)), ratio_mapper_, SLOT(map()));
        connect(diagram->reset_size, SIGNAL(clicked(bool)), reset_mapper_, SLOT(map()));

        // mappings and signals for the overview of the schema
        preview_mapper_->setMapping(diagram->preview, line_count);
        connect(diagram->preview, SIGNAL(clicked(bool)), preview_mapper_, SLOT(map()));
    }

    QWidget *widgetDiagramsList = new QWidget();
    widgetDiagramsList->setLayout(layoutDiagramsList);
    QScrollArea *scrollDiagramsList = new QScrollArea();
    scrollDiagramsList->setWidget(widgetDiagramsList);

    return(scrollDiagramsList);
}

/*!
 * Sets up the left side of the dialogue
 * @return The widget representing the last half of the dialogue
 */
QWidget *ExportDialog::lastPart()
{
    QWidget *returnWidget = new QWidget();

    // The last side of the dialogue is a vertical stack of elements
    QVBoxLayout *vboxLayout = new QVBoxLayout(returnWidget);
    QHBoxLayout *hboxLayout = new QHBoxLayout();

    QLabel *labelDirpath = new QLabel(tr("Target folder:"), this);
    dirpath = new QLineEdit(this);
    dirpath->setText(QDir::toNativeSeparators(QDir::homePath()));
    QCompleter *completer = new QCompleter(this);
    completer->setModel(new QDirModel(completer));
    dirpath->setCompleter(completer);
    QPushButton *buttonBrowse = new QPushButton(tr("Browse"), this);

    hboxLayout->addWidget(labelDirpath);
    hboxLayout->addWidget(dirpath);
    hboxLayout->addWidget(buttonBrowse);
    hboxLayout->addStretch();

    vboxLayout->addLayout(hboxLayout);

    QHBoxLayout *hboxLayout1 = new QHBoxLayout();
    hboxLayout1->addWidget(new QLabel(tr("Format:"), this));
    hboxLayout1->addWidget(format = new QComboBox(this));
    format->addItem(tr("PNG (*.png)"), "PNG");
    format->addItem(tr("JPEG (*.jpg)"), "JPG");
    format->addItem(tr("Bitmap (*.bmp)"), "BMP");
    format->addItem(tr("SVG (*.svg)"), "SVG");
    hboxLayout1->addStretch();

    vboxLayout->addLayout(hboxLayout1);

    vboxLayout->addWidget(setupOptionsGroupBox());
    vboxLayout->addStretch();

    setTabOrder(dirpath, buttonBrowse);
    setTabOrder(buttonBrowse, format);
    setTabOrder(format, draw_frame);
    setTabOrder(draw_frame, draw_grid);

    connect(buttonBrowse, SIGNAL(released()), SLOT(slot_chooseDirectory()));

    return(returnWidget);
}

/*!
 * Implements the part of the dialogue where the user enters the
 * options of the desired image.
 * @return The QGroupBox to set options for the image
 */
QGroupBox *ExportDialog::setupOptionsGroupBox()
{
    QGroupBox *groupboxOptions = new QGroupBox(tr("Options"), this);
    QGridLayout *layoutOptions = new QGridLayout(groupboxOptions);

    // Choose the options to export
    draw_grid = new QCheckBox(tr("Draw grid"), groupboxOptions);
    draw_frame = new QCheckBox(tr("Draw frame"), groupboxOptions);
    draw_frame->setChecked(true);

    layoutOptions->addWidget(draw_grid, 0, 0);
    layoutOptions->addWidget(draw_frame, 1, 0);

    connect(draw_frame, SIGNAL(clicked()), SLOT(slot_changeUseFrame()));

    return(groupboxOptions);
}

//! @return number of schematics to export
int ExportDialog::diagramsToExportCount() const
{
    int checked_diagrams_count = 0;
    foreach(ExportDiagramLine *diagram, diagramsList.values()) {
        if(diagram->must_export->isChecked()) {
            ++ checked_diagrams_count;
        }
    }
    return(checked_diagrams_count);
}

/*!
 * @param schematic An schematic
 * @return the aspect ratio of the schema
 */
qreal ExportDialog::diagramRatio(SchematicScene *schematic)
{
    QSizeF diagram_size = diagramSize(schematic);
    qreal diagram_ratio = diagram_size.width() / diagram_size.height();
    return(diagram_ratio);
}

/*!
 * @param schematic An schematic
 * @return dimensions of the schematic, taking into account the type of export: frame
 * or elements
 */
QSizeF ExportDialog::diagramSize(SchematicScene *schematic)
{
    bool state_useFrame = schematic->isFrameVisible();

    schematic->setFrameVisible(draw_frame->isChecked());

    QRectF diagram_rect = schematic->imageBoundingRect();
    QSizeF diagram_size = QSizeF(diagram_rect.width(), diagram_rect.height());

    schematic->setFrameVisible(state_useFrame);

    return(diagram_size);
}

/*!
 * This method adjusts the width of the schematic if and only if
 * "Constrain Proportions" is selected.
 * @param diagram_id number of schema
 */
void ExportDialog::slot_correctWidth(int diagram_id)
{
    ExportDialog::ExportDiagramLine *current_diagram = diagramsList[diagram_id];
    if(!current_diagram) {
        return;
    }
    if(!(current_diagram->keep_ratio->isChecked())) {
        return;
    }

    qreal diagram_ratio = diagramRatio(current_diagram->schema);

    current_diagram->width->blockSignals(true);
    current_diagram->width->setValue(
            current_diagram->height->value() * diagram_ratio);
    current_diagram->width->blockSignals(false);
}

/*!
 * This method adjusts the height of the schematic ifand only if
 * "Constrain Proportions" is selected.
 * @param diagram_id number of schema
 */
void ExportDialog::slot_correctHeight(int diagram_id)
{
    ExportDialog::ExportDiagramLine *current_diagram = diagramsList[diagram_id];
    if(!current_diagram) {
        return;
    }
    if(!(current_diagram->keep_ratio->isChecked())) {
        return;
    }

    qreal diagram_ratio = diagramRatio(current_diagram->schema);

    current_diagram->height->blockSignals(true);
    current_diagram->height->setValue(
            current_diagram->width->value() / diagram_ratio);
    current_diagram->height->blockSignals(false);
}

/*!
 * Takes into account the fact that we must now keep or not
 * the proportion of schematics
 * @param diagram_id number of schema
 */
void ExportDialog::slot_keepRatioChanged(int diagram_id)
{
    ExportDialog::ExportDiagramLine *current_diagram = diagramsList[diagram_id];
    if(!current_diagram) return;

    // manages the icon of the button "Keep proportions"
    if(current_diagram->keep_ratio->isChecked()) {
        current_diagram->keep_ratio->setIcon(
                QPixmap(Qucs::bitmapDirectory() + "object-locked.png"));
    }
    else {
        current_diagram->keep_ratio->setIcon(
                QPixmap(Qucs::bitmapDirectory() + "object-unlocked.png"));
    }

    // does nothing if"Constrain Proportions" is not checked
    if(!(current_diagram->keep_ratio->isChecked())) {
        return;
    }

    // on the contrary, ifit is enabled, adjust the height depending on the width
    slot_correctHeight(diagram_id);
}

/*!
 * Reset the size of a schematic
 * @param diagram_id number of schema
 */
void ExportDialog::slot_resetSize(int diagram_id)
{
    ExportDialog::ExportDiagramLine *current_diagram = diagramsList[diagram_id];
    if(!current_diagram) {
        return;
    }

    QSizeF diagram_size = diagramSize(current_diagram->schema);

    current_diagram->width->blockSignals(true);
    current_diagram->height->blockSignals(true);
    current_diagram->width->setValue(diagram_size.width());
    current_diagram->height->setValue(diagram_size.height());
    current_diagram->width->blockSignals(false);
    current_diagram->height->blockSignals(false);
}

//! Slot asking the user to choose a folder
void ExportDialog::slot_chooseDirectory()
{
    QString user_dir = QFileDialog::getExistingDirectory(this,
            tr("Export to folder", "dialog title"),
            QDir::homePath());
    if(!user_dir.isEmpty()) {
        dirpath->setText(user_dir);
    }
}

//! Slot called to export an image
void ExportDialog::slot_export()
{

    QList<ExportDiagramLine *> diagrams_to_export;
    foreach(ExportDiagramLine *diagram, diagramsList.values()) {
        if(diagram->must_export->isChecked()) {
            diagrams_to_export << diagram;
        }
    }

    // verification #1 : check each schema has different filename
    QSet<QString> filenames;
    foreach(ExportDiagramLine *diagram, diagrams_to_export) {
        QString diagram_file = diagram->file_name->text();
        if(!diagram_file.isEmpty()) {
            filenames << diagram_file;
        }
    }
    if(filenames.count() != diagrams_to_export.count()) {
        QMessageBox::warning(this, tr("Names of target files", "message box title"),
                tr("You must enter a different filename for each schematic to export.",
                    "message box content"));
        return;
    }

    // verification #2 : a path to a file must have been specified
    QDir target_dir_path(dirpath->text());
    if(dirpath->text().isEmpty() || !target_dir_path.exists()) {
        QMessageBox::warning(this, tr("Folder not specified", "message box title"),
                tr("You must specify the path to the folder where the files will be exported.",
                    "message box content"), QMessageBox::Ok);
        return;
    }

    // exports each schema to selected
    foreach(ExportDiagramLine *diagram, diagrams_to_export) {
        exportDiagram(diagram);
    }

    accept();
}

/*!
 * Exports a schematic
 * @param diagram The line describing the schematic to export and the way
 * to export it
 */
void ExportDialog::exportDiagram(ExportDiagramLine *diagram)
{
    // retrieves the format to use (short and extended)
    QString format_acronym = format->itemData(format->currentIndex()).toString();
    QString format_extension = "." + format_acronym.toLower();

    // determines the file name to use
    QString diagram_path = diagram->file_name->text();

    // determines the path of the file
    QDir target_dir_path(dirpath->text());
    diagram_path = target_dir_path.absoluteFilePath(diagram_path);

    // retrieves information about the specified file
    QFileInfo file_infos(diagram_path);

    // verification that it is possible to write the file
    if(file_infos.exists() && !file_infos.isWritable()) {
        QMessageBox::critical(this, tr("Could not write into file", "message box title"),
                QString(tr("It looks like you do not have the necessary permissions to write in the file %1.",
                        "message box content")).arg(diagram_path),
                QMessageBox::Ok);
        return;
    }

    // opens the file
    QFile target_file(diagram_path);

    // saves the image in the file
    if(format_acronym == "SVG") {
        generateSvg(
                diagram->schema,
                diagram->width->value(),
                diagram->height->value(),
                diagram->keep_ratio->isChecked(),
                target_file );
    }
    else {
        QImage image = generateImage(
                diagram->schema,
                diagram->width->value(),
                diagram->height->value(),
                diagram->keep_ratio->isChecked() );
        image.save(&target_file, format_acronym.toUtf8().data());
    }

    target_file.close();
}

/*!
 * Generate the image to export
 * @param sch Schematic to export
 * @param width  width to export
 * @param height height to export
 * @param keep_aspect_ratio True to keep ratio, false otherwise
 * @return Exported image
 */
QImage ExportDialog::generateImage(SchematicScene *sch, int width, int height,
        bool keep_aspect_ratio)
{
    saveReloadDiagramParameters(sch, true);

    QImage image(width, height, QImage::Format_RGB32);
    image.fill(qRgb(255, 255, 255));
    sch->toPaintDevice(image, width, height,
            keep_aspect_ratio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);

    saveReloadDiagramParameters(sch, false);

    return(image);
}

/*!
 * Exports the schema in SVG
 * @param sch Schematic to export to SVG
 * @param width  width to export
 * @param height height to export
 * @param keep_aspect_ratio True to keep ratio, false otherwise
 * @param file File to save the SVG code
 */
void ExportDialog::generateSvg(SchematicScene *sch, int width, int height,
        bool keep_aspect_ratio, QFile &file)
{
    saveReloadDiagramParameters(sch, true);

    // QPicture generated from the schematic
    QSvgGenerator svg_engine;
    svg_engine.setOutputDevice(&file);

    sch->toPaintDevice(svg_engine, width, height,
            keep_aspect_ratio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);

    saveReloadDiagramParameters(sch, false);
}

/*!
 * Saves or restores the parameters of the schematic
 * @param sch Schematic whose settings are to be saved or restored
 * @param save True to save the settings of the schematic and apply those
 * defined by the form, false to restore the settings
 */
void ExportDialog::saveReloadDiagramParameters(SchematicScene *sch, bool save)
{
    static bool state_drawFrame;
    static bool state_drawGrid;

    if(save) {
        // memorize the parameters for the schematic
        state_drawFrame = sch->isFrameVisible();
        state_drawGrid = sch->isGridVisible();

        sch->setGridVisible(draw_grid->isChecked());
        sch->setFrameVisible(draw_frame->isChecked());
    }
    else {
        // restore the parameters for the schematic
        sch->setGridVisible(state_drawGrid);
        sch->setFrameVisible(state_drawFrame);
    }
}

/*!
 * Slot called when the user changes the area of schema which must be
 * exported. It must then adjust dimensons of schematics.
 */
void ExportDialog::slot_changeUseFrame()
{
    foreach(int diagram_id, diagramsList.keys()) {
        ExportDiagramLine *diagram = diagramsList[diagram_id];
        slot_resetSize(diagram_id);
    }
}

/*!
 * This slot enables or disables the button "Export" depending on the number of
 * schematics selected.
 */
void ExportDialog::slot_checkDiagramsCount()
{
    QPushButton *export_button = buttons->button(QDialogButtonBox::Save);
    export_button->setEnabled(diagramsToExportCount());
}

/*!
 * Modify the extensions of files according to the selected format
 * @param force_extension True to add the extension ifnot present
 * or false to simply modify ifit is incorrect.
 */
void ExportDialog::slot_changeFilesExtension(bool force_extension)
{
    // retrieves the format to use (short and extended)
    QString format_acronym = format->itemData(format->currentIndex()).toString();
    QString format_extension = "." + format_acronym.toLower();

    foreach(ExportDiagramLine *diagram, diagramsList.values()) {
        QString diagram_filename = diagram->file_name->text();

        // case 1 : extension is present and correct: it does nothing
        if(diagram_filename.endsWith(format_extension, Qt::CaseInsensitive)) {
            continue;
        }

        QFileInfo diagram_filename_info(diagram_filename);
        // case 2 : extension is absent
        if(diagram_filename_info.suffix().isEmpty()){
            if(force_extension) {
                diagram_filename = diagram_filename_info.completeBaseName() + format_extension;
            }
        }
        // case 3 : extension is present but wrong
        else {
            diagram_filename = diagram_filename_info.completeBaseName() + format_extension;
        }

        diagram->file_name->setText(diagram_filename);
    }
}

/*!
 * This method shows a dialog to resize and preview the schematic to export
 * @param diagram_id number of schema
 */
void ExportDialog::slot_previewDiagram(int diagram_id)
{
    ExportDialog::ExportDiagramLine *current_diagram = diagramsList[diagram_id];
    if(!current_diagram) {
        return;
    }

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

    QImage preview_image = generateImage(
            current_diagram->schema,
            current_diagram->width->value(),
            current_diagram->height->value(),
            current_diagram->keep_ratio->isChecked()
            );

    QGraphicsPixmapItem *qgpi = new QGraphicsPixmapItem(QPixmap::fromImage(preview_image));
    preview_scene->addItem(qgpi);
    preview_scene->setSceneRect(QRectF(0.0, 0.0, preview_image.width(), preview_image.height()));

    preview_dialog.exec();
}

/*!
 * Constructor
 * @param sch Schematic
 */
ExportDialog::ExportDiagramLine::ExportDiagramLine(SchematicScene *sch)
{
    schema = sch;
    must_export = new QCheckBox();
    must_export->setChecked(true);

    // title and file name of the schema
    QString diagram_title = schema->fileName();
    if(diagram_title.isEmpty()) diagram_title = QObject::tr("Untitled");
    QString diagram_filename = schema->fileName();
    if(diagram_filename.isEmpty()) diagram_filename = QObject::tr("schematic");

    title_label = new QLabel(diagram_title);

    file_name = new QLineEdit();
    file_name->setText(diagram_filename);
    file_name->setMinimumWidth(180);

    QSize diagram_size = QSize(schema->sceneRect().width(), schema->sceneRect().height());

    width = new QSpinBox();
    width->setRange(1, 10000);
    width->setSuffix(tr("px"));
    width->setValue(diagram_size.width());

    height = new QSpinBox();
    height->setRange(1, 10000);
    height->setSuffix(tr("px"));
    height->setValue(diagram_size.height());

    x_label = new QLabel("x");

    keep_ratio = new QPushButton();
    keep_ratio->setCheckable(true);
    keep_ratio->setChecked(true);
    keep_ratio->setIcon(QPixmap(Qucs::bitmapDirectory() + "object-locked.png"));
    keep_ratio->setToolTip(QObject::tr("Keep proportions"));

    reset_size = new QPushButton();
    reset_size->setIcon(QPixmap(Qucs::bitmapDirectory() + "clearFilterText.png"));
    reset_size->setToolTip(QObject::tr("Restore dimensions"));

    preview = new QPushButton();
    preview->setIcon(QPixmap(Qucs::bitmapDirectory() + "viewmag1.png"));
    preview->setToolTip(QObject::tr("Preview"));
}

//! Destructor
ExportDialog::ExportDiagramLine::~ExportDiagramLine()
{
}

//! @return layout the widgets needed to manage the  size of a schematic prior to export.
QBoxLayout *ExportDialog::ExportDiagramLine::sizeLayout()
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(width);
    layout->addWidget(x_label);
    layout->addWidget(height);
    layout->addWidget(keep_ratio);
    layout->addWidget(reset_size);
    layout->addWidget(preview);
    return(layout);
}
