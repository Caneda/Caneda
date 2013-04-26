/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2013 by Pablo Daniel Pareja Obregon                       *
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
#include <QVariant>

// Forward declarations
class QPaintDevice;
class QPrinter;
class QUndoStack;

namespace Caneda
{
    // Forward declarations
    class DocumentViewManager;
    class IContext;
    class IView;

    /*!
     * \brief This class represents the actual document interface
     * (scene), in a manner similar to Qt's Graphics View Architecture,
     * serving as an interface for all documents that can be handled by
     * Caneda. This class manages document specific methods like saving,
     * loading, exporting to different formats, as well as containing the
     * actual scene. The scene itself may be included as a pointer to
     * another class that contains all the scene specific methods (for
     * example a graphics scene). The scene, in its turn, serves as a
     * container for item objects and handles their manipulation.
     *
     * \sa IContext, IView, \ref DocumentViewFramework
     */
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

        virtual QUndoStack* undoStack() = 0;

        virtual bool canCut() const = 0;
        virtual bool canCopy() const = 0;
        virtual bool canPaste() const = 0;

        virtual void cut() = 0;
        virtual void copy() = 0;
        virtual void paste() = 0;

        virtual void selectAll() = 0;

        virtual void intoHierarchy() = 0;
        virtual void popHierarchy() = 0;

        virtual void alignTop() = 0;
        virtual void alignBottom() = 0;
        virtual void alignLeft() = 0;
        virtual void alignRight() = 0;
        virtual void distributeHorizontal() = 0;
        virtual void distributeVertical() = 0;
        virtual void centerHorizontal() = 0;
        virtual void centerVertical() = 0;

        virtual void simulate() = 0;

        virtual bool printSupportsFitInPage() const = 0;
        virtual void print(QPrinter *printer, bool fitInPage) = 0;
        virtual void exportImage(QPaintDevice &device) = 0;
        virtual QSizeF documentSize() = 0;

        virtual bool load(QString *errorMessage = 0) = 0;
        virtual bool save(QString *errorMessage = 0) = 0;

        virtual IView* createView() = 0;
        QList<IView*> views() const;

        virtual void launchPropertiesDialog() = 0;

    public Q_SLOTS:
        void emitDocumentChanged();

    Q_SIGNALS:
        void documentChanged(IDocument *who);
        void statusBarMessage(const QString &text);

        // Avoid private declarations as subclasses might need direct access.
    protected:
        friend class DocumentViewManager;
        QString m_fileName;

        void setNormalAction();
    };

} // namespace Caneda

#endif //CANEDA_IDOCUMENT_H
