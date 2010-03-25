/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "component.h"
#include "property.h"
#include "dialogs/ui_property.h"

#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QTableView>

class Component;
class QStandardItemModel;

typedef QModelIndex& IndexRef;
typedef const QModelIndex& IndexConstRef;

class PropertyModel : public QAbstractTableModel
{
    Q_OBJECT;
public:
    enum {
        OptionsRole = Qt::UserRole + 1
    };

    PropertyModel(PropertyMap map, QObject *parent = 0);

    int rowCount(IndexConstRef) const { return propMap.size(); }
    int columnCount(IndexConstRef) const { return 3; }

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
    Q_OBJECT;
public:
    PropertyValueDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
            const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
            const QModelIndex &index) const;
};


class PropertyDialog : public QDialog, public Ui::PropertyDialogBase
{
    Q_OBJECT;
public:
    PropertyDialog(Component *comp, Qucs::UndoOption opt, QWidget *parent = 0);

public Q_SLOTS:
    void accept();

private:
    PropertyModel *m_model;
    Component *m_component;
    Qucs::UndoOption m_undoOption;
};


#endif //PROPERTYDIALOG_H
