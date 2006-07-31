/***************************************************************************
                              mouseactions.cpp
                             ------------------
    begin                : Thu Aug 28 2003
    copyright            : (C) 2003 by Michael Margraf
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

#include "qucs.h"
#include "main.h"
#include "node.h"
#include "schematic.h"
#include "mouseactions.h"
#include "components/component.h"
#include "components/spicedialog.h"
#include "components/optimizedialog.h"
#include "components/componentdialog.h"
#include "diagrams/diagramdialog.h"
#include "diagrams/markerdialog.h"
#include "diagrams/tabdiagram.h"
#include "diagrams/timingdiagram.h"
#include "dialogs/labeldialog.h"

#include <qinputdialog.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qevent.h>
#include <qaction.h>
#include <qtabwidget.h>

#include <limits.h>
#include <stdlib.h>


MouseActions::MouseActions()
{
  selElem  = 0;  // no component/diagram is selected
  isMoveEqual = false;  // mouse cursor move x and y the same way
  focusElement = 0;

  // ...............................................................
  // initialize menu appearing by right mouse button click on component
  ComponentMenu = new QPopupMenu(QucsMain);
  focusMEvent   = new QMouseEvent(QEvent::MouseButtonPress, QPoint(0,0),
				  Qt::NoButton, Qt::NoButton);
}


MouseActions::~MouseActions()
{
  delete ComponentMenu;
  delete focusMEvent;
}

// -----------------------------------------------------------
void MouseActions::setPainter(Schematic *Doc, QPainter *p)
{
  // contents to viewport transformation
  p->translate(-Doc->contentsX(), -Doc->contentsY());
  p->scale(Doc->Scale, Doc->Scale);
  p->translate(-Doc->ViewX1, -Doc->ViewY1);
  p->setPen(Qt::DotLine);
  p->setRasterOp(Qt::NotROP);  // background should not be erased
}

// -----------------------------------------------------------
bool MouseActions::pasteElements(Schematic *Doc)
{
  QClipboard *cb = QApplication::clipboard();   // get system clipboard
  QString s = cb->text(QClipboard::Clipboard);
  QTextStream stream(&s, IO_ReadOnly);
  movingElements.clear();
  if(!Doc->paste(&stream, &movingElements)) return false;

  Element *pe;
  int xmax, xmin, ymax, ymin;
  xmin = ymin = INT_MAX;
  xmax = ymax = INT_MIN;
  // First, get the max and min coordinates of all selected elements.
  for(pe = movingElements.first(); pe != 0; pe = movingElements.next()) {
    if(pe->Type == isWire) {
      if(pe->x1 < xmin) xmin = pe->x1;
      if(pe->x2 > xmax) xmax = pe->x2;
      if(pe->y1 < ymin) ymin = pe->y1;
      if(pe->y2 > ymax) ymax = pe->y2;
    }
    else {
      if(pe->cx < xmin) xmin = pe->cx;
      if(pe->cx > xmax) xmax = pe->cx;
      if(pe->cy < ymin) ymin = pe->cy;
      if(pe->cy > ymax) ymax = pe->cy;
    }
  }

  xmin = -((xmax+xmin) >> 1);   // calculate midpoint
  ymin = -((ymax+ymin) >> 1);
  Doc->setOnGrid(xmin, ymin);

  // moving with mouse cursor in the midpoint
  for(pe = movingElements.first(); pe != 0; pe = movingElements.next())
    if(pe->Type & isLabel) {
      pe->cx += xmin;  pe->x1 += xmin;
      pe->cy += ymin;  pe->y1 += ymin;
    }
    else
      pe->setCenter(xmin, ymin, true);

  return true;
}

// -----------------------------------------------------------
void MouseActions::editLabel(Schematic *Doc, WireLabel *pl)
{
  LabelDialog *Dia = new LabelDialog(pl, Doc);
  int Result = Dia->exec();
  if(Result == 0) return;

  QString Name  = Dia->NodeName->text();
  QString Value = Dia->InitValue->text();
  delete Dia;

  if(Name.isEmpty() && Value.isEmpty()) { // if nothing entered, delete label
    pl->pOwner->Label = 0;   // delete name of wire
    delete pl;
  }
  else {
/*    Name.replace(' ', '_');	// label must not contain spaces
    while(Name.at(0) == '_') Name.remove(0,1);  // must not start with '_'
    if(Name.isEmpty()) return;
    if(Name == pl->Name) return;*/
    if(Result == 1) return;  // nothing changed

    int old_x2 = pl->x2;
    pl->setName(Name);   // set new name
    pl->initValue = Value;
    if(pl->cx > (pl->x1+(pl->x2>>1)))
      pl->x1 -= pl->x2 - old_x2; // don't change position due to text width
  }

  Doc->sizeOfAll(Doc->UsedX1, Doc->UsedY1, Doc->UsedX2, Doc->UsedY2);
  Doc->viewport()->update();
  drawn = false;
  Doc->setChanged(true, true);
}

// -----------------------------------------------------------
// Reinserts all elements (moved by the user) back into the schematic.
void MouseActions::endElementMoving(Schematic *Doc, QPtrList<Element> *movElements)
{
  Element *pe;

  // First the wires with length zero are removed. This is important
  // if they are labeled. These labels must be put in the schematic
  // before all other elements.
  for(pe = movElements->first(); pe != 0; ) {
    if(pe->Type == isWire)
      if(pe->x1 == pe->x2) if(pe->y1 == pe->y2) {
	if(((Wire*)pe)->Label) {
	  Doc->insertNodeLabel((WireLabel*)((Wire*)pe)->Label);
	  ((Wire*)pe)->Label = 0;
	}
	movElements->removeRef(pe);
	delete (Wire*)pe;
	pe = movElements->current();
	continue;
      }
    pe = movElements->next();
  }


  for(pe = movElements->first(); pe!=0; pe = movElements->next()) {
//    pe->isSelected = false;  // deselect first (maybe afterwards pe == NULL)
    switch(pe->Type) {
      case isWire:
	Doc->insertWire((Wire*)pe);
	break;
      case isDiagram:
	Doc->Diagrams->append((Diagram*)pe);
	break;
      case isPainting:
	Doc->Paintings->append((Painting*)pe);
	break;
      case isComponent:
      case isAnalogComponent:
      case isDigitalComponent:
	Doc->insertRawComponent((Component*)pe, false);
	break;
      case isMovingLabel:
      case isHMovingLabel:
      case isVMovingLabel:
	Doc->insertNodeLabel((WireLabel*)pe);
	break;
      case isMarker:
	((Marker*)pe)->pGraph->Markers.append((Marker*)pe);
	break;
    }
  }

  movElements->clear();
  if((MAx3 != 0) || (MAy3 != 0))  // moved or put at the same place ?
    Doc->setChanged(true, true);

  // enlarge viewarea if components lie outside the view
  Doc->sizeOfAll(Doc->UsedX1, Doc->UsedY1, Doc->UsedX2, Doc->UsedY2);
  Doc->enlargeView(Doc->UsedX1, Doc->UsedY1, Doc->UsedX2, Doc->UsedY2);
  Doc->viewport()->update();
  drawn = false;
}

// -----------------------------------------------------------
// Moves elements in "movElements" by x/y
void MouseActions::moveElements(QPtrList<Element> *movElements, int x, int y)
{
  Wire *pw;
  Element *pe;
  for(pe = movElements->first(); pe != 0; pe = movElements->next()) {
    if(pe->Type == isWire) {
      pw = (Wire*)pe;   // connecting wires are not moved completely

      if(((unsigned long)pw->Port1) > 3) {
	pw->x1 += x;  pw->y1 += y;
	if(pw->Label) { pw->Label->cx += x;  pw->Label->cy += y; }
      }
      else {  if(long(pw->Port1) & 1) { pw->x1 += x; }
              if(long(pw->Port1) & 2) { pw->y1 += y; } }

      if(((unsigned long)pw->Port2) > 3) { pw->x2 += x;  pw->y2 += y; }
      else {  if(long(pw->Port2) & 1) pw->x2 += x;
              if(long(pw->Port2) & 2) pw->y2 += y; }

      if(pw->Label) {      // root of node label must lie on wire
        if(pw->Label->cx < pw->x1) pw->Label->cx = pw->x1;
        if(pw->Label->cy < pw->y1) pw->Label->cy = pw->y1;
        if(pw->Label->cx > pw->x2) pw->Label->cx = pw->x2;
        if(pw->Label->cy > pw->y2) pw->Label->cy = pw->y2;
      }

    }
    else pe->setCenter(x, y, true);
  }
}


