#include "KDecoration2Lim/decoration.h"
#include "KDecoration2Lim/decorationbuttongroup.h"
#include "KDecoration2Lim/decoratedclient.h"
#include "KDecoration2Lim/decorationsettings.h"
#include "KDecoration2Lim/appmenumodel.h"
#include "KDecoration2Lim/private/decoratedclientprivate.h"

#include <KDecoration2/Private/DecorationBridge>

#include <kwinglobals.h>

#include <QKeyEvent>
#include <QThread>
#include <compositor.h>
#include <compositor.h>
#include <plasmawindowmanagement.h>
#include <plasmawindowmanagement.h>
#include <registry.h>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/plasmawindowmanagement.h>
#include <KWaylandServer/plasmawindowmanagement_interface.h>
#include <KWaylandServer/surface_interface.h>
#include <KWaylandServer/compositor_interface.h>
#include <KWaylandServer/display.h>
#include <KWaylandServer/plasmawindowmanagement_interface.h>

#include <QApplication>

namespace KDecoration2Lim {

namespace  {

DecorationBridge *findBridge(const QVariantList &args)
{
    for (const auto &arg : args) {
        if (auto bridge = arg.toMap().value(QStringLiteral("bridge")).value<DecorationBridge *>()) {
            return bridge;
        }
    }
    Q_UNREACHABLE();
}

}

using KWayland::Client::ConnectionThread;
using KWayland::Client::Registry;
using KWayland::Client::PlasmaWindowManagement;

Decoration::Decoration (QObject *parent, const QVariantList &args)
    : KDecoration2::Decoration(parent, args)
    , m_settings(QSharedPointer<DecorationSettings>::create(findBridge(args)))
    , m_client(new DecoratedClient(KDecoration2::Decoration::client().toStrongRef().data()))
{
//    ConnectionThread *connection = new ConnectionThread;
//    connection->setSocketName("wayland-1");
//    QThread *thread = new QThread;
//    connection->moveToThread(thread);
//    thread->start();

//    connect(connection, &ConnectionThread::connected, [this, connection] {
//        qDebug() << "Successfully connected to Wayland server at socket:" << connection->socketName();
//        Registry registry;
//        registry.create(connection);
//        registry.setup();

//        this->connect(registry, &KWayland::Client::Registry::plasmaWindowManagementAnnounced, [this, registry](quint32 name, quint32 version) {
//            KWayland::Client::PlasmaWindowManagement *pwm = registry->createPlasmaWindowManagement(name, version, this);

//            WindowSystem::WaylandInterface *wI = qobject_cast<WindowSystem::WaylandInterface *>(m_wm);

//            if (wI) {
//                wI->initWindowManagement(pwm);
//            }
//        });

////        PlasmaWindowManagement *s = registry.createPlasmaWindowManagement(0, 0);
//        auto test = KWayland::Client::ConnectionThread::fromApplication();
//       qCDebug(category) << "CONNECTION=" << test;
//    });
//    connect(connection, &ConnectionThread::failed, [connection] {
//        qDebug() << "Failed to connect to Wayland server at socket:" << connection->socketName();
//    });
//    connection->initConnection();

    setupWaylandIntegration();

     //    installKeyLogger();
//    qApp->installNativeEventFilter(this);
//    initApplicationMenu();
//    m_client->windowId();
//    qCDebug(category) << "ID'S =" << m_client->windowId() << m_client->decorationId();

//    const auto &surfaces = KWaylandServer::SurfaceInterface::surfaces();
//    for (const auto &surface : surfaces) {
//        auto pwmi = new KWaylandServer::PlasmaWindowManagementInterface(surface->compositor()->display(), this);
//        qDebug() << "PWMI count = " << pwmi->windows().count();
//        qDebug() << "SOCKET NAMES =" << surface->compositor()->display()->socketNames();
//        qDebug() << "SURFACE=" << surface;
//    }
//     printChildren(QGuiApplication::instance());
}

void Decoration::setupWaylandIntegration()
{
    if (!KWindowSystem::isPlatformWayland()) {
        return;
    }

    using namespace KWayland::Client;

    ConnectionThread *connection = new ConnectionThread();
    connection->setSocketName("wayland-2");
    QThread *thread = new QThread;
    connection->moveToThread(thread);
    thread->start();

    connect(connection, &ConnectionThread::connected, this, [this, thread, connection] {
qCDebug(category) << "CONNECTED";
        Registry *registry{new Registry(this)};
        registry->create(connection);

        QObject::connect(registry, &Registry::plasmaWindowManagementAnnounced, this,
                         [this, registry](quint32 name, quint32 version) {
            PlasmaWindowManagement *pwm = registry->createPlasmaWindowManagement(name, version, this);
            connect(pwm, &PlasmaWindowManagement::windowCreated, this, []() {
                qCDebug(category) << "WINDOW CREATED";
            });
        }, Qt::QueuedConnection);

        registry->setup();
            auto connection2 = ConnectionThread::fromApplication(thread);
            qCDebug(category) << "CONNECTION =" << connection2 << qApp->platformNativeInterface();
//        connection->roundtrip();
    }, Qt::QueuedConnection);

    connection->initConnection();

//    printChildren(QGuiApplication::instance());
}

Decoration::~Decoration() = default;
void Decoration::printChildren(const QObject *object)
{
    const auto children = object->children();
    for (auto it = children.begin(); it != children.end(); ++it) {
        const QMetaObject *meta = (*it)->metaObject();
        qCDebug(category) << meta->className();
        printChildren(*it);
    }
}

QWeakPointer<DecoratedClient> Decoration::client() const
{
    return m_client.toWeakRef();
}

QVector<QPointer<DecorationButton>> Decoration::buttons() const
{
    QVector<QPointer<DecorationButton>> buttons;

    const auto &buttonGroups = findChildren<DecorationButtonGroup*>();
    for (const auto &buttonGroup : buttonGroups) {
        buttons += buttonGroup->buttons();
    }

    return buttons;
}

QPointer<DecorationButton> Decoration::findButton(DecorationButtonType type) const
{
    const auto &constButtons = buttons();
    const auto it = std::find_if(constButtons.cbegin(),
                                 constButtons.cend(),
                                 [type](const QPointer<DecorationButton> &button) {
        return button->type() == type;
    });

    if (it == constButtons.end()) {
        return nullptr;
    }

    return *it;
}

QSharedPointer<DecorationSettings> Decoration::settings() const
{
    return m_settings;
}

bool Decoration::event(QEvent *event)
{
    bool retval = KDecoration2::Decoration::event(event);

    // this enables moving the window by click and holding the left mouse button and move on a button
    if (event->type() == QEvent::MouseButtonPress) {
        const auto *mouseEvent = static_cast<QMouseEvent*>(event);
        const auto &constButtons = buttons();
        bool isAppMenuEntryButton = std::any_of(constButtons.begin(),
                                                        constButtons.end(),
                                                        [mouseEvent] (const QPointer<DecorationButton> &button) {
            return button->geometry().contains(mouseEvent->localPos())
                    && (button->type() == DecorationButtonType::ApplicationMenuEntry
                        || button->type() == DecorationButtonType::Overflow);
        });

        if (isAppMenuEntryButton) {
            event->setAccepted(false);
            return QObject::event(event);
        }
    }

    return retval;
}

void Decoration::showApplicationMenu(int actionId)
{
    const bool hasCompactApplicationMenu = settings()->hasCompactApplicationMenu();
    auto buttons = this->buttons();
    auto it = std::find_if(buttons.constBegin(),
                           buttons.constEnd(),
                           [actionId, hasCompactApplicationMenu](DecorationButton *button) {
            if (!button->isVisible()) {
                return false;
            }

            if (!hasCompactApplicationMenu) {
                return (button->type() == DecorationButtonType::ApplicationMenuEntry
                        && button->actionId() == actionId)
                        || (button->type() == DecorationButtonType::Overflow
                            && button->actionId() <= actionId);
            } else {
                return button->type() == DecorationButtonType::ApplicationMenu;
            }
    });

    if (it != buttons.constEnd()) {
        const auto button = *it;
        if (!hasCompactApplicationMenu) {
            requestShowApplicationSubMenu(button->geometry().toRect(), actionId, button->type() == DecorationButtonType::Overflow);
        } else {
            requestShowApplicationMenu(button->geometry().toRect(), actionId);
        }
    }
}

void Decoration::requestShowApplicationSubMenu(const QRect &rect, int actionId, bool isOverflowMenu)
{
    if (auto *appMenuEnabledPrivate = dynamic_cast<KDecoration2Lim::ApplicationMenuEnabledDecoratedClientPrivate *>(m_client->d.get())) {
         appMenuEnabledPrivate->requestShowApplicationSubMenu(rect, actionId, isOverflowMenu);
     }
}

//bool Decoration::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
//{
//    Q_UNUSED(result);
//    Q_UNUSED(eventType);

//    auto e = static_cast<xcb_generic_event_t *>(message);
//    const uint8_t type = e->response_type & ~0x80;
//    if (type == XCB_KEY_PRESS) {
//        auto *event = reinterpret_cast<xcb_key_press_event_t *>(e);
////        qCDebug(category) << "TYPE=" << type << "EVENT=" << event;
//        if (event) {
////        qCDebug(category) << "key pressed=" << event->event;
//        }
//    }
//    return false;
//}

//void Decoration::installKeyLogger()
//{
//    const auto children = QCoreApplication::instance()->children();
//    for (auto it = children.begin(); it != children.end(); ++it) {
//        const QMetaObject *meta = (*it)->metaObject();

//        if (qstrcmp(meta->className(), "KWin::InputRedirection") != 0) {
//            continue;
//        }

//        connect(*it, SIGNAL(keyStateChanged(quint32,InputRedirection::KeyboardKeyState)), this, SLOT(globalKeyEvent(quint32)), Qt::DirectConnection);
//    }
//}

//void Decoration::globalKeyEvent(quint32 key)
//{
//    Q_UNUSED(key);
////    qCDebug(category) << "Decoration::globalKeyEvent: key=" << key;
//}
}
