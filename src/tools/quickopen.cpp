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
#include "modelviewhelpers.h"
#include "settings.h"

#include <QAction>
#include <QActionGroup>
#include <QFileSystemModel>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QRegularExpression>
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

        buttonUp = new QToolButton(this);
        buttonUp->setIcon(Caneda::icon("go-up"));
        buttonUp->setStatusTip(tr("Go up one folder"));
        buttonUp->setToolTip(tr("Go up one folder"));
        buttonUp->setWhatsThis(tr("Go up one folder"));

        buttonBack = new QToolButton(this);
        buttonBack->setIcon(Caneda::icon("go-previous"));
        buttonBack->setStatusTip(tr("Go previous folder"));
        buttonBack->setToolTip(tr("Go previous folder"));
        buttonBack->setWhatsThis(tr("Go previous folder"));
        buttonBack->setEnabled(false);

        buttonForward = new QToolButton(this);
        buttonForward->setIcon(Caneda::icon("go-next"));
        buttonForward->setStatusTip(tr("Go next folder"));
        buttonForward->setToolTip(tr("Go next folder"));
        buttonForward->setWhatsThis(tr("Go next folder"));
        buttonForward->setEnabled(false);

        buttonHome = new QToolButton(this);
        buttonHome->setIcon(Caneda::icon("go-home"));
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

        filterNone = new QAction(Caneda::icon("view-sidetree"), tr("Show all"), this);
        filterGroup->addAction(filterNone);
        QAction *filtersSeparator = new QAction(this);
        filtersSeparator->setSeparator(true);
        filterSchematics = new QAction(Caneda::icon("application-x-caneda-schematic"), tr("Show schematics"), this);
        filterGroup->addAction(filterSchematics);
        filterSymbols = new QAction(Caneda::icon("application-x-caneda-symbol"), tr("Show symbols"), this);
        filterGroup->addAction(filterSymbols);
        filterSimulations = new QAction(Caneda::icon("application-x-spice-simulation-raw"), tr("Show simulations"), this);
        filterGroup->addAction(filterSimulations);
        filterText = new QAction(Caneda::icon("text-plain"), tr("Show text files"), this);
        filterGroup->addAction(filterText);

        filterNone->setCheckable(true);
        filterSchematics->setCheckable(true);
        filterSymbols->setCheckable(true);
        filterSimulations->setCheckable(true);
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
        m_model->setIconProvider(new IconProvider());
        m_model->setRootPath(QDir::homePath());

        // Create proxy model and set its properties.
        m_proxyModel = new FilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        m_proxyModel->setSourceModel(m_model);

        // Create a list view, set properties and proxy model
        m_listView = new QListView(this);
        m_listView->installEventFilter(this);
        m_listView->setModel(m_proxyModel);
        m_listView->setRootIndex(m_proxyModel->mapFromSource(m_model->index(QDir::homePath())));
        layout->addWidget(m_listView);

        // Signals and slots connections
        connect(buttonUp,      &QToolButton::clicked, this, &QuickOpen::slotUpFolder);
        connect(buttonBack,    &QToolButton::clicked, this, &QuickOpen::slotBackFolder);
        connect(buttonForward, &QToolButton::clicked, this, &QuickOpen::slotForwardFolder);
        connect(buttonHome,    &QToolButton::clicked, this, &QuickOpen::slotHomeFolder);

        connect(filterNone,        &QAction::triggered, this, &QuickOpen::filterFileTypes);
        connect(filterSchematics,  &QAction::triggered, this, &QuickOpen::filterFileTypes);
        connect(filterSymbols,     &QAction::triggered, this, &QuickOpen::filterFileTypes);
        connect(filterSimulations, &QAction::triggered, this, &QuickOpen::filterFileTypes);
        connect(filterText,        &QAction::triggered, this, &QuickOpen::filterFileTypes);

        connect(m_filterEdit, &QLineEdit::textChanged,   this, &QuickOpen::filterTextChanged);
        connect(m_filterEdit, &QLineEdit::returnPressed, this, &QuickOpen::open);
        connect(m_listView,   &QListView::activated,     this, &QuickOpen::open);

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
        if(event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

            if(keyEvent->modifiers() == Qt::AltModifier) {
                if(keyEvent->key() == Qt::Key_Up) {
                    buttonUp->animateClick();  // Call this instead of slot directly to get visual feedback
                    return true;
                }
                if(keyEvent->key() == Qt::Key_Left) {
                    buttonBack->animateClick();  // Call this instead of slot directly to get visual feedback
                    return true;
                }
                if(keyEvent->key() == Qt::Key_Right) {
                    buttonForward->animateClick();  // Call this instead of slot directly to get visual feedback
                    return true;
                }
                if(keyEvent->key() == Qt::Key_Home) {
                    buttonHome->animateClick();  // Call this instead of slot directly to get visual feedback
                    return true;
                }
            }

            if(keyEvent->modifiers() == Qt::ControlModifier) {
                if(keyEvent->key() == Qt::Key_F) {
                    m_filterEdit->setFocus();
                    return true;
                }
            }

            if(object == m_filterEdit) {
                if(keyEvent->key() == Qt::Key_Down) {
                    // Set the row next to the currently selected one
                    if(m_listView->currentIndex() == m_listView->model()->index(0,0, m_listView->rootIndex())) {
                        m_listView->setCurrentIndex(m_listView->model()->index(1,0, m_listView->rootIndex()));
                    }
                    else {
                        m_listView->setCurrentIndex(m_listView->model()->index(0,0, m_listView->rootIndex()));
                    }

                    // Set the focus in the treeview
                    m_listView->setFocus();

                    return true;
                }
            }

        }

        return QMenu::eventFilter(object, event);
    }

    //! \brief Filters items according to user input on a QLineEdit.
    void QuickOpen::filterTextChanged()
    {
        // Update the root index in the proxy model to avoid becoming rootless
        // while filtering.
        QModelIndex currentRootIndex = m_proxyModel->mapToSource(m_listView->rootIndex());
        m_proxyModel->setSourceRoot(currentRootIndex);

        // Filter the proxy model
        QString text = m_filterEdit->text();
        QRegularExpression regExp(text, QRegularExpression::CaseInsensitiveOption);
        m_proxyModel->setFilterRegularExpression(regExp);

        // Select the first item in the result
        m_listView->setCurrentIndex(m_listView->model()->index(0,0, m_listView->rootIndex()));
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
            foreach(const QString &suffix, context->supportedSuffixes()) {
                filters << "*." + suffix;
            }
        }
        else if(action == filterSymbols) {
            m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
            IContext *context = SymbolContext::instance();
            foreach(const QString &suffix, context->supportedSuffixes()) {
                filters << "*." + suffix;
            }
        }
        else if(action == filterSimulations) {
            m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
            IContext *context = SimulationContext::instance();
            foreach(const QString &suffix, context->supportedSuffixes()) {
                filters << "*." + suffix;
            }
        }
        else if(action == filterText) {
            m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
            IContext *context = TextContext::instance();
            foreach(const QString &suffix, context->supportedSuffixes()) {
                filters << "*." + suffix;
            }
        }

        m_model->setNameFilters(filters);
        m_model->setNameFilterDisables(false);
    }

    //! \brief Accept the dialog and open the selected item.
    void QuickOpen::open()
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
