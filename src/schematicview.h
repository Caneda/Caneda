/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2013 by Pablo Daniel Pareja Obregon                  *
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

#ifndef SCHEMATIC_VIEW_H
#define SCHEMATIC_VIEW_H

#include "iview.h"

namespace Caneda
{
    // Forward declrations
    class CGraphicsView;
    class SchematicDocument;

    /*!
     * \brief This class represents the schematic view interface
     * implementation.
     *
     * This class represents the view for a document, in a manner
     * similar to Qt's Graphics View Architecture, and provides the view
     * widget, which visualizes the contents of a scene. The view is included
     * as a pointer to CGraphicsView, that contains all the view specific
     * methods. You can attach several views to the same scene, to provide
     * different viewports into the same data set of the document (for example,
     * when using split views).
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa SchematicContext, SchematicDocument
     */
    class SchematicView : public IView
    {
        Q_OBJECT

    public:
        SchematicView(SchematicDocument *document);

        // IView interface methods
        virtual QWidget* toWidget() const;
        virtual IContext* context() const;

        virtual void zoomIn();
        virtual void zoomOut();
        virtual void zoomFitInBest();
        virtual void zoomOriginal();

        virtual IView* duplicate();

        virtual void updateSettingsChanges();
        // End of IView interface methods

    private Q_SLOTS:
        void onWidgetFocussedIn();
        void onWidgetFocussedOut();

    private:
        CGraphicsView *m_cGraphicsView;
    };

} // namespace Caneda

#endif //SCHEMATIC_VIEW_H
