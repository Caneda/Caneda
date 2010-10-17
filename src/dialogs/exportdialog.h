/***************************************************************************
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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

#include "ui_exportdialog.h"

#include <QDialog>

class QFile;

namespace Caneda
{
    // Forward declations
    class SchematicDocument;
    class CGraphicsScene;

    /*!
     * \brief This represents the dialog for exporting a schematic
     * to an image.
     */
    class ExportDialog : public QDialog
    {
        Q_OBJECT;

    public:
        ExportDialog(SchematicDocument *, QWidget *parent = 0);
        ~ExportDialog();

    public Q_SLOTS:
        void slotChooseDirectory();
        void slotCorrectWidth();
        void slotCorrectHeight();
        void slotLockRatioChanged();
        void slotResetSize();
        void slotPreview();
        void slotChangeFilesExtension();
        void slotExport();

    private:
        qreal diagramRatio();
        QSizeF diagramSize();

        QImage generateImage();
        void generateSvg(QFile &);
        void saveReloadDiagramParameters(bool = true);

        CGraphicsScene *m_scene;

        Ui::ExportDialog ui;
    };

} // namespace Caneda

#endif //EXPORTDIALOG_H
