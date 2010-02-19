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

#ifndef PRINT_DIALOG_H
#define PRINT_DIALOG_H

#include "schematicscene.h"

class QCheckBox;
class QLineEdit;
class QRadioButton;

/*!
 * This class represents the configuration dialog to print a
 * schematic.
 * It also takes care of the print itself
 */
class PrintDialog : public QWidget
{
    Q_OBJECT;

public:
    PrintDialog(SchematicScene *, QWidget * = 0);
    ~PrintDialog();

    QString fileName() const;
    void setFileName(const QString &);

    QString docName() const;
    void setDocName(const QString &);

    int pagesCount(bool = false) const;

    int horizontalPagesCount(bool = false) const;
    int verticalPagesCount(bool = false) const;

private:
    void buildPrintTypeDialog();

private Q_SLOTS:
    void updatePrintTypeDialog();
    void browseFilePrintTypeDialog();
    void acceptPrintTypeDialog();
    void print(bool);

private:
    SchematicScene *schema;
    QPrinter *printer;
    QString docname;
    QString filename;

    QDialog *dialog;
    QRadioButton *printerChoice;
    QRadioButton *pdfChoice;
    QRadioButton *psChoice;
    QLineEdit *editFilepath;
    QCheckBox *fitInPage;
    QPushButton *browseButton;
};

#endif //PRINT_DIALOG_H
