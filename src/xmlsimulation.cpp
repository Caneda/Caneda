/***************************************************************************
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
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

#include "xmlsimulation.h"

#include "simulationdocument.h"
#include "simulationscene.h"
#include "xmlutilities.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>

namespace Caneda
{
    //! Constructor
    XmlSimulation::XmlSimulation(SimulationDocument *doc) :
        m_simulationDocument(doc)
    {
    }

    bool XmlSimulation::save()
    {
        SimulationScene *scene = simulationScene();
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

    bool XmlSimulation::load()
    {
        SimulationScene *scene = simulationScene();
        if(!scene) {
            return false;
        }

        QFile file(fileName());
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot load document ")+fileName());
            return false;
        }

        QTextStream stream(&file);

        bool result = loadFromText(stream.readAll());

        file.close();
        return result;
    }

    QString XmlSimulation::saveText()
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
        QFileInfo info(fileName());
        writer->writeStartElement("simulation");
        writer->writeAttribute("name", info.baseName());
        writer->writeStartElement("data");
        writer->writeAttribute("filename", QString(info.baseName()+".raw"));

        //Now we copy all the elements and properties in the simulation (axis, waveforms, etc)
        // TODO: Implement this

        //Finally we finish the document
        writer->writeEndElement(); //</data>
        writer->writeEndDocument(); //</caneda>

        delete writer;
        return retVal;
    }

    bool XmlSimulation::loadFromText(const QString& text)
    {
        // TODO: Implement this
        // Read all the elements and properties in the simulation (axis, waveforms, etc),
        // along with the data from filename() + ".raw"
        return true;
    }

    SimulationDocument* XmlSimulation::simulationDocument() const
    {
        return m_simulationDocument;
    }

    SimulationScene* XmlSimulation::simulationScene() const
    {
        return m_simulationDocument ? m_simulationDocument->simulationScene() : 0;
    }

    QString XmlSimulation::fileName() const
    {
        return m_simulationDocument ? m_simulationDocument->fileName() : QString();
    }

} // namespace Caneda
