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

#ifndef SHORTCUTS_DIALOG_H
#define SHORTCUTS_DIALOG_H

#include "ui_shortcutsdialog.h"

#include <QAbstractTableModel>
#include <QStyledItemDelegate>

// Forward declarations.
class QSortFilterProxyModel;
class QWidget;

namespace Caneda
{
    /*!
     * \brief This class provides display and editing facilities for data items
     * from a model.
     *
     * When displaying data from models in Qt item views, e.g., a QTableView,
     * the individual items are drawn by a delegate. Also, when an item is
     * edited, it provides an editor widget, which is placed on top of the item
     * view while editing takes place. QStyledItemDelegate is the default
     * delegate for all Qt item views, and is installed upon them when they are
     * created. To create a custom editor, for example a spinbox to edit
     * numbers in a QTableView, the QStyledItemDelegate must be subclassed.
     * This class is such a subclass.
     *
     * The QStyledItemDelegate class is one of the Model/View Classes and is
     * part of Qt's model/view framework. The delegate allows the display and
     * editing of items to be developed independently from the model and view.
     *
     * For every QStyledItemDelegate subclass, the following virtual functions
     * must be reimplemented:
     *
     * \li createEditor() returns the widget used to change data from the model
     * and can be reimplemented to customize editing behavior.
     * \li setEditorData() provides the widget with data to manipulate.
     * \li setModelData() returns updated data to the model.
     * \li updateEditorGeometry() ensures that the editor is displayed
     * correctly with respect to the item view.
     *
     * \sa QStyledItemDelegate, ShortcutsDialogModel
     */
    class ShortcutDelegate : public QStyledItemDelegate
    {
        Q_OBJECT

    public:
        explicit ShortcutDelegate(QObject *parent = 0);

        virtual QWidget *createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const;

        virtual void setEditorData(QWidget *editor,
                                   const QModelIndex &index) const;

        virtual void setModelData(QWidget *editor,
                                  QAbstractItemModel *model,
                                  const QModelIndex &index) const;

        virtual void updateEditorGeometry(QWidget *editor,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const;
    };

    /*!
     * \brief Model to provide the abstract interface for action items and
     * shortcuts in a table.
     *
     * This class derives from QAbstractTableModel and provides the abstract
     * interface for action items in a table model class. While the
     * ShortcutsDialog class implements the user interface, this class
     * interacts with the data itself.
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
     * \sa QAbstractTableModel, ShortcutsDialog
     */
    class ShortcutsDialogModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit ShortcutsDialogModel(QList<QAction*> actions, QObject *parent = 0);

        int rowCount(const QModelIndex& = QModelIndex() ) const { return m_actions.size(); }
        int columnCount(const QModelIndex& = QModelIndex() ) const { return 2; }

        QVariant data(const QModelIndex&, int role) const;
        QVariant headerData(int section, Qt::Orientation o, int role) const;

        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    private:
        friend class ShortcutsDialog;

        QList<QAction*> m_actions;
    };

    /*!
     * \brief Dialog to select action shortcuts.
     *
     * This dialog presents to the user the aplication available shortcuts
     * allowing to set any new shortcut or restore the default ones.
     *
     * This class handles the user interface part of the dialog, and
     * presentation part to the user, while ShortcutsDialogModel class
     * handles the data interaction itself.
     *
     * \sa ShortcutsDialogModel, QSortFilterProxyModel
     */
    class ShortcutsDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit ShortcutsDialog(QWidget *parent = 0);

    private Q_SLOTS:
        void filterTextChanged();
        void restoreDefaults();
        void applyShortcuts();

    private:
        ShortcutsDialogModel *m_model;
        QSortFilterProxyModel *m_proxyModel;

        QList<QAction*> m_actions;

        Ui::ShortcutsDialog ui;
    };

} // namespace Caneda

#endif //SHORTCUTS_DIALOG_H
