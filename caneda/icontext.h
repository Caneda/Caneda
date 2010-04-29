#ifndef CANEDA_ICONTEXT_H
#define CANEDA_ICONTEXT_H

#include "globals.h"

#include <QObject>

// Forward declaration
class QFileInfo;
class QToolBar;
class QWidget;

namespace Caneda
{
    // Forward declarations.
    class IDocument;
    class IView;

    class IContext : public QObject
    {
    Q_OBJECT
    public:
        IContext(QObject *parent = 0);
        virtual ~IContext();

        virtual void init();

        virtual QToolBar* toolBar();
        virtual QWidget* statusBarWidget();
        virtual QWidget* sideBarWidget(Caneda::SideBarRole role);

        virtual bool canOpen(const QFileInfo& info) const = 0;
        virtual QStringList fileNameFilters() const = 0;

        virtual IDocument* open(const QString& filename, QString *errorMessage = 0) = 0;
        virtual IView* createView(IDocument *document) = 0;

    };
} // namespace Caneda

#endif //CANEDA_ICONTEXT_H
