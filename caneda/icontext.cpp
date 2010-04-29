#include "icontext.h"

namespace Caneda
{
    /*!
     * \class IContext
     *
     * This class provides an interface for a context which is used by IDocument
     * and IView. This class also provides objects like toolbar, statusbar etc which
     * is specific to particular context.
     *
     * The context class can also be used to host functionalites shared by all
     * views and documents of same type.
     *
     * \see IDocument, IView
     */

    IContext::IContext(QObject *parent) : QObject(parent)
    {

    }

    IContext::~IContext()
    {

    }

    void IContext::init()
    {

    }

    QToolBar* IContext::toolBar()
    {
        return 0;
    }

    QWidget* IContext::statusBarWidget()
    {
        return 0;
    }

    QWidget* IContext::sideBarWidget(Caneda::SideBarRole role)
    {
        return 0;
    }


} // namespace Caneda
