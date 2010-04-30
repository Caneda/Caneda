#ifndef CANEDA_IDOCUMENT_H
#define CANEDA_IDOCUMENT_H

#include <QObject>
#include <QString>

namespace Caneda
{
    // Forward declarations
    class DocumentViewManager;
    class IContext;
    class IView;

    class IDocument : public QObject
    {
        Q_OBJECT
    public:
        IDocument();
        virtual ~IDocument();

        QString fileName() const;
        void setFileName(const QString &fileName);

        // Virtual methods.
        virtual IContext* context() = 0;

        virtual bool isModified() const = 0;

        virtual bool canUndo() const = 0;
        virtual bool canRedo() const = 0;

        virtual void undo() = 0;
        virtual void redo() = 0;

        virtual bool canCut() const = 0;
        virtual bool canCopy() const = 0;
        virtual bool canPaste() const = 0;

        virtual void cut() = 0;
        virtual void copy() = 0;
        virtual void paste() = 0;

        virtual bool load(QString *errorMessage = 0) = 0;
        virtual bool save(QString *errorMessage = 0) = 0;

        QList<IView*> views() const;

        //TODO: Print specific interface methods

    Q_SIGNALS:
        void documentChanged();
        void statusBarMessage(const QString& text);

        // Avoid private declarations as subclasses might need direct access.
    protected:
        friend class DocumentViewManager;
        QString m_fileName;
    };
}

#endif // CANEDA_IDOCUMENT_H
