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

namespace Caneda
{
/*!
 * \page ModelsFormat Caneda's Models Format Specification
 *
 * \tableofcontents
 *
 * This document describes Caneda's models format specification. While Caneda
 * uses a custom xml format for all its document types, there is the need to
 * interpret the different objects and properties available and know the way to
 * export those objects and properties to other languages. In this way, we are
 * able to use our schematics in other softwares or generate complex documents,
 * for example to export a schematic circuit to LaTeX for later publication.
 *
 * In the same way as is the case for the document formats, the idea behind the
 * models format specification is to mantain as much simplicity as possible,
 * without sacrificing functionality.
 *
 * In this sense, models are the representation of a component in different
 * scenarios. For example, a component can have certain syntax to be used in a
 * SPICE circuit, and a different one in a kicad schematic or a LaTeX diagram.
 * Having a way to extract information from our schematic and interpret it in
 * different ways allow us to export the circuit to other software and
 * simulator engines.
 *
 * An example for a model tag, with several kind of models could be:
 *
\code
<models>
    <model type="spice" syntax="L%label %port{1} %port{2} %property{C}"/>
    <model type="kicad" syntax="L%label %port{1} %port{2} %property{C}"/>
    <model type="vhdl" syntax="L%label %port{1} %port{2} %property{C}"/>
</models>
\endcode
 *
 * All schematic components available in a library, must have at least a symbol
 * file. The symbol file describes the component's symbol (drawing) and its
 * main properties. The corresponding schematic (if available) has the
 * component's circuit description. For simple components, instead of using a
 * schematic for the circuit description, the models tag may be directly used
 * by following a set of rules.
 *
 * All symbol's properties must have an equally named property in the
 * schematic, allowing the user to modify the component's attributes though
 * properties modification.
 *
 * \section Syntax Models Syntax Rules
 * The general syntax rules follow. The parser implementation, for the case of
 * the SPICE output format is FormatSpice::generateNetlist(). In fact, these
 * rules are specifically designed to avoid conflicts with the SPICE syntax so,
 * in the future, the rules may be changed for other formats, or a better
 * syntax may be developed.
 *
 * Each "part" or "block" of a SPICE model is separated by spaces. Parts of
 * each block may include a parameter or value of the component that has to be
 * used. To retrieve those parameters or values, we use escape sequences. Each
 * escape sequence begins with a "%" and a special keyword, which is a command
 * indicating what goes next, followed by optional arguments. For example, an
 * escape sequence may be \%port{A} indicating that a port must be added, and in
 * particular of all ports, the port A must be written. If not "%" is given,
 * the text must be copied "as is".
 *
 * The general rules used for the parser are:
 * \li Models should be always strings.
 * \li All text or strings in a model line is copied "as is" unless it
 * corresponds to an escape sequence.
 * \li Escape sequences start with the "%" character and are followed by a
 * special keyword. There is a limited number of escape sequences, if the
 * keyword provided after the escape character "%" has not a defined action
 * provided in this specification, the "%" character is copied to the output
 * "as is" (as in the case of any other word or character). If an escape
 * sequence is defined, the whole command block (escape sequence plus
 * parameters) must be replaced by its corresponding action result.
 * \li After an escape sequence one or more arguments may follow, enclosed by
 * brackets "{}". The interpretation and processing of the arguments depends
 * upon the escape sequence used.
 *
 * Currently, the escape sequences implemented are:
 * \li <b>\%label</b> : This escape sequence indicates that the label of the component
 * must be used.
 * \li <b>\%port{args}</b> : This escape sequence indicates that the port provided as
 * an argument must be searched in the netlist and the resulting net number (or
 * name) must be used. For example, \%port{in}.
 * \li <b>\%property{args}</b> : This escape sequence indicates that the property name
 * provided as an argument must be searched in among the properties of the
 * component and the resulting value must be used. For example, \%property{R}.
 * \li <b>\%model{args}</b> : This escape sequence indicates that the model provided as
 * an argument must be saved to a list of models and included only once in the
 * output file (typically at the end of file). For example, if we are providing
 * a PNP transistor model named PNP_CUSTOM, we don't want the model definition
 * included more than once in the SPICE netlist output file. The usage of the
 * model itself (in the component line) may be repeated several times across
 * different components, but it is included each time by using a
 * \%property{model} escape sequence (where model indicates the model name).
 * \li <b>\%subcircuit{args}</b> : This escape sequence indicates that the subcircuit
 * provided as an argument must be saved to a list of subcircuits and included
 * only once in the output file (typically at the end of file). For example, if
 * we are providing an OPAMP subcircuit named OP741, we don't want the
 * subcircuit definition included more than once in the SPICE netlist output
 * file. The usage of the subcircuit itself (in the component line) may be
 * repeated several times across different instances, but it is included each
 * time by using a \%property{model} escape sequence (where model indicates the
 * subcircuit name). This is very similar to the models escape sequence.
 * Although theoretically a model escape sequence could be used, the inclusion
 * of a separate escape sequence allows for more flexibility.
 * \li <b>\%directive{args}</b> : This escape sequence indicates that the string
 * or sentence provided as an argument must be saved to a list of strings and
 * included only once in the output file (typically at the end of file). For example,
 * if we are providing an include directive, we typically want the include string
 * only once in the SPICE netlist output file. This is similar to the use of
 * \%model{args} and \%property{model} escape sequences, but the parser does not
 * append any text to the output and the string is copied "as is". This escape
 * sequence is normally used for the remaining spice directives that do not classify
 * into one of the previous escape sequences.
 * \li <b>\%librarypath</b> : This escape sequence indicates that the library path
 * directory of the component must be used.
 * \li <b>\%filepath</b> : This escape sequence indicates that the file path
 * directory of the component must be used. This is different from the library path
 * in the sense that the schematic typically resides in a different location from
 * the component library. For example, in an include spice directive the file with
 * the definitions to be included may reside in the library folder (\%librarypath)
 * or in the schematic folder (\%filepath) to allow the user for modifications.
 * \li <b>\%n</b> : This escape sequence indicates that a new line must be used.
 *
 * \section Symbols Symbol Format
 * This file format is implemented by the FormatXmlSymbol class, and described
 * in the \ref DocumentFormats page. Besides describing the Symbol document,
 * there is a Models tag that groups all available models together. Following
 * there is an example of a sinusoidal voltage source, and its implementation
 * for a SPICE model, using the syntax described above.
 *
\code
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE caneda>
<component name="Voltage Source Sinusoidal" version="0.1.0" label="V">

    ...

    // Models tag:
    // This tag is used to group all available models relative to this component.
    <models>
        <model type="spice" syntax="V%label %port{+} %port{-} DC %property{Va} AC %property{Va} 0
                                    SIN( %property{Voff} %property{Va} %property{freq} %property{td} %property{theta} )"/>
    </models>
</component>
\endcode
 *
 * \sa  FormatXmlSymbol, \ref DocumentFormats
 */

} // namespace Caneda