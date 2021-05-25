#include "KDecoration2Lim/decorationbutton.h"
#include "KDecoration2Lim/decoration.h"
#include "KDecoration2Lim/decoratedclient.h"

#include <QApplication>
#include <QMouseEvent>
#include <QTimer>

namespace KDecoration2Lim {

DecorationButton::DecorationButton(DecorationButtonType type, const QPointer<Decoration> &decoration, QObject *parent)
    : KDecoration2::DecorationButton( toBaseButtonType(type), qobject_cast<KDecoration2::Decoration *>(decoration), parent)
    , m_type(type)
{
    const auto *settingsPtr = decoration->settings().data();
    const auto *clientPtr = decoration->client().toStrongRef().data();

    if (type == DecorationButtonType::ApplicationMenuEntry) {
        setCheckable(true);

        QObject::connect(this, &DecorationButton::clicked, decoration.data(), [this, decoration] {
            decoration->requestShowApplicationSubMenu(geometry().toRect(), this->actionId());
        },
        Qt::QueuedConnection);

        connect(this, &DecorationButton::hoveredChanged, this, [this, decoration, clientPtr](bool hovered) {
            if (hovered && clientPtr->isApplicationMenuActive()) {
                decoration->requestShowApplicationSubMenu(geometry().toRect(), this->actionId());
            }
        });

        connect(clientPtr, &DecoratedClient::applicationSubMenuActiveChanged, this, [this](bool active, int actionId) {
            if (this->actionId() == actionId) {
                setChecked(active);
            } else if (isHovered()) {
                QTimer::singleShot(1, this, [this]() { update(); });
            }
        });
    } else if (type == DecorationButtonType::Overflow) {
        setCheckable(true);

        QObject::connect(this, &DecorationButton::clicked, decoration.data(), [this, decoration] {
            decoration->requestShowApplicationSubMenu(geometry().toRect(), this->actionId(), true);
        },
        Qt::QueuedConnection);

        connect(this, &DecorationButton::hoveredChanged, this, [this, decoration, clientPtr](bool hovered) {
            if (hovered && clientPtr->isApplicationMenuActive()) {
                decoration->requestShowApplicationSubMenu(geometry().toRect(), this->actionId(), true);
            }
        });

        connect(clientPtr, &DecoratedClient::applicationSubMenuActiveChanged, this, [this](bool active, int actionId) {
            if (this->actionId() == actionId) {
                setChecked(active);
            }
        });
    }

    if (type == DecorationButtonType::ApplicationMenu
            || type ==DecorationButtonType::ApplicationMenuEntry
            || type == DecorationButtonType::Overflow) {
        connect(clientPtr, &DecoratedClient::hasApplicationMenuChanged, this, &DecorationButton::updateApplicationMenuState);
        connect(settingsPtr, &DecorationSettings::hasCompactApplicationMenuChanged, this, &DecorationButton::updateApplicationMenuState);
        updateApplicationMenuState();
    }
}

DecorationButton::~DecorationButton() = default;

bool DecorationButton::event(QEvent *event)
{
    // when the window is moved by press and holding a menu button, the mouse release event is swallowed by
    // the hovermove event and the button stays in the pressed state. send a fake mouse release event
    bool result = KDecoration2::DecorationButton::event(event);
    if ((type() == DecorationButtonType::ApplicationMenuEntry || type() == DecorationButtonType::Overflow)
            && event->type() == QEvent::HoverLeave && isPressed()) {
        auto *hoverEvent = static_cast<QHoverEvent*>(event);
        auto mouseEvent = QMouseEvent(QEvent::MouseButtonRelease, hoverEvent->oldPos(), Qt::LeftButton, Qt::LeftButton, hoverEvent->modifiers());
        QCoreApplication::instance()->sendEvent(this, &mouseEvent);
    }

    return result;
}


void DecorationButton::updateApplicationMenuState()
{
    const auto *settingsPtr = decoration()->settings().data();
    auto *clientPtr = decoration()->client().toStrongRef().data();
    bool visible = clientPtr->hasApplicationMenu();
    if (type() == DecorationButtonType::ApplicationMenu) {
        visible = visible && settingsPtr->hasCompactApplicationMenu();
    } else if (type() == DecorationButtonType::ApplicationMenuEntry
               || type() == DecorationButtonType::Overflow) {
        visible = visible && !settingsPtr->hasCompactApplicationMenu();
    }

    setVisible(visible);
}

QPointer<Decoration> DecorationButton::decoration() const
{
    return qobject_cast<Decoration*>(KDecoration2::DecorationButton::decoration());
}

DecorationButtonType DecorationButton::type() const
{
    return m_type;
}

int DecorationButton::actionId() const
{
    return m_actionId;
}

void DecorationButton::setActionId(int actionId)
{
    if (m_actionId == actionId) {
        return;
    }

    m_actionId = actionId;
}

const QString &DecorationButton::label() const
{
    return m_label;
}

void DecorationButton::setLabel(const QString &label)
{
    if (m_label == label) {
        return;
    }

    m_label = label;
    emit labelChanged(m_label);
}

}
