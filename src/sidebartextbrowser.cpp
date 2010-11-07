/***************************************************************************
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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

#include "sidebartextbrowser.h"

#include "documentviewmanager.h"
#include "global.h"
#include "settings.h"
#include "textdocument.h"

#include <QFileSystemModel>
#include <QDebug>
#include <QLineEdit>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QTextCodec>
#include <QToolButton>
#include <QVBoxLayout>

namespace Caneda
{
    //*************************************************************
    //********************* FileFilterProxyModel ******************
    //*************************************************************

    //! \brief This helps in filtering sidebar display corresponding to lineedit.
    class FileFilterProxyModel : public QSortFilterProxyModel
    {
    public:
        FileFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent)
        {
        }

        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
        {
            QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
            QFileSystemModel *fileModel = static_cast<QFileSystemModel*>(sourceModel());

            if(fileModel != NULL && fileModel->isDir(index0)) {
                return true;
            }

            return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
        }
    };

    //*************************************************************
    //******************** SidebarTextBrowser *********************
    //*************************************************************

    //! Constructor
    SidebarTextBrowser::SidebarTextBrowser(QWidget *parent) : QWidget(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        QHBoxLayout *hl = new QHBoxLayout();
        layout->addLayout(hl);
        m_filterEdit = new QLineEdit();
        hl->addWidget(m_filterEdit);

        m_clearButton = new QToolButton();
        m_clearButton->setIcon(QIcon(Caneda::bitmapDirectory() + "clearFilterText.png"));
        m_clearButton->setShortcut(Qt::ALT + Qt::Key_C);
        m_clearButton->setWhatsThis(
                tr("Clear Filter Text\n\nClears the filter text thus reshowing all components"));

        hl->addWidget(m_clearButton);
        m_clearButton->setEnabled(false);

        QSettings qSettings;

        Settings *settings = Settings::instance();
        settings->load(qSettings);

        /* Load library database settings */
        QString libpath = settings->currentValue("sidebarLibrary").toString() +
                          "/components/hdl";
        if(QFileInfo(libpath).exists() == false) {
            qDebug() << "Error loading text libraries";
            return;
        }

        m_fileModel = new QFileSystemModel;
        QModelIndex rootModelIndex = m_fileModel->setRootPath(libpath);

        m_proxyModel = new FileFilterProxyModel(this);
        m_proxyModel->setDynamicSortFilter(true);
        m_proxyModel->setSourceModel(m_fileModel);
        m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

        m_treeView = new QTreeView;
        m_treeView->setModel(m_proxyModel);
        m_treeView->setRootIndex(m_proxyModel->mapFromSource(rootModelIndex));

        m_treeView->setHeaderHidden(true);
        m_treeView->setColumnHidden(1, 1);
        m_treeView->setColumnHidden(2, 1);
        m_treeView->setColumnHidden(3, 1);
        m_treeView->setAnimated(true);
        m_treeView->setAlternatingRowColors(true);

        layout->addWidget(m_treeView);

        connect(m_filterEdit, SIGNAL(textChanged(const QString &)),
                this, SLOT(filterTextChanged()));
        connect(m_clearButton, SIGNAL(clicked()), m_filterEdit, SLOT(clear()));

        connect(m_fileModel, SIGNAL(modelReset()), m_treeView, SLOT(expandAll()));
        connect(m_treeView, SIGNAL(activated(const QModelIndex&)), this,
                SLOT(slotOnDoubleClicked(const QModelIndex&)));

        setWindowTitle(tr("Text Templates"));
    }

    SidebarTextBrowser::~SidebarTextBrowser()
    {
        m_treeView->setModel(0);
    }

    void SidebarTextBrowser::filterTextChanged()
    {
        QString text = m_filterEdit->text();
        m_clearButton->setEnabled(!text.isEmpty());
        QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
        m_proxyModel->setFilterRegExp(regExp);
    }

    void SidebarTextBrowser::slotOnDoubleClicked(const QModelIndex& index)
    {
        QModelIndex sourceIndex = m_proxyModel->mapToSource(index);

        if(m_fileModel->isDir(sourceIndex)) {
            return;
        }

        // It is a file so we paste the template
        QFile file(m_fileModel->fileInfo(sourceIndex).absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << tr("Could not open file for reading");
            return;
        }

        QTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));

        QString content = stream.readAll();
        file.close();

        IDocument *doc = DocumentViewManager::instance()->currentDocument();
        TextDocument *textDoc = qobject_cast<TextDocument*>(doc);

        if (textDoc) {
            textDoc->pasteTemplate(content);
        }
    }

} // namespace Caneda
