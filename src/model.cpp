/***************************************************************************
 * Copyright (C) 2015 by Pablo Daniel Pareja Obregon                       *
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

#include "model.h"

#include "xmlutilities.h"

#include <QDebug>

namespace Caneda
{
    /*!
     * \brief Constructs a Models class from a given scene and ModelsMap.
     *
     * \param propMap The ModelsMap to use on initialization.
     */
    Models::Models(const ModelsMap &_modelsMap)
    {
        m_modelsMap = _modelsMap;
    }

    //! \brief Sets model \a type to \a model value in the ModelsMap.
    void Models::setModel(const QString& type, const QString& model)
    {
        m_modelsMap[type] = model;
    }

    /*!
     * \brief Set all the models values through a ModelsMap.
     *
     * This method sets the models values by updating the m_modelsMap
     * to \a modMap.
     *
     * \param modMap The new models map to be set.
     *
     * \sa ModelsMap
     */
    void Models::setModelsMap(const ModelsMap& modMap)
    {
        m_modelsMap = modMap;
    }

    //! \brief Helper method to write all models in \a m_modelsMap to xml.
    void Models::writeData(Caneda::XmlWriter *writer)
    {
        writer->writeStartElement("models");

        foreach(const QString model, m_modelsMap) {
            writer->writeEmptyElement("property");
            writer->writeAttribute("type", m_modelsMap.key(model));
            writer->writeAttribute("syntax", model);
        }

        writer->writeEndElement(); // </models>
    }

    //! \brief Helper method to read xml saved models into \a m_modelsMap.
    void Models::readData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "models");

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "model") {
                    QXmlStreamAttributes attribs(reader->attributes());
                    QString modelType = attribs.value("type").toString();
                    QString modelSyntax = attribs.value("syntax").toString();

                    setModel(modelType, modelSyntax);

                    // Read till end element
                    reader->readUnknownElement();
                }
                else {
                    reader->readUnknownElement();
                }
            }
        }

    }

} // namespace Caneda
