/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2016 by Pablo Daniel Pareja Obregon                  *
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

#ifndef CANEDA_IVIEW_H
#define CANEDA_IVIEW_H

#include "global.h"

#include <QObject>

// Forward declaration
class QComboBox;
class QToolBar;
class QWidget;

namespace Caneda
{
    // Forward declarations.
    class CGraphicsView;
    class CSimulationView;
    class DocumentViewManager;
    class LayoutDocument;
    class IContext;
    class IDocument;
    class SchematicDocument;
    class SimulationDocument;
    class SymbolDocument;
    class TextDocument;
    class TextEdit;

    /*************************************************************************
     *                       General IView Structure                         *
     *************************************************************************/
    /*!
     * \brief This class represents the view for a document, in a manner
     * similar to Qt's Graphics View Architecture. The IView class provides
     * the view widget, which visualizes the contents of a scene. The view
     * itself may be included as a pointer to another class that contains
     * all the view specific methods (for example a graphics view). You can
     * attach several views to the same scene, to provide different viewports
     * into the same data set of the document (for example, when using split
     * views).
     *
     * \sa IContext, IDocument, \ref DocumentViewFramework
     */
    class IView : public QObject
    {
        Q_OBJECT

    public:
        IView(IDocument *document);

        IDocument* document() const;

        virtual QWidget* toWidget() const = 0;
        virtual IContext* context() const = 0;

        virtual void zoomIn() = 0;
        virtual void zoomOut() = 0;
        virtual void zoomFitInBest() = 0;
        virtual void zoomOriginal() = 0;

        virtual IView* duplicate() = 0;

        virtual void updateSettingsChanges() = 0;

        QToolBar* toolBar() const;

    Q_SIGNALS:
        void focussedIn(IView *view);
        void focussedOut(IView *view);
        void closed(IView *view);
        void statusBarMessage(const QString &text);

    private Q_SLOTS:
        void onDocumentViewManagerChanged();
        void onDocumentSelectorIndexChanged(int index);
        void slotSplitHorizontal();
        void slotSplitVertical();
        void slotCloseView();

    protected:
        IDocument * const m_document;
        QToolBar *m_toolBar;
        QComboBox *m_documentSelector;

        friend class DocumentViewManager;
    };


    /*************************************************************************
     *                  Specialized IView Implementations                    *
     *************************************************************************/
    /*!
     * \brief This class represents the layout view interface
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
     * \sa LayoutContext, LayoutDocument
     */
    class LayoutView : public IView
    {
        Q_OBJECT

    public:
        LayoutView(LayoutDocument *document);

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

    /*!
     * \brief This class represents the simulation view interface
     * implementation.
     *
     * This class represents the view for a document, in a manner
     * similar to Qt's Graphics View Architecture, and provides the view
     * widget, which visualizes the contents of a scene. The view is included
     * as a pointer to a CSimulationView, that contains all the view specific
     * methods. You can attach several views to the same scene, to provide
     * different viewports into the same data set of the document (for example,
     * when using split views).
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa SimulationContext, SimulationDocument
     */
    class SimulationView : public IView
    {
        Q_OBJECT

    public:
        SimulationView(SimulationDocument *document);

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
        CSimulationView *m_simulationView;  //! \brief Plot widget.
    };

    /*!
     * \brief This class represents the symbol view interface
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
     * \sa SymbolContext, SymbolDocument
     */
    class SymbolView : public IView
    {
        Q_OBJECT

    public:
        SymbolView(SymbolDocument *document);

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

    /*!
     * \brief This class represents the text view interface implementation.
     *
     * This class represents the view for a document, in a manner
     * similar to Qt's Graphics View Architecture, and provides the view
     * widget, which visualizes the contents of the document. The view is
     * included as a pointer to TextEdit, that contains all the view specific
     * methods. You can attach several views to the same document, to provide
     * different viewports into the same data set of the document (for example,
     * when using split views).
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa TextContext, TextDocument
     */
    class TextView : public IView
    {
        Q_OBJECT

    public:
        TextView(TextDocument *document);

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
        void onFocussed();

    private:
        TextEdit *m_textEdit;

        void setZoomLevel(qreal level);

        const qreal m_originalZoom;
        ZoomRange m_zoomRange;
        qreal m_currentZoom;
    };

} // namespace Caneda

#endif //CANEDA_IVIEW_H
