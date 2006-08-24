/***************************************************************************
                                qucshelp.h
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

#ifndef QUCSHELP_H
#define QUCSHELP_H

#include <qmainwindow.h>
#include <qtextbrowser.h>
#include <qdir.h>
#include <qfont.h>
#include <qstringlist.h>

struct tQucsSettings {
  int x, y, dx, dy;    // position and size of main window
  QFont font;
  QString BitmapDir;
  QString LangDir;
  QString DocDir;
  QString Language;
};

extern tQucsSettings QucsSettings;
extern QDir QucsHelpDir;
class QAction;
class QListViewItem;
class QListView;
class HtmlDataFetcher;
class QDockWindow;

class QucsHelp : public QMainWindow  {
  Q_OBJECT
  public:
    QucsHelp(const QString& page);
    ~QucsHelp();

  private slots:
    void slotSourceChanged(const QString& str);
    void slotToggleSidebar(bool);
    void slotToggleSidebarAction(bool);
    void previousLink();
    void nextLink();
    void displaySelectedChapter();

  private:
    void setupActions();
    void createSidebar();

    QTextBrowser *textBrowser;
    uint currentIndex;
    QStringList links;
    QAction *previousAction;
    QAction *nextAction;
    QAction *viewBrowseDock;
    QListView *chaptersView;
    QDockWindow *dock;
    HtmlDataFetcher *dataFetcher;
};

#endif
