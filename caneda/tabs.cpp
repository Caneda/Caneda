/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "tabs.h"

#include "actionmanager.h"
#include "documentviewmanager.h"
#include "idocument.h"
#include "iview.h"
#include "mainwindow.h"

#include <QDebug>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabBar>
#include <QWheelEvent>


namespace Caneda
{
    // icon for unsaved files (diskette)
    static const char *smallsave_xpm[] = {
        "16 17 66 1", " 	c None",
        ".	c #595963","+	c #E6E6F1","@	c #465460","#	c #FEFEFF",
        "$	c #DEDEEE","%	c #43535F","&	c #D1D1E6","*	c #5E5E66",
        "=	c #FFFFFF","-	c #C5C5DF",";	c #FCF8F9",">	c #BDBDDA",
        ",	c #BFBFDC","'	c #C4C4DF",")	c #FBF7F7","!	c #D6D6E9",
        "~	c #CBCBE3","{	c #B5B5D6","]	c #BCBCDA","^	c #C6C6E0",
        "/	c #CFCFE5","(	c #CEC9DC","_	c #D8D8EA",":	c #DADAEB",
        "<	c #313134","[	c #807FB3","}	c #AEAED1","|	c #B7B7D7",
        "1	c #E2E2EF","2	c #9393C0","3	c #E3E3F0","4	c #DDD5E1",
        "5	c #E8E8F3","6	c #2F2F31","7	c #7B7BAF","8	c #8383B5",
        "9	c #151518","0	c #000000","a	c #C0C0DC","b	c #8E8FBD",
        "c	c #8989BA","d	c #E7EEF6","e	c #282829","f	c #6867A1",
        "g	c #7373A9","h	c #A7A7CD","i	c #8080B3","j	c #7B7CB0",
        "k	c #7070A8","l	c #6D6DA5","m	c #6E6EA6","n	c #6969A2",
        "o	c #7A79AF","p	c #DCDCEC","q	c #60609A","r	c #7777AC",
        "s	c #5D5D98","t	c #7676AB","u	c #484785","v	c #575793",
        "w	c #50506A","x	c #8787B8","y	c #53536E","z	c #07070E",
        "A	c #666688",
        "        .       ",
        "       .+.      ",
        "      .+@#.     ",
        "     .$%###.    ",
        "    .&*####=.   ",
        "   .-.#;#####.  ",
        "  .>,'.#)!!!!~. ",
        " .{].'^./(!_:<[.",
        ".}|.1./2.3456789",
        "0a.$11.bc.defg9 ",
        " 011h11.ij9kl9  ",
        "  0_1h1h.mno9   ",
        "   0p12h9qr9    ",
        "    0hh9st9     ",
        "     09uv9w     ",
        "      0x9y      ",
        "       zA       "
    };

    static const char *empty_xpm[] = {  // provides same height than "smallsave_xpm"
        "1 17 1 1", "  c None", " ", " ", " ", " ", " ",
        " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " "
    };

    Tab::Tab(IView *view, QWidget *parent) : QWidget(parent)
    {
        addView(view);

        QHBoxLayout *layout = new QHBoxLayout(this);

        QSplitter *splitter = new QSplitter();
        splitter->setContentsMargins(0, 0, 0, 0);
        splitter->addWidget(view->toWidget());

        layout->addWidget(splitter);
        setContentsMargins(0, 0, 0, 0);
    }

    Tab::~Tab()
    {
    }

    IView* Tab::activeView() const
    {
        return m_views.isEmpty() ? 0 : m_views.first();
    }

    QString Tab::tabText() const
    {
        IView *view = activeView();
        QString title;
        if (view) {
            title = QFileInfo(view->document()->fileName()).fileName();
        }

        if (title.isEmpty()) {
            title = tr("Untitled");
        }

        return title;
    }

    QIcon Tab::tabIcon() const
    {
        IView *view = activeView();
        if (view) {
            return view->document()->isModified() ? modifiedIcon() : unmodifiedIcon();
        }

        return QIcon();
    }

    void Tab::splitView(IView *view, IView *newView,
            Qt::Orientation splitOrientation)
    {
        QWidget *asWidget = view->toWidget();
        QSplitter *parentSplitter = qobject_cast<QSplitter*>(asWidget->parentWidget());

        Q_ASSERT_X(parentSplitter != 0, Q_FUNC_INFO,
                "Fix me: Parent widget of view not splitter");

        if (parentSplitter->orientation() != splitOrientation &&
                parentSplitter->count() == 1) {
            parentSplitter->setOrientation(splitOrientation);
        }

        if (parentSplitter->orientation() == splitOrientation) {
            parentSplitter->addWidget(newView->toWidget());
        } else {
            int index = parentSplitter->indexOf(asWidget);
            asWidget->setParent(0);

            QSplitter *newSplitter = new QSplitter(splitOrientation);
            newSplitter->setContentsMargins(0, 0, 0, 0);
            newSplitter->addWidget(asWidget);
            newSplitter->addWidget(newView->toWidget());
            parentSplitter->insertWidget(index, newSplitter);
        }

        addView(newView);
    }

