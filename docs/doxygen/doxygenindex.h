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

This is the main doxygen index, to be included in the doxygen documentation
to introduce the developer to the framework and explain the principal classes
and any other thing relevant.

namespace Caneda
{
/*!
 * \mainpage Caneda Documentation
 *
 * \tableofcontents
 *
 * Caneda is an open source EDA software focused on easy of use and portability.
 * While in the short term schematic capture and simulation is the primary goal,
 * in the long term future, PCB and layout edition will be covered. The software
 * aims to support all kinds of circuit simulation types, e.g. DC, AC,
 * S-parameter and harmonic balance analysis.
 *
 * Caneda is currently under a heavy development state. It is not yet
 * recommended for the final user and some basic functions may still be missing
 * (simulations are not yet implemented!). Bugs may arise and some nasty crashes
 * occur. However if you don't mind being an early tester, do not hesitate to
 * try it! Any feedback is welcome, from bugs, artworks to UI changes
 * recommendations. Although we're not quite ready there, we are slowly getting
 * to the point where simulation is being introduced.
 *
 * Caneda is currently looking for help. If you want to help as a developer,
 * create new components to be included in the next realease or help us write
 * the manuals and documentation, don't hesitate to contact us. Your help will
 * be much appreciated.
 *
 * \section Introduction Introduction to the Main Classes
 * A good way to start developing for Caneda is going through Caneda's main
 * classes and reading the principal methods until you understand the inner
 * workings of the program.
 *
 * Caneda's main classes are MainWindow, which implements the window the user
 * interacts with, IContext, IDocument and IView which form part of Caneda's
 * Document-View framework and CGraphicsScene which provides a canvas for
 * managing graphics elements.
 *
 * \li <b>The MainWindow Class:</b> \copybrief MainWindow
 * \li <b>\ref DocumentViewFramework</b>: Caneda's Document-View
 * framework provides a convenient approach to model-view programming, much
 * like Qt's Graphics View Architecture. The main intent is to allow the
 * program to add any document type (vhdl, pcb, etc) without having to manually
 * change the bulk of the existing code. The main classes in Caneda's
 * Document-View framework are IContext, IDocument and IView, which must be
 * inherited to create more specific uses for each file type.
 * \li <b>The CGraphicsScene Class:</b> \copybrief CGraphicsScene
 *
 * Finally, you can find information on the several file formats used in
 * \ref DocumentFormats.
 *
 * \section CodingStyle Caneda Coding Style
 * Caneda uses the same coding style as Qt. Keeping the code consistant is
 * important for readability both for fixing bugs and bringing in new
 * developers. Base all dialogs on .ui files (ie create them using Qt Designer)
 * so that layouts can be perfected effectively. SaveDocumentsDialog is an
 * example for this.
 *
 * For a more in-depth guide through our coding style, please read
 * \ref CodingStylePage.
 *
 * \section Contact Contact
 * Caneda is currently looking for help. If you want to help as a developer,
 * create new components to be included in the next realease or help us write
 * the manuals and documentation, don't hesitate to contact us. Your help will
 * be much appreciated.
 *
 * Any feedback is also welcome, from bugs, artworks to UI changes
 * recommendations.
 *
 * \li <b>Maintainer:</b> Pablo Daniel Pareja Obregón <parejaobregon(at)gmail.com>
 * \li <b>Feature Requests:</b> http://sourceforge.net/p/caneda/feature-requests/
 * \li <b>Bugs Discovered:</b> http://sourceforge.net/p/caneda/bugs/
 *
 * \section License License
 * Caneda is released under the GPLv2. You can see a copy of the license in
 * http://www.gnu.org/licenses/gpl-2.0.html or in the LICENSE file included
 * with the sources.
 *
 * \sa MainWindow, IContext, IDocument, IView, DocumentViewManager, Tab, TabWidget,
 * CGraphicsScene
 */

} // namespace Caneda