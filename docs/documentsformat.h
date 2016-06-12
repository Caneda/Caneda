/***************************************************************************
 * Copyright (C) 2013-2015 by Pablo Daniel Pareja Obregon                  *
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

namespace Caneda
{
/*!
 * \page DocumentFormats Caneda's Document Formats
 *
 * \tableofcontents
 *
 * This document describes Caneda's document formats. Caneda uses a custom xml
 * format for all its document types. The idea behind all document types is to
 * mantain as much simplicity as possible, without sacrificing functionality.
 *
 * Although currently Caneda has its own document formats, this does not mean
 * in the future they can be replaced by better, more standard formats.
 * However, at the moment of this writing, the authors could not find a well
 * suited, standard document format for use in a whole EDA suite that at the
 * same time provides a unified schema throughout all document scenarios
 * (schematic, layout, pcb, etc).
 *
 * All schematic components available in a library, must have a symbol file and
 * an optional schematic file. The symbol file describes the component's symbol
 * (drawing) and its main properties. The corresponding schematic has the
 * component's circuit description. For simple components, instead of using a
 * schematic for the circuit description, a models tag may be directly used by
 * following a set of rules defined in \ref ModelsFormat. When using a
 * schematic for circuit description, all the symbol's properties must have an
 * equal named property in the schematic, allowing the user to modify the
 * component's attributes though properties modification.
 *
 * Caneda's document file format handling is in charge of the following classes:
 * \li FormatXmlSchematic
 * \li FormatXmlSymbol
 * \li FormatXmlLayout
 * \li FormatRawSimulation. This class does not implement a Caneda's specific
 * format, but rather reads the standard spice simulation raw waveform data.
 *
 * \section Schematics Schematic Format
 * This file format is implemented by the FormatXmlSchematic class.
 *
\code
// Heading:
// All documents must begin with the following heading, showing the document
// version and document type (caneda).
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE caneda>
// Caneda version:
// The following tag indicates the Caneda version that was used to create this
// document.
//
// Attibutes list:
//        * version (#required): The version of Caneda for which the file was
//          written.
<caneda version="0.1.0">

    // The basic schematic document structure is divided in 3 main tag types, each
    // one containing a different aspect of the schematic:
    //        * components: The components tag represents a list of all the
    //          components included in the schematic.
    //        * wires: The wires tag represents a list of connections between the
    //          different components and ports included in the schematic.
    //        * paintings: The paintings tag represents a list of tags that
    //          describe some type of paint operation, for example an arrow, a
    //          line, a rectangle, etc.

    <components>
        // Component tag:
        // Each component tag describes some type of component included in the
        // schematic, for example a resistor, a capacitor or a user created
        // component.
        //
        // Attibutes list:
        //      * name (#required): Represents the name of component. This
        //        string is used to identify this type of component in the
        //        library, however a component key is formed by using the
        //        library and this name, therefore it is not neccessary for
        //        this name to be unique though all Caneda (but yes in each
        //        library).
        //      * library (#required): Represents the library containing the
        //        component. This string, along with the name tag is used to
        //        identify this component between all possible components in
        //        the program instance.
        //      * pos (#required): This tuple represents the painting location
        //        inside the symbols coordinate system.
        //      * transform (#required): This tag indicates the transform
        //        matrix of the component, thus allowing for rotations and other
        //        special operations to be applied.
        <component name="Resistor" library="Passive" pos="80,-70" transform="1,0,0,1,0,0">

            // Properties tag:
            // This tag is used to group all properties relative to this
            // component.
            //
            // Attibutes list:
            //      * pos (#required): This tuple represents the properties
            //        location inside the schematic coordinate system.
            <properties pos="-24,6">
                // Property tag:
                // This tag is used to represent the different properties of
                // the component.
                //
                // Attibutes:
                //      * name (#required): Represents the name of the
                //        property.
                //      * value (#required): Represents the value of the
                //        property.
                //      * visible (#required): This tag value is either "true"
                //        or "false". This indicates whether the property text
                //        is visible on the schematic or not.
                <property name="R" value="50" visible="true"/>
                <property name="Tc1" value="0.0" visible="false"/>
                <property name="Tc2" value="0.0" visible="false"/>
                <property name="Temp" value="26.5" visible="false"/>
                <property name="Tnom" value="26.5" visible="false"/>
                <property name="label" value="R1" visible="true"/>
            </properties>
        </component>

        <component name="Amplifier" library="Active" pos="230,-70" transform="1,0,0,1,0,0">
            <properties pos="-30,20">
                <property name="G" value="10" visible="true"/>
                <property name="Zin" value="50" visible="false"/>
                <property name="Zout" value="50" visible="false"/>
                <property name="label" value="Amp4" visible="true"/>
            </properties>
        </component>
    </components>

    // Ports tag:
    // This tag is used to group all ports and special nets relative to this
    // component. A port is the logical unit used to perform connections between
    // the symbol and the schematic.
    <ports>
        // Port tag:
        // This tag is used to represent a single port. A port is the logical
        // unit used to perform connections.
        //
        // Attibutes list:
        //      * name (#required): This string can be used to identify ports.
        //      * pos (#required): This tuple represents the port location
        //        inside the schematic coordinate system.
        <port name="a" pos="-20,0"/>
        <port name="b" pos="20,0"/>
    </ports>

    <wires>
        // Wire tag:
        // This element is used to store each wire segment representation
        // in the schematic. Each succesive item will be a different segment.
        // A typical connection will have one or more wire segments connected
        // together to form a wire connection between the two ports or
        // components involved.
        //
        // Attibutes list:
        //      * start (#required): This tuple represents the start port
        //        location of the wire inside the schematic coordinate
        //        system.
        //      * end (#required): This tuple represents the end port
        //        location of the wire inside the schematic coordinate
        //        system.
        <wire start="100,-70" end="200,-70"/>
        <wire start="60,-70" end="-70,-70"/>
        <wire start="-70,210" end="-70,-70"/>
        <wire start="50,210" end="-70,210"/>
    </wires>

    <paintings>
        // Painting tag:
        // Each painting tag describes some type of paint operation, for
        // example an arrow, a line, a rectangle, etc. The name attribute is
        // required as well as the dimensions and position of the painting.
        //
        // Attibutes list:
        //      * name (#required): This string specifies which type of
        //        painting this tag represents.
        //      * geometry: This tag is optional and represents the geometry of
        //        the painting item, indicated as a bounding rectangle. The
        //        actual tag may be one of (line, ellipse, rectangle),
        //        depending on the painting type.
        //      * pos (#required): This tuple represents the painting location
        //        inside the symbols coordinate system.
        //      * transform (#required): This tag indicates the transform
        //        matrix of the painting, thus allowing for rotations and other
        //        special operations to be applied.
        <painting name="arrow" line="0,0,290,260" pos="-150,170" transform="0,-1,1,0,0,0">
            // Several painting properties follow. Each painting has its own
            // kind of properties, but the general rule is to keep the symbol
            // specific options as attributes of the properties tag, and style
            // properties such as pen and brush attibutes as succesive tags.
            <properties headStyle="1" headSize="12,20"/>
            <pen width="0" color="#000000" style="1"/>
            <brush color="#000000" style="1"/>
        </painting>

        <painting name="ellipse" ellipse="0,0,200,180" pos="-130,-50" transform="1,0,0,1,0,0">
            <pen width="0" color="#000000" style="1"/>
            <brush color="#000000" style="0"/>
        </painting>
    </paintings>
</component>
\endcode
 *
 * \section Symbols Symbol Format
 * This file format is implemented by the FormatXmlSymbol class.
 *
\code
// Heading:
// All documents must begin with the following heading, showing the document
// version and document type (caneda).
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE caneda>

// Component tag:
// All symbols description begin with a component tag.
//
// Attibutes list:
//        * name (#required): Represents the name of component. This string is
//          used to identify this type of component in the library, however a
//          component key is formed by using the library and this name,
//          therefore it is not neccessary for this name to be unique though
//          all Caneda (but yes in each library).
//        * version (#required): The version of Caneda for which the component
//          was written.
//        * label (#required): The default label that is to be used when
//          instancing this component in a schematic.
<component name="resistor" version="0.1.0"  label="R">

    // Displaytext tag:
    // This tag is used as a string which will be used to represent the
    // component in the sidebar, dialog boxes, libraries, etc. Since this is
    // used for display, it requires internationalization support. Therefore
    // the english default text is enclosed as "C" in the lang attibute and
    // the other languages texts are enclosed in their two lettered locale
    // representation attibute.
    //
    // The lang attribute is required.
    <displaytext>
        <lang lang="C">Resistor</lang>
        <lang lang="fr">Resistance</lang>
    </displaytext>

    // Description tag:
    // This tag is used to give a short description of the component. This can
    // be regarded as help text. Here also the internationalization rules
    // apply. Enclose in C attribute for default language (english) and locale
    // tag for other languages.
    <description>
        <lang lang="C">A dissipative device. Ohms law apply</lang>
    </description>

    // Symbol tag:
    // This element is used to store the symbol representation of the
    // component. Each succesive item will be a painting describing some type
    // of paint operation, for example an arrow, a line, a rectangle, etc.
    <symbol>
        // Painting tag:
        // Each painting tag describes some type of paint operation, for
        // example an arrow, a line, a rectangle, etc. The name attribute is
        // required as well as the dimensions and position of the painting.
        //
        // Attibutes list:
        //      * name (#required): This string specifies which type of
        //        painting this tag represents.
        //      * geometry: This tag is optional and represents the geometry of
        //        the painting item, indicated as a bounding rectangle. The
        //        actual tag may be one of (line, ellipse, rectangle),
        //        depending on the painting type.
        //      * pos (#required): This tuple represents the painting location
        //        inside the symbols coordinate system.
        //      * transform (#required): This tag indicates the transform
        //        matrix of the painting, thus allowing for rotations and other
        //        special operations to be applied.
        <painting name="arrow" line="0,0,290,260" pos="-150,170" transform="0,-1,1,0,0,0">
            // Several painting properties follow. Each painting has its own
            // kind of properties, but the general rule is to keep the symbol
            // specific options as attributes of the properties tag, and style
            // properties such as pen and brush attibutes as succesive tags.
            <properties headStyle="1" headSize="12,20"/>
        </painting>

        <painting name="ellipse" ellipse="0,0,200,180" pos="-130,-50" transform="1,0,0,1,0,0"/>
    </symbol>

    // Ports tag:
    // This tag is used to group all ports relative to this component. A port
    // is the logical unit used to perform connections.
    <ports>
        // Port tag:
        // This tag is used to represent a single port. A port is the logical
        // unit used to perform connections.
        //
        // Attibutes list:
        //      * name (#required): This string can be used to identify nodes.
        //      * pos (#required): This tuple represents the port location
        //        inside the symbols coordinate system.
        <port name="a" pos="-20,0"/>
        <port name="b" pos="20,0"/>
    </ports>

    // Properties tag:
    // This tag is used to group all properties relative to this component.
    <properties>
        // Property tag:
        // This tag is used to represent the different properties of the
        // component. The child tag of the property tag is a description tag.
        // Again use a lang tag to enclose the text in english and other
        // languages.
        //
        // Attibutes:
        //      * name (#required): Represents the name of the property.
        //      * default (#required): Represents the default value
        //      * unit (#required): The unit used.
        //      * visible (#required): The value is either "true" or "false".
        //        This indicates whether the property text is visible on the
        //        schematic or not.
        <property name="R" default="50" unit="Ω" visible="true">
            <description>
                <lang lang="C">Ohmic resistance</lang>
                <lang lang="it">Resistenza ohmica</lang>
            </description>
        </property>
        <property name="Temp" default="26.5" unit="°C" visible="false">
            <description>
                <lang lang="C">Temperature</lang>
                <lang lang="fr">Température</lang>
            </description>
        </property>
    </properties>

    // Models tag:
    // This tag is used to group all available models relative to this component.
    //
    // Models are the representation of a component in different scenarios.
    // For example, a component can have certain syntax to be used in a spice
    // circuit, and a different one in a kicad schematic. Having a way to
    // extract information from our schematic and interpret it in different
    // ways allow us to export the circuit to other softwares and simulator
    // engines. Models should be always strings.
    <models>
        // Model tag:
        // This tag is used to represent a particular model of the component
        // relative to one output language/format/etc.
        //
        // Attibutes:
        //      * type (#required): Represents the name of the output format.
        //      * syntax (#required): Represents the syntax value to use during
        //        file export.
        <model type="spice" syntax="I%label %port{+} %port{-} DC %property{Ia} AC %property{Ia} 0
                                    SIN( %property{Ioff} %property{Ia} %property{freq} %property{td} %property{theta} )"/>
    </models>
</component>
\endcode
 *
 * For more information on how to use models inside the symbol documents, see
 * the \ref ModelsFormat page.
 *
 * \section Layouts Layout Format
 * This file format is implemented by the FormatXmlLayout class. This format
 * is currently under specification, and is therefore having heavy
 * modifications, as the file evolves.
 *
 * \todo Improve or finish this file format's specification.
 *
\code
// Heading:
// All documents must begin with the following heading, showing the document
// version and document type (caneda).
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE caneda>
// Caneda version:
// The following tag indicates the Caneda version that was used to create this
// document.
//
// Attibutes list:
//        * version (#required): The version of Caneda for which the file was
//          written.
<caneda version="0.1.0">

    // Paintings tag:
    // The paintings tag represents a list of tags that describe some type of
    // layer mask or geometry.
    <paintings>
        // Painting tag:
        // Each painting tag describes some type of layer mask or geometry, for
        // example a line, a rectangle, etc. The name attribute is required as
        // well as the dimensions and position of the painting.
        //
        // Attibutes list:
        //      * name (#required): This string's value is always "layer" and
        //        in the future may be replaced by the layer number.
        //      * rect (#required): This tag represents the geometry of the
        //        layer item, indicated as a bounding rectangle.
        //      * pos (#required): This tuple represents the layer mask
        //        location inside the layout coordinate system.
        //      * transform (#required): This tag indicates the transform
        //        matrix of the layer mask, thus allowing for rotations and
        //        other special operations to be applied.
        <painting name="layer" rect="0,0,390,180" pos="320,-80" transform="1,0,0,1,0,0">
            // Property tag:
            // This tag is used to represent the different properties of the
            // layer.
            //
            // Attibutes:
            //      * layerName (#required): Represents the layer name as a
            //        number, that will be translated into a layer type by the
            //        program (metal1, active, poly, etc).
            //      * netLabel (#required): Represents the net to which this
            //        layer corresponds. May be used in the future to perform a
            //        layout to schematic asossiation.
            <properties layerName="4" netLabel=""/>
        </painting>

        <painting name="layer" rect="0,0,290,80" pos="-40,-110" transform="1,0,0,1,0,0">
            <properties layerName="2" netLabel=""/>
        </painting>
    </paintings>
</component>
\endcode
 *
 * \sa  FormatXmlSchematic, FormatXmlSymbol, FormatXmlLayout, FormatRawSimulation, \ref ModelsFormat
 */

} // namespace Caneda