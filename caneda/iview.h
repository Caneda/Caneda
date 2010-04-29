#ifndef CANEDA_IVIEW_H
#define CANEDA_IVIEW_H

#include <QtGlobal>

// Forward declaration
class QWidget;

namespace Caneda
{
    // Forward declarations.
    class IContext;
    class IDocument;
    class DocumentViewManager;

    class IView
    {
    public:
        IView(IDocument *document);
        virtual ~IView();

        IDocument* document() const;

        virtual QWidget* toWidget() const = 0;
        virtual IContext* context() const = 0;

        virtual void setZoom(qreal percentage) = 0;

        virtual IView* duplicate() const = 0;

    // SIGNALS:
    protected:
        virtual void focussedIn(IView *who) = 0;
        virtual void closed(IView *who) = 0;

    protected:
        IDocument * const m_document;

        friend class DocumentViewManager;
    };
} // namespace Caneda

#endif //CANEDA_IVIEW_H