// ***********************************************************************
// **********                                                   **********
// **********       Functions for serving mouse moving          **********
// **********                                                   **********
// ***********************************************************************
void MouseActions::MMoveElement(Schematic *Doc, QMouseEvent *Event)
{
  if(selElem == 0) return;

  int x  = Event->pos().x();
  int y  = Event->pos().y();
  int fx = int(float(x)/Doc->Scale) + Doc->ViewX1;
  int fy = int(float(y)/Doc->Scale) + Doc->ViewY1;
  int gx = fx;
  int gy = fy;
  Doc->setOnGrid(gx, gy);
  

  QPainter painter(Doc->viewport());
  setPainter(Doc, &painter);

  if(selElem->Type == isPainting) {
    QPainter paintUnscaled(Doc->viewport());
    paintUnscaled.setRasterOp(Qt::NotROP); // not erasing background

    x -= Doc->contentsX();
    y -= Doc->contentsY();
    ((Painting*)selElem)->MouseMoving(&painter, fx, fy, gx, gy,
                                       &paintUnscaled, x, y, drawn);
    drawn = true;
    return;
  }  // of "if(isPainting)"


  // ********** it is a component or diagram
  if(drawn) selElem->paintScheme(&painter); // erase old scheme
  drawn = true;
  selElem->setCenter(gx, gy);
  selElem->paintScheme(&painter); // paint scheme at new position
}

// -----------------------------------------------------------
void MouseActions::MMoveWire2(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  setPainter(Doc, &painter);

  if(drawn)
    if(MAx1 == 0) {
      painter.drawLine(MAx3, MAy3, MAx3, MAy2); // erase old
      painter.drawLine(MAx3, MAy2, MAx2, MAy2); // erase old
    }
    else {
      painter.drawLine(MAx3, MAy3, MAx2, MAy3); // erase old
      painter.drawLine(MAx2, MAy3, MAx2, MAy2); // erase old
    }
  else drawn = true;

  MAx2  = int(float(Event->pos().x())/Doc->Scale) + Doc->ViewX1;
  MAy2  = int(float(Event->pos().y())/Doc->Scale) + Doc->ViewY1;
  Doc->setOnGrid(MAx2, MAy2);

  if(MAx1 == 0) {
    painter.drawLine(MAx3, MAy3, MAx3, MAy2); // paint
    painter.drawLine(MAx3, MAy2, MAx2, MAy2); // paint
  }
  else {
    painter.drawLine(MAx3, MAy3, MAx2, MAy3); // paint
    painter.drawLine(MAx2, MAy3, MAx2, MAy2); // paint
  }

  QucsMain->MouseDoubleClickAction = &MouseActions::MDoubleClickWire2;
}

// -----------------------------------------------------------
void MouseActions::MMoveWire1(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  setPainter(Doc, &painter);

  if(drawn) {
    painter.drawLine(MAx1, MAy3, MAx2, MAy3); // erase old
    painter.drawLine(MAx3, MAy1, MAx3, MAy2);
  }
  drawn = true;

  MAx3  = int(float(Event->pos().x())/Doc->Scale) + Doc->ViewX1;
  MAy3  = int(float(Event->pos().y())/Doc->Scale) + Doc->ViewY1;
  Doc->setOnGrid(MAx3, MAy3);

  MAx1  = Doc->contentsX()+Doc->ViewX1;
  MAy1  = Doc->contentsY()+Doc->ViewY1;
  MAx2  = MAx1 + Doc->visibleWidth();
  MAy2  = MAy1 + Doc->visibleHeight();

  painter.drawLine(MAx1, MAy3, MAx2, MAy3); // paint
  painter.drawLine(MAx3, MAy1, MAx3, MAy2);
}

// -----------------------------------------------------------
// Paints a rectangle to select elements within it.
void MouseActions::MMoveSelect(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  setPainter(Doc, &painter);

  if(drawn) painter.drawRect(MAx1, MAy1, MAx2, MAy2); // erase old rectangle
  drawn = true;
  MAx2 = int(float(Event->pos().x())/Doc->Scale) + Doc->ViewX1 - MAx1;
  MAy2 = int(float(Event->pos().y())/Doc->Scale) + Doc->ViewY1 - MAy1;
  if(isMoveEqual)     // x and y size must be equal ?
    if(abs(MAx2) > abs(MAy2)) {
      if(MAx2<0) MAx2 = -abs(MAy2); else MAx2 = abs(MAy2);
    }
    else { if(MAy2<0) MAy2 = -abs(MAx2); else MAy2 = abs(MAx2); }
  painter.drawRect(MAx1, MAy1, MAx2, MAy2); // paint new rectangle
}

// -----------------------------------------------------------
void MouseActions::MMoveResizePainting(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  setPainter(Doc, &painter);

  MAx1 = int(float(Event->pos().x())/Doc->Scale) + Doc->ViewX1;
  MAy1 = int(float(Event->pos().y())/Doc->Scale) + Doc->ViewY1;
  Doc->setOnGrid(MAx1, MAy1);
  ((Painting*)focusElement)->MouseResizeMoving(MAx1, MAy1, &painter);
}

// -----------------------------------------------------------
// Moves components by keeping the mouse button pressed.
void MouseActions::MMoveMoving(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  setPainter(Doc, &painter);

  MAx2 = int(Event->pos().x()/Doc->Scale) + Doc->ViewX1;
  MAy2 = int(Event->pos().y()/Doc->Scale) + Doc->ViewY1;

  Doc->setOnGrid(MAx2, MAy2);
  MAx3 = MAx1 = MAx2 - MAx1;
  MAy3 = MAy1 = MAy2 - MAy1;

  movingElements.clear();
  Doc->copySelectedElements(&movingElements);
  Doc->viewport()->repaint();

  Wire *pw;
  // Changes the position of all moving elements by dx/dy
  for(Element *pe=movingElements.first(); pe!=0; pe=movingElements.next()) {
    if(pe->Type == isWire) {
      pw = (Wire*)pe;   // connecting wires are not moved completely

      if(((unsigned long)pw->Port1) > 3) { pw->x1 += MAx1;  pw->y1 += MAy1; }
      else {  if(long(pw->Port1) & 1) { pw->x1 += MAx1; }
              if(long(pw->Port1) & 2) { pw->y1 += MAy1; } }

      if(((unsigned long)pw->Port2) > 3) { pw->x2 += MAx1;  pw->y2 += MAy1; }
      else {  if(long(pw->Port2) & 1) pw->x2 += MAx1;
              if(long(pw->Port2) & 2) pw->y2 += MAy1; }

      if(pw->Label) {      // root of node label must lie on wire
        if(pw->Label->cx < pw->x1) pw->Label->cx = pw->x1;
        if(pw->Label->cy < pw->y1) pw->Label->cy = pw->y1;
        if(pw->Label->cx > pw->x2) pw->Label->cx = pw->x2;
        if(pw->Label->cy > pw->y2) pw->Label->cy = pw->y2;
      }

    }
    else pe->setCenter(MAx1, MAy1, true);

    pe->paintScheme(&painter);
  }

  drawn = true;
  MAx1 = MAx2;
  MAy1 = MAy2;
  QucsMain->MouseMoveAction = &MouseActions::MMoveMoving2;
  QucsMain->MouseReleaseAction = &MouseActions::MReleaseMoving;

}

// -----------------------------------------------------------
// Moves components by keeping the mouse button pressed.
void MouseActions::MMoveMoving2(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  setPainter(Doc, &painter);

  MAx2 = int(float(Event->pos().x())/Doc->Scale) + Doc->ViewX1;
  MAy2 = int(float(Event->pos().y())/Doc->Scale) + Doc->ViewY1;

  Element *pe;
  if(drawn) // erase old scheme
    for(pe = movingElements.first(); pe != 0; pe = movingElements.next())
      pe->paintScheme(&painter);
//      if(pe->Type == isWire)  if(((Wire*)pe)->Label)
//        if(!((Wire*)pe)->Label->isSelected)
//          ((Wire*)pe)->Label->paintScheme(&painter);

  drawn = true;
  if((Event->state() & Qt::ControlButton) == 0)
    Doc->setOnGrid(MAx2, MAy2);  // use grid only if CTRL key not pressed
  MAx1 = MAx2 - MAx1;
  MAy1 = MAy2 - MAy1;
  MAx3 += MAx1;  MAy3 += MAy1;   // keep track of the complete movement

  moveElements(&movingElements, MAx1, MAy1);  // moves elements by MAx1/MAy1

  // paint afterwards to avoid conflict between wire and label painting
  for(pe = movingElements.first(); pe != 0; pe = movingElements.next())
    pe->paintScheme(&painter);
//    if(pe->Type == isWire)  if(((Wire*)pe)->Label)
//      if(!((Wire*)pe)->Label->isSelected)
//        ((Wire*)pe)->Label->paintScheme(&painter);

  MAx1 = MAx2;
  MAy1 = MAy2;
}

// -----------------------------------------------------------
// Moves components after paste from clipboard.
void MouseActions::MMovePaste(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  setPainter(Doc, &painter);

  MAx1 = int(float(Event->pos().x())/Doc->Scale) + Doc->ViewX1;
  MAy1 = int(float(Event->pos().y())/Doc->Scale) + Doc->ViewY1;
  Doc->setOnGrid(MAx1, MAy1);

  for(Element *pe=movingElements.first(); pe!=0; pe=movingElements.next()) {
    if(pe->Type & isLabel) {
      pe->cx += MAx1;  pe->x1 += MAx1;
      pe->cy += MAy1;  pe->y1 += MAy1;
    }
    else
      pe->setCenter(MAx1, MAy1, true);
    pe->paintScheme(&painter);
  }

  drawn = true;
  QucsMain->MouseMoveAction = &MouseActions::MMoveMoving2;
  QucsMain->MouseReleaseAction = &MouseActions::MReleasePaste;
}