    void Tab::closeView(IView *view)
    {
        if (!m_views.contains(view)) {
            qDebug() << Q_FUNC_INFO << "View " << (void*)view << "doesn't exist"
                << "in this tab";
            return;
        }

        QWidget *asWidget = view->toWidget();
        QSplitter *parentSplitter = qobject_cast<QSplitter*>(asWidget->parentWidget());

        Q_ASSERT_X(parentSplitter != 0, Q_FUNC_INFO,
                "Fix me: Parent widget of view not splitter");

        asWidget->setParent(0);
        m_views.removeAll(view);
        // Remember, we do not delete the view itself here. Its handled in
        // DocumentViewManager.

        bool removeThisTab = false;

        // Get rid of unnecessary splitters recursively.
        while (1) {
            if (parentSplitter->count() > 0) break;

            QWidget *ancestor = parentSplitter->parentWidget();

            if (static_cast<QWidget*>(this) == ancestor) {
                removeThisTab = true;
                break;
            }

            parentSplitter->setParent(0);
            parentSplitter->deleteLater();

            parentSplitter = qobject_cast<QSplitter*>(ancestor);

            Q_ASSERT_X(parentSplitter != 0, Q_FUNC_INFO,
                    "Fix me: Parent widget of splitter is neither Tab nor QSplitter");
        }

        if (removeThisTab) {
            QStackedWidget *stackedWidget = qobject_cast<QStackedWidget*>(parentWidget());
            Q_ASSERT_X(stackedWidget != 0, Q_FUNC_INFO,
                    "Tab's parent is not StackedWidget");

            TabWidget *tabWidget = qobject_cast<TabWidget*>(stackedWidget->parentWidget());
            Q_ASSERT_X(tabWidget != 0, Q_FUNC_INFO,
                    "StackedWidget's parent is not TabWidget");
            tabWidget->removeTab(tabWidget->indexOf(this));
            deleteLater();
        }
    }

    void Tab::onViewFocussedIn(IView *view)
    {
        // Apply LRU algo by pushing the least recently used
        // view to the front of the list.
        int i = m_views.indexOf(view);
        if (i >= 0 && i < m_views.size()) {
            m_views.takeAt(i);
            m_views.insert(0, view);
        }

        updateTitle();
    }

    void Tab::onDocumentChanged(IDocument *document)
    {
        updateTitle();
    }

    void Tab::onStatusBarMessage(const QString &message)
    {
        emit statusBarMessage(this, message);
    }

    void Tab::updateTitle()
    {
        emit tabInfoChanged(this);
    }

    void Tab::closeEvent(QCloseEvent *event)
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        QList<IDocument*> documents;
        QSet<IDocument*> processedDocuments;
        foreach (IView *view, m_views) {
            IDocument *document = view->document();

            if (processedDocuments.contains(document)) {
                continue;
            }

            processedDocuments << document;

            documents << document;
        }

        if (!documents.isEmpty()) {
            bool status = manager->saveDocuments(documents);
            if (!status) {
                event->ignore();
                return;
            }
        }

        const bool askForSave = false;
        while (1) {
            if (m_views.isEmpty()) break;

            IView *view = m_views.first();
            manager->closeView(view, askForSave);
        }

