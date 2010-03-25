/***************************************************************************
                                canedahelp.h
                               ------------
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

#ifndef CANEDAHELP_H
#define CANEDAHELP_H

#include <QtGui/QMainWindow>
#include <QtGui/QFont>
#include <QtCore/QDir>
#include <QtCore/QStringList>

struct tCanedaSettings {
  int x, y, dx, dy;    // position and size of main window
  QFont font;
  QString BitmapDir;
  QString LangDir;
  QString DocDir;
  QString Language;
};

extern tCanedaSettings CanedaSettings;
extern QDir CanedaHelpDir;
class QAction;
class QListView;
class HtmlDataFetcher;
class QDockWidget;
class QTextBrowser;
class StringListModel;
class QModelIndex;
class QItemSelection;
class QUrl;

class CanedaHelp : public QMainWindow  {
  Q_OBJECT

  public:
  CanedaHelp(const QString& page,QWidget *parent=0l);
    ~CanedaHelp();

  private slots:
    void slotSourceChanged(const QUrl& str);
    void previousLink();
    void nextLink();
    void displaySelectedChapter(const QItemSelection & index);

  private:
    void setupActions();
    void createSidebar();

    QTextBrowser *textBrowser;
    int currentIndex;
    QStringList links;
    QAction *previousAction;
    QAction *nextAction;
    StringListModel *model;
    QListView *chaptersView;
    QDockWidget *dock;
    HtmlDataFetcher *dataFetcher;

};

#endif