// -----------------------------------------------------------
// Paints a cross under the mouse cursor to show the delete modus.
void MouseActions::MMoveDelete(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  painter.setRasterOp(Qt::NotROP);  // background should not be erased

  if(drawn) {
    painter.drawLine(MAx3-15, MAy3-15, MAx3+15, MAy3+15); // erase old
    painter.drawLine(MAx3-15, MAy3+15, MAx3+15, MAy3-15);
  }
  drawn = true;

  MAx3  = Event->pos().x() - Doc->contentsX();
  MAy3  = Event->pos().y() - Doc->contentsY();

  painter.drawLine(MAx3-15, MAy3-15, MAx3+15, MAy3+15); // paint
  painter.drawLine(MAx3-15, MAy3+15, MAx3+15, MAy3-15);
}

// -----------------------------------------------------------
// Paints a label above the mouse cursor to show the set wire label modus.
void MouseActions::MMoveLabel(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  painter.setRasterOp(Qt::NotROP);  // background should not be erased

  if(drawn) {
    painter.drawLine(MAx3, MAy3, MAx3+10, MAy3-10); // erase old
    painter.drawLine(MAx3+10, MAy3-10, MAx3+20, MAy3-10);
    painter.drawLine(MAx3+10, MAy3-10, MAx3+10, MAy3-17);

    painter.drawLine(MAx3+12, MAy3-12, MAx3+15, MAy3-23);   // "A"
    painter.drawLine(MAx3+14, MAy3-17, MAx3+17, MAy3-17);
    painter.drawLine(MAx3+19, MAy3-12, MAx3+16, MAy3-23);
  }
  drawn = true;

  MAx3  = Event->pos().x() - Doc->contentsX();
  MAy3  = Event->pos().y() - Doc->contentsY();

  painter.drawLine(MAx3, MAy3, MAx3+10, MAy3-10); // paint new
  painter.drawLine(MAx3+10, MAy3-10, MAx3+20, MAy3-10);
  painter.drawLine(MAx3+10, MAy3-10, MAx3+10, MAy3-17);

  painter.drawLine(MAx3+12, MAy3-12, MAx3+15, MAy3-23);   // "A"
  painter.drawLine(MAx3+14, MAy3-17, MAx3+17, MAy3-17);
  painter.drawLine(MAx3+19, MAy3-12, MAx3+16, MAy3-23);
}

// -----------------------------------------------------------
// Paints a triangle above the mouse cursor to show the set marker modus.
void MouseActions::MMoveMarker(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  painter.setRasterOp(Qt::NotROP);  // background should not be erased

  if(drawn) {
    painter.drawLine(MAx3, MAy3-2, MAx3-8, MAy3-10); // erase old
    painter.drawLine(MAx3+1, MAy3-3, MAx3+8, MAy3-10);
    painter.drawLine(MAx3-7, MAy3-10, MAx3+7, MAy3-10);
  }
  drawn = true;

  MAx3  = Event->pos().x() - Doc->contentsX();
  MAy3  = Event->pos().y() - Doc->contentsY();

  painter.drawLine(MAx3, MAy3-2, MAx3-8, MAy3-10); // paint new
  painter.drawLine(MAx3+1, MAy3-3, MAx3+8, MAy3-10);
  painter.drawLine(MAx3-7, MAy3-10, MAx3+7, MAy3-10);
}

// -----------------------------------------------------------
// Paints rounded arrows above the mouse cursor to show the
// "mirror about y axis" modus.
void MouseActions::MMoveMirrorY(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  painter.setRasterOp(Qt::NotROP);  // background should not be erased

  if(drawn) {
    painter.drawLine(MAx3-11, MAy3-4, MAx3-9, MAy3-9); // erase old
    painter.drawLine(MAx3-11, MAy3-3, MAx3-6, MAy3-3);
    painter.drawLine(MAx3+11, MAy3-4, MAx3+9, MAy3-9);
    painter.drawLine(MAx3+11, MAy3-3, MAx3+6, MAy3-3);
    painter.drawArc(MAx3-10, MAy3-8, 21, 10, 16*20, 16*140);
  }
  drawn = true;

  MAx3  = Event->pos().x() - Doc->contentsX();
  MAy3  = Event->pos().y() - Doc->contentsY();

  painter.drawLine(MAx3-11, MAy3-4, MAx3-9, MAy3-9); // paint new
  painter.drawLine(MAx3-11, MAy3-3, MAx3-6, MAy3-3);
  painter.drawLine(MAx3+11, MAy3-4, MAx3+9, MAy3-9);
  painter.drawLine(MAx3+11, MAy3-3, MAx3+6, MAy3-3);
  painter.drawArc(MAx3-10, MAy3-8, 21, 10, 16*20, 16*140);
}

// -----------------------------------------------------------
// Paints rounded arrows beside the mouse cursor to show the
// "mirror about x axis" modus.
void MouseActions::MMoveMirrorX(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  painter.setRasterOp(Qt::NotROP);  // background should not be erased

  if(drawn) {
    painter.drawLine(MAx3-4, MAy3-11, MAx3-9, MAy3-9); // erase old
    painter.drawLine(MAx3-3, MAy3-11, MAx3-3, MAy3-6);
    painter.drawLine(MAx3-4, MAy3+11, MAx3-9, MAy3+9);
    painter.drawLine(MAx3-3, MAy3+11, MAx3-3, MAy3+6);
    painter.drawArc(MAx3-8, MAy3-10, 10, 21, 16*110, 16*140);
  }
  drawn = true;

  MAx3  = Event->pos().x() - Doc->contentsX();
  MAy3  = Event->pos().y() - Doc->contentsY();

  painter.drawLine(MAx3-4, MAy3-11, MAx3-9, MAy3-9); // paint new
  painter.drawLine(MAx3-3, MAy3-11, MAx3-3, MAy3-6);
  painter.drawLine(MAx3-4, MAy3+11, MAx3-9, MAy3+9);
  painter.drawLine(MAx3-3, MAy3+11, MAx3-3, MAy3+6);
  painter.drawArc(MAx3-8, MAy3-10, 10, 21, 16*110, 16*140);
}

// -----------------------------------------------------------
// Paints a rounded arrow above the mouse cursor to show the "rotate" modus.
void MouseActions::MMoveRotate(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  painter.setRasterOp(Qt::NotROP);  // background should not be erased

  if(drawn) {
    painter.drawLine(MAx3-6, MAy3+8, MAx3-6, MAy3+1); // erase old
    painter.drawLine(MAx3-7, MAy3+8, MAx3-12, MAy3+8);
    painter.drawArc(MAx3-10, MAy3-10, 21, 21, -16*20, 16*240);
  }
  drawn = true;

  MAx3  = Event->pos().x() - Doc->contentsX();
  MAy3  = Event->pos().y() - Doc->contentsY();

  painter.drawLine(MAx3-6, MAy3+8, MAx3-6, MAy3+1); // paint new
  painter.drawLine(MAx3-7, MAy3+8, MAx3-12, MAy3+8);
  painter.drawArc(MAx3-10, MAy3-10, 21, 21, -16*20, 16*240);
}

// -----------------------------------------------------------
// Paints a rectangle beside the mouse cursor to show the "activate" modus.
void MouseActions::MMoveActivate(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  painter.setRasterOp(Qt::NotROP);  // background should not be erased

  if(drawn) {
    painter.drawRect(MAx3, MAy3-9, 14, 10); // erase old
    painter.drawLine(MAx3, MAy3-9, MAx3+13, MAy3);
    painter.drawLine(MAx3, MAy3, MAx3+13, MAy3-9);
  }
  drawn = true;

  MAx3  = Event->pos().x() - Doc->contentsX();
  MAy3  = Event->pos().y() - Doc->contentsY();

  painter.drawRect(MAx3, MAy3-9, 14, 10); // paint new
  painter.drawLine(MAx3, MAy3-9, MAx3+13, MAy3);
  painter.drawLine(MAx3, MAy3, MAx3+13, MAy3-9);
}

// -----------------------------------------------------------
// Paints a grid beside the mouse cursor to show the "on grid" modus.
void MouseActions::MMoveOnGrid(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  painter.setRasterOp(Qt::NotROP);  // background should not be erased

  if(drawn) {
    painter.drawLine(MAx3+10, MAy3+ 3, MAx3+25, MAy3+3); // erase old
    painter.drawLine(MAx3+10, MAy3+ 7, MAx3+25, MAy3+7);
    painter.drawLine(MAx3+10, MAy3+11, MAx3+25, MAy3+11);
    painter.drawLine(MAx3+13, MAy3, MAx3+13, MAy3+15);
    painter.drawLine(MAx3+17, MAy3, MAx3+17, MAy3+15);
    painter.drawLine(MAx3+21, MAy3, MAx3+21, MAy3+15);
  }
  drawn = true;

  MAx3  = Event->pos().x() - Doc->contentsX();
  MAy3  = Event->pos().y() - Doc->contentsY();

  painter.drawLine(MAx3+10, MAy3+ 3, MAx3+25, MAy3+3); // paint new
  painter.drawLine(MAx3+10, MAy3+ 7, MAx3+25, MAy3+7);
  painter.drawLine(MAx3+10, MAy3+11, MAx3+25, MAy3+11);
  painter.drawLine(MAx3+13, MAy3, MAx3+13, MAy3+15);
  painter.drawLine(MAx3+17, MAy3, MAx3+17, MAy3+15);
  painter.drawLine(MAx3+21, MAy3, MAx3+21, MAy3+15);
}

