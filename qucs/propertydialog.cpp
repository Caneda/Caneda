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

#include "propertydialog.h"
#include "schematicscene.h"
#include "undocommands.h"

#include <QComboBox>
#include <QHeaderView>

PropertyModel::PropertyModel(PropertyMap map, QObject *parent) :
    QAbstractTableModel(parent),
    propMap(map),
    keys(map.keys())
{
}

QVariant PropertyModel::data(IndexConstRef index, int role) const
{
    if(!index.isValid() || index.row() >= propMap.size()) {
        return QVariant();
    }

    QString key = keys.at(index.row());

    if(role == Qt::DisplayRole || role == Qt::EditRole) {
        switch(index.column()) {
            case 0: return key;
            case 1: return propMap[key].value();
            case 2: return propMap[key].description();
            default: ;
        }
    }
    else if(role == Qt::CheckStateRole && index.column() == 0) {
        return QVariant(propMap[key].isVisible());
    }
    else if(role == OptionsRole && index.column() == 1) {
        return propMap[keys[index.row()]].options();
    }
    return QVariant();
}

QVariant PropertyModel::headerData(int section, Qt::Orientation o, int role) const
{
    if(role != Qt::DisplayRole) {
        return QVariant();
    }

    if(o == Qt::Vertical) {
        return QAbstractTableModel::headerData(section, o, role);
    }
    else {
        switch(section) {
            case 0: return tr("Name");
            case 1: return tr("Value");
            case 2: return tr("Description");
        }
    }
    return QVariant();
}

Qt::ItemFlags PropertyModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags;
    if(!index.isValid()) {
        flags = Qt::ItemIsEnabled;
    }
    else {
        flags = QAbstractTableModel::flags(index);
        if(index.column() == 0) {
            flags |= Qt::ItemIsUserCheckable;
        }
        else if(index.column() == 1) {
            flags |= Qt::ItemIsEditable;
        }
    }
    return flags;
}

bool PropertyModel::setData(const QModelIndex &index, const QVariant &value,
        int role)
{
    if(index.isValid()){
        if(role == Qt::EditRole && index.column() == 1) {
            bool res = propMap[keys[index.row()]].setValue(value);
            if(res) {
                emit dataChanged(index, index);
            }
            return res;
        }
        else if(role == Qt::CheckStateRole && index.column() == 0) {
            Property &prop = propMap[keys[index.row()]];
            prop.setVisible(!prop.isVisible());
            emit dataChanged(index, index);
            return true;
        }
    }

    return false;
}

PropertyValueDelegate::PropertyValueDelegate(QObject *parent) : QItemDelegate(parent)
{
}

QWidget *PropertyValueDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option, const QModelIndex & index) const
{
    QStringList valOptions;
    valOptions = index.model()->data(index, PropertyModel::OptionsRole).toStringList();

    if(index.column() != 1 || valOptions.isEmpty()) {
        return QItemDelegate::createEditor(parent, option, index);
    }

    QComboBox *editor = new QComboBox(parent);
    editor->addItems(valOptions);
    return editor;
}

void PropertyValueDelegate::setEditorData(QWidget *editor,
        const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    if(!comboBox) {
        QItemDelegate::setEditorData(editor, index);
        return;
    }

    QString value = index.model()->data(index, Qt::DisplayRole).toString();
    QStringList options = index.model()->data(index, PropertyModel::OptionsRole).toStringList();
    int comboIndex = options.indexOf(value);

    comboBox->setCurrentIndex(comboIndex);
}

void PropertyValueDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    if(!comboBox) {
        QItemDelegate::setModelData(editor, model, index);
        return;
    }
    model->setData(index, comboBox->currentText());
}

//! Constructor
PropertyDialog::PropertyDialog(Component *comp, Qucs::UndoOption opt, QWidget *parent) :
    QDialog(parent),
    m_component(comp),
    m_undoOption(opt)
{
    setupUi(this);
    m_model = new PropertyModel(m_component->propertyMap(), this);
    tableView->setModel(m_model);

    tableView->setItemDelegate(new PropertyValueDelegate(this));
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->resizeColumnsToContents();
}

void PropertyDialog::accept()
{
    PropertyMapCmd *cmd = new PropertyMapCmd(m_component, m_component->propertyMap(),
            m_model->propMap);
    if(m_undoOption == Qucs::PushUndoCmd && m_component->schematicScene()) {
        m_component->schematicScene()->undoStack()->push(cmd);
    }
    else {
        cmd->redo();
        delete cmd;
    }
    QDialog::accept();
}
