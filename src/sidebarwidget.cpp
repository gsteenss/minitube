/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2009, Flavio Tordini <flavio.tordini@gmail.com>

Minitube is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Minitube is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Minitube.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#include "sidebarwidget.h"
#include "refinesearchbutton.h"
#include "refinesearchwidget.h"
#include "sidebarheader.h"
#ifdef APP_EXTRA
#include "extra.h"
#endif

namespace The {
QHash<QString, QAction*>* globalActions();
}

SidebarWidget::SidebarWidget(QWidget *parent) :
    QWidget(parent) {
    playlist = 0;

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);

    sidebarHeader = new SidebarHeader();
    layout->addWidget(sidebarHeader);

    // hidden message widget
    messageLabel = new QLabel(this);
    messageLabel->setMargin(10);
    messageLabel->setBackgroundRole(QPalette::ToolTipBase);
    messageLabel->setForegroundRole(QPalette::ToolTipText);
    messageLabel->setAutoFillBackground(true);
    messageLabel->setWordWrap(true);
    messageLabel->setTextFormat(Qt::RichText);
    messageLabel->setTextInteractionFlags(
                Qt::LinksAccessibleByKeyboard |
                Qt::LinksAccessibleByMouse);
    connect(messageLabel, SIGNAL(linkActivated(QString)),
            SIGNAL(suggestionAccepted(QString)));
    messageLabel->hide();
    layout->addWidget(messageLabel);

    stackedWidget = new QStackedWidget(this);
    layout->addWidget(stackedWidget);

    setup();
}

void SidebarWidget::setup() {
    refineSearchButton = new RefineSearchButton(this);
    refineSearchButton->setStatusTip(tr("Refine Search")
                                     + " (" + QKeySequence(Qt::CTRL + Qt::Key_R).toString(QKeySequence::NativeText) + ")");
    refineSearchButton->hide();
    connect(refineSearchButton, SIGNAL(clicked()), SLOT(showRefineSearchWidget()));

    refineSearchWidget = new RefineSearchWidget(this);
    connect(refineSearchWidget, SIGNAL(done()), SLOT(hideRefineSearchWidget()));
    stackedWidget->addWidget(refineSearchWidget);

    setMouseTracking(true);
    mouseTimer = new QTimer(this);
    mouseTimer->setInterval(5000);
    mouseTimer->setSingleShot(true);
    connect(mouseTimer, SIGNAL(timeout()), refineSearchButton, SLOT(hide()));
}

void SidebarWidget::setPlaylist(QListView *playlist) {
    this->playlist = playlist;
    playlist->installEventFilter(this);
    stackedWidget->addWidget(playlist);
}

void SidebarWidget::showPlaylist() {
    stackedWidget->setCurrentWidget(playlist);
    The::globalActions()->value("refine-search")->setChecked(false);
}

void SidebarWidget::showRefineSearchWidget() {
    if (!refineSearchWidget->isEnabled()) return;
    refineSearchWidget->setDirty(false);
    stackedWidget->setCurrentWidget(refineSearchWidget);
    // refineSearchWidget->setFocus();
#ifdef APP_EXTRA
    Extra::fadeInWidget(playlist, refineSearchWidget);
#endif
    refineSearchButton->hide();
    The::globalActions()->value("refine-search")->setChecked(true);
}

void SidebarWidget::hideRefineSearchWidget() {
    stackedWidget->setCurrentWidget(playlist);
    playlist->setFocus();
#ifdef APP_EXTRA
    Extra::fadeInWidget(refineSearchWidget, playlist);
#endif
    The::globalActions()->value("refine-search")->setChecked(false);
}

void SidebarWidget::toggleRefineSearch(bool show) {
    if (show) showRefineSearchWidget();
    else hideRefineSearchWidget();
}

void SidebarWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    refineSearchButton->move(
                playlist->viewport()->width() - refineSearchButton->minimumWidth(),
                height() - refineSearchButton->minimumHeight());
}

void SidebarWidget::enterEvent(QEvent *) {
    if (stackedWidget->currentWidget() != refineSearchWidget)
        showRefineSearchButton();
}

void SidebarWidget::leaveEvent(QEvent *) {
    refineSearchButton->hide();
}

void SidebarWidget::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    handleMouseMove();
}

bool SidebarWidget::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseMove) handleMouseMove();
    return QWidget::eventFilter(obj, event);
}

void SidebarWidget::handleMouseMove() {
    if (stackedWidget->currentWidget() != refineSearchWidget) {
        showRefineSearchButton();
        mouseTimer->start();
    }
}

void SidebarWidget::showRefineSearchButton() {
    if (!refineSearchWidget->isEnabled()) return;
    refineSearchButton->move(
                playlist->viewport()->width() - refineSearchButton->minimumWidth(),
                height() - refineSearchButton->minimumHeight());
    refineSearchButton->show();
}

void SidebarWidget::showSuggestions(const QStringList &suggestions) {
    QString message = tr("Did you mean: %1");

    QString suggestionLinks;
    foreach (QString suggestion, suggestions) {
        suggestionLinks += "<a href='" + suggestion + "'>" + suggestion + "</a> ";
    }
    message = message.arg(suggestionLinks);

    QString html =
            "<html>"
            "<style>"
            "a { color: palette(text); text-decoration: none; font-weight: bold }"
            "</style>"
            "<body>%1</body>"
            "</html>";
    html = html.arg(message);
    messageLabel->setText(html);
    messageLabel->show();
}

void SidebarWidget::hideSuggestions() {
    messageLabel->hide();
    messageLabel->clear();
}