        event->accept();
    }

    void Tab::addView(IView *view)
    {
        if (m_views.contains(view)) {
            qDebug() << Q_FUNC_INFO << "View " << (void*)view
                << " is already added.";
            return;
        }

        m_views.append(view);

        connect(view, SIGNAL(focussedIn(IView*)), this,
                SLOT(onViewFocussedIn(IView*)));
        connect(view->document(), SIGNAL(documentChanged(IDocument*)),
                this, SLOT(onDocumentChanged(IDocument*)));
        connect(view, SIGNAL(statusBarMessage(const QString &)), this,
                SLOT(onStatusBarMessage(const QString &)));

        updateTitle();
    }

    QIcon Tab::modifiedIcon() const
    {
        static QIcon modifiedIcon = QIcon(smallsave_xpm);
        return modifiedIcon;
    }

    QIcon Tab::unmodifiedIcon() const
    {
        static QIcon unmodifiedIcon = QIcon(empty_xpm);
        return unmodifiedIcon;
    }

    class TabBarPrivate : public QTabBar
    {
    public:
        TabBarPrivate(QWidget *parent=0l) : QTabBar(parent)
        {
            setDrawBase(true);
        }
        ~TabBarPrivate() {}

    protected:
        void wheelEvent(QWheelEvent *ev)
        {
            if(count() > 1) {
                int current = currentIndex();
                if(ev->delta() < 0) {
                    current = (current + 1) % count();
                }
                else {
                    current--;
                    if(current < 0 ) {
                        current = count() - 1;
                    }
                }
                setCurrentIndex(current);
            }
        }
    };

    TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent)
    {
        setTabBar(new TabBarPrivate(this));
        connect(this, SIGNAL(tabCloseRequested(int)), this,
                SLOT(onTabCloseRequested(int)));
    }

    QList<Tab*> TabWidget::tabs() const
    {
        QList<Tab*> tabs;
        for (int i = 0; i < count(); ++i) {
            Tab *tab = qobject_cast<Tab*>(widget(i));
            if (!tab) {
                qDebug() << Q_FUNC_INFO << "Consists of tab that isn't a Tab";
            }
            tabs << tab;
        }
        return tabs;
    }

    void TabWidget::addTab(Tab *tab)
    {
        insertTab(-1, tab);
    }

    void TabWidget::insertTab(int index, Tab *tab)
    {
        QTabWidget::insertTab(index, tab, tab->tabIcon(), tab->tabText());
        connect(tab, SIGNAL(tabInfoChanged(Tab*)), this, SLOT(updateTabInfo(Tab*)));
        connect(tab, SIGNAL(statusBarMessage(Tab*, const QString&)), this,
                SLOT(onStatusBarMessage(Tab*, const QString&)));
    }

    Tab* TabWidget::currentTab() const
    {
        return qobject_cast<Tab*>(currentWidget());
    }

    void TabWidget::setCurrentTab(Tab *tab)
    {
        setCurrentWidget(tab);
    }

    void TabWidget::highlightView(IView *view)
    {
        QWidget *asWidget = view->toWidget();
        if (!asWidget) return;

        QWidget *parentWidget = asWidget->parentWidget();
        Tab *parentTab = 0;

        while (parentWidget) {
            parentTab = qobject_cast<Tab*>(parentWidget);
            if (parentTab) break;

            parentWidget = parentWidget->parentWidget();
        }

        if (!parentTab) return;

        setCurrentTab(parentTab);
        asWidget->setFocus();
    }

    void TabWidget::closeView(IView *view)
    {
        QWidget *asWidget = view->toWidget();
        if (!asWidget) return;

        QWidget *parentWidget = asWidget->parentWidget();
        Tab *parentTab = 0;

        while (parentWidget) {
            parentTab = qobject_cast<Tab*>(parentWidget);
            if (parentTab) break;

            parentWidget = parentWidget->parentWidget();
        }

        if (!parentTab) return;

        parentTab->closeView(view);
    }

    Tab* TabWidget::tabForView(IView *view) const
    {
        QWidget *widget = view->toWidget();

        Tab *tab = 0;
        while (1) {
            if (!widget) {
                break;
            }

            tab = qobject_cast<Tab*>(widget->parentWidget());
            if (tab) {
                break;
            }

            widget = widget->parentWidget();
        }

        return tab;
    }

    void TabWidget::updateTabInfo(Tab *tab)
    {
        int index = indexOf(tab);
        if (index < 0 || index >= count()) {
            return;
        }

        setTabIcon(index, tab->tabIcon());
        setTabText(index, tab->tabText());

        if (index == currentIndex()) {
            MainWindow *mw = MainWindow::instance();
            mw->updateTitle();

            IView *view = tab->activeView();
            if (!view) {
                return;
            }

            IDocument *document = view->document();
            if (!document) {
                return;
            }

            mw->action("editCut")->setEnabled(document->canCut());
            mw->action("editCopy")->setEnabled(document->canCopy());
            mw->action("editUndo")->setEnabled(document->canUndo());
            mw->action("editRedo")->setEnabled(document->canRedo());
        }
    }

    void TabWidget::onStatusBarMessage(Tab *tab, const QString &message)
    {
        int index = indexOf(tab);
        if (index == currentIndex()) {
            emit statusBarMessage(message);
        }
    }

    void TabWidget::onTabCloseRequested(int index)
    {
        QWidget *w = widget(index);
        w->close();
    }

} // namespace Caneda
