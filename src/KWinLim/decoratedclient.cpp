#include "KWinLim/decoratedclient.h"
#include "KDecoration2Lim/decoration.h"
#include "KDecoration2Lim/decoratedclient.h"
#include "KDecoration2Lim/appmenumodel.h"

//#include <KDecoration2/Decoration>

#include "config-breeze.h"

#if BREEZE_HAVE_X11
#include <QCoreApplication>
#include <QX11Info>
#endif

#include <QMenu>
#include <QAction>
#include <QPalette>
#include <QMouseEvent>
#include <QApplication>
#include <QTimer>

namespace KWin {
namespace Decoration {

// AppMenuModel;

DecoratedClientImpl::DecoratedClientImpl(KDecoration2::ApplicationMenuEnabledDecoratedClientPrivate *clientPrivate, KDecoration2Lim::DecoratedClient *decoratedClient, KDecoration2Lim::Decoration *decoration)
    : QObject()
    , ApplicationMenuEnabledDecoratedClientPrivate(decoratedClient, decoration)
    , m_clientPrivate(clientPrivate)
    , m_client(decoratedClient)
    , m_decoration(decoration)
    , m_appMenuModel(nullptr)
    , m_overflowMenu(nullptr)
    , m_activeActionId(-1)
    , m_isApplicationMenuActive(false)
{
//    QTimer::singleShot(000, this, [this]() {
        initApplicationMenu();
//    });

    connect(dynamic_cast<QObject*>(m_clientPrivate), &QObject::destroyed, this, &QObject::deleteLater);
}

#define GET_DELEGATE(type, method) \
type DecoratedClientImpl::method() const \
{ \
    return m_clientPrivate->method(); \
}

#define ACTION_DELEGATE(method) \
void DecoratedClientImpl::method() \
{ \
    m_clientPrivate->method(); \
}

#define ACTION_ARG_DELEGATE(type, method) \
void DecoratedClientImpl::method(type arg) \
{ \
    m_clientPrivate->method(arg); \
}

GET_DELEGATE(bool, isActive)
GET_DELEGATE(QString, caption)
GET_DELEGATE(int, desktop)
GET_DELEGATE(bool, isOnAllDesktops)
GET_DELEGATE(bool, isShaded)
GET_DELEGATE(QIcon, icon)
GET_DELEGATE(bool, isMaximized)
GET_DELEGATE(bool, isMaximizedHorizontally)
GET_DELEGATE(bool, isMaximizedVertically)
GET_DELEGATE(bool, isKeepAbove)
GET_DELEGATE(bool, isKeepBelow)
GET_DELEGATE(bool, isCloseable)
GET_DELEGATE(bool, isMaximizeable)
GET_DELEGATE(bool, isMinimizeable)
GET_DELEGATE(bool, providesContextHelp)
GET_DELEGATE(bool, isModal)
GET_DELEGATE(bool, isShadeable)
GET_DELEGATE(bool, isMoveable)
GET_DELEGATE(bool, isResizeable)
GET_DELEGATE(WId, windowId)
GET_DELEGATE(WId, decorationId)
GET_DELEGATE(int, width)
GET_DELEGATE(int, height)
GET_DELEGATE(QSize, size)
GET_DELEGATE(QPalette, palette)
GET_DELEGATE(Qt::Edges, adjacentScreenEdges)
ACTION_ARG_DELEGATE(const QString &, requestShowToolTip)
ACTION_DELEGATE(requestHideToolTip)
ACTION_DELEGATE(requestClose)
ACTION_ARG_DELEGATE(Qt::MouseButtons, requestToggleMaximization)
ACTION_DELEGATE(requestMinimize)
ACTION_DELEGATE(requestContextHelp)
ACTION_DELEGATE(requestToggleOnAllDesktops)
ACTION_DELEGATE(requestToggleShade)
ACTION_DELEGATE(requestToggleKeepAbove)
ACTION_DELEGATE(requestToggleKeepBelow)
ACTION_ARG_DELEGATE(const QRect &, requestShowWindowMenu)

#undef GET_DELEGATE
#undef ACTION_DELEGATE
#undef ACTION_ARG_DELEGATE

QColor KWin::Decoration::DecoratedClientImpl::color(KDecoration2::ColorGroup group, KDecoration2::ColorRole role) const
{
    return m_client->color(group, role);
}

bool DecoratedClientImpl::hasApplicationMenu() const
{
    return m_clientPrivate->hasApplicationMenu();
}

bool DecoratedClientImpl::isApplicationMenuActive() const
{
    return m_clientPrivate->isApplicationMenuActive() || activeApplicationSubMenu() >= 0;
}

QVector<QPair<int, QString>> DecoratedClientImpl::applicationMenuEntries() const
{
    return m_applicationMenuEntries;
}

int DecoratedClientImpl::activeApplicationSubMenu() const
{
    if (!m_isApplicationMenuActive) {
        return -1;
    }

    return m_activeActionId;
}

void DecoratedClientImpl::showApplicationMenu(int actionId)
{
    m_clientPrivate->showApplicationMenu(actionId);
}

void DecoratedClientImpl::requestShowApplicationMenu(const QRect &rect, int actionId)
{
//    m_clientPrivate->requestShowApplicationMenu(rect, actionId);
    requestShowApplicationSubMenu(rect, actionId, false);
}

void DecoratedClientImpl::requestShowApplicationSubMenu(const QRect &rect, int actionId, bool isOverflowMenu)
{
    auto *oldMenu = !m_overflowMenu.isNull() ? m_overflowMenu.data() :
                                               (applicationMenuEntry(m_activeActionId) ?
                                                    applicationMenuEntry(m_activeActionId)->menu() : nullptr);
    if (oldMenu) {
        bool oldMenuIsOverflow = oldMenu == m_overflowMenu.data();
        // Same menu requested ?
        if (m_activeActionId == actionId && oldMenuIsOverflow == isOverflowMenu) {
            if (!oldMenu->isVisible()) { // menus is not visible
                auto globalPoint = mapToGlobal(rect.bottomLeft());
                oldMenu->popup(globalPoint);
            }
            // do nothing
            return;
        } else {
            // other item requested close the old one
            oldMenu->close();
        }
    }

    QMenu *menu = nullptr;
    if (isOverflowMenu) {
        // compose overflow menu
        m_overflowMenu = new QMenu;
        m_overflowMenu->setTitle("OverflowMenu");
        m_overflowMenu->setObjectName("OverflowMenu");
        m_overflowMenu->setAttribute(Qt::WA_DeleteOnClose);
        m_overflowMenu->installEventFilter(this);
        connect(m_overflowMenu, &QMenu::aboutToShow, this, [this, actionId] { updateActiveAction(actionId, true); });
        connect(m_overflowMenu, &QMenu::aboutToHide, this, [this, actionId] { updateActiveAction(actionId, false); });

        const auto &menuEntries = applicationMenuEntries();
        // add all menu items where actionId is greater or equal to the requested actionId
        for (const auto &menuEntry : menuEntries) {
            if (menuEntry.first < actionId) {
                continue;
            }

            auto *action = applicationMenuEntry(menuEntry.first);
            if (!action || !action->menu()) {
                continue;
            }

            m_overflowMenu->addAction(action);
        }

        menu = m_overflowMenu;
    } else {
        auto *action = applicationMenuEntry(actionId);
        if (!action || !(menu = action->menu())) {
            return;
        }
    }

    // show menu
    auto globalPoint = mapToGlobal(rect.bottomLeft());
    menu->popup(globalPoint);
}

void DecoratedClientImpl::initApplicationMenu()
{
//    qDebug() << "DecoratedClientImpl::initApplicationMenu: PID =" << qApp->applicationPid() << qApp->applicationName();
    m_appMenuModel = new AppMenuModel(this);
    connect(m_appMenuModel, &AppMenuModel::modelReset, this, []() {
//       qDebug() << "RESETALKJDFLKSAJDLKASJDLKJAD";
    });
    connect(m_appMenuModel, &AppMenuModel::modelReset, this, &DecoratedClientImpl::updateApplicationMenuEntries, Qt::QueuedConnection);
return;

    if (QX11Info::isPlatformX11()) {
#if BREEZE_HAVE_X11
        WId windowId = m_client->windowId();

        if (windowId != 0) {
            m_appMenuModel = new AppMenuModel(this);
            connect(m_appMenuModel, &AppMenuModel::modelReset, this, &DecoratedClientImpl::updateApplicationMenuEntries, Qt::QueuedConnection);
//            m_appMenuModel->setWinId(windowId);
        }
#endif
    } else if (KWindowSystem::isPlatformWayland()) {
#if HAVE_Wayland
        // TODO
#endif
    }
}

QAction *DecoratedClientImpl::applicationMenuEntry(int actionId) const
{
    if (!m_appMenuModel || actionId < 0 || actionId >= m_appMenuModel->rowCount()) {
        return nullptr;
    }

    const QModelIndex index = m_appMenuModel->index(actionId, 0);
    const QVariant data = m_appMenuModel->data(index, AppMenuModel::ActionRole);
    QAction *action = (QAction *)data.value<void *>();

    return action;
}

void DecoratedClientImpl::updateApplicationMenuEntries()
{
    m_applicationMenuEntries.clear();

    for (int row = 0; row < m_appMenuModel->rowCount(); row++) {
        const auto *item = applicationMenuEntry(row);
        if (item->iconText().isEmpty()) {
            continue;
        }

        if (item->menu()) {
            // in case the same menu item is reused by the framwork.
            item->menu()->removeEventFilter(this);
            item->menu()->installEventFilter(this);
        }

        connect(item->menu(), &QMenu::aboutToShow, this, [this, row] { updateActiveAction(row, true); });
        connect(item->menu(), &QMenu::aboutToHide, this, [this, row] { updateActiveAction(row, false); });
        const auto pair = qMakePair(row, item->iconText());
        m_applicationMenuEntries.append(pair);
    }

    emit m_client->hasApplicationMenuChanged(hasApplicationMenu());
}

void DecoratedClientImpl::updateActiveAction(int actionId, bool active)
{
    if (m_activeActionId == actionId
            && m_isApplicationMenuActive == active) {
        return;
    }

    if (!m_overflowMenu.isNull()
            && m_overflowMenu->isVisible()
            && sender() != m_overflowMenu.data()) {
        return;
    }

    m_activeActionId = actionId;
    m_isApplicationMenuActive = active;
    emit m_client->applicationSubMenuActiveChanged(active, actionId);
}

//* scoped pointer convenience typedef
template <typename T> using ScopedPointer = QScopedPointer<T, QScopedPointerPodDeleter>;

QPoint DecoratedClientImpl::windowPos() const
{
#if BREEZE_HAVE_X11
    if( !QX11Info::isPlatformX11()) {
        return QPoint(0, 0);
    }

    // pointer to connection
    auto connection(QX11Info::connection());


    /*
get root position matching position
need to use xcb because the embedding of the widget
breaks QT's mapToGlobal and other methods
*/
//    QPoint rootPosition( position );
    xcb_get_geometry_cookie_t cookie(xcb_get_geometry(connection, windowId()));
    ScopedPointer<xcb_get_geometry_reply_t> reply( xcb_get_geometry_reply(connection, cookie, nullptr));
    if( reply ) {
        // translate coordinates
        xcb_translate_coordinates_cookie_t coordCookie(xcb_translate_coordinates(
                                                            connection, windowId(), reply.data()->root,
                                                            -reply.data()->border_width,
                                                            -reply.data()->border_width));

        ScopedPointer<xcb_translate_coordinates_reply_t> coordReply(xcb_translate_coordinates_reply(connection, coordCookie, nullptr));

        if (coordReply) {
            return QPoint(coordReply.data()->dst_x, coordReply.data()->dst_y);
        }
    }

#endif

    return QPoint(0, 0);
}

QPoint DecoratedClientImpl::mapToGlobal(const QPoint &pos) const
{
    QPoint globalPoint = pos + windowPos();
    globalPoint.setY(globalPoint.y() - decoration()->borderTop());

    return globalPoint;
}

QPoint DecoratedClientImpl::mapFromGlobal(const QPoint &pos) const
{
    QPoint localPoint = pos - windowPos();
    localPoint.setY(localPoint.y() + decoration()->borderTop());

    return localPoint;
}

bool DecoratedClientImpl::eventFilter(QObject *watched, QEvent *event)
{
    auto *menu = qobject_cast<QMenu *>(watched);
    if (!menu) {
        return false;
    }

    // popup menu steals input focus, this disable hover events on the buttons
    // redirect popup menu mouse move event to the decoration
    // this way hover move events are still received when the popup is open
    if (event->type() == QEvent::MouseMove) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        auto localPoint = mapFromGlobal(mouseEvent->globalPos());
        auto hoverEvent = QHoverEvent(QEvent::HoverMove, localPoint, localPoint, mouseEvent->modifiers());
        QCoreApplication::instance()->sendEvent(decoration() , &hoverEvent);
    } else if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        auto key = keyEvent->key();
        if (key == Qt::Key_Left || key == Qt::Key_Right) {
            // check if the active menu item has a submenu, if it does let the menu handle the event
            auto *activeAction = menu->activeAction();
            if (activeAction && m_overflowMenu && watched != m_overflowMenu) {
                return false;
            }

            if (activeAction && (activeAction->menu())
                    // check if the direction key matches the language layout direction
                    && ((qApp->layoutDirection() == Qt::LeftToRight && key == Qt::Key_Right)
                        || (qApp->layoutDirection() == Qt::RightToLeft && key == Qt::Key_Left))) {
                return false;
            }

            int requestedIndex = (key == Qt::Key_Left) ? m_activeActionId - 1 : m_activeActionId + 1;
            m_decoration->showApplicationMenu(requestedIndex);
            event->accept();
            return true;
        }
    }

    return false;
}

} // namespace Decoration
} // namespace KWin