// -----------------------------------------------------------
// Paints symbol beside the mouse to show the "move component text" modus.
void MouseActions::MMoveMoveTextB(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  painter.setRasterOp(Qt::NotROP);  // background should not be erased

  if(drawn) {
    painter.drawLine(MAx3+14, MAy3   , MAx3+16, MAy3); // erase old
    painter.drawLine(MAx3+23, MAy3   , MAx3+25, MAy3);
    painter.drawLine(MAx3+13, MAy3   , MAx3+13, MAy3+ 3);
    painter.drawLine(MAx3+13, MAy3+ 7, MAx3+13, MAy3+10);
    painter.drawLine(MAx3+14, MAy3+10, MAx3+16, MAy3+10);
    painter.drawLine(MAx3+23, MAy3+10, MAx3+25, MAy3+10);
    painter.drawLine(MAx3+26, MAy3   , MAx3+26, MAy3+ 3);
    painter.drawLine(MAx3+26, MAy3+ 7, MAx3+26, MAy3+10);
  }
  drawn = true;

  MAx3 = Event->pos().x() - Doc->contentsX();
  MAy3 = Event->pos().y() - Doc->contentsY();

  painter.drawLine(MAx3+14, MAy3   , MAx3+16, MAy3); // paint new
  painter.drawLine(MAx3+23, MAy3   , MAx3+25, MAy3);
  painter.drawLine(MAx3+13, MAy3   , MAx3+13, MAy3+ 3);
  painter.drawLine(MAx3+13, MAy3+ 7, MAx3+13, MAy3+10);
  painter.drawLine(MAx3+14, MAy3+10, MAx3+16, MAy3+10);
  painter.drawLine(MAx3+23, MAy3+10, MAx3+25, MAy3+10);
  painter.drawLine(MAx3+26, MAy3   , MAx3+26, MAy3+ 3);
  painter.drawLine(MAx3+26, MAy3+ 7, MAx3+26, MAy3+10);
}

// -----------------------------------------------------------
void MouseActions::MMoveMoveText(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  setPainter(Doc, &painter);

  if(drawn)
    painter.drawRect(MAx1, MAy1, MAx2, MAy2); // erase old
  drawn = true;

  int newX = int(float(Event->pos().x())/Doc->Scale) + Doc->ViewX1;
  int newY = int(float(Event->pos().y())/Doc->Scale) + Doc->ViewY1;
  MAx1 += newX - MAx3;
  MAy1 += newY - MAy3;
  MAx3  = newX;
  MAy3  = newY;

  painter.drawRect(MAx1, MAy1, MAx2, MAy2); // paint new
}

// -----------------------------------------------------------
// Paints symbol beside the mouse to show the "Zoom in" modus.
void MouseActions::MMoveZoomIn(Schematic *Doc, QMouseEvent *Event)
{
  QPainter painter(Doc->viewport());
  painter.setRasterOp(Qt::NotROP);  // background should not be erased

  if(drawn) {
    painter.drawLine(MAx3+14, MAy3   , MAx3+22, MAy3); // erase old
    painter.drawLine(MAx3+18, MAy3-4 , MAx3+18, MAy3+4);
    painter.drawEllipse(MAx3+12, MAy3-6, 13, 13);
  }
  drawn = true;

  MAx3 = Event->pos().x() - Doc->contentsX();
  MAy3 = Event->pos().y() - Doc->contentsY();

  painter.drawLine(MAx3+14, MAy3   , MAx3+22, MAy3);  // paint new
  painter.drawLine(MAx3+18, MAy3-4 , MAx3+18, MAy3+4);
  painter.drawEllipse(MAx3+12, MAy3-6, 13, 13);
}


// ************************************************************************
// **********                                                    **********
// **********    Functions for serving mouse button clicking     **********
// **********                                                    **********
// ************************************************************************

// Is called from several MousePress functions to show right button menu.
void MouseActions::rightPressMenu(Schematic *Doc, QMouseEvent *Event, int x, int y)
{
  MAx1 = x;
  MAy1 = y;
  focusElement = Doc->selectElement(x, y, false);

  if(focusElement)  // remove special function (4 least significant bits)
    focusElement->Type &= isSpecialMask;


  // define menu
  ComponentMenu->clear();
  while(true) {
    if(focusElement) {
      focusElement->isSelected = true;
      ComponentMenu->insertItem(
         QObject::tr("Edit Properties"), QucsMain, SLOT(slotEditElement()));

      if((focusElement->Type & isComponent) == 0) break;
    }
    else {
      QucsMain->symEdit->addTo(ComponentMenu);
      QucsMain->fileSettings->addTo(ComponentMenu);
    }
    if(!QucsMain->moveText->isOn())
      QucsMain->moveText->addTo(ComponentMenu);
    break;
  }
  while(true) {
    if(focusElement)
      if(focusElement->Type == isGraph) break;
    if(!QucsMain->onGrid->isOn())
      QucsMain->onGrid->addTo(ComponentMenu);
    QucsMain->editCopy->addTo(ComponentMenu);
    if(!QucsMain->editPaste->isOn())
      QucsMain->editPaste->addTo(ComponentMenu);
    break;
  }
  if(!QucsMain->editDelete->isOn())
    QucsMain->editDelete->addTo(ComponentMenu);
  if(focusElement) if(focusElement->Type == isMarker) {
    ComponentMenu->insertSeparator();
    QString s = QObject::tr("power matching");
    if( ((Marker*)focusElement)->pGraph->Var == "Sopt" )
      s = QObject::tr("noise matching");
    ComponentMenu->insertItem(s, QucsMain, SLOT(slotPowerMatching()));
    if( ((Marker*)focusElement)->pGraph->Var.left(2) == "S[" )
      ComponentMenu->insertItem(QObject::tr("2-port matching"), QucsMain,
                                SLOT(slot2PortMatching()));
  }
  do {
    if(focusElement) {
      if(focusElement->Type == isDiagram) break;
      if(focusElement->Type == isGraph) {
        QucsMain->graph2csv->addTo(ComponentMenu);
        break;
      }
    }
    ComponentMenu->insertSeparator();
    if(focusElement) if(focusElement->Type & isComponent)
      if(!QucsMain->editActivate->isOn())
        QucsMain->editActivate->addTo(ComponentMenu);
    if(!QucsMain->editRotate->isOn())
      QucsMain->editRotate->addTo(ComponentMenu);
    if(!QucsMain->editMirror->isOn())
      QucsMain->editMirror->addTo(ComponentMenu);
    if(!QucsMain->editMirrorY->isOn())
      QucsMain->editMirrorY->addTo(ComponentMenu);
  } while(false);

  *focusMEvent = *Event;  // remember event for "edit component" action
  ComponentMenu->popup(Event->globalPos());
  Doc->viewport()->update();
  drawn = false;
}

// -----------------------------------------------------------
void MouseActions::MPressLabel(Schematic *Doc, QMouseEvent*, int x, int y)
{
  Wire *pw = 0;
  WireLabel *pl=0;
  Node *pn = Doc->selectedNode(x, y);
  if(!pn) {
    pw = Doc->selectedWire(x, y);
    if(!pw) return;
  }

  QString Name, Value;
  Element *pe=0;
  // is wire line already labeled ?
  if(pw) pe = Doc->getWireLabel(pw->Port1);
  else pe = Doc->getWireLabel(pn);
  if(pe) {
    if(pe->Type & isComponent) {
      QMessageBox::information(0, QObject::tr("Info"),
		QObject::tr("The ground potential cannot be labeled!"));
      return;
    }
    pl = ((Conductor*)pe)->Label;
  }

  LabelDialog *Dia = new LabelDialog(pl, Doc);
  if(Dia->exec() == 0) return;

  Name  = Dia->NodeName->text();
  Value = Dia->InitValue->text();
  delete Dia;

  if(Name.isEmpty() && Value.isEmpty() ) { // if nothing entered, delete name
    if(pe) {
      if(((Conductor*)pe)->Label)
        delete ((Conductor*)pe)->Label; // delete old name
      ((Conductor*)pe)->Label = 0;
    }
    else {
      if(pw) pw->setName("", "");   // delete name of wire
      else pn->setName("", "");
    }
  }
  else {
/*    Name.replace(' ', '_');	// label must not contain spaces
    while(Name.at(0) == '_') Name.remove(0,1);  // must not start with '_'
    if(Name.isEmpty()) return;
*/
    if(pe) {
      if(((Conductor*)pe)->Label)
        delete ((Conductor*)pe)->Label; // delete old name
      ((Conductor*)pe)->Label = 0;
    }

    int xl = x+30;
    int yl = y-30;
    Doc->setOnGrid(xl, yl);
    // set new name
    if(pw) pw->setName(Name, Value, x-pw->x1 + y-pw->y1, xl, yl);
    else pn->setName(Name, Value, xl, yl);
  }

  Doc->sizeOfAll(Doc->UsedX1, Doc->UsedY1, Doc->UsedX2, Doc->UsedY2);
  Doc->viewport()->update();
  drawn = false;
  Doc->setChanged(true, true);
}

