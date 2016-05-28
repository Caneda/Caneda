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

#include "quickopen.h"

#include "global.h"
#include "icontext.h"
#include "settings.h"
#include "sidebaritemsbrowser.h"

#include <QFileSystemModel>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

namespace Caneda
{
    /*!
     * \brief Constructor.
     *
     * \param parent Parent of the widget.
     */
    QuickOpen::QuickOpen(QWidget *parent) : QMenu(parent)
    {
        // Set window geometry
        setMinimumSize(300,300);

        QVBoxLayout *layout = new QVBoxLayout(this);

        // Create the toolbar
        QToolBar *toolbar = new QToolBar(this);

        QToolButton *buttonUp = new QToolButton(this);
        buttonUp->setIcon(Caneda::icon("go-up"));
        buttonUp->setShortcut(Qt::Key_Backspace);
        buttonUp->setStatusTip(tr("Go up one folder"));
        buttonUp->setToolTip(tr("Go up one folder"));
        buttonUp->setWhatsThis(tr("Go up one folder"));

        buttonBack = new QToolButton(this);
        buttonBack->setIcon(Caneda::icon("go-previous"));
        buttonBack->setShortcut(Qt::ALT + Qt::Key_Left);
        buttonBack->setStatusTip(tr("Go previous folder"));
        buttonBack->setToolTip(tr("Go previous folder"));
        buttonBack->setWhatsThis(tr("Go previous folder"));
        buttonBack->setEnabled(false);

        buttonForward = new QToolButton(this);
        buttonForward->setIcon(Caneda::icon("go-next"));
        buttonForward->setShortcut(Qt::ALT + Qt::Key_Right);
        buttonForward->setStatusTip(tr("Go next folder"));
        buttonForward->setToolTip(tr("Go next folder"));
        buttonForward->setWhatsThis(tr("Go next folder"));
        buttonForward->setEnabled(false);

        QToolButton *buttonHome = new QToolButton(this);
        buttonHome->setIcon(Caneda::icon("go-home"));
        buttonHome->setShortcut(Qt::CTRL + Qt::Key_Home);
        buttonHome->setStatusTip(tr("Go to the home folder"));
        buttonHome->setToolTip(tr("Go to the home folder"));
        buttonHome->setWhatsThis(tr("Go to the home folder"));

        // Create the filter button
        QToolButton *buttonFilters = new QToolButton(this);
        QMenu *filterMenu = new QMenu(this);
        filterGroup = new QActionGroup(this);

        buttonFilters->setIcon(Caneda::icon("configure"));
        buttonFilters->setPopupMode(QToolButton::InstantPopup);
        buttonFilters->setMenu(filterMenu);

        filterNone = new QAction(Caneda::icon("view-sidetree"), tr("Show all"), filterGroup);
        filterSchematics = new QAction(Caneda::icon("document-new"), tr("Show schematics"), filterGroup);
        filterSymbols = new QAction(Caneda::icon("draw-freehand"), tr("Show symbols"), filterGroup);
        filterLayouts = new QAction(Caneda::icon("view-grid"), tr("Show layouts"), filterGroup);
        filterText = new QAction(Caneda::icon("text-plain"), tr("Show text files"), filterGroup);

        filterNone->setCheckable(true);
        filterSchematics->setCheckable(true);
        filterSymbols->setCheckable(true);
        filterLayouts->setCheckable(true);
        filterText->setCheckable(true);

        Settings *settings = Settings::instance();
        int index = settings->currentValue("quickopen/filter").toInt();
        filterGroup->actions().at(index)->setChecked(true);

        filterMenu->addActions(filterGroup->actions());

        // Add the buttons to the toolbar
        toolbar->addWidget(buttonUp);
        toolbar->addWidget(buttonBack);
        toolbar->addWidget(buttonForward);
        toolbar->addWidget(buttonHome);

        QWidget *spacer = new QWidget(this);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        toolbar->addWidget(spacer);

        toolbar->addWidget(buttonFilters);
        layout->addWidget(toolbar);

        // Set lineEdit properties
        m_filterEdit = new QLineEdit(this);
        m_filterEdit->setClearButtonEnabled(true);
        m_filterEdit->setPlaceholderText(tr("Search..."));
        m_filterEdit->installEventFilter(this);
        layout->addWidget(m_filterEdit);

        // Create a new filesystem model
        m_model = new QFileSystemModel(this);
        m_model->setRootPath(QDir::homePath());

        // Create proxy model and set its properties.
        m_proxyModel = new FilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        m_proxyModel->setSourceModel(m_model);

        // Create a list view, set properties and proxy model
        m_listView = new QListView(this);
        m_listView->setModel(m_proxyModel);
        m_listView->setRootIndex(m_proxyModel->mapFromSource(m_model->index(QDir::homePath())));
        layout->addWidget(m_listView);

        // Signals and slots connections
        connect(buttonUp, SIGNAL(clicked()), this, SLOT(slotUpFolder()));
        connect(buttonBack, SIGNAL(clicked()), this, SLOT(slotBackFolder()));
        connect(buttonForward, SIGNAL(clicked()), this, SLOT(slotForwardFolder()));
        connect(buttonHome, SIGNAL(clicked()), this, SLOT(slotHomeFolder()));

        connect(filterNone, SIGNAL(triggered(bool)), this, SLOT(filterFileTypes()));
        connect(filterSchematics, SIGNAL(triggered(bool)), this, SLOT(filterFileTypes()));
        connect(filterSymbols, SIGNAL(triggered(bool)), this, SLOT(filterFileTypes()));
        connect(filterLayouts, SIGNAL(triggered(bool)), this, SLOT(filterFileTypes()));
        connect(filterText, SIGNAL(triggered(bool)), this, SLOT(filterFileTypes()));

        connect(m_filterEdit, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged()));
        connect(m_filterEdit, SIGNAL(returnPressed()), this, SLOT(itemSelected()));
        connect(m_listView, SIGNAL(activated(QModelIndex)), this, SLOT(itemSelected()));

