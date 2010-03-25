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

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include "qucsview.h"
#include "schematicscene.h"

#include <QDialog>

class QBoxLayout;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QFile;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSignalMapper;
class QSpinBox;
class QSvgGenerator;

/*!
 * This class represents the dialog for exporting a schema
 * as an image according to the selection of the user.
 * \todo In the long run base it on .ui file.
 */
class ExportDialog : public QDialog
{
    Q_OBJECT;

public:
    ExportDialog(QList<SchematicScene *> schemasToExport, QWidget *parent = 0);
    virtual ~ExportDialog();

private:
    struct ExportDiagramLine
    {
        ExportDiagramLine(SchematicScene *);
        virtual ~ExportDiagramLine();

        SchematicScene *schema;
        QBoxLayout *sizeLayout();

        QCheckBox *must_export;
        QLabel *title_label;
        QLineEdit *file_name;
        QSpinBox *width;
        QLabel *x_label;
        QSpinBox *height;
        QPushButton *keep_ratio;
        QPushButton *reset_size;
        QPushButton *preview;
    };

private:
    QList<SchematicScene *> schemas;
    QHash<int, ExportDialog::ExportDiagramLine *> diagramsList;

    QWidget *diagramsListPart();
    QWidget *lastPart();
    QGroupBox *setupOptionsGroupBox();

    qreal diagramRatio(SchematicScene *);
    QSizeF diagramSize(SchematicScene *);
    int diagramsToExportCount() const;
    void exportDiagram(ExportDiagramLine *);
    QImage generateImage(SchematicScene *, int, int, bool);
    void generateSvg(SchematicScene *, int, int, bool, QFile &);
    void saveReloadDiagramParameters(SchematicScene *, bool = true);

    QLineEdit *dirpath;
    QComboBox *format;
    QCheckBox *draw_grid;
    QCheckBox *draw_frame;
    QDialogButtonBox *buttons;

    QSignalMapper *preview_mapper_;
    QSignalMapper *width_mapper_;
    QSignalMapper *height_mapper_;
    QSignalMapper *ratio_mapper_;
    QSignalMapper *reset_mapper_;

public Q_SLOTS:
    void slot_correctWidth(int);
    void slot_correctHeight(int);
    void slot_keepRatioChanged(int);
    void slot_resetSize(int);
    void slot_chooseDirectory();
    void slot_export();
    void slot_changeUseFrame();
    void slot_checkDiagramsCount();
    void slot_changeFilesExtension(bool = false);
    void slot_previewDiagram(int);
};
#endif
