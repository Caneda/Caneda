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

#include "messagewidget.h"

#include <QAction>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QShowEvent>
#include <QTimeLine>
#include <QToolButton>
#include <QStyle>

namespace Caneda
{
    /*************************************************************************
     *                       MessageWidgetPrivate                            *
     *************************************************************************/
    class MessageWidgetPrivate
    {
    public:
        void init(MessageWidget *);

        MessageWidget *q;
        QFrame *content;
        QLabel *iconLabel;
        QLabel *textLabel;
        QToolButton *closeButton;
        QTimeLine *timeLine;
        QIcon icon;

        MessageWidget::MessageType messageType;
        bool wordWrap;
        QList<QToolButton *> buttons;
        QPixmap contentSnapShot;

        void createLayout();
        void updateSnapShot();
        void updateLayout();
        void slotTimeLineChanged(qreal);
        void slotTimeLineFinished();

        int bestContentHeight() const;
    };

    void MessageWidgetPrivate::init(MessageWidget *q_ptr)
    {
        q = q_ptr;

        q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        timeLine = new QTimeLine(500, q);
        QObject::connect(timeLine, SIGNAL(valueChanged(qreal)), q, SLOT(slotTimeLineChanged(qreal)));
        QObject::connect(timeLine, SIGNAL(finished()), q, SLOT(slotTimeLineFinished()));

        content = new QFrame(q);
        content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        wordWrap = false;

        iconLabel = new QLabel(content);
        iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        iconLabel->hide();

        textLabel = new QLabel(content);
        textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        QObject::connect(textLabel, SIGNAL(linkActivated(QString)), q, SIGNAL(linkActivated(QString)));
        QObject::connect(textLabel, SIGNAL(linkHovered(QString)), q, SIGNAL(linkHovered(QString)));

        QAction *closeAction = new QAction(q);
        closeAction->setText(MessageWidget::tr("&Close"));
        closeAction->setToolTip(MessageWidget::tr("Close message"));
        closeAction->setIcon(q->style()->standardIcon(QStyle::SP_DialogCloseButton));

        QObject::connect(closeAction, SIGNAL(triggered(bool)), q, SLOT(animatedHide()));

        closeButton = new QToolButton(content);
        closeButton->setAutoRaise(true);
        closeButton->setDefaultAction(closeAction);

        q->setMessageType(MessageWidget::Information);
    }