// -----------------------------------------------------------
void MouseActions::MPressSelect(Schematic *Doc, QMouseEvent *Event, int x, int y)
{
  bool Ctrl;
  if(Event->state() & Qt::ControlButton) Ctrl = true;
  else Ctrl = false;

  int No=0;
  MAx1 = x;
  MAy1 = y;
  focusElement = Doc->selectElement(x, y, Ctrl, &No);
  isMoveEqual = false;   // moving not neccessarily square


  if(focusElement)
  switch(focusElement->Type) {
    case isPaintingResize:  // resize painting ?
      focusElement->Type = isPainting;
      QucsMain->MouseReleaseAction = &MouseActions::MReleaseResizePainting;
      QucsMain->MouseMoveAction = &MouseActions::MMoveResizePainting;
      QucsMain->MousePressAction = 0;
      QucsMain->MouseDoubleClickAction = 0;
      Doc->grabKeyboard();  // no keyboard inputs during move actions
      return;

    case isDiagramResize:  // resize diagram ?
      if(((Diagram*)focusElement)->Name.left(4) != "Rect")
        if(((Diagram*)focusElement)->Name.at(0) != 'T')
          if(((Diagram*)focusElement)->Name != "Curve")
            isMoveEqual = true;  // diagram must be square

      focusElement->Type = isDiagram;
      MAx1 = focusElement->cx;
      MAx2 = focusElement->x2;
      if(((Diagram*)focusElement)->State & 1) {
        MAx1 += MAx2;
        MAx2 *= -1;
      }
      MAy1 =  focusElement->cy;
      MAy2 = -focusElement->y2;
      if(((Diagram*)focusElement)->State & 2) {
        MAy1 += MAy2;
        MAy2 *= -1;
      }

      QucsMain->MouseReleaseAction = &MouseActions::MReleaseResizeDiagram;
      QucsMain->MouseMoveAction = &MouseActions::MMoveSelect;
      QucsMain->MousePressAction = 0;
      QucsMain->MouseDoubleClickAction = 0;
      Doc->grabKeyboard(); // no keyboard inputs during move actions
      return;

    case isDiagramScroll:  // scroll in tabular ?
      focusElement->Type = isDiagram;

      if(((Diagram*)focusElement)->Name == "Time") {
        if(((TimingDiagram*)focusElement)->scroll(MAx1))
          Doc->setChanged(true, true, 'm'); // 'm' = only the first time
      }
      else {
        if(((TabDiagram*)focusElement)->scroll(MAy1))
          Doc->setChanged(true, true, 'm'); // 'm' = only the first time
      }
      Doc->viewport()->update();
      drawn = false;
      return;

    case isComponentText:  // property text of component ?
      focusElement->Type &= (~isComponentText) | isComponent;

      MAx3 = No;
      QucsMain->slotApplyCompText();
      return;
  }



  QucsMain->MousePressAction = 0;
  QucsMain->MouseDoubleClickAction = 0;
  Doc->grabKeyboard();  // no keyboard inputs during move actions
  Doc->viewport()->update();
  drawn = false;

  if(focusElement == 0) {
    MAx2 = 0;  // if not clicking on an element => open a rectangle
    MAy2 = 0;
    QucsMain->MouseReleaseAction = &MouseActions::MReleaseSelect2;
    QucsMain->MouseMoveAction = &MouseActions::MMoveSelect;
  }
  else {  // element could be moved
    if(!Ctrl) {
      if(!focusElement->isSelected)// Don't move selected elements if clicked
        Doc->deselectElements(focusElement); // element was not selected.
      focusElement->isSelected = true;
    }
    Doc->setOnGrid(MAx1, MAy1);
    QucsMain->MouseMoveAction = &MouseActions::MMoveMoving;
  }
}

// -----------------------------------------------------------
void MouseActions::MPressDelete(Schematic *Doc, QMouseEvent*, int x, int y)
{
  Element *pe = Doc->selectElement(x, y, false);
  if(pe) {
    pe->isSelected = true;
    Doc->deleteElements();

    Doc->sizeOfAll(Doc->UsedX1, Doc->UsedY1, Doc->UsedX2, Doc->UsedY2);
    Doc->viewport()->update();
    drawn = false;
  }
}

// -----------------------------------------------------------
void MouseActions::MPressActivate(Schematic *Doc, QMouseEvent*, int x, int y)
{
  MAx1 = x;
  MAy1 = y;
  if(!Doc->activateSpecifiedComponent(x, y)) {
//    if(Event->button() != Qt::LeftButton) return;
    MAx2 = 0;  // if not clicking on a component => open a rectangle
    MAy2 = 0;
    QucsMain->MousePressAction = 0;
    QucsMain->MouseReleaseAction = &MouseActions::MReleaseActivate;
    QucsMain->MouseMoveAction = &MouseActions::MMoveSelect;
  }
  Doc->viewport()->update();
  drawn = false;
}

// -----------------------------------------------------------
void MouseActions::MPressMirrorX(Schematic *Doc, QMouseEvent*, int x, int y)
{
  // no use in mirroring wires or diagrams
  Component *c = Doc->selectedComponent(x, y);
  if(c) {
    if(c->Ports.count() < 1) return;  // only mirror components with ports
    c->mirrorX();
    Doc->setCompPorts(c);
  }
  else {
    Painting *p = Doc->selectedPainting(x, y);
    if(p == 0) return;
    p->mirrorX();
  }

  Doc->viewport()->update();
  drawn = false;
  Doc->setChanged(true, true);
}

// -----------------------------------------------------------
void MouseActions::MPressMirrorY(Schematic *Doc, QMouseEvent*, int x, int y)
{
  // no use in mirroring wires or diagrams
  Component *c = Doc->selectedComponent(x, y);
  if(c) {
    if(c->Ports.count() < 1) return;  // only mirror components with ports
    c->mirrorY();
    Doc->setCompPorts(c);
  }
  else {
    Painting *p = Doc->selectedPainting(x, y);
    if(p == 0) return;
    p->mirrorY();
  }

  Doc->viewport()->update();
  drawn = false;
  Doc->setChanged(true, true);
}

// -----------------------------------------------------------
void MouseActions::MPressRotate(Schematic *Doc, QMouseEvent*, int x, int y)
{
  Element *e = Doc->selectElement(x, y, false);
  if(e == 0) return;
  e->Type &= isSpecialMask;  // remove special functions

  
  WireLabel *pl;
  int x1, y1, x2, y2;
//  e->isSelected = false;
  switch(e->Type) {
    case isComponent:
    case isAnalogComponent:
    case isDigitalComponent:
      if(((Component*)e)->Ports.count() < 1)
        break;  // do not rotate components without ports
      ((Component*)e)->rotate();
      Doc->setCompPorts((Component*)e);
      // enlarge viewarea if component lies outside the view
      ((Component*)e)->entireBounds(x1,y1,x2,y2, Doc->textCorr());
      Doc->enlargeView(x1, y1, x2, y2);
      break;

    case isWire:
      pl = ((Wire*)e)->Label;
      ((Wire*)e)->Label = 0;    // prevent label to be deleted
      Doc->Wires->setAutoDelete(false);
      Doc->deleteWire((Wire*)e);
      ((Wire*)e)->Label = pl;
      ((Wire*)e)->rotate();
      Doc->setOnGrid(e->x1, e->y1);
      Doc->setOnGrid(e->x2, e->y2);
      if(pl)  Doc->setOnGrid(pl->cx, pl->cy);
      Doc->insertWire((Wire*)e);
      Doc->Wires->setAutoDelete(true);
      if (Doc->Wires->containsRef ((Wire*)e))
        Doc->enlargeView(e->x1, e->y1, e->x2, e->y2);
      break;

    case isPainting:
      ((Painting*)e)->rotate();
      // enlarge viewarea if component lies outside the view
      ((Painting*)e)->Bounding(x1,y1,x2,y2);
      Doc->enlargeView(x1, y1, x2, y2);
      break;

    default:
      return;
  }
  Doc->viewport()->update();
  drawn = false;
  Doc->setChanged(true, true);
}

