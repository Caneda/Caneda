#ifndef CANEDA_IVIEW_H
#define CANEDA_IVIEW_H

#include <QObject>

// Forward declaration
class QWidget;

namespace Caneda
{
    // Forward declarations.
    class IContext;
    class IDocument;
    class DocumentViewManager;

    class IView : public QObject
    {
        Q_OBJECT
    public:
        IView(IDocument *document);
        virtual ~IView();

        IDocument* document() const;

        virtual QWidget* toWidget() const = 0;
        virtual IContext* context() const = 0;

        virtual void setZoom(qreal percentage) = 0;

        virtual IView* duplicate() const = 0;

    Q_SIGNALS:
        void focussedIn(IView *who);
        void closed(IView *who);

    protected:
        IDocument * const m_document;

        friend class DocumentViewManager;
    };
} // namespace Caneda

#endif //CANEDA_IVIEW_H
