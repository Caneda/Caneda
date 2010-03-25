/***************************************************************************
                          qucshelp.cpp  -  description
                             -------------------
    begin                : Sun Jan 11 2004
    copyright            : (C) 2004 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "qucshelp.h"
#include "htmldatafetcher.h"

#include <QtGui/QPushButton>
#include <QtGui/QAction>
#include <QtGui/QPixmap>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QApplication>
#include <QtGui/QListView>
#include <QtGui/QTextBrowser>
#include <QtGui/QToolBar>
#include <QtGui/QDockWidget>

#include <QtCore/QAbstractListModel>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>

class StringListModel : public QAbstractListModel
{
public:
  StringListModel(const QStringList &strings, QObject *parent = 0) : 
    QAbstractListModel(parent), stringList(strings) {}
  ~StringListModel() {}
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;

private:
  QStringList stringList;
};

int StringListModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return stringList.count();
}

QVariant StringListModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || index.row() >= stringList.size() || role != Qt::DisplayRole )
    return QVariant();
  return stringList.at(index.row());
}

QVariant StringListModel::headerData(int section, Qt::Orientation orientation,
				     int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal)
    return QString("Column %1").arg(section);
  else
    return QString("Row %1").arg(section);
}

QucsHelp::QucsHelp(const QString& page,QWidget *parent) : QMainWindow(parent)
{
  currentIndex = 0;
  dataFetcher = new HtmlDataFetcher();
  links = dataFetcher->fetchLinksToFiles(QucsHelpDir.filePath("index.html"));
  // set application icon
  setWindowIcon (QPixmap(QucsSettings.BitmapDir + "big.qucs.xpm"));
  setWindowTitle(tr("Qucs Help System"));

  textBrowser = new QTextBrowser(this);
  textBrowser->setMinimumSize(400,200);
  setCentralWidget(textBrowser);
  createSidebar();//beware of order : may crash if changed
  setupActions(); // "     "   "      "    "    "  "
  

  textBrowser->setSource(QUrl(QucsHelpDir.filePath(links[0])));

  if(!page.isEmpty())
    textBrowser->setSource(QUrl(QucsHelpDir.filePath(page)));
}

QucsHelp::~QucsHelp()
{}

void QucsHelp::setupActions()
{
  QToolBar *toolbar = addToolBar(tr("Main toolbar"));
  QMenuBar *bar = menuBar();
  statusBar();

  QMenu *fileMenu = bar->addMenu(tr("&File"));
  QMenu *viewMenu = bar->addMenu(tr("&View"));
  bar->addSeparator();
  QMenu *helpMenu = bar->addMenu(tr("&Help"));
  
  QAction *quitAction = fileMenu->addAction(QIcon(QucsSettings.BitmapDir + "quit.png"),tr("&Quit"),qApp,SLOT(quit()),Qt::CTRL+Qt::Key_Q);

  QAction *backAction = viewMenu->addAction(QIcon(QucsSettings.BitmapDir + "back.png"), tr("&Back"),textBrowser,SLOT(backward()),
					    Qt::ALT+Qt::Key_Left);
  QAction *forwardAction = viewMenu->addAction(QIcon(QucsSettings.BitmapDir + "forward.png"),tr("&Forward"),textBrowser,
					       SLOT(forward()),Qt::ALT+Qt::Key_Right);
  QAction *homeAction = viewMenu->addAction(QIcon(QucsSettings.BitmapDir + "home.png"),tr("&Home"),textBrowser,
					    SLOT(home()),Qt::CTRL+Qt::Key_H);

  previousAction = viewMenu->addAction(QIcon(QucsSettings.BitmapDir + "previous.png"),tr("&Previous"),this,SLOT(previousLink()));
  nextAction = viewMenu->addAction(QIcon(QucsSettings.BitmapDir + "next.png"),tr("&Next"),this,SLOT(nextLink()));
  viewMenu->addSeparator();

  QAction *viewBrowseDock = dock->toggleViewAction();
  viewMenu->addAction(viewBrowseDock);
  viewBrowseDock->setStatusTip(tr("Enables/disables the table of contents"));
  viewBrowseDock->setWhatsThis(tr("Table of Contents\n\nEnables/disables the table of contents"));

  helpMenu->addAction(tr("&About Qt"),qApp,SLOT(aboutQt()));

  connect(textBrowser,SIGNAL(backwardAvailable(bool)),backAction,SLOT(setEnabled(bool)));
  connect(textBrowser,SIGNAL(forwardAvailable(bool)),forwardAction,SLOT(setEnabled(bool)));
  connect(textBrowser,SIGNAL(sourceChanged(const QUrl &)),this,SLOT(slotSourceChanged(const QUrl &)));
  
  toolbar->addAction(backAction);
  toolbar->addAction(forwardAction);
  toolbar->addSeparator();
  toolbar->addAction(homeAction);
  toolbar->addAction(previousAction);
  toolbar->addAction(nextAction);
  toolbar->addSeparator();
  toolbar->addAction(quitAction);
}


void QucsHelp::createSidebar()
{
  dock = new QDockWidget(tr("Table of Contents"),this);
  dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable); 
  dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  addDockWidget(Qt::LeftDockWidgetArea, dock);

  QStringList l = dataFetcher->fetchChapterTexts(QucsHelpDir.filePath("index.html"));
  model = new StringListModel(l);

  chaptersView = new QListView(dock);
  chaptersView->setSelectionMode(QAbstractItemView::SingleSelection);
  chaptersView->setModel(model);

  dock->setWidget(chaptersView);
    
  connect(chaptersView->selectionModel(),SIGNAL(selectionChanged ( const QItemSelection & , const QItemSelection &  )),
	  this,SLOT(displaySelectedChapter( const QItemSelection &)));
  chaptersView->setFocus();
}

void QucsHelp::displaySelectedChapter(const QItemSelection & is)
{
  const QModelIndex index = is.indexes()[0];
  if(index.isValid())
    textBrowser->setSource(QUrl(QucsHelpDir.filePath(links[index.row()])));
}

//This slot updates next and previous actions i.e enabling/disabling
void QucsHelp::slotSourceChanged(const QUrl& ustr)
{
  QString str = ustr.path();
  bool found = false;
  for(int i=0;i < links.count(); i++)
  {
    if(str.endsWith(links[i]))
    {
      currentIndex = i;
      previousAction->setEnabled(bool(i!=0));
      nextAction->setEnabled(bool(i+1 != links.count()));
      if(!(chaptersView->selectionModel()->isRowSelected(i,QModelIndex())))
      	chaptersView->selectionModel()->setCurrentIndex(model->index(i),QItemSelectionModel::ClearAndSelect);
      found = true;
      break;
    }
  }
  if(found == false) // some error
  {
    qDebug("QucsHelp::slotSourceChanged():  Link mismatch");
    return;
  }
}


void QucsHelp::previousLink()
{
  if(currentIndex > 0)
    --currentIndex;
  textBrowser->setSource(QUrl(QucsHelpDir.filePath(links[currentIndex])));
}

void QucsHelp::nextLink()
{
  ++currentIndex;
  if(currentIndex >= links.count())
    currentIndex = links.count()-1;
  textBrowser->setSource(QUrl(QucsHelpDir.filePath(links[currentIndex])));
}
