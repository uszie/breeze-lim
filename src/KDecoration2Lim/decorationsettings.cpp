#include "KDecoration2Lim/decorationsettings.h"

namespace KDecoration2Lim {

DecorationSettings::DecorationSettings(DecorationBridge *bridge, QObject *parent)
    : KDecoration2::DecorationSettings(bridge, parent)
    , m_menuBarSpacing(6)
    , m_hasCompactApplicationMenu(true)
    , m_captionFormat(CaptionFormat::Full)
{
    connect(this, &KDecoration2::DecorationSettings::decorationButtonsLeftChanged, this, [this](const QVector<KDecoration2::DecorationButtonType> &buttons) {
        Q_EMIT decorationButtonsLeftChanged(toLimButtonVector(buttons));
    });

    connect(this, &KDecoration2::DecorationSettings::decorationButtonsRightChanged, this, [this](const QVector<KDecoration2::DecorationButtonType> &buttons) {
        Q_EMIT decorationButtonsRightChanged(toLimButtonVector(buttons));
    });

    setHasCompactApplicationMenu(false);
    setCaptionFormat(CaptionFormat::Compact);
}

QVector<DecorationButtonType> DecorationSettings::decorationButtonsLeft() const
{
    return toLimButtonVector(KDecoration2::DecorationSettings::decorationButtonsLeft());
}

QVector<DecorationButtonType> DecorationSettings::decorationButtonsRight() const
{
    return toLimButtonVector(KDecoration2::DecorationSettings::decorationButtonsRight());
}

int DecorationSettings::menuBarSpacing() const
{
    return m_menuBarSpacing;
}

void DecorationSettings::setMenuBarSpacing(int spacing)
{
    if (m_menuBarSpacing == spacing) {
        return;
    }

    m_menuBarSpacing = spacing;
    Q_EMIT menuBarSpacingChanged(m_menuBarSpacing);
}

bool DecorationSettings::hasCompactApplicationMenu() const
{
    return m_hasCompactApplicationMenu;
}

void DecorationSettings::setHasCompactApplicationMenu(bool compact)
{
    if (m_hasCompactApplicationMenu == compact) {
        return;
    }

    m_hasCompactApplicationMenu = compact;
    Q_EMIT hasCompactApplicationMenuChanged(m_hasCompactApplicationMenu);
}

CaptionFormat DecorationSettings::captionFormat() const
{
    return m_captionFormat;
}

void DecorationSettings::setCaptionFormat(CaptionFormat format)
{
    if (m_captionFormat == format) {
        return;
    }

    m_captionFormat = format;
    Q_EMIT captionFormatChanged(m_captionFormat);
}

QVector<DecorationButtonType> DecorationSettings::toLimButtonVector(const QVector<KDecoration2::DecorationButtonType> &buttons) const
{
    QVector<DecorationButtonType> limButtons;
    for (const auto &button : buttons) {
        limButtons.append(toLimButtonType(button));
    }

    return limButtons;
}

} // namespace KDecoration2Lim
