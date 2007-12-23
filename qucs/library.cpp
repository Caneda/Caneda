/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "library.h"
#include "xmlutilities.h"
#include "schematicscene.h"
#include "qucs-tools/global.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>

#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>

//! Constructs library item from reader with file path \a path and svgpainter \a painter.
LibraryItem::LibraryItem(Qucs::XmlReader *reader, QString path, SvgPainter *painter) :
   m_svgPainter(painter)
{
   m_valid = parseLibrary(reader, path);
}

//! Returns the shared data of component from given name.
ComponentDataPtr LibraryItem::componentDataPtr(const QString& name) const
{
   return !m_componentHash.contains(name) ?
      ComponentDataPtr() : m_componentHash[name];
}

//! Renders an svg to given painter given \a component and \a symbol.
void LibraryItem::render(QPainter *painter, QString component, QString symbol) const
{
   const ComponentDataPtr dataPtr = componentDataPtr(component);
   if(!dataPtr.constData()) {
      qWarning() << "LibraryItem::render() : " <<  component << " not found";
      return;
   }
   if(symbol.isEmpty() ||
      !dataPtr->propertyMap["symbol"].options().contains(symbol))
   {
      symbol = dataPtr->propertyMap["symbol"].value().toString();
   }
   QString svgId = dataPtr->name + '/' + symbol;
   m_svgPainter->paint(painter, svgId);
}

/*! Returns the component rendered to pixmap.
 *
 * \param component Component to be rendered.
 * \param symbol Symbol to be rendered. Empty string if default is to rendered.
 */
QPixmap LibraryItem::renderedPixmap(QString component,
                                     QString symbol) const
{
   const ComponentDataPtr dataPtr = componentDataPtr(component);
   if(!dataPtr.constData()) {
      qWarning() << "LibraryItem::renderToPixmap() : " <<  component << " not found";
      return QPixmap();
   }
   if(symbol.isEmpty() ||
      !dataPtr->propertyMap["symbol"].options().contains(symbol))
   {
      symbol = dataPtr->propertyMap["symbol"].value().toString();
   }
   QString svgId = dataPtr->name + '/' + symbol;
   QRectF rect = m_svgPainter->boundingRect(svgId);
   QPixmap pix;
   if (!QPixmapCache::find(svgId, pix)) {
      pix = QPixmap(rect.toRect().size());
      pix.fill(Qt::transparent);
      QPainter painter(&pix);
      painter.setWindow(rect.toRect());
      m_svgPainter->paint(&painter, svgId);
   }
   return pix;
}

bool LibraryItem::parseLibrary(Qucs::XmlReader *reader, QString filePath)
{
   Q_ASSERT(reader->isStartElement() && reader->name() == "library");
   SchematicScene *dummyScene = new SchematicScene(0,0,500,500);
   dummyScene->setSvgPainter(m_svgPainter);
   QDir fileDir(filePath);

   m_libraryName = reader->attributes().value("name").toString();
   if(m_libraryName.isEmpty()) {
      reader->raiseError("Invalid or no 'name' attribute in library tag");
      return false;
   }

   while(!reader->atEnd()) {
      reader->readNext();

      if(reader->isEndElement())
         break;

      if(reader->isStartElement()) {
         if(reader->name() == "displaytext") {
            m_displayText = reader->readLocaleText(Qucs::localePrefix());
            Q_ASSERT(reader->isEndElement() && reader->name() == "displaytext");
         }
         else if(reader->name() == "description") {
            m_description = reader->readLocaleText(Qucs::localePrefix());
            Q_ASSERT(reader->isEndElement() && reader->name() == "description");
         }
         else if(reader->name() == "component") {
            QString externalPath = reader->attributes().value("href").toString();
            if(externalPath.isEmpty()) {
               registerComponentData(reader, filePath, dummyScene);
            }
            else {
               QString absFilePath = fileDir.absoluteFilePath(externalPath);
               bool status = parseExternalComponent(absFilePath, dummyScene);
               if(!status) {
                  QString errorString("Parsing external component data file %1"
                                      "failed");
                  errorString = errorString.arg(absFilePath);
                  reader->raiseError(errorString);
               } else {
                  //ignore rest of component' tag as the main data is only external
                  reader->readUnknownElement();
               }
            }
         }
         else {
            reader->readUnknownElement();
         }
      }
   }
   delete dummyScene;
   return true;
}