    void MessageWidgetPrivate::createLayout()
    {
        delete content->layout();

        content->resize(q->size());

        qDeleteAll(buttons);
        buttons.clear();

        Q_FOREACH (QAction *action, q->actions()) {
            QToolButton *button = new QToolButton(content);
            button->setDefaultAction(action);
            button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            buttons.append(button);
        }

        // AutoRaise reduces visual clutter, but we don't want to turn it on if
        // there are other buttons, otherwise the close button will look different
        // from the others.
        closeButton->setAutoRaise(buttons.isEmpty());

        if (wordWrap) {
            QGridLayout *layout = new QGridLayout(content);
            // Set alignment to make sure icon does not move down if text wraps
            layout->addWidget(iconLabel, 0, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
            layout->addWidget(textLabel, 0, 1);

            QHBoxLayout *buttonLayout = new QHBoxLayout;
            buttonLayout->addStretch();
            Q_FOREACH (QToolButton *button, buttons) {
                // For some reason, calling show() is necessary if wordwrap is true,
                // otherwise the buttons do not show up. It is not needed if
                // wordwrap is false.
                button->show();
                buttonLayout->addWidget(button);
            }
            buttonLayout->addWidget(closeButton);
            layout->addItem(buttonLayout, 1, 0, 1, 2);
        } else {
            QHBoxLayout *layout = new QHBoxLayout(content);
            layout->addWidget(iconLabel);
            layout->addWidget(textLabel);

            Q_FOREACH (QToolButton *button, buttons) {
                layout->addWidget(button);
            }

            layout->addWidget(closeButton);
        };

        if (q->isVisible()) {
            q->setFixedHeight(content->sizeHint().height());
        }
        q->updateGeometry();
    }

    void MessageWidgetPrivate::updateLayout()
    {
        if (content->layout()) {
            createLayout();
        }
    }

    void MessageWidgetPrivate::updateSnapShot()
    {
        // Attention: updateSnapShot calls QWidget::render(), which causes the whole
        // window layouts to be activated. Calling this method from resizeEvent()
        // can lead to infinite recursion, see:
        // https://bugs.kde.org/show_bug.cgi?id=311336
        contentSnapShot = QPixmap(content->size() * q->devicePixelRatio());
        contentSnapShot.setDevicePixelRatio(q->devicePixelRatio());
        contentSnapShot.fill(Qt::transparent);
        content->render(&contentSnapShot, QPoint(), QRegion(), QWidget::DrawChildren);
    }

    void MessageWidgetPrivate::slotTimeLineChanged(qreal value)
    {
        q->setFixedHeight(qMin(value * 2, qreal(1.0)) * content->height());
        q->update();
    }

    void MessageWidgetPrivate::slotTimeLineFinished()
    {
        if (timeLine->direction() == QTimeLine::Forward) {
            // Show
            // We set the whole geometry here, because it may be wrong if a
            // MessageWidget is shown right when the toplevel window is created.
            content->setGeometry(0, 0, q->width(), bestContentHeight());

            // notify about finished animation
            emit q->showAnimationFinished();
        } else {
            // hide and notify about finished animation
            q->hide();
            emit q->hideAnimationFinished();
        }
    }

    int MessageWidgetPrivate::bestContentHeight() const
    {
        int height = content->heightForWidth(q->width());
        if (height == -1) {
            height = content->sizeHint().height();
        }
        return height;
    }


    /*************************************************************************
     *                           MessageWidget                               *
     *************************************************************************/
    MessageWidget::MessageWidget(QWidget *parent)
        : QFrame(parent)
        , d(new MessageWidgetPrivate)
    {
        d->init(this);
    }

    MessageWidget::MessageWidget(const QString &text, QWidget *parent)
        : QFrame(parent)
        , d(new MessageWidgetPrivate)
    {
        d->init(this);
        setText(text);
    }

    MessageWidget::~MessageWidget()
    {
        delete d;
    }

    QString MessageWidget::text() const
    {
        return d->textLabel->text();
    }

    void MessageWidget::setText(const QString &text)
    {
        d->textLabel->setText(text);
        updateGeometry();
    }

    MessageWidget::MessageType MessageWidget::messageType() const
    {
        return d->messageType;
    }

    static QColor darkShade(QColor c)
    {
        qreal contrast = 0.7; // taken from kcolorscheme for the dark shade

        qreal darkAmount;
        if (c.lightnessF() < 0.006) { /* too dark */
            darkAmount = 0.02 + 0.40 * contrast;
        } else if (c.lightnessF() > 0.93) { /* too bright */
            darkAmount = -0.06 - 0.60 * contrast;
        } else {
            darkAmount = (-c.lightnessF()) * (0.55 + contrast * 0.35);
        }

        qreal v = c.lightnessF() + darkAmount;
        v = v > 0.0 ? (v < 1.0 ? v : 1.0) : 0.0;
        c.setHsvF(c.hslHueF(), c.hslSaturationF(), v);
        return c;
    }

    void MessageWidget::setMessageType(MessageWidget::MessageType type)
    {
        d->messageType = type;
        QColor bg0, bg1, bg2, border, fg;
        switch (type) {
        case Positive:
            bg1.setRgb(0, 110,  40); // values taken from kcolorscheme.cpp (Positive)
            break;
        case Information:
            bg1 = palette().highlight().color();
            break;
        case Warning:
            bg1.setRgb(176, 128, 0); // values taken from kcolorscheme.cpp (Neutral)
            break;
        case Error:
            bg1.setRgb(191, 3, 3); // values taken from kcolorscheme.cpp (Negative)
            break;
        }

        // Colors
        fg = palette().highlightedText().color();
        bg0 = bg1.lighter(110);
        bg2 = bg1.darker(110);
        border = darkShade(bg1);

        d->content->setStyleSheet(
            QString(QLatin1String(".QFrame {"
                                  "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                  "    stop: 0 %1,"
                                  "    stop: 0.1 %2,"
                                  "    stop: 1.0 %3);"
                                  "border-radius: 5px;"
                                  "border: 1px solid %4;"
                                  "margin: %5px;"
                                  "}"
                                  ".QLabel { color: %6; }"
                                 ))
            .arg(bg0.name())
            .arg(bg1.name())
            .arg(bg2.name())
            .arg(border.name())
            // DefaultFrameWidth returns the size of the external margin + border width. We know our border is 1px, so we subtract this from the frame normal QStyle FrameWidth to get our margin
            .arg(style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, this) - 1)
            .arg(fg.name())
        );
    }

