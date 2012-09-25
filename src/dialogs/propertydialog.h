/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
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

#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include "ui_propertydialog.h"

#include "propertygroup.h"

#include <QAbstractTableModel>

// Forward declarations.
class QSortFilterProxyModel;

namespace Caneda
{
    /*!
     * \brief Model to provide the abstract interface for property items
     * in a table.
     *
     * This class derives from QAbstractTableModel and provides the abstract
     * interface for property items in a table model class. While PropertyDialog
     * implements the user interface, this class interacts with the data
     * itself.
     *
     * This class defines a standard interface that must used to be able to
     * interoperate with other components in Qt's model/view framework. The
     * underlying data model is exposed as a simple table of rows and columns.
     * Each item has a unique index specified by a QModelIndex.
     *
     * Each item has a number of data elements associated with it and they can
     * be retrieved by specifying a role (see Qt::ItemDataRole) to the model's
     * data() function. Data for each role is set using a particular
     * Qt::ItemDataRole. Data for individual roles are set individually with
     * setData(). Items can be queried with flags() (see Qt::ItemFlag) to see
     * if they can be selected, dragged, or manipulated in other ways.
     *
     * The model has a rowCount() and a columnCount(). Rows can be inserted and
     * removed with insertRows(), and removeRows().
     *
     * \sa QAbstractTableModel, PropertyDialog
     */
    class PropertyModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        PropertyModel(PropertyMap map, QObject *parent = 0);

        int rowCount(const QModelIndex& = QModelIndex() ) const { return propMap.size(); }
        int columnCount(const QModelIndex& = QModelIndex() ) const { return 4; }

        QVariant data(const QModelIndex&, int role) const;
        QVariant headerData(int section, Qt::Orientation o, int role) const;

        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

        bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
        bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());

    private:
        friend class PropertyDialog;

        PropertyMap propMap;
        QList<QString> keys;
    };

    /*!
     * \brief Dialog to modify properties in a PropertyGroup
     *
     * This dialog presents to the user the properties of the selected
     * component/scene. By default, properties are presented with a
     * QLineEdit. For better representation it is recommended to have
     * string properties rather than numeric. In this way, the user can
     * use prefixes, like 'p' for pico, 'u' for micro, etc. Even
     * parametric properties could be used as string properties, using
     * for example brackets as in '{R}'.
     *
     * This class handles the user interface part of the dialog, and
     * presentation part to the user, while PropertyModel class handles
     * the data interaction itself.
     *
     * \sa PropertyGroup, PropertyModel, QSortFilterProxyModel
     */
    class PropertyDialog : public QDialog
    {
        Q_OBJECT

    public:
        PropertyDialog(PropertyGroup *propGroup, QWidget *parent = 0);

    public Q_SLOTS:
        void accept();

    private Q_SLOTS:
        void filterTextChanged();
        void addProperty();
        void removeProperty();

    private:
        PropertyModel *m_model;
        QSortFilterProxyModel *m_proxyModel;
        PropertyGroup *m_propertyGroup;

        Ui::PropertyDialog ui;
    };

} // namespace Caneda

#endif //PROPERTYDIALOG_H
