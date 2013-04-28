/***************************************************************************
 * Copyright (C) 2013 by Pablo Daniel Pareja Obregon                       *
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
 * a schematic file. The symbol file describes the component's symbol (drawing)
 * and its main properties. The corresponding schematic has the component's
 * circuit description. All the symbol's properties must have an equal named
 * property in the schematic, allowing for the user to modify the component's
 * attributes though properties modification.
 *
 * The only components that are allowed not to have a circuit schematic are the
 * basic spice components (resistors, capacitors, etc), as these are described
 * by their spice behaviour. This case is not yet implemented, but will be used
 * during schematic export to spice netlist (in simulations) and must be
 * somehow indicated in the symbol file.
 *
 * Caneda's document file format handling is in charge of the following classes:
 * \li FormatXmlSchematic
 * \li FormatXmlSymbol
 * \li FormatXmlLayout
 * \li FormatRawSimulation. This class does not implement a Caneda's specific
 * format, rather reads the standard spice simulation raw waveform data.
 *
 * \section Schematics Schematic Format
 * This file format is implemented by the FormatXmlSchematic class.
 *
 * \todo To be documented...
 *
 * \section Symbols Symbol Format
 * This file format is implemented by the FormatXmlSymbol class.
 *
 * \code{.xml}
<!-- Component tag:
     All symbols description begin with a component tag. The attibutes used are:
        * name (#required): Represents the name of component. This string is
          used to identify this type of component in the library, however a
          component key is formed by using the library and this name, therefore
          it is not neccessary for this name to be unique though all Caneda
          (but yes in each library).

        * version (#required): The version of Caneda for which the component was
          written.
-->
<component name="resistor" version="0.1.0">

    <!-- Displaytext tag:
         This tag is used as a string which will be used to represent the
         component in the sidebar, dialog boxes, libraries, etc. Since this is
         used for display, it requires internationalization support. Therefore
         the english default text is enclosed as "C" in the lang attibute and
         the other languages texts are enclosed in their two lettered locale
         representation attibute.

         The lang attribute is required.
    -->
    <displaytext>
        <lang lang="C">Resistor</lang>
        <lang lang="fr">Resistance</lang>
    </displaytext>

    <!-- Description tag:
         This tag is used to give a short description of the component. This can
         be regarded as help text. Here also the internationalization rules
         apply. Enclose in C attribute for default language (english) and locale
         tag for other languages.
    -->
    <description>
        <lang lang="C">A dissipative device. Ohms law apply</lang>
    </description>

    <!-- Symbol tag:
         This element is used to store the symbol representation of the
         component. Each succesive item will be a painting describing some type
         of paint operation, for example an arrow, a line, a rectangle, etc.
    -->
    <symbol>
        <!-- Painting tag:
             Each painting tag describes some type of paint operation, for
             example an arrow, a line, a rectangle, etc. The name attribute is
             required and specified which type of painting this tags
             represents.

             The name attibute is required.
        -->
        <painting name="arrow">
            <!-- Several painting properties follow. Each painting has its own
                 kind of properties, but the general rule is to keep the
                 dimensions and position of the painting as attributes of the
                 properties tag, and style properties as succesive tags.
            -->
            <properties line="0,0,290,260" pos="-150,170" headStyle="1" headSize="12,20"/>
            <pen width="0" color="#000000" style="1"/>
            <brush color="#000000" style="1"/>
            <transform matrix="0,-1,1,0,0,0"/>
        </painting>

        <painting name="ellipse">
            <properties ellipse="0,0,200,180" pos="-130,-50"/>
            <pen width="0" color="#000000" style="1"/>
            <brush color="#000000" style="0"/>
            <transform matrix="1,0,0,1,0,0"/>
        </painting>

    </symbol>

    <!-- Ports tag:
         This tag is used to group all ports relative to this component. A port
         is the logical unit used to perform connections.
    -->
    <ports>
        <!-- Port tag:
             This tag is used to represent a single port. A port is the logical
             unit used to perform connections.
             Attibutes list:
                * name (#required): This string can be used to identify nodes.
                * pos (#required): This tuple represents the port location
                  inside the symbols coordinate system.
        -->
        <port name="a" pos="-20,0"/>
        <port name="b" pos="20,0"/>
    </ports>

    <!-- Properties tag:
         This tag is used to group all properties relative to this component.
    -->
    <properties>
        <!-- Property tag:
             This tag is used to represent the different properties of the
             component. The child tag of the property tag is a description tag.
             Again use a lang tag to enclose the text in english and other
             languages.

             Attibutes:
                * name (#required): Represents the name of the property.
                * default (#required) : Represents the default value
                * unit (#required): The unit used.
                * visible (#required): The value is either "true" or "false".
                  This indicates whether the property text is visible on the
                  schematic or not.
        -->
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

</component>

 * \endcode
 *
 * \section Layouts Layout Format
 * This file format is implemented by the FormatXmlLayout class.
 *
 * \todo To be documented...
 *
 * \section OldFormat Old Document Format
 * This section is kept as a reference. This was the old file format used in
 * Caneda, but was progressibly replaced. This is not used anymore but is kept
 * here as a reference.
 *
 * \code{.xml}
<!-- component tag:
     All component description begin with component
     tag. The attibutes used are:

     name (#required): Represents the name of component. This string is
         used to identify this type of component, caching ... Therefore
         this should be unique.

     version (#required): The version of Caneda for which the component
         was written.
-->
<component name="resistor" version="0.1.0">

   <!-- displaytext tag:
        This tag is used to represent a string which will be used to
        represent the component in sidebar, dialog boxes, library..
        Since this is used as for display, this requires
        internationalization support.
        Therefore the english default text is enclosed in "C" tag and the
        other language text is enclosed in their two lettered locale
        representation tag.

        C is required and is the first element
   -->
   <displaytext>
      <lang lang="C">Resistor</lang>
      <lang lang="fr">Resistance</lang>
   </displaytext>

   <!-- description tag:
        This tag is used to give a short description of the
        component. This can be regarded as help text. Here also the
        internationalization rules apply. Enclose in C tag for default
        language(english) and locale tag for other languages.
   -->
   <description>
      <lang lang="C">A dissipative device. Ohm law apply</lang>
   </description>

   <!-- schematics contain one or more schema
        default is required even in case of one component
   -->
   <schematics default="ISOlike">

     <!--
          This element is used to represent the schematic representation of the
          component. It infact refers to svg. There are two ways to do
          this. Firstly the svg can be described in an external file.
          In that case the attribute svgfile should be set the URI of the svg file.
          The second possibility is to include directly svg.

          Attribute

          Attibutes list:
          href (#required if type is "external"): Relative file path
     -->
     <schematic name="ISOlike">
       <!--
            port position temporary will be changed in next version
            name of the port same as in ports tags below
               x (#required) - format is "x" where x is double
               numbers indicating position in the svg.
               y (#required) - format is "y" where y is double
               numbers indicating position in the svg.
       -->
       <port name="a" x="0" y="0"/>
       <port name="b" x="4.5" y="0"/>
       <!-- if internal -->
       <svg xmlns="http://www.w3.org/2000/svg"
            version="1.2" baseProfile="tiny"
            viewBox="-30 -11 60 22">
         <desc>Resistor ISO norm (04-01-01)</desc>
         <g id="resistor">
           <path d="M -30 0 H -18"/>
           <rect x="-18" y="-9" width="36" height="18" stroke="yellow"/>
           <path d="M 18 0 H 30"/>
         </g>
       </svg>
     </schematic>
     <schematic name="ISOlikebis" href="svg/resistorsvg">
       <!--
            port position temporary will be changed in next version
            name of the port same as in ports tags below
               x (#required) - format is "x" where x is double
               numbers indicating position in the svg.
               y (#required) - format is "y" where y is double
               numbers indicating position in the svg.
       -->
       <port name="a" x="0" y="0"/>
       <port name="b" x="4.5" y="0"/>
     </schematic>
   </schematics>

   <!-- ports tag:
        This tag encloses the port tags.
        -->
   <ports>

      <!-- port tag:
           This tag is used to represent port. Port is logical unit used to
           determine connections.
           Attibutes list:

           name (#required): This string can be used to identify
               nodes.

           type (#required) - value is either "analog" or "digital" or "digital bus" :
               This is required for future reference.
           -->
      <port name="a" type="analog"/>
      <port name="b" type="analog"/>

   </ports>

   <!-- properties tag:
        This tag is used to multiple enclose property tags.
        -->
   <properties>

      <!-- property tag:
               This tag is used to represent the various properties of the
               component.
               The child tags of property tag is description tag. Again use C
               tag to enclose text in english and locale tag otherwise.

           Attibutes:
               name (#required): Represents the name of the property.

               type (#required): Represents the data type used. Currently
                  "boolean",
                  "double", "int" and "String" should be enough.

               unit (#optional): The unit used.

               visible (#optional) - value is either "true" or "false" :
                  If this attibute is not specified, then the default value is
                  false.
                  THis indicates whether the property text is visible on the
                  schematic or not.

               default (#optional) : Represents the default value

               options (#optional): This represents a comma separated list of
                  strings which are the only values that can be taken by the value
                  of the property. defaultvalue should conform to this if
                  specified.
                  BTW  we assumed that if options exist for a property then the
                  property should be string type.
           -->
      <property name="R" type="double" unit="ohm" visible="true"
                default="50">
         <description>
            <lang lang="C">ohmic resistance in Ohms</lang>
         </description>
      </property>
   </properties>
</component>

 * \endcode
 *
 * \sa  FormatXmlSchematic, FormatXmlSymbol, FormatXmlLayout, FormatRawSimulation
 */

} // namespace Caneda