/***************************************************************************
 * Copyright (C) 2011 Aurélien Gâteau <agateau@kde.org>                    *
 * Copyright (C) 2014 Dominik Haumann <dhaumann@kde.org>                   *
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

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <QFrame>

namespace Caneda
{
    // Forward declarations
    class MessageWidgetPrivate;

    /*!
     * \brief A widget to provide feedback or propose opportunistic interactions.
     *
     * MessageWidget can be used to provide inline positive or negative
     * feedback, or to implement opportunistic interactions.
     *
     * As a feedback widget, MessageWidget provides a less intrusive alternative
     * to "OK Only" message boxes. If you want to avoid a modal MessageBox,
     * consider using MessageWidget instead.
     *
     * <b>Negative feedback</b>
     *
     * The MessageWidget can be used as a secondary indicator of failure: the
     * first indicator is usually the fact the action the user expected to happen
     * did not happen.
     *
     * Example: User fills a form, clicks "Submit".
     *
     * @li Expected feedback: form closes
     * @li First indicator of failure: form stays there
     * @li Second indicator of failure: a MessageWidget appears on top of the
     * form, explaining the error condition
     *
     * When used to provide negative feedback, MessageWidget should be placed
     * close to its context. In the case of a form, it should appear on top of the
     * form entries.
     *
     * MessageWidget should get inserted in the existing layout. Space should not
     * be reserved for it, otherwise it becomes "dead space", ignored by the user.
     * MessageWidget should also not appear as an overlay to prevent blocking
     * access to elements the user needs to interact with to fix the failure.
     *
     * <b>Positive feedback</b>
     *
     * MessageWidget can be used for positive feedback but it shouldn't be
     * overused. It is often enough to provide feedback by simply showing the
     * results of an action.
     *
     * Examples of acceptable uses:
     *
     * @li Confirm success of "critical" transactions
     * @li Indicate completion of background tasks
     *
     * Example of unadapted uses:
     *
     * @li Indicate successful saving of a file
     * @li Indicate a file has been successfully removed
     *
     * <b>Opportunistic interaction</b>
     *
     * Opportunistic interaction is the situation where the application suggests to
     * the user an action he could be interested in perform, either based on an
     * action the user just triggered or an event which the application noticed.
     *
     * Example of acceptable uses:
     *
     * @li A browser can propose remembering a recently entered password
     * @li A music collection can propose ripping a CD which just got inserted
     * @li A chat application may notify the user a "special friend" just connected
     *
     * @author Aurélien Gâteau <agateau@kde.org>
     */
    class MessageWidget : public QFrame
    {
        Q_OBJECT
        Q_ENUMS(MessageType)

        Q_PROPERTY(QString text READ text WRITE setText)
        Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap)
        Q_PROPERTY(bool closeButtonVisible READ isCloseButtonVisible WRITE setCloseButtonVisible)
        Q_PROPERTY(MessageType messageType READ messageType WRITE setMessageType)
        Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    public:

        /**
         * Available message types.
         * The background colors are chosen depending on the message type.
         */
        enum MessageType {
            Positive,
            Information,
            Warning,
            Error
        };

        /**
         * Constructs a MessageWidget with the specified @p parent.
         */
        explicit MessageWidget(QWidget *parent = 0);

        /**
         * Constructs a MessageWidget with the specified @p parent and
         * contents @p text.
         */
        explicit MessageWidget(const QString &text, QWidget *parent = 0);

        /**
         * Destructor.
         */
        ~MessageWidget();

        /**
         * Get the text of this message widget.
         * @see setText()
         */
        QString text() const;

        /**
         * Check whether word wrap is enabled.
         *
         * If word wrap is enabled, the message widget wraps the displayed text
         * as required to the available width of the widget. This is useful to
         * avoid breaking widget layouts.
         *
         * @see setWordWrap()
         */
        bool wordWrap() const;

        /**
         * Check whether the close button is visible.
         *
         * @see setCloseButtonVisible()
         */
        bool isCloseButtonVisible() const;

        /**
         * Get the type of this message.
         * By default, the type is set to MessageWidget::Information.
         *
         * @see MessageWidget::MessageType, setMessageType()
         */
        MessageType messageType() const;

        /**
         * Add @p action to the message widget.
         * For each action a button is added to the message widget in the
         * order the actions were added.
         *
         * @param action the action to add
         * @see removeAction(), QWidget::actions()
         */
        void addAction(QAction *action);

        /**
         * Remove @p action from the message widget.
         *
         * @param action the action to remove
         * @see MessageWidget::MessageType, addAction(), setMessageType()
         */
        void removeAction(QAction *action);

        /**
         * Returns the preferred size of the message widget.
         */
        QSize sizeHint() const Q_DECL_OVERRIDE;

        /**
         * Returns the minimum size of the message widget.
         */
        QSize minimumSizeHint() const Q_DECL_OVERRIDE;

        /**
         * Returns the required height for @p width.
         * @param width the width in pixels
         */
        int heightForWidth(int width) const Q_DECL_OVERRIDE;

        /**
         * The icon shown on the left of the text. By default, no icon is shown.
         */
        QIcon icon() const;

        /**
         * Check whether the hide animation started by calling animatedHide()
         * is still running. If animations are disabled, this function always
         * returns @e false.
         *
         * @see animatedHide(), hideAnimationFinished()
         */
        bool isHideAnimationRunning() const;

        /**
         * Check whether the show animation started by calling animatedShow()
         * is still running. If animations are disabled, this function always
         * returns @e false.
         *
         * @see animatedShow(), showAnimationFinished()
         */
        bool isShowAnimationRunning() const;

    public Q_SLOTS:
        /**
         * Set the text of the message widget to @p text.
         * If the message widget is already visible, the text changes on the fly.
         *
         * @param text the text to display, rich text is allowed
         * @see text()
         */
        void setText(const QString &text);

        /**
         * Set word wrap to @p wordWrap. If word wrap is enabled, the text()
         * of the message widget is wrapped to fit the available width.
         * If word wrap is disabled, the message widget's minimum size is
         * such that the entire text fits.
         *
         * @param wordWrap disable/enable word wrap
         * @see wordWrap()
         */
        void setWordWrap(bool wordWrap);

        /**
         * Set the visibility of the close button. If @p visible is @e true,
         * a close button is shown that calls animatedHide() if clicked.
         *
         * @see closeButtonVisible(), animatedHide()
         */
        void setCloseButtonVisible(bool visible);

        /**
         * Set the message type to @p type.
         * By default, the message type is set to MessageWidget::Information.
         *
         * @see messageType(), MessageWidget::MessageType
         */
        void setMessageType(MessageWidget::MessageType type);

        /**
         * Show the widget using an animation.
         */
        void animatedShow();

        /**
         * Hide the widget using an animation.
         */
        void animatedHide();

        /**
         * Define an icon to be shown on the left of the text
         */
        void setIcon(const QIcon &icon);

    Q_SIGNALS:
        /**
         * This signal is emitted when the user clicks a link in the text label.
         * The URL referred to by the href anchor is passed in contents.
         * @param contents text of the href anchor
         * @see QLabel::linkActivated()
         */
        void linkActivated(const QString &contents);

        /**
         * This signal is emitted when the user hovers over a link in the text label.
         * The URL referred to by the href anchor is passed in contents.
         * @param contents text of the href anchor
         * @see QLabel::linkHovered()
         */
        void linkHovered(const QString &contents);

        /**
         * This signal is emitted when the hide animation is finished, started by
         * calling animatedHide(). If animations are disabled, this signal is
         * emitted immediately after the message widget got hidden.
         *
         * @note This signal is @e not emitted if the widget was hidden by
         *       calling hide(), so this signal is only useful in conjunction
         *       with animatedHide().
         *
         * @see animatedHide()
         */
        void hideAnimationFinished();

        /**
         * This signal is emitted when the show animation is finished, started by
         * calling animatedShow(). If animations are disabled, this signal is
         * emitted immediately after the message widget got shown.
         *
         * @note This signal is @e not emitted if the widget was shown by
         *       calling show(), so this signal is only useful in conjunction
         *       with animatedShow().
         *
         * @see animatedShow()
         */
        void showAnimationFinished();

    protected:
        void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

        bool event(QEvent *event) Q_DECL_OVERRIDE;

        void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

    private:
        MessageWidgetPrivate *const d;
        friend class MessageWidgetPrivate;

        Q_PRIVATE_SLOT(d, void slotTimeLineChanged(qreal))
        Q_PRIVATE_SLOT(d, void slotTimeLineFinished())
    };

} // namespace Caneda

#endif //MESSAGEWIDGET_H
