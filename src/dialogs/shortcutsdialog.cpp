/***************************************************************************
 * Copyright (C) 2016 by Pablo Daniel Pareja Obregon                       *
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

#include "shortcutsdialog.h"

#include "actionmanager.h"
#include "settings.h"

#include <QKeySequenceEdit>
#include <QSortFilterProxyModel>

namespace Caneda
{
    //*************************************************************
    //********************* ShortcutDelegate **********************
    //*************************************************************
    ShortcutDelegate::ShortcutDelegate(QObject *parent) : QStyledItemDelegate(parent)
    {
    }

    QWidget *ShortcutDelegate::createEditor(QWidget *parent,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
    {
        QKeySequenceEdit *editor = new QKeySequenceEdit(parent);
        return (QWidget*)editor;
    }

    void ShortcutDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QKeySequenceEdit *keyEditor = static_cast<QKeySequenceEdit*>(editor);
        keyEditor->setKeySequence(QKeySequence(index.data(Qt::EditRole).toString()));
    }

    void ShortcutDelegate::setModelData(QWidget *editor,
                                        QAbstractItemModel *model,
                                        const QModelIndex &index) const
    {
        QKeySequenceEdit *keyEditor = static_cast<QKeySequenceEdit*>(editor);
        model->setData(index, keyEditor->keySequence());
    }

    void ShortcutDelegate::updateEditorGeometry(QWidget *editor,
                                                const QStyleOptionViewItem &option,
                                                const QModelIndex &index) const
    {
        editor->setGeometry(option.rect);
    }

    //*************************************************************
    //******************* ShortcutsDialogModel ********************
    //*************************************************************
    /*!
     * \brief Constructor.
     *
     * \param actions List of available application actions which shortcuts
     * will be set.
     * \param parent Parent of this object.
     */
    ShortcutsDialogModel::ShortcutsDialogModel(QList<QAction *> actions, QObject *parent) :
        QAbstractTableModel(parent),
        m_actions(actions)
    {
    }

    /*!
     * \brief Returns the data stored for the item referred by index.
     *
     * This class returns the item data corresponding to index position.
     * For example, if we are editing an item in the first column, the
     * data corresponds to the action name, hence the return value is
     * the object name of the action in the form of a QString.
     *
     * \param index Item to return data from
     * \param role Role of the item (editable, checkable, etc).
     * \return data stored for given item
     */
    QVariant ShortcutsDialogModel::data(const QModelIndex& index, int role) const
    {
        if(!index.isValid() || index.row() >= m_actions.size()) {
            return QVariant();
        }

        if(role == Qt::DisplayRole && index.column() == 0) {
            return m_actions.at(index.row())->text().remove(QChar('&'));
        }
        else if(role == Qt::DisplayRole && index.column() == 1) {
            return m_actions.at(index.row())->shortcut().toString();
        }

        if(role == Qt::DecorationRole && index.column() == 0) {
            return QVariant(m_actions.at(index.row())->icon());
        }

        if(role == Qt::SizeHintRole) {
            return QSize(150, 32);
        }

        return QVariant();
    }

    /*!
     * \brief Returns header data (text) for the given column
     *
     * This method defines column header text to be displayed on the
     * associated table view.
     */
    QVariant ShortcutsDialogModel::headerData(int section, Qt::Orientation o, int role) const
    {
        if(role != Qt::DisplayRole) {
            return QVariant();
        }

        if(o == Qt::Vertical) {
            return QAbstractTableModel::headerData(section, o, role);
        }
        else {
            switch(section) {
                case 0: return tr("Action");
                case 1: return tr("Shortcut");
            }
        }
        return QVariant();
    }

    /*!
     * \brief Returns item flags according to its position. This flags
     * are responsible for the item editable or checkable state.
     *
     * \param index Item for which its flags must be returned.
     * \return Qt::ItemFlags Item's flags.
     */
    Qt::ItemFlags ShortcutsDialogModel::flags(const QModelIndex& index) const
    {
        if(!index.isValid()) {
            return Qt::ItemIsEnabled;
        }

        Qt::ItemFlags flags = QAbstractTableModel::flags(index);

        // Column 1 is editable (shortcut button)
        if(index.column() == 1) {
            flags |= Qt::ItemIsEditable;
        }
        // Column 0 is not editable
        else {
            flags |= Qt::NoItemFlags;
        }

        return flags;
    }

    /*!
     * \brief Sets data in a ShortcutsDialogModel item.
     *
     * Sets the data in a ShortcutsDialogModel item, ie. modifies the user
     * edited data.
     *
     * \param index Item to be edited.
     * \param value New value to be set.
     * \param role Role of the item. Helps identify what are we editing (editable
     * item, checkable item, etc).
     * \return True on success, false otherwise.
     */
    bool ShortcutsDialogModel::setData(const QModelIndex& index, const QVariant& value,
            int role)
    {
        if(index.isValid()){
            if(index.column() == 1) {
                m_actions.at(index.row())->setShortcut(value.value<QKeySequence>());
            }

            emit dataChanged(index, index);
            return true;
        }

        return false;
    }

    //*************************************************************
    //******************** ShortcutsDialog ************************
    //*************************************************************
    /*!
     * \brief Constructor.
     *
     * \param parent Parent of this object.
     */
    ShortcutsDialog::ShortcutsDialog(QWidget *parent) : QDialog(parent)
    {
        ui.setupUi(this);

        // Get the list of actions
        ActionManager *am = ActionManager::instance();
        m_actions = am->actions();

        // Create a new table model
        m_model = new ShortcutsDialogModel(m_actions, this);

        // Create proxy model and set its properties.
        m_proxyModel = new QSortFilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        m_proxyModel->setSourceModel(m_model);
        m_proxyModel->sort(0);

        // Set QTableView proxy model and item delegate
        ui.tableView->setModel(m_proxyModel);
        ui.tableView->setItemDelegateForColumn(1, new ShortcutDelegate(this));

        // Signals and slots connections
        connect(ui.lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged()));
        connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(applyShortcuts()));
        connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    }

    //! \brief Filters actions according to user input on a QLineEdit.
    void ShortcutsDialog::filterTextChanged()
    {
        QString text = ui.lineEdit->text();
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
    }

    //! \brief Load default action shortcuts
    void ShortcutsDialog::restoreDefaults()
    {
        m_model->beginResetModel();

        Settings *settings = Settings::instance();

        foreach(QAction *action, m_actions) {
            QString name = "shortcuts/" + action->objectName();
            action->setShortcut(settings->defaultValue(name).value<QKeySequence>());
        }

        m_model->endResetModel();
    }

    //! \brief Save all shortcuts changes
    void ShortcutsDialog::applyShortcuts()
    {
        accept();
    }

} // namespace Caneda
