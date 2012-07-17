/***************************************************************************
 * Copyright (C) 2009-2012 by Pablo Daniel Pareja Obregon                  *
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

#include "xmlsymbol.h"

#include "cgraphicsscene.h"
#include "component.h"
#include "global.h"
#include "symboldocument.h"

#include "paintings/painting.h"

#include "xmlutilities/xmlutilities.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>

namespace Caneda
{
    //! Constructor
    XmlSymbol::XmlSymbol(SymbolDocument *doc) :
        m_symbolDocument(doc)
    {
    }

    bool XmlSymbol::save()
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!scene) {
            return false;
        }

        QString text = saveText();

        if(text.isEmpty()) {
            qDebug("Looks buggy! Null data to save! Was this expected?");
        }

        QFile file(fileName());
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot save document!"));
            return false;
        }
        QTextStream stream(&file);
        stream << text;
        file.close();

        return true;
    }

    bool XmlSymbol::load()
    {
        return false;
    }

    QString XmlSymbol::saveText()
    {
        QString retVal;
        Caneda::XmlWriter *writer = new Caneda::XmlWriter(&retVal);
        writer->setAutoFormatting(true);

        //Fist we start the document and write current version
        writer->writeStartDocument();
        writer->writeDTD(QString("<!DOCTYPE caneda>"));
        writer->writeStartElement("caneda");
        writer->writeAttribute("version", Caneda::version());

        //Write all view details
        writer->writeStartElement("component");
        QFileInfo info(fileName());
        writer->writeAttribute("name", info.baseName());
        writer->writeAttribute("label", "comp");

        writer->writeStartElement("displaytext");
        writer->writeLocaleText("C", "User created component");
        //   TODO: When available use this to save user defined displaytext
        //   writer->writeLocaleText("C", scene->displayText());
        writer->writeEndElement(); //</displaytext>

        writer->writeStartElement("description");
        writer->writeLocaleText("C", "User created component based on user symbol");
        //   TODO: When available use this to save user defined description
        //   writer->writeLocaleText("C", scene->description());
        writer->writeEndElement(); //</description>

        writer->writeStartElement("symbol");
        writer->writeAttribute("name", "userdefined");


        //Now we copy all the elements and properties in the schematic
        QList<QGraphicsItem*> items = cGraphicsScene()->items();
        QList<Painting*> paintings = filterItems<Painting>(items, RemoveItems);
        if(!paintings.isEmpty()) {
            foreach(Painting *p, paintings) {
                p->saveData(writer);
            }
        }
        //Finally we finish the document
        writer->writeEndElement(); //</symbol>

        //TODO Write properties
        writer->writeStartElement("properties");
        writer->writeEndElement(); //</properties>

        writer->writeEndDocument(); //</caneda>

        delete writer;
        return retVal;
    }

    SymbolDocument* XmlSymbol::symbolDocument() const
    {
        return m_symbolDocument;
    }

    CGraphicsScene* XmlSymbol::cGraphicsScene() const
    {
        return m_symbolDocument ? m_symbolDocument->cGraphicsScene() : 0;
    }

    QString XmlSymbol::fileName() const
    {
        return m_symbolDocument ? m_symbolDocument->fileName() : QString();
    }

} // namespace Caneda