// -----------------------------------------------------------
void MouseActions::MPressElement(Schematic *Doc, QMouseEvent *Event, int, int)
{
  if(selElem == 0) return;
  QPainter painter(Doc->viewport());
  setPainter(Doc, &painter);

  
  int x1, y1, x2, y2;
  if(selElem->Type & isComponent) {
    Component *Comp = (Component*)selElem;
    switch(Event->button()) {
      case Qt::LeftButton :
	// left mouse button inserts component into the schematic
	Comp->textSize(x1, y1);
	Doc->insertComponent(Comp);
	Comp->textSize(x2, y2);
	if(Comp->tx < Comp->x1) Comp->tx -= x2 - x1;

	// enlarge viewarea if component lies outside the view
	Comp->entireBounds(x1,y1,x2,y2, Doc->textCorr());
	Doc->enlargeView(x1, y1, x2, y2);

	drawn = false;
	Doc->viewport()->update();
	Doc->setChanged(true, true);
	Comp = Comp->newOne(); // component is used, so create a new one
	break;

      case Qt::RightButton :  // right mouse button rotates the component
	if(Comp->Ports.count() == 0)
	  break;  // do not rotate components without ports
	Comp->paintScheme(&painter); // erase old component scheme
	Comp->rotate();
	Comp->paintScheme(&painter); // paint new component scheme
	break;

      default: ;   // avoids compiler warnings
    }
    selElem = Comp;
    return;

  }  // of "if(isComponent)"
  else if(selElem->Type == isDiagram) {
    if(Event->button() != Qt::LeftButton) return;

    Diagram *Diag = (Diagram*)selElem;
    QFileInfo Info(Doc->DocName);
    // dialog is Qt::WDestructiveClose !!!
    DiagramDialog *dia =
       new DiagramDialog(Diag,
           Info.dirPath() + QDir::separator() + Doc->DataSet, Doc);
    if(dia->exec() == QDialog::Rejected) {  // don't insert if dialog canceled
      Doc->viewport()->update();
      drawn = false;
      return;
    }

    Doc->Diagrams->append(Diag);
    Doc->enlargeView(Diag->cx, Diag->cy-Diag->y2, Diag->cx+Diag->x2, Diag->cy);
    Doc->setChanged(true, true);   // document has been changed

    Doc->viewport()->repaint();
    Diag = Diag->newOne(); // the component is used, so create a new one
    Diag->paintScheme(&painter);
    selElem = Diag;
    return;
  }  // of "if(isDiagram)"


  // ***********  it is a painting !!!
  if(((Painting*)selElem)->MousePressing()) {
    Doc->Paintings->append((Painting*)selElem);
    ((Painting*)selElem)->Bounding(x1,y1,x2,y2);
    Doc->enlargeView(x1, y1, x2, y2);
    selElem = ((Painting*)selElem)->newOne();

    drawn = false;
    Doc->viewport()->update();
    Doc->setChanged(true, true);
  }
}

// -----------------------------------------------------------
// Is called if starting point of wire is pressed
void MouseActions::MPressWire1(Schematic *Doc, QMouseEvent*, int x, int y)
{
  QPainter painter(Doc->viewport());
  setPainter(Doc, &painter);

  if(drawn) {
    painter.drawLine(MAx1, MAy3, MAx2, MAy3); // erase old mouse cross
    painter.drawLine(MAx3, MAy1, MAx3, MAy2);
  }
  drawn = false;

  MAx1 = 0;   // paint wire corner first up, then left/right
  MAx3 = x;
  MAy3 = y;
  Doc->setOnGrid(MAx3, MAy3);

  QucsMain->MouseMoveAction = &MouseActions::MMoveWire2;
  QucsMain->MousePressAction = &MouseActions::MPressWire2;
  // Double-click action is set in "MMoveWire2" to not initiate it
  // during "Wire1" actions.
}

// -----------------------------------------------------------
// Is called if ending point of wire is pressed
void MouseActions::MPressWire2(Schematic *Doc, QMouseEvent *Event, int x, int y)
{
  QPainter painter(Doc->viewport());
  setPainter(Doc, &painter);

  int set = 0;
  switch(Event->button()) {
  case Qt::LeftButton :
    if(MAx1 == 0) { // which wire direction first ?
      if(MAx2 != MAx3) {
	set = Doc->insertWire(new Wire(MAx3, MAy2, MAx2, MAy2));
	if(set & 2) {
	  // if last port is connected, then start a new wire
	  QucsMain->MouseMoveAction = &MouseActions::MMoveWire1;
	  QucsMain->MousePressAction = &MouseActions::MPressWire1;
	  QucsMain->MouseDoubleClickAction = 0;
	}
	if(MAy2 != MAy3)
	  set |= Doc->insertWire(new Wire(MAx3, MAy3, MAx3, MAy2));
      }
      else if(MAy2 != MAy3) {
	set = Doc->insertWire(new Wire(MAx3, MAy3, MAx3, MAy2));
	if(set & 2) {
	  // if last port is connected, then start a new wire
	  QucsMain->MouseMoveAction = &MouseActions::MMoveWire1;
	  QucsMain->MousePressAction = &MouseActions::MPressWire1;
	  QucsMain->MouseDoubleClickAction = 0;
	} }
    }
    else {
      if(MAy2 != MAy3) {
	set = Doc->insertWire(new Wire(MAx2, MAy3, MAx2, MAy2));
	if(set & 2) {
	  // if last port is connected, then start a new wire
	  QucsMain->MouseMoveAction = &MouseActions::MMoveWire1;
	  QucsMain->MousePressAction = &MouseActions::MPressWire1;
	  QucsMain->MouseDoubleClickAction = 0;
	}
	if(MAx2 != MAx3)
	  set |= Doc->insertWire(new Wire(MAx3, MAy3, MAx2, MAy3));
      }
      else if(MAx2 != MAx3) {
	set = Doc->insertWire(new Wire(MAx3, MAy3, MAx2, MAy3));
	if(set & 2) {
	  // if last port is connected, then start a new wire
	  QucsMain->MouseMoveAction = &MouseActions::MMoveWire1;
	  QucsMain->MousePressAction = &MouseActions::MPressWire1;
	  QucsMain->MouseDoubleClickAction = 0;
	} }
    }
    Doc->viewport()->update();
    drawn = false;
    if(set) Doc->setChanged(true, true);
    MAx3 = MAx2;
    MAy3 = MAy2;
    break;

  case Qt::RightButton :  // right mouse button changes the wire corner
    if(MAx1 == 0) {
      painter.drawLine(MAx3, MAy3, MAx3, MAy2); // erase old
      painter.drawLine(MAx3, MAy2, MAx2, MAy2); // erase old
    }
    else {
      painter.drawLine(MAx3, MAy3, MAx2, MAy3); // erase old
      painter.drawLine(MAx2, MAy3, MAx2, MAy2); // erase old
    }

    MAx2  = x;
    MAy2  = y;
    Doc->setOnGrid(MAx2, MAy2);

    MAx1 ^= 1;    // change the painting direction of wire corner
    if(MAx1 == 0) {
      painter.drawLine(MAx3, MAy3, MAx3, MAy2); // paint
      painter.drawLine(MAx3, MAy2, MAx2, MAy2); // paint
    }
    else {
      painter.drawLine(MAx3, MAy3, MAx2, MAy3); // paint
      painter.drawLine(MAx2, MAy3, MAx2, MAy2); // paint
    }
    break;

  default: ;    // avoids compiler warnings
  }
}

// -----------------------------------------------------------
// Is called for setting a marker on a diagram's graph
void MouseActions::MPressMarker(Schematic *Doc, QMouseEvent*, int x, int y)
{
  MAx1 = x;
  MAy1 = y;
  Marker *pm = Doc->setMarker(MAx1, MAy1);

  if(pm) {
    int x0 = pm->Diag->cx;
    int y0 = pm->Diag->cy;
    Doc->enlargeView(x0+pm->x1, y0-pm->y1-pm->y2, x0+pm->x1+pm->x2, y0-pm->y1);
  }
  Doc->viewport()->update();
  drawn = false;
}

// -----------------------------------------------------------
void MouseActions::MPressOnGrid(Schematic *Doc, QMouseEvent*, int x, int y)
{
  Element *pe = Doc->selectElement(x, y, false);
  if(pe) {
    pe->Type &= isSpecialMask;  // remove special functions (4 lowest bits)

    // onGrid is toggle action -> no other element can be selected
    pe->isSelected = true;
    Doc->elementsOnGrid();

    Doc->sizeOfAll(Doc->UsedX1, Doc->UsedY1, Doc->UsedX2, Doc->UsedY2);
    Doc->viewport()->update();
    drawn = false;
  }
}

// -----------------------------------------------------------
void MouseActions::MPressMoveText(Schematic *Doc, QMouseEvent*, int x, int y)
{
  MAx1 = x;
  MAy1 = y;
  focusElement = Doc->selectCompText(MAx1, MAy1, MAx2, MAy2);

  if(focusElement) {
    MAx3 = MAx1;
    MAy3 = MAy1;
    MAx1 = ((Component*)focusElement)->cx + ((Component*)focusElement)->tx;
    MAy1 = ((Component*)focusElement)->cy + ((Component*)focusElement)->ty;
    Doc->viewport()->update();
    drawn = false;
    QucsMain->MouseMoveAction = &MouseActions::MMoveMoveText;
    QucsMain->MouseReleaseAction = &MouseActions::MReleaseMoveText;
    Doc->grabKeyboard();  // no keyboard inputs during move actions
  }
}