        // Filter the selected filetype and start with the focus on the filter
        filterGroup->actions().at(index)->trigger();
        m_filterEdit->setFocus();
    }

    //! \brief Set the current folder to \a path.
    void QuickOpen::setCurrentFolder(const QString& path)
    {
        m_listView->setRootIndex(m_proxyModel->mapFromSource(m_model->index(path)));

        previousPages.clear();
        nextPages.clear();

        buttonBack->setEnabled(false);
        buttonForward->setEnabled(false);
    }

    //! \brief Go up one folder in the filesystem.
    void QuickOpen::slotUpFolder()
    {
        previousPages << m_proxyModel->mapToSource(m_listView->rootIndex());
        m_listView->setRootIndex(m_listView->rootIndex().parent());
        nextPages.clear();

        buttonBack->setEnabled(true);
        buttonForward->setEnabled(false);
    }

    //! \brief Go the the previous folder.
    void QuickOpen::slotBackFolder()
    {
        if(!previousPages.isEmpty()) {
            nextPages << m_proxyModel->mapToSource(m_listView->rootIndex());
            m_listView->setRootIndex(m_proxyModel->mapFromSource(previousPages.last()));
            previousPages.removeLast();

            buttonForward->setEnabled(true);
            if(previousPages.isEmpty()) {
                buttonBack->setEnabled(false);
            }
        }
    }

    //! \brief Go the the next folder.
    void QuickOpen::slotForwardFolder()
    {
        if(!nextPages.isEmpty()) {
            previousPages << m_proxyModel->mapToSource(m_listView->rootIndex());
            m_listView->setRootIndex(m_proxyModel->mapFromSource(nextPages.last()));
            nextPages.removeLast();

            buttonBack->setEnabled(true);
            if(nextPages.isEmpty()) {
                buttonForward->setEnabled(false);
            }
        }
    }

    //! \brief Go the the home folder.
    void QuickOpen::slotHomeFolder()
    {
        previousPages << m_proxyModel->mapToSource(m_listView->rootIndex());
        m_listView->setRootIndex(m_proxyModel->mapFromSource(m_model->index(QDir::homePath())));
        nextPages.clear();

        buttonBack->setEnabled(true);
        buttonForward->setEnabled(false);
    }

    //! \brief Filter event to select the listView on down arrow key event
    bool QuickOpen::eventFilter(QObject *object, QEvent *event)
    {
        if(object == m_filterEdit) {
            if(event->type() == QEvent::KeyPress) {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
                if(keyEvent->key() == Qt::Key_Down) {

                    // Set the row next to the currently selected one
                    if(m_listView->currentIndex() == m_listView->rootIndex().child(0,0)) {
                        m_listView->setCurrentIndex(m_listView->rootIndex().child(1,0));
                    }
                    else {
                        m_listView->setCurrentIndex(m_listView->rootIndex().child(0,0));
                    }

                    // Set the focus in the treeview
                    m_listView->setFocus();

                    return true;
                }
            }

            return false;
        }

        return QMenu::eventFilter(object, event);
    }

    //! \brief Filters actions according to user input on a QLineEdit.
    void QuickOpen::filterTextChanged()
    {
        QString text = m_filterEdit->text();
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
        m_listView->setCurrentIndex(m_listView->rootIndex().child(0,0));
    }

    //! \brief Filters the view to show only selected file type.
    void QuickOpen::filterFileTypes()
    {
        QAction *action = qobject_cast<QAction*>(sender());
        if(!action) {
            return;
        }

        Settings *settings = Settings::instance();
        int index = filterGroup->actions().indexOf(action);
        settings->setCurrentValue("quickopen/filter", QVariant(index));

        QStringList filters;
        if(action == filterNone) {
            // Default filter (show all but hidden files)
            m_model->setFilter(QDir::Dirs|QDir::AllDirs|QDir::Files|
                               QDir::Drives|QDir::NoDot|QDir::NoDotDot|
                               QDir::AllEntries);
        }
        else if(action == filterSchematics) {
            m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
            IContext *context = SchematicContext::instance();
            filters << "*." + context->defaultSuffix();
        }
        else if(action == filterSymbols) {
            m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
            IContext *context = SymbolContext::instance();
            filters << "*." + context->defaultSuffix();
        }
        else if(action == filterLayouts) {
            m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
            IContext *context = LayoutContext::instance();
            filters << "*." + context->defaultSuffix();
        }
        else if(action == filterText) {
            m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
            IContext *context = TextContext::instance();
            filters << "*." + context->defaultSuffix();
        }

        m_model->setNameFilters(filters);
        m_model->setNameFilterDisables(false);
    }

    //! \brief Accept the dialog and open the selected item.
    void QuickOpen::itemSelected()
    {
        if(m_listView->currentIndex().isValid()) {

            if(m_model->isDir(m_proxyModel->mapToSource(m_listView->currentIndex()))) {
                previousPages << m_proxyModel->mapToSource(m_listView->rootIndex());
                m_listView->setRootIndex(m_listView->currentIndex());
                nextPages.clear();

                buttonBack->setEnabled(true);
                buttonForward->setEnabled(false);

                m_filterEdit->clear();
                m_filterEdit->setFocus();
            }
            // it is a file so we let the main window handle the action
            else {
                emit itemSelected(m_model->fileInfo(m_proxyModel->mapToSource(m_listView->currentIndex())).absoluteFilePath());
                hide();
            }

        }
    }

} // namespace Caneda
