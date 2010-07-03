/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef CANEDA_IDOCUMENT_H
#define CANEDA_IDOCUMENT_H

#include <QObject>
#include <QString>
#include <QVariant>

// Forward declarations
class QPaintDevice;
class QPrinter;

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

        virtual void selectAll() = 0;

        virtual bool printSupportsFitInPage() const = 0;
        virtual void print(QPrinter *printer, bool fitInPage) = 0;
        virtual void exportToPaintDevice(QPaintDevice *device,
                const QVariantMap &configuration) = 0;

        virtual bool load(QString *errorMessage = 0) = 0;
        virtual bool save(QString *errorMessage = 0) = 0;

        virtual IView* createView() = 0;

        virtual void updateSettingsChanges() = 0;

        QList<IView*> views() const;

        //TODO: Print specific interface methods

    public Q_SLOTS:
        void emitDocumentChanged();

    Q_SIGNALS:
        void documentChanged(IDocument *who);
        void statusBarMessage(const QString& text);

        // Avoid private declarations as subclasses might need direct access.
    protected:
        friend class DocumentViewManager;
        QString m_fileName;
    };
}

#endif // CANEDA_IDOCUMENT_H