// -----------------------------------------------------------
void MouseActions::MPressZoomIn(Schematic *Doc, QMouseEvent*, int x, int y)
{
  MAx1 = x;
  MAy1 = y;
  MAx2 = 0;  // rectangle size
  MAy2 = 0;

  QucsMain->MouseMoveAction = &MouseActions::MMoveSelect;
  QucsMain->MouseReleaseAction = &MouseActions::MReleaseZoomIn;
  Doc->grabKeyboard();  // no keyboard inputs during move actions
  Doc->viewport()->update();
  drawn = false;
}


// ***********************************************************************
// **********                                                   **********
// **********    Functions for serving mouse button releasing   **********
// **********                                                   **********
// ***********************************************************************
void MouseActions::MReleaseSelect(Schematic *Doc, QMouseEvent *Event)
{
  bool ctrl;
  if(Event->state() & Qt::ControlButton) ctrl = true;
  else ctrl = false;

  if(!ctrl) Doc->deselectElements(focusElement);

  if(focusElement)  if(Event->button() == Qt::LeftButton)
    if(focusElement->Type == isWire) {
      Doc->selectWireLine(focusElement, ((Wire*)focusElement)->Port1, ctrl);
      Doc->selectWireLine(focusElement, ((Wire*)focusElement)->Port2, ctrl);
    }

  Doc->releaseKeyboard();  // allow keyboard inputs again
  QucsMain->MousePressAction = &MouseActions::MPressSelect;
  QucsMain->MouseReleaseAction = &MouseActions::MReleaseSelect;
  QucsMain->MouseDoubleClickAction = &MouseActions::MDoubleClickSelect;
  QucsMain->MouseMoveAction = 0;   // no element moving
  Doc->viewport()->update();
  drawn = false;
}

// -----------------------------------------------------------
// Is called after the rectangle for selection is released.
void MouseActions::MReleaseSelect2(Schematic *Doc, QMouseEvent *Event)
{
  if(Event->button() != Qt::LeftButton) return;

  bool Ctrl;
  if(Event->state() & Qt::ControlButton) Ctrl = true;
  else Ctrl = false;

  // selects all elements within the rectangle
  Doc->selectElements(MAx1, MAy1, MAx1+MAx2, MAy1+MAy2, Ctrl);

  Doc->releaseKeyboard();  // allow keyboard inputs again
  QucsMain->MouseMoveAction = 0;
  QucsMain->MousePressAction = &MouseActions::MPressSelect;
  QucsMain->MouseReleaseAction = &MouseActions::MReleaseSelect;
  QucsMain->MouseDoubleClickAction = &MouseActions::MDoubleClickSelect;
  Doc->viewport()->update();
  drawn = false;
}

// -----------------------------------------------------------
void MouseActions::MReleaseActivate(Schematic *Doc, QMouseEvent *Event)
{
  if(Event->button() != Qt::LeftButton) return;

  // activates all components within the rectangle
  Doc->activateCompsWithinRect(MAx1, MAy1, MAx1+MAx2, MAy1+MAy2);

  QucsMain->MouseMoveAction = &MouseActions::MMoveActivate;
  QucsMain->MousePressAction = &MouseActions::MPressActivate;
  QucsMain->MouseReleaseAction = 0;
  QucsMain->MouseDoubleClickAction = 0;
  Doc->viewport()->update();
  drawn = false;
}

// -----------------------------------------------------------
// Is called after moving selected elements.
void MouseActions::MReleaseMoving(Schematic *Doc, QMouseEvent *Event)
{
  if(Event->button() != Qt::LeftButton) return;
  endElementMoving(Doc, &movingElements);
  Doc->releaseKeyboard();  // allow keyboard inputs again

  QucsMain->MouseMoveAction = 0;
  QucsMain->MousePressAction = &MouseActions::MPressSelect;
  QucsMain->MouseReleaseAction = &MouseActions::MReleaseSelect;
  QucsMain->MouseDoubleClickAction = &MouseActions::MDoubleClickSelect;
}

// -----------------------------------------------------------
void MouseActions::MReleaseResizeDiagram(Schematic *Doc, QMouseEvent *Event)
{
  if(Event->button() != Qt::LeftButton) return;

  MAx3  = focusElement->cx;
  MAy3  = focusElement->cy;
  if(MAx2 < 0) {    // resize diagram
    if(MAx2 > -10) MAx2 = -10;   // not smaller than 10 pixels
    focusElement->x2 = -MAx2;
    focusElement->cx = MAx1+MAx2;
  }
  else {
    if(MAx2 < 10) MAx2 = 10;
    focusElement->x2 = MAx2;
    focusElement->cx = MAx1;
  }
  if(MAy2 < 0) {
    if(MAy2 > -10) MAy2 = -10;
    focusElement->y2 = -MAy2;
    focusElement->cy = MAy1;
  }
  else {
    if(MAy2 < 10) MAy2 = 10;
    focusElement->y2 = MAy2;
    focusElement->cy = MAy1+MAy2;
  }
  MAx3 -= focusElement->cx;
  MAy3 -= focusElement->cy;

  Diagram *pd = (Diagram*)focusElement;
  pd->updateGraphData();
  for(Graph *pg = pd->Graphs.first(); pg != 0; pg = pd->Graphs.next())
    for(Marker *pm = pg->Markers.first(); pm!=0; pm = pg->Markers.next()) {
      pm->x1 += MAx3;      // correct changes due to move of diagram corner
      pm->y1 += MAy3;
    }

  int x1, x2, y1, y2;
  pd->Bounding(x1, x2, y1, y2);
  Doc->enlargeView(x1, x2, y1, y2);

  QucsMain->MouseMoveAction = 0;
  QucsMain->MousePressAction = &MouseActions::MPressSelect;
  QucsMain->MouseReleaseAction = &MouseActions::MReleaseSelect;
  QucsMain->MouseDoubleClickAction = &MouseActions::MDoubleClickSelect;
  Doc->releaseKeyboard();  // allow keyboard inputs again

  Doc->viewport()->update();
  drawn = false;
  Doc->setChanged(true, true);
}

// -----------------------------------------------------------
void MouseActions::MReleaseResizePainting(Schematic *Doc, QMouseEvent *Event)
{
  if(Event->button() != Qt::LeftButton) return;

  QucsMain->MouseMoveAction = 0;
  QucsMain->MousePressAction = &MouseActions::MPressSelect;
  QucsMain->MouseReleaseAction = &MouseActions::MReleaseSelect;
  QucsMain->MouseDoubleClickAction = &MouseActions::MDoubleClickSelect;
  Doc->releaseKeyboard();  // allow keyboard inputs again

  Doc->viewport()->update();
  drawn = false;
  Doc->setChanged(true, true);
}

// -----------------------------------------------------------
void MouseActions::MReleasePaste(Schematic *Doc, QMouseEvent *Event)
{
  int x1, y1, x2, y2;
  QFileInfo Info(Doc->DocName);
  QPainter painter(Doc->viewport());

  Element *pe;
  switch(Event->button()) {
  case Qt::LeftButton :
    // insert all moved elements into document
    for(pe = movingElements.first(); pe!=0; pe = movingElements.next()) {
      pe->isSelected = false;
      switch(pe->Type) {
	case isWire:
	  if(pe->x1 == pe->x2) if(pe->y1 == pe->y2)  break;
	  Doc->insertWire((Wire*)pe);
	  if (Doc->Wires->containsRef ((Wire*)pe))
	    Doc->enlargeView(pe->x1, pe->y1, pe->x2, pe->y2);
	  else pe = NULL;
	  break;
	case isDiagram:
	  Doc->Diagrams->append((Diagram*)pe);
	  ((Diagram*)pe)->loadGraphData(Info.dirPath() + QDir::separator() + 
					Doc->DataSet);
	  Doc->enlargeView(pe->cx, pe->cy-pe->y2, pe->cx+pe->x2, pe->cy);
	  break;
	case isPainting:
	  Doc->Paintings->append((Painting*)pe);
	  ((Painting*)pe)->Bounding(x1,y1,x2,y2);
	  Doc->enlargeView(x1, y1, x2, y2);
	  break;
	case isMovingLabel:
	  pe->Type = isNodeLabel;
	  Doc->placeNodeLabel((WireLabel*)pe);
	  break;
	case isComponent:
	case isAnalogComponent:
	case isDigitalComponent:
	  Doc->insertComponent((Component*)pe);
	  ((Component*)pe)->entireBounds(x1,y1,x2,y2, Doc->textCorr());
	  Doc->enlargeView(x1, y1, x2, y2);
	  break;
      }
    }

    pasteElements(Doc);
    QucsMain->MouseMoveAction = &MouseActions::MMovePaste;
    QucsMain->MousePressAction = 0;
    QucsMain->MouseReleaseAction = 0;
    QucsMain->MouseDoubleClickAction = 0;

    drawn = false;
    Doc->viewport()->update();
    Doc->setChanged(true, true);
    break;

  // ............................................................
  case Qt::RightButton :  // right button rotates the elements
    setPainter(Doc, &painter);

    if(drawn) // erase old scheme
      for(pe = movingElements.first(); pe != 0; pe = movingElements.next())
        pe->paintScheme(&painter);
    drawn = true;

    x1  = int(float(Event->pos().x())/Doc->Scale) + Doc->ViewX1;
    y1  = int(float(Event->pos().y())/Doc->Scale) + Doc->ViewY1;
    Doc->setOnGrid(x1, y1);

    for(pe = movingElements.first(); pe != 0; pe = movingElements.next()) {
      switch(pe->Type) {
        case isComponent:
        case isAnalogComponent:
        case isDigitalComponent:
	  ((Component*)pe)->rotate(); // rotate !before! rotating the center
          x2 = x1 - pe->cx;
          pe->setCenter(pe->cy - y1 + x1, x2 + y1);
          break;
        case isWire:
	  x2    = pe->x1;
          pe->x1 = pe->y1 - y1 + x1;
          pe->y1 = x1 - x2 + y1;
          x2    = pe->x2;
          pe->x2 = pe->y2 - y1 + x1;
          pe->y2 = x1 - x2 + y1;
          break;
        case isPainting:
	  ((Painting*)pe)->rotate(); // rotate !before! rotating the center
          ((Painting*)pe)->getCenter(x2, y2);
          pe->setCenter(y2 - y1 + x1, x1 - x2 + y1);
          break;
        default:
	  x2 = x1 - pe->cx;   // if diagram -> only rotate cx/cy
          pe->setCenter(pe->cy - y1 + x1, x2 + y1);
          break;
      }
      pe->paintScheme(&painter);
    }
    break;

  default: ;    // avoids compiler warnings
  }
}

