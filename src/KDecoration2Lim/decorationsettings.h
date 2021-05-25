#pragma once

#include "KDecoration2Lim/decorationdefines.h"

#include <KDecoration2/DecorationSettings>

namespace KDecoration2Lim {

using DecorationBridge = KDecoration2::DecorationBridge;

class DecorationSettings : public KDecoration2::DecorationSettings
{
    Q_OBJECT

    /**
    * The suggested ordering of the decoration buttons on the left.
    **/
    Q_PROPERTY(QVector<KDecoration2Lim::DecorationButtonType> decorationButtonsLeft READ decorationButtonsLeft NOTIFY decorationButtonsLeftChanged)
    /**
    * The suggested ordering of the decoration buttons on the right.
    **/
    Q_PROPERTY(QVector<KDecoration2Lim::DecorationButtonType> decorationButtonsRight READ decorationButtonsRight NOTIFY decorationButtonsRightChanged)
    /**
    * The spacing used between menu items in the menubar.
    **/
    Q_PROPERTY(int menuBarSpacing READ menuBarSpacing NOTIFY menuBarSpacingChanged)
    /**
    * Compact uses only on button to show the application menu, otherwise the menu is layed out in the titlebar.
    **/
    Q_PROPERTY(bool hasCompactApplicationMenu READ hasCompactApplicationMenu NOTIFY hasCompactApplicationMenuChanged)
    /**
    * Define how to show the caption. the options are Full and Compact.
    * Full will show application name and document title.
    * Compact will only show document title.
    **/
    Q_PROPERTY(CaptionFormat captionFormat READ captionFormat NOTIFY captionFormatChanged)

public:
    explicit DecorationSettings(DecorationBridge *bridge, QObject *parent = nullptr);

    QVector<DecorationButtonType> decorationButtonsLeft() const;
    QVector<DecorationButtonType> decorationButtonsRight() const;
    int menuBarSpacing() const;
    bool hasCompactApplicationMenu() const;
    CaptionFormat captionFormat() const;

Q_SIGNALS:
    void decorationButtonsLeftChanged(const QVector<KDecoration2Lim::DecorationButtonType> &);
    void decorationButtonsRightChanged(const QVector<KDecoration2Lim::DecorationButtonType> &);
    void menuBarSpacingChanged(int spacing);
    void hasCompactApplicationMenuChanged(bool compact);
    void captionFormatChanged(CaptionFormat format);

private:
    void setMenuBarSpacing(int spacing);
    void setHasCompactApplicationMenu(bool compact);
    void setCaptionFormat(CaptionFormat format);
    QVector<KDecoration2Lim::DecorationButtonType> toLimButtonVector(const QVector<KDecoration2::DecorationButtonType> &buttons) const;

    int m_menuBarSpacing;
    bool m_hasCompactApplicationMenu;
    CaptionFormat m_captionFormat;
};

} // namespace KDecoration2Lim

