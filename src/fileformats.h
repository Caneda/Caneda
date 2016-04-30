/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2009-2016 by Pablo Daniel Pareja Obregon                  *
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

#ifndef FILE_FORMATS_H
#define FILE_FORMATS_H

#include "component.h"

// Forward declarations
class QString;

namespace Caneda
{
    // Forward declarations
    class GraphicsScene;
    class ChartSeries;
    class ChartScene;
    class LayoutDocument;
    class SchematicDocument;
    class SimulationDocument;
    class SymbolDocument;
    class XmlReader;
    class XmlWriter;

    typedef QList<QPair<Port *, QString> > PortsNetlist;

    /*!
     * \brief This class handles all the access to the schematic documents file
     * format.
     *
     * This class is in charge of saving and loading all schematic related
     * documents. This is the only class that knows about schematic document
     * formats, and has the access functions to return a SchematicDocument,
     * with all of its components.
     *
     * \sa \ref DocumentFormats
     */
    class FormatXmlSchematic
    {
    public:
        FormatXmlSchematic(SchematicDocument *doc = 0);

        bool save();
        bool load();

        SchematicDocument* schematicDocument() const;
        GraphicsScene* graphicsScene() const;

        QString fileName() const;

    private:
        QString saveText();
        void saveComponents(Caneda::XmlWriter *writer);
        void savePorts(Caneda::XmlWriter *writer);
        void saveWires(Caneda::XmlWriter *writer);
        void savePaintings(Caneda::XmlWriter *writer);

        bool loadFromText(const QString& text);
        void loadComponents(Caneda::XmlReader *reader);
        void loadPorts(Caneda::XmlReader *reader);
        void loadWires(Caneda::XmlReader *reader);
        void loadPaintings(Caneda::XmlReader *reader);

        SchematicDocument *m_schematicDocument;
    };

    /*!
     * \brief This class handles all the access to the symbol documents file
     * format.
     *
     * This class is in charge of saving and loading all symbol related
     * documents. This is the only class that knows about symbol document
     * formats, and has the access functions to return a SymbolDocument,
     * with all of its components.
     *
     * \sa \ref DocumentFormats
     */
    class FormatXmlSymbol
    {
    public:
        FormatXmlSymbol(SymbolDocument *doc = 0);
        FormatXmlSymbol(ComponentData *component);

        bool save();
        bool load();

        SymbolDocument* symbolDocument() const;
        GraphicsScene* graphicsScene() const;
        ComponentData* component() const;

        QString fileName() const;

    private:
        QString saveText();
        void saveSymbol(Caneda::XmlWriter *writer);
        void savePorts(Caneda::XmlWriter *writer);
        void saveProperties(Caneda::XmlWriter *writer);
        void saveModels(Caneda::XmlWriter *writer);

        bool loadFromText(const QString& text);
        void loadSymbol(Caneda::XmlReader *reader);
        void loadPorts(Caneda::XmlReader *reader);
        void loadProperties(Caneda::XmlReader *reader);
        void loadModels(Caneda::XmlReader *reader);

        SymbolDocument *m_symbolDocument;
        ComponentData *m_component;
        QString m_fileName;
    };

    /*!
     * \brief This class handles all the access to the layout documents file
     * format.
     *
     * This class is in charge of saving and loading all layout related
     * documents. This is the only class that knows about layout document
     * formats, and has the access functions to return a LayoutDocument,
     * with all of its components.
     *
     * \sa \ref DocumentFormats
     */
    class FormatXmlLayout
    {
    public:
        FormatXmlLayout(LayoutDocument *doc = 0);

        bool save();
        bool load();

        LayoutDocument* layoutDocument() const;
        GraphicsScene* graphicsScene() const;

        QString fileName() const;

    private:
        QString saveText();
        void savePaintings(Caneda::XmlWriter *writer);

        bool loadFromText(const QString& text);
        void loadPaintings(Caneda::XmlReader *reader);

        LayoutDocument *m_layoutDocument;
    };

    /*!
     * \brief This class handles all the access to the raw spice simulation
     * documents file format.
     *
     * This class is in charge of saving and loading all raw spice simulation
     * related documents. This is the only class that knows about raw spice
     * simulation document formats, and has the access functions to return a
     * SimulationDocument, with all of its components.
     *
     * This class does not handle document saving, as waveform data saving will
     * not be supported at the moment (raw waveform data is only generated and
     * saved by the simulator).
     *
     * \sa \ref DocumentFormats
     */
    class FormatSpice
    {
    public:
        FormatSpice(SchematicDocument *doc = 0);

        bool save();

        SchematicDocument* schematicDocument() const;
        GraphicsScene* graphicsScene() const;
        QString fileName() const;

    private:
        QString generateNetlist();
        PortsNetlist generateNetlistTopology();
        void replacePortNames(PortsNetlist *netlist);

        SchematicDocument *m_schematicDocument;
    };

    /*!
     * \brief This class handles all the access to the raw spice simulation
     * documents file format.
     *
     * This class is in charge of saving and loading all raw spice simulation
     * related documents. This is the only class that knows about raw spice
     * simulation document formats, and has the access functions to return a
     * SimulationDocument, with all of its components.
     *
     * This class does not handle document saving, as waveform data saving will
     * not be supported at the moment (raw waveform data is only generated and
     * saved by the simulator).
     *
     * \sa \ref DocumentFormats
     */
    class FormatRawSimulation
    {
    public:
        FormatRawSimulation(SimulationDocument *doc = 0);

        bool load();

    private:
        void parseFile(QTextStream *file);
        void parseAsciiData(QTextStream *file, const int nvars, const int npoints, const bool real);
        void parseBinaryData(QTextStream *file, const int nvars, const int npoints, const bool real);

        ChartScene* chartScene() const;

        SimulationDocument *m_simulationDocument;

        QList<ChartSeries*> plotCurves;       // List of magnitude curves.
        QList<ChartSeries*> plotCurvesPhase;  // List of phase curves.
    };

} // namespace Caneda

#endif //FILE_FORMATS_H