    QSize MessageWidget::sizeHint() const
    {
        ensurePolished();
        return d->content->sizeHint();
    }

    QSize MessageWidget::minimumSizeHint() const
    {
        ensurePolished();
        return d->content->minimumSizeHint();
    }

    bool MessageWidget::event(QEvent *event)
    {
        if (event->type() == QEvent::Polish && !d->content->layout()) {
            d->createLayout();
        }
        return QFrame::event(event);
    }

    void MessageWidget::resizeEvent(QResizeEvent *event)
    {
        QFrame::resizeEvent(event);

        if (d->timeLine->state() == QTimeLine::NotRunning) {
            d->content->resize(width(), d->bestContentHeight());
        }
    }

    int MessageWidget::heightForWidth(int width) const
    {
        ensurePolished();
        return d->content->heightForWidth(width);
    }

    void MessageWidget::paintEvent(QPaintEvent *event)
    {
        QFrame::paintEvent(event);
        if (d->timeLine->state() == QTimeLine::Running) {
            QPainter painter(this);
            painter.setOpacity(d->timeLine->currentValue() * d->timeLine->currentValue());
            painter.drawPixmap(0, 0, d->contentSnapShot);
        }
    }

    bool MessageWidget::wordWrap() const
    {
        return d->wordWrap;
    }

    void MessageWidget::setWordWrap(bool wordWrap)
    {
        d->wordWrap = wordWrap;
        d->textLabel->setWordWrap(wordWrap);
        QSizePolicy policy = sizePolicy();
        policy.setHeightForWidth(wordWrap);
        setSizePolicy(policy);
        d->updateLayout();
        // Without this, when user does wordWrap -> !wordWrap -> wordWrap, a minimum
        // height is set, causing the widget to be too high.
        // Mostly visible in test programs.
        if (wordWrap) {
            setMinimumHeight(0);
        }
    }

    bool MessageWidget::isCloseButtonVisible() const
    {
        return d->closeButton->isVisible();
    }

    void MessageWidget::setCloseButtonVisible(bool show)
    {
        d->closeButton->setVisible(show);
        updateGeometry();
    }

    void MessageWidget::addAction(QAction *action)
    {
        QFrame::addAction(action);
        d->updateLayout();
    }

    void MessageWidget::removeAction(QAction *action)
    {
        QFrame::removeAction(action);
        d->updateLayout();
    }

    void MessageWidget::animatedShow()
    {
        if (!style()->styleHint(QStyle::SH_Widget_Animate, 0, this)) {
            show();
            emit showAnimationFinished();
            return;
        }

        if (isVisible()) {
            return;
        }

        QFrame::show();
        setFixedHeight(0);
        int wantedHeight = d->bestContentHeight();
        d->content->setGeometry(0, -wantedHeight, width(), wantedHeight);

        d->updateSnapShot();

        d->timeLine->setDirection(QTimeLine::Forward);
        if (d->timeLine->state() == QTimeLine::NotRunning) {
            d->timeLine->start();
        }
    }

    void MessageWidget::animatedHide()
    {
        if (!style()->styleHint(QStyle::SH_Widget_Animate, 0, this)) {
            hide();
            emit hideAnimationFinished();
            return;
        }

        if (!isVisible()) {
            return;
        }

        d->content->move(0, -d->content->height());
        d->updateSnapShot();

        d->timeLine->setDirection(QTimeLine::Backward);
        if (d->timeLine->state() == QTimeLine::NotRunning) {
            d->timeLine->start();
        }
    }

    bool MessageWidget::isHideAnimationRunning() const
    {
        return (d->timeLine->direction() == QTimeLine::Backward)
            && (d->timeLine->state() == QTimeLine::Running);
    }

    bool MessageWidget::isShowAnimationRunning() const
    {
        return (d->timeLine->direction() == QTimeLine::Forward)
            && (d->timeLine->state() == QTimeLine::Running);
    }

    QIcon MessageWidget::icon() const
    {
        return d->icon;
    }

    void MessageWidget::setIcon(const QIcon &icon)
    {
        d->icon = icon;
        if (d->icon.isNull()) {
            d->iconLabel->hide();
        } else {
            const int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
            d->iconLabel->setPixmap(d->icon.pixmap(size));
            d->iconLabel->show();
        }
    }

    #include "moc_messagewidget.cpp"

} // namespace Caneda
