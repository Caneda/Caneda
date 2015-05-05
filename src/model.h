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

#ifndef MODEL_H
#define MODEL_H

#include <QMap>
#include <QString>

namespace Caneda
{
    //Forward declarations
    class XmlWriter;
    class XmlReader;

    //! \def ModelsMap This is a typedef to map models with strings.
    typedef QMap<QString, QString> ModelsMap;

    /*!
     * \brief This class represents the models of a component.
     *
     * Models are the representation of a component in different scenarios.
     * For example, a component can have certain syntax to be used in a spice
     * circuit, and a different one in a kicad schematic. Having a way to
     * extract information from our schematic and interpret it in different
     * ways allow us to export the circuit to other softwares and simulator
     * engines.
     *
     * Models should be always strings. Gouping several models into a QMap
     * (m_modelsMap) provides a convenient way of handling them all together,
     * and filter them according to the export operation being used.
     *
     * \sa ModelsMap
     */
    class Models
    {
    public:
        Models(const ModelsMap& _modelsMap = ModelsMap());

        //! Returns selected model from model map.
        QString model(const QString& type) const { return m_modelsMap[type]; }
        void setModel(const QString& type, const QString& model);

        //! Returns the model map (actually a copy of model map).
        ModelsMap modelsMap() const { return m_modelsMap; }
        void setModelsMap(const ModelsMap& modMap);

        void writeData(Caneda::XmlWriter *writer);
        void readData(Caneda::XmlReader *reader);

    private:
        //! QMap holding actual models.
        ModelsMap m_modelsMap;
    };

} // namespace Caneda

#endif //MODEL_H
