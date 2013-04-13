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
 * \li The MainWindow Class: \copybrief MainWindow
 * \li Caneda's Document-View framework: \copybrief IContext
 * \li The CGraphicsScene Class: \copybrief CGraphicsScene
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
 * \li <b> Maintainer:</b> Pablo Daniel Pareja Obregón <parejaobregon(at)gmail.com>
 * \li <b> Feature Requests:</b> http://sourceforge.net/p/caneda/feature-requests/
 * \li <b> Bugs Discovered:</b> http://sourceforge.net/p/caneda/bugs/
 * 
 * \section License License
 * Caneda is released under the GPLv2. You can see a copy of the license in 
 * http://www.gnu.org/licenses/gpl-2.0.html or in the LICENSE file included 
 * with the sources.
 * 
 * \sa MainWindow, IContext, IDocument, IView, DocumentViewManager, Tab, TabWidget,
 * CGraphicsScene
 */


/*!
 * \page CodingStylePage Caneda Coding Style
 * 
 * \tableofcontents
 * 
 * Extracted from Arora Coding Style. Feel free to edit to our needs.
 * 
 * \section Introduction Introduction
 * 
 * Caneda uses the same coding style as Qt.  If you use the git_hooks there is
 * a pre-commit_checkstyle that will warn you of style errors.  Keeping the
 * code consistant is important for readability both for fixing bugs and
 * bringing in new developers. Base all dialogs on .ui files (ie create them
 * using Qt Designer) so that layouts can be perfected effectively.
 * SaveDocumentsDialog is an example for this.
 * 
 * \subsection Indentation Indentation
 * \li 4 spaces are used for indentation
 * \li Spaces, not tabs!
 * 
 * \subsection DeclaringVariables Declaring variables
 * \li Declare each variable on a separate line
 * \li Avoid short (e.g., a,rbarr,nughdeget) names whenever possible
 * \li Single character variable names are only okay for counters and
 * temporaries, where the purpose of the variable is obvious
 * \li Wait with declaring a variable until it is needed
 *
 * \code{.cpp}
 * // Wrong
 * int a, b;
 * char *c, *d;
 * \endcode
 * 
 * \code{.cpp}
 * // Correct
 * int height;
 * int width;
 * char *nameOfThis;
 * char *nameOfThat;
 * \endcode
 * 
 * \li Variables and functions start with a small letter. Each consecutive word
 * in a variable's name starts with a capital letter
 * \li Avoid abbreviations
 * 
 * \code{.cpp}
 * // Wrong
 * short Cntr;
 * char ITEM_DELIM = '\t';
 * \endcode
 * 
 * \code{.cpp}
 * // Correct
 * short counter;
 * char itemDelimiter = '\t';
 * \endcode
 * 
 * \li Classes always start with a big letter.
 * 
 * \subsection Whitespace Whitespace
 * \li Use blank lines to group statements together where suited
 * \li Always use only one blank line
 * \li Always use a single space after a keyword, and before a curly brace.
 * 
 * \code{.cpp}
 * // Wrong
 * if(foo){
 * }
 * \endcode
 * 
 * \code{.cpp}
 * // Correct
 * if (foo) {
 * }
 * \endcode
 * 
 * \li For pointers or references, always use a single space before * or &,
 * but never after. Exception: template parameters - use no space before and no
 * space after * or &.
 * \li No space after a cast.
 * \li Avoid C-style casts when possible.
 * 
 * \code{.cpp}
 * // Wrong
 * char* blockOfMemory = (char* ) malloc(data.size());
 * \endcode
 * 
 * \code{.cpp}
 * // Correct
 * char *blockOfMemory = (char *)malloc(data.size());
 * char *blockOfMemory = reinterpret_cast<char*>(malloc(data.size()));
 * \endcode
 * 
 * \subsection Braces Braces
 * \li As a base rule, the left curly brace goes on the same line as the start
 * of the statement:
 * 
 * \code{.cpp}
 * // Wrong
 * if (codec)
 * {
 * }
 * \endcode
 * 
 * \code{.cpp}
 * // Correct
 * if (codec) {
 * }
 * \endcode
 * 
 * \li Exception: Function implementations and class declarations always have
 * the left brace on the start of a line:
 * 
 * \code{.cpp}
 * static void foo(int g)
 * {
 *     qDebug("foo: %i", g);
 * }
 * \endcode
 * 
 * \code{.cpp}
 * class Moo
 * {
 * };
 * \endcode
 * 
 * \li Use curly braces when the body of a conditional statement contains more
 * than one line, and also if a single line statement is somewhat complex.
 * 
 * \code{.cpp}
 * // Wrong
 * if (address.isEmpty()) {
 *     return false;
 * }
 * 
 * for (int i = 0; i < 10; ++i) {
 *     qDebug("%i", i);
 * }
 * \endcode
 * 
 * \code{.cpp}
 * // Correct
 * if (address.isEmpty())
 *     return false;
 * 
 * for (int i = 0; i < 10; ++i)
 *     qDebug("%i", i);
 * \endcode
 * 
 * \li Exception 1: Use braces also if the parent statement covers several
 * lines / wraps
 * 
 * \code{.cpp}
 * // Correct
 * if (address.isEmpty() || !isValid()
 *     || !codec) {
 *     return false;
 * }
 * \endcode
 * 
 * \li Exception 2: Use braces also in if-then-else blocks where either the
 * if-code or the else-code covers several lines
 * 
 * \code{.cpp}
 * // Wrong
 * if (address.isEmpty())
 *     return false;
 * else {
 *     qDebug("%s", qPrintable(address));
 *     ++it;
 * }
 * \endcode
 * 
 * \code{.cpp}
 * // Correct
 * if (address.isEmpty()) {
 *     return false;
 * } else {
 *     qDebug("%s", qPrintable(address));
 *     ++it;
 * }
 * \endcode
 * 
 * \code{.cpp}
 * // Wrong
 * if (a)
 *     if (b)
 *         ...
 *     else
 *         ...
 * \endcode
 * 
 * \code{.cpp}
 * // Correct
 * if (a) {
 *     if (b)
 *         ...
 *     else
 *         ...
 * }
 * \endcode
 * 
 * \li Use curly braces when the body of a conditional statement is empty
 * 
 * \code{.cpp}
 * // Wrong
 * while (a);
 * \endcode
 * 
 * \code{.cpp}
 * // Correct
 * while (a) {}
 * \endcode
 * 
 * \subsection Parentheses Parentheses
 * \li Use parentheses to group expressions:
 * 
 * \code{.cpp}
 * // Wrong
 * if (a && b || c)
 * \endcode
 * 
 * \code{.cpp}
 * // Correct
 * if ((a && b) || c)
 * \endcode
 * 
 * \code{.cpp}
 * // Wrong
 * a + b & c
 * \endcode
 * 
 * \code{.cpp}
 * // Correct
 * (a + b) & c
 * \endcode
 * 
 * \subsection SwitchStatements Switch statements
 * \li The case labels are on the same column as the switch
 * \li Every case must have a break (or return) statement at the end or a
 * comment to indicate that there's intentionally no break
 * 
 * \code{.cpp}
 * switch (myEnum) {
 *     case Value1:
 *         doSomething();
 *         break;
 *     case Value2:
 *         doSomethingElse();
 *         // fall through
 *     default:
 *         defaultHandling();
 *         break;
 * }
 * \endcode
 * 
 * \subsection LineBreaks Line breaks
 * \li Keep lines shorter than 100 characters; insert line breaks if necessary.
 * \li Commas go at the end of a broken line; operators start at the beginning
 * of the new line. The operator is at the end of the line to avoid having to
 * scroll if your editor is too narrow.
 * 
 * \code{.cpp}
 * // Correct
 * if (longExpression
 *     + otherLongExpression
 *     + otherOtherLongExpression) {
 * }
 * \endcode
 * 
 * \code{.cpp}
 * // Wrong
 * if (longExpression +
 *     otherLongExpression +
 *     otherOtherLongExpression) {
 * }
 * \endcode
 * 
 * \subsection ForwardDeclarations Forward declarations
 * Use forward declaration when possible. Suppose you want to define a new
 * class B that uses objects of class A. If B only uses references or pointers
 * to A, use forward declarations (you don't need to include <A.h> in the
 * header, but include it in the cpp). This will in turn speed a little bit the
 * compilation.
 * 
 * \code{.cpp}
 * class A ;
 * 
 * class B {
 *   private:
 *       A *fPtrA;
 *   public:
 *       void mymethod(const& A) const;
 * };
 * \endcode
 * 
 * B derives from A or B explicitely (or implicitely) uses objects of class A.
 * You then need to include <A.h>
 * 
 * \code{.cpp}
 * #include <A.h>
 * 
 * class B : public A{
 * 
 * };
 * 
 * class C {
 *   private:
 *       A fA;
 *   public:
 *       void mymethod(A par);   
 * }
 * \endcode
 * 
 * \section GeneralException General exception 
 * Feel free to break a rule if it makes your code look bad.
 * 
 */

} // namespace Caneda