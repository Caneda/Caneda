/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef QUCSVIEW_H
#define QUCSVIEW_H

#include <QDateTime>
#include <QIcon>
#include <QString>

class QPainter;
class QWidget;
class SchematicView;

class QucsView
{
public:
    QucsView();
    virtual ~QucsView() {}

    // Emit signal while reimplementing
    virtual void setFileName(const QString& _fileName) = 0;
    virtual QString fileName() const = 0; // With path

    virtual bool load() = 0;
    virtual bool save() = 0;

    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;

    virtual void showAll() = 0;
    virtual void showNoZoom() = 0;

    virtual bool isSchematicView() const = 0;

    virtual QWidget* toWidget() const = 0;
    virtual SchematicView* toSchematicView() const;
    //TODO: Add other view convienience methods

    virtual bool isModified() const = 0;
    virtual void setModified(bool b) = 0;

    virtual void copy() const = 0;
    virtual void cut() = 0;
    virtual void paste() = 0;

    // Returns text to be displayed on tab
    QString tabText() const;

    // A helper function for setFileName and load
    bool load(const QString& name);

    static QIcon modifiedTabIcon();
    static QIcon unmodifiedTabIcon();

signals:
    virtual void modificationChanged(bool b) = 0;
    virtual void fileNameChanged(const QString& filename) = 0;
    // This signal is emitted with the above for convienience.
    virtual void titleToBeUpdated() = 0;

protected:
    // To check for external modification
    QDateTime lastSaved;
};

#endif //QUCSVIEW_H
