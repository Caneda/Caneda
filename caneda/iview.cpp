#include "iview.h"
#include "documentviewmanager.h"

namespace Caneda
{
    /*!
     * \class IView
     *
     * This class serves as an interface for any View visualization supported by
     * Caneda.
     */

    /*!
     * \fn IView::document()
     *
     * \brief Returns the document represented by this view.
     */

    /*!
     * \fn IView::toWidget()
     *
     * \brief Returns this view as a QWidget.
     * This has many advantages, like to add this view to tab widget,
     * connect signals/slots which expects a QObjet pointer etc.
     */

    /*!
     * \fn IView::context()
     *
     * \brief Returns the context that handles documents, views of specific type.
     * \note It is enough to create the context object only once per new type.
     * \see IContext
     */

    /*!
     * \fn IView::setZoom(qreal percentage)
     *
     * \brief Zooms the view to the value pointed by percentage argument.
     */

    IView::IView(IDocument *document) : m_document(document)
    {
        Q_ASSERT(document != 0);
    }

    IView::~IView()
    {

    }

    IDocument* IView::document() const
    {
        return m_document;
    }

} // namespace Caneda
