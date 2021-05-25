#include "KDecoration2Lim/decorationbuttongroup.h"
#include "KDecoration2Lim/decoration.h"
#include "KDecoration2Lim/decorationbutton.h"
#include "KDecoration2Lim/decoratedclient.h"
#include "KDecoration2Lim/decorationsettings.h"

#include <QHoverEvent>
#include <QPainter>

namespace KDecoration2Lim {

using Position = DecorationButtonGroup::Position;

DecorationButtonGroup::DecorationButtonGroup(Position type, Decoration *parent, std::function<DecorationButton *(DecorationButtonType, Decoration *, QObject *)> buttonCreator)
    : KDecoration2::DecorationButtonGroup(parent)
    , m_decoration(parent)
    , m_type(type)
    , m_isHovered(false)
    , m_minimumSizeHint(geometry().size())
    , m_sizeHint(geometry().size())
    , m_overflowWidth(std::numeric_limits<qreal>::max())
    , m_appMenuModel(nullptr)
    , m_buttonCreator(buttonCreator)
{
    updateButtons();

    const auto *clientPtr = parent->client().toStrongRef().data();
    const auto &settings = parent->settings();
    const auto changed = type == Position::Left ? &DecorationSettings::decorationButtonsLeftChanged : &DecorationSettings::decorationButtonsRightChanged;
    connect(settings.data(), changed, this, &DecorationButtonGroup::updateButtons);

    if (type == Position::Menu) {
        connect(this, &DecorationButtonGroup::overflowWidthChanged, this, &DecorationButtonGroup::updateWidth);
        connect(clientPtr, &DecoratedClient::hasApplicationMenuChanged, this, &DecorationButtonGroup::updateButtons);
        connect(settings.data(), &DecorationSettings::hasCompactApplicationMenuChanged, this, &DecorationButtonGroup::updateButtons);
        connect(settings.data(), &DecorationSettings::decorationButtonsLeftChanged, this, &DecorationButtonGroup::updateButtons);
        connect(settings.data(), &DecorationSettings::decorationButtonsRightChanged, this, &DecorationButtonGroup::updateButtons);
        parent->installEventFilter(this);
    } else {
        const auto changed = type == Position::Left ? &DecorationSettings::decorationButtonsLeftChanged : &DecorationSettings::decorationButtonsRightChanged;
        connect(settings.data(), changed, this, &DecorationButtonGroup::updateButtons);
    }
}

QPointer<Decoration> DecorationButtonGroup::decoration() const
{
    return qobject_cast<Decoration*>(KDecoration2::DecorationButtonGroup::decoration());
}

DecorationButtonGroup::~DecorationButtonGroup() = default;

Position DecorationButtonGroup::type() const
{
    return m_type;
}

bool DecorationButtonGroup::isHovered() const
{
    return m_isHovered;
}

QSizeF DecorationButtonGroup::minimumSizeHint() const
{
    return m_minimumSizeHint;
}

QSizeF DecorationButtonGroup::sizeHint() const
{
    return m_sizeHint;
}


void DecorationButtonGroup::setHovered(bool hovered)
{
    if (m_isHovered == hovered) {
        return;
    }

    m_isHovered = hovered;
    emit hoveredChanged(m_isHovered);
}

qreal DecorationButtonGroup::overflowWidth() const
{
    return m_overflowWidth;
}

void DecorationButtonGroup::setOverflowWidth(qreal width)
{
    if (m_overflowWidth == width) {
        return;
    }

    m_overflowWidth = width;
    emit overflowWidthChanged(m_overflowWidth);
}

namespace
{
static bool s_layoutRecursion = false;
}

void DecorationButtonGroup::updateWidth()
{
    if (s_layoutRecursion) {
        return;
    }
    s_layoutRecursion = true;

    updateSizeHints();

    const auto *decorationPtr = decoration().data();
    const auto *clientPtr = decorationPtr->client().toStrongRef().data();
    if (!clientPtr->hasApplicationMenu()
            || decorationPtr->settings()->hasCompactApplicationMenu()) {
        s_layoutRecursion = false;
        return;
    }

    const auto &overflowButton = findButton(DecorationButtonType::Overflow);
    if (overflowButton.isNull()) {
        s_layoutRecursion = false;
        return;
    }

    bool isClipped = false;
    const auto &constButtons = buttons();
    qreal totalWidth = minimumSizeHint().width();
    for (const auto *it = constButtons.cbegin(); it != constButtons.cend(); ++it) {
        auto *button = static_cast<DecorationButton*>(it->data());
        if (button->type() != DecorationButtonType::ApplicationMenuEntry) {
            continue;
        }

        totalWidth += button->size().width();
        totalWidth += spacing();
        bool isLastMenuEntry = (*(it+1))->type() == DecorationButtonType::Overflow;
        if (isLastMenuEntry) {
            totalWidth -= overflowButton->size().width();
        }

        if (totalWidth > overflowWidth() && !isClipped) {
            overflowButton->setActionId(button->actionId());
            isClipped = true;
        }
        button->setVisible(!isClipped);
    }

    overflowButton->setVisible(isClipped);

    s_layoutRecursion = false;
}

void DecorationButtonGroup::updateSizeHints()
{
    qreal minWidth = 0.0;
    qreal preferedWidth = 0.0;
    const auto &constButtons = buttons();
    for (const auto *it = constButtons.cbegin(); it != constButtons.cend(); ++it) {
        const auto &button = *it;
        if (button->type() == DecorationButtonType::ApplicationMenuEntry) {
            preferedWidth += button->size().width();
            preferedWidth += spacing();
            continue;
        }

        if (!button->isVisible()
                && button->type() != DecorationButtonType::Overflow) {
            continue;
        }

        minWidth += button->size().width();
        minWidth += spacing();
        preferedWidth += button->size().width();
        preferedWidth += spacing();
    }

    minWidth = minWidth > 0.0 ? minWidth - spacing() : 0.0;
    preferedWidth = preferedWidth > 0.0 ? preferedWidth - spacing() : 0.0;

    QSizeF size = geometry().size();
    size.setWidth(minWidth);
    m_minimumSizeHint = size;

    size.setWidth(preferedWidth);
    m_sizeHint = size;
}

QVector<QPointer<DecorationButton>> DecorationButtonGroup::buttons() const
{
    QVector<QPointer<DecorationButton>> limButtons;
    const auto &constButtons = KDecoration2::DecorationButtonGroup::buttons();
    for (const auto &button : constButtons) {
        limButtons.append(qobject_cast<DecorationButton*>(button.data()));
    }

    return limButtons;
}

void DecorationButtonGroup::updateButtons()
{
    // remove all buttons
    const auto &constButtons = buttons();
    for (const auto &button : constButtons) {
        removeButton(button.data());
        button.data()->deleteLater();
    }

    // re-add all buttons
    const auto settings = m_decoration->settings();
    if (type() == Position::Menu) {
        if (!settings->hasCompactApplicationMenu()
                && (settings->decorationButtonsLeft() + settings->decorationButtonsRight())
                .contains(DecorationButtonType::ApplicationMenu)) {
            addApplicationMenuButtons();
        }
        updateWidth();
    } else {
        const auto &buttonTypes = (m_type == Position::Left) ? settings->decorationButtonsLeft() : settings->decorationButtonsRight();
        for (DecorationButtonType type : buttonTypes) {
            addButton(type, -1, QString());
        }
    }
}

void DecorationButtonGroup::addApplicationMenuButtons()
{
    const auto &clientPtr = m_decoration->client().toStrongRef();
    const auto &entries = clientPtr->applicationMenuEntries();
    for (const auto &entry : entries) {
        addButton(DecorationButtonType::ApplicationMenuEntry, entry.first, entry.second);
    }

    addButton(DecorationButtonType::Overflow, -1, QString());
}

void DecorationButtonGroup::addButton(DecorationButtonType type, int actionId, const QString &label)
{
    auto *button = m_buttonCreator(type, m_decoration.data(), this);
    button->setActionId(actionId);
    button->setLabel(label);
    KDecoration2::DecorationButtonGroup::addButton(button);
    connect(button, &DecorationButton::visibilityChanged, this, &DecorationButtonGroup::updateWidth);
    connect(button, &DecorationButton::geometryChanged, this, &DecorationButtonGroup::updateWidth);
}


QPointer<DecorationButton> DecorationButtonGroup::findButton(DecorationButtonType type) const
{
    const auto &constButtons = buttons();
    const auto it = std::find_if(constButtons.cbegin(), constButtons.cend(), [type](const QPointer<DecorationButton> &button) {
        return button->type() == type;
    });

    if (it == constButtons.end()) {
        return nullptr;
    }

    return *it;
}

bool DecorationButtonGroup::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != this->parent()) {
        return QObject::eventFilter(watched, event);
    }

    if (event->type() == QEvent::HoverMove) {
        QHoverEvent *hoverEvent = static_cast<QHoverEvent*>(event);
        setHovered(geometry().toRect().contains(hoverEvent->pos()));
    }

    return QObject::eventFilter(watched, event);
}

}
