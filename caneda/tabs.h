#ifndef CANEDA_TABCONTENT_H
#define CANEDA_TABCONTENT_H

#include <QTabWidget>

namespace Caneda
{
    // Forward declarations.
    class IView;
    class Manager;

    class Tab: public QWidget
    {
    Q_OBJECT
    public:
        Tab(IView *view, QWidget *parent = 0);
        ~Tab();

        IView* activeView() const;

        QString tabText() const;
        QIcon tabIcon() const;

        void closeView(IView *view);
        void splitView(IView *view, IView *newView,
                Qt::Orientation splitOrientation);

    public Q_SLOTS:
        void onViewFocussedIn(IView *view);

    protected:
        void closeEvent(QCloseEvent *event);

    private:
        void addView(IView *view);
        QList<IView*> m_views;

        friend class Manager;
    };

    class TabWidget : public QTabWidget
    {
    Q_OBJECT
    public:
        TabWidget(QWidget *parent = 0);
        QList<Tab*> tabs() const;

        Tab* tabForView(IView *view) const;

        void addTab(Tab *tab);
        void insertTab(int index, Tab *tab);

        Tab* currentTab() const;
        void setCurrentTab(Tab *tab);

        void highlightView(IView *view);
        void closeView(IView *view);
    };

} // namespace Caneda

#endif // CANEDA_TABCONTENT_H
