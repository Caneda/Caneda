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

#include "property.h"
#include "propertygroup.h"

#include <QAbstractTableModel>
#include <QItemDelegate>

// Forward declarations.
class QStandardItemModel;
class QSortFilterProxyModel;

namespace Caneda
{
    typedef QModelIndex& IndexRef;
    typedef const QModelIndex& IndexConstRef;

    class PropertyModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        enum {
            OptionsRole = Qt::UserRole + 1
        };

        PropertyModel(PropertyMap map, QObject *parent = 0);

        int rowCount(IndexConstRef) const { return propMap.size(); }
        int columnCount(IndexConstRef) const { return 4; }

        QVariant data(IndexConstRef, int role) const;
        QVariant headerData(int section, Qt::Orientation o, int role) const;

        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value,
                int role = Qt::EditRole);

    private:
        friend class PropertyDialog;

        PropertyMap propMap;
        QList<QString> keys;
    };

    class PropertyValueDelegate : public QItemDelegate
    {
        Q_OBJECT

    public:
        PropertyValueDelegate(QObject *parent = 0);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model,
                const QModelIndex &index) const;
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

    private:
        PropertyModel *m_model;
        QSortFilterProxyModel *m_proxyModel;

        PropertyGroup *m_propertyGroup;

        Ui::PropertyDialog ui;
    };

} // namespace Caneda

#endif //PROPERTYDIALOG_H
