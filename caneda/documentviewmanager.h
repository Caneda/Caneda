#ifndef CANEDA_DOCUMENTVIEWMANAGER_H
#define CANEDA_DOCUMENTVIEWMANAGER_H

#include <QObject>

namespace Caneda
{
    class DocumentData;
    class IContext;
    class IDocument;
    class IView;
    class SingletonManager;
    class TabWidget;

    class DocumentViewManager : public QObject
    {
        Q_OBJECT
    public:
        static DocumentViewManager* instance();
        ~DocumentViewManager();

        void highlightView(IView *view);
        void highlightViewForDocument(IDocument *document);

        bool openFile(const QString &fileName);
        bool closeFile(const QString &fileName);

        bool saveDocuments(const QList<IDocument*> &documents);
        bool closeDocuments(const QList<IDocument*> &documents);

        bool splitView(IView *view, Qt::Orientation orientation);

        bool saveView(IView *view);
        bool closeView(IView *view);

        IDocument* currentDocument() const;
        IView* currentView() const;

        IDocument* documentForFileName(const QString &fileName) const;
        QList<IView*> viewsForDocument(const IDocument *document) const;

    private Q_SLOTS:
        void onViewFocussedIn(IView *who);

    private:
        DocumentViewManager(QObject *parent = 0);
        DocumentData* documentDataForFileName(const QString &fileName) const;
        DocumentData* documentDataForDocument(IDocument *document) const;
        bool closeViewHelper(IView *view, bool askForSave, bool closeDocumentIfLastView);

        void setupContexts();
        TabWidget* tabWidget() const;

        QList<DocumentData*> m_documentDataList;
        QList<IContext*> m_contexts;

        friend class SingletonManager;
    };
} // namespace Caneda

#endif //CANEDA_DOCUMENTVIEWMANAGER_H