bool LibraryItem::parseExternalComponent(QString path, SchematicScene *scene)
{
   QFile file(path);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QMessageBox::warning(0, QObject::tr("File open"),
                           QObject::tr("Cannot open file %1").arg(path));
      return false;
   }
   QTextStream in(&file);
   in.setCodec("UTF-8");
   QString data = in.readAll();

   Qucs::XmlReader reader(data);
   while(!reader.atEnd()) {
      reader.readNext();

      if(reader.isStartElement() && reader.name() == "component")
         break;
   }
   if(reader.isStartElement() && reader.name() == "component") {
      QFileInfo info(path);
      QString parentPath = info.dir().absolutePath();
      registerComponentData(&reader, parentPath, scene);
   }
   return !reader.hasError();
}

//! Registers svg as well as the component's shared data.
void LibraryItem::registerComponentData(Qucs::XmlReader *reader, QString path, SchematicScene *scene)
{
   //Automatically registers svgs on success
   Qucs::Component *dummy = new Qucs::Component(reader, path, scene);
   if(!reader->hasError()) {
      Q_ASSERT(!m_componentHash.contains(dummy->name()));
      m_componentHash.insert(dummy->name(), dummy->dataPtr());
   }
   delete dummy;
}

Library::Library()
{
   m_svgPainter = SvgPainter::defaultSvgPainter();
}

Library::~Library()
{
   qDebug() << "Library::~Library()";
}

/*!
 * Returns default instance of library.
 */
Library* Library::defaultInstance()
{
   static Library *library = new Library();
   return library;
}

//! Returns library item corresponding to name.
const LibraryItem* Library::libraryItem(const QString& str) const
{
   if(!m_libraryHash.contains(str)) {
      qWarning() << "Library::libraryItem : Library item " << str << " not found";
      return 0;
   }
   return m_libraryHash[str];
}

/*!
 * \brief Constructs a new component given its name and library.
 *
 * \param componentName The component's name.
 * \param scene The scene on which component is to be rendered.
 * \param library The library to which the \a componentName belongs. If this is empty,
 *  it searches for all libraries for component and returns first match.
 * \return Component on success and null pointer on failure.
 */
Qucs::Component* Library::newComponent(QString componentName, SchematicScene *scene,
                                       QString library)
{
   ComponentDataPtr data;
   if(library.isEmpty()) {
      LibraryHash::const_iterator it = m_libraryHash.constBegin(),
         end = m_libraryHash.constEnd();
      while(it != end) {
         data = it.value()->componentDataPtr(componentName);
         if(data.constData())
            break;
         ++it;
      }
   }
   else {
      if(m_libraryHash.contains(library)) {
         data = m_libraryHash[library]->componentDataPtr(componentName);
      }
   }
   if(data.constData()) {
      Qucs::Component* comp = new Qucs::Component(data, scene);
      comp->setSymbol(comp->symbol());
      return comp;
   }
   return 0;
}

//! Load's library indicated by path \a libPath.
bool Library::loadLibrary(const QString& libPath)
{
   QFile file(libPath);
   if(!file.open(QIODevice::ReadOnly)) {
      QMessageBox::warning(0, QObject::tr("File open"),
                           QObject::tr("Cannot open file %1").arg(libPath));
      return false;
   }
   QString libParentPath = QFileInfo(libPath).dir().absolutePath();

   QTextStream in(&file);
   in.setCodec("UTF-8");
   QString data = in.readAll();

   Qucs::XmlReader reader(data);
   while(!reader.atEnd()) {
      reader.readNext();

      if(reader.isEndElement())
         break;

      if(reader.isStartElement()) {
         if(reader.name() == "library") {
            LibraryItem *info = new LibraryItem(&reader, libParentPath, m_svgPainter);
            if(reader.hasError()) {
               QMessageBox::critical(0, QObject::tr("Load library"),
                                     QObject::tr("Parsing library failed with following error"
                                                 "\"%1\"").arg(reader.errorString()));
               delete info;
               return false;
            }

            m_libraryHash.insert(info->libraryName(), info);
         }
         else {
            reader.readUnknownElement();
         }
      }
   }
   return !reader.hasError();
}
