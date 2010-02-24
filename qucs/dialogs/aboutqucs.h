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

#ifndef ABOUTQUCS_H
#define ABOUTQUCS_H

#include <QDialog>

class QLabel;

//! This class represents the dialog "About Qucs".
class AboutQUCS : public QDialog {
    Q_OBJECT

public:
        // constructor, destructor
        AboutQUCS(QWidget * = 0);
        virtual ~AboutQUCS();

        // methods
private:
        QWidget *title() const;
        QWidget *aboutTab() const;
        QWidget *authorsTab() const;
        QWidget *translatorsTab() const;
        QWidget *contributorsTab() const;
        QWidget *licenseTab() const;
        void addAuthor(QLabel *, const QString &, const QString &, const QString &) const;
};

#endif //ABOUTQUCS_H