// -----------------------------------------------------------
void MouseActions::MReleaseMoveText(Schematic *Doc, QMouseEvent *Event)
{
  if(Event->button() != Qt::LeftButton) return;

  QucsMain->MouseMoveAction = &MouseActions::MMoveMoveTextB;
  QucsMain->MouseReleaseAction = 0;
  Doc->releaseKeyboard();  // allow keyboard inputs again

  ((Component*)focusElement)->tx = MAx1 - ((Component*)focusElement)->cx;
  ((Component*)focusElement)->ty = MAy1 - ((Component*)focusElement)->cy;
  Doc->viewport()->update();
  drawn = false;
  Doc->setChanged(true, true);
}

// -----------------------------------------------------------
void MouseActions::MReleaseZoomIn(Schematic *Doc, QMouseEvent *Event)
{
  if(Event->button() != Qt::LeftButton) return;

  MAx1 = Event->pos().x();
  MAy1 = Event->pos().y();
  float DX = float(abs(MAx2));
  float DY = float(abs(MAy2));
  if((Doc->Scale * DX) < 6.0) {
    MAx1  = (MAx1<<1) - (Doc->visibleWidth()>>1)  - Doc->contentsX();
    MAy1  = (MAy1<<1) - (Doc->visibleHeight()>>1) - Doc->contentsY();
    Doc->zoom(1.5);    // a simple click zooms by constant factor
  }
  else {
    float xScale = float(Doc->visibleWidth())  / DX;
    float yScale = float(Doc->visibleHeight()) / DY;
    if(xScale > yScale) xScale = yScale;
    yScale  = Doc->Scale;
    xScale /= yScale;
    Doc->zoom(xScale);    

    if(MAx2 > 0)  MAx1 -= int(float(MAx2)*yScale);
    if(MAy2 > 0)  MAy1 -= int(float(MAy2)*yScale);
    MAx1 = int(float(MAx1) * xScale) - Doc->contentsX();
    MAy1 = int(float(MAy1) * xScale) - Doc->contentsY();
  }
  Doc->scrollBy(MAx1, MAy1);

  QucsMain->MouseMoveAction = &MouseActions::MMoveZoomIn;
  QucsMain->MouseReleaseAction = 0;
  Doc->releaseKeyboard();  // allow keyboard inputs again
}


// ***********************************************************************
// **********                                                   **********
// **********    Functions for mouse button double clicking     **********
// **********                                                   **********
// ***********************************************************************
void MouseActions::editElement(Schematic *Doc, QMouseEvent *Event)
{
  if(focusElement == 0) return;

  Graph *pg;
  Component *c;
  Diagram *dia;
  DiagramDialog *ddia;
  MarkerDialog *mdia;
  int x1, y1, x2, y2;

  QFileInfo Info(Doc->DocName);
  int x = int(Event->pos().x()/Doc->Scale) + Doc->ViewX1;
  int y = int(Event->pos().y()/Doc->Scale) + Doc->ViewY1;

  switch(focusElement->Type) {
    case isComponent:
    case isAnalogComponent:
    case isDigitalComponent:
         c = (Component*)focusElement;
         if(c->Model == "GND") return;

         if(c->Model == "SPICE") {
           SpiceDialog *sd = new SpiceDialog((SpiceFile*)c, Doc);
           if(sd->exec() != 1) break;   // dialog is WDestructiveClose
         }
         else if(c->Model == ".Opt") {
           OptimizeDialog *od = new OptimizeDialog((Optimize_Sim*)c, Doc);
           if(od->exec() != 1) break;   // dialog is WDestructiveClose
         }
         else {
           ComponentDialog * cd = new ComponentDialog(c, Doc);
           if(cd->exec() != 1) break;   // dialog is WDestructiveClose

           Doc->Components->findRef(c);
           Doc->Components->take();
           Doc->setComponentNumber(c); // for ports/power sources
           Doc->Components->append(c);
         }

         Doc->setChanged(true, true);
         c->entireBounds(x1,y1,x2,y2, Doc->textCorr());
         Doc->enlargeView(x1,y1,x2,y2);
         break;

    case isDiagram :
         dia = (Diagram*)focusElement;
         if(dia->Name.at(0) == 'T')  // don't open dialog on scrollbar
           if(dia->Name == "Time") {
             if(dia->cy < y) {
	       if(((TimingDiagram*)focusElement)->scroll(MAx1))
	         Doc->setChanged(true, true, 'm'); // 'm' = only the first time
	       break;
             }
	   }
           else {
             if(dia->cx > x) {
	       if(((TabDiagram*)focusElement)->scroll(MAy1))
	         Doc->setChanged(true, true, 'm'); // 'm' = only the first time
	       break;
             }
	   }

	 ddia = new DiagramDialog(dia,
		Info.dirPath() + QDir::separator() + Doc->DataSet, Doc);
         if(ddia->exec() != QDialog::Rejected)   // is WDestructiveClose
           Doc->setChanged(true, true);

	 dia->Bounding(x1, x2, y1, y2);
	 Doc->enlargeView(x1, x2, y1, y2);
	 break;

    case isGraph :
	 pg = (Graph*)focusElement;
	 // searching diagram for this graph
	 for(dia = Doc->Diagrams->last(); dia != 0; dia = Doc->Diagrams->prev())
	   if(dia->Graphs.findRef(pg) >= 0)
	     break;
	 if(!dia) break;

	 
	 ddia = new DiagramDialog(dia,
	 	Info.dirPath() + QDir::separator() + Doc->DataSet, Doc, pg);
	 if(ddia->exec() != QDialog::Rejected)   // is WDestructiveClose
	   Doc->setChanged(true, true);
         break;

    case isWire:
         MPressLabel(Doc, Event, x, y);
         break;

    case isNodeLabel:
    case isHWireLabel:
    case isVWireLabel:
         editLabel(Doc, (WireLabel*)focusElement);
         break;

    case isPainting:
         if( ((Painting*)focusElement)->Dialog() )
           Doc->setChanged(true, true);
         break;

    case isMarker:
         mdia = new MarkerDialog((Marker*)focusElement, Doc);
         if(mdia->exec() > 1)
           Doc->setChanged(true, true);
         break;
  }

  // Very strange: Now an open VHDL editor gets all the keyboard input !?!
  // I don't know why it only happens here, nor am I sure whether it only
  // happens here. Anyway, I hope the best and give the focus back to the
  // current document.
  Doc->setFocus();

  Doc->viewport()->update();
  drawn = false;
}

// -----------------------------------------------------------
void MouseActions::MDoubleClickSelect(Schematic *Doc, QMouseEvent *Event)
{
  Doc->releaseKeyboard();  // allow keyboard inputs again
  QucsMain->editText->setHidden(true);
  editElement(Doc, Event);
}

// -----------------------------------------------------------
void MouseActions::MDoubleClickWire2(Schematic *Doc, QMouseEvent *Event)
{
  int x = int(Event->pos().x()/Doc->Scale) + Doc->ViewX1;
  int y = int(Event->pos().y()/Doc->Scale) + Doc->ViewY1;

  MPressWire2(Doc, Event, x, y);

  QucsMain->MouseMoveAction = &MouseActions::MMoveWire1;
  QucsMain->MousePressAction = &MouseActions::MPressWire1;
  QucsMain->MouseDoubleClickAction = 0;
}
