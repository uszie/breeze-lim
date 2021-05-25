#pragma once

#include <kdecoration2/decorationdefines.h>
#include <QLoggingCategory>

namespace KDecoration2Lim
{
/**
 * The DecorationButtonType is a helper type for the DecorationButton.
 * A Decoration should provide a DecorationButton for each of the types,
 * if it wants to provide further buttons it should use the Custom type.
 * The DecorationButton gets configured depending on the type. E.g. the
 * Close button gets disabled if the DecoratedClient is not closeable.
 **/
enum class DecorationButtonType
{
    /**
     * The Menu button requests showing the window menu on left or right click.
     **/
    Menu,
    /**
     * The ApplicationMenu button requests showing the application's menu on left or right click.
     */
    ApplicationMenu,
    /**
     * The OnAllDesktops button requests toggling the DecoratedClient's on all desktops state.
     * The DecoratedButton is only visible if multiple virtual desktops are available.
     **/
    OnAllDesktops,
    /**
     * The Minimize button requests minimizing the DecoratedClient. The DecorationButton is only
     * enabled if the DecoratedClient is minimizeable.
     **/
    Minimize,
    /**
     * The Maximize button requests maximizing the DecoratedClient. The DecorationButton is checkable
     * and if the DecoratedClient is maximized the DecorationButton is checked. The DecorationButton
     * supports multiple mouse buttons to change horizontal, vertical and overall maximized state.
     *
     * The DecorationButton is only enabled if the DecoratedClient is maximizeable.
     **/
    Maximize,
    /**
     * The Close button requests closing the DecoratedClient. The DecorationButton is only enabled
     * if the DecoratedClient is closeable.
     **/
    Close,
    /**
     * The ContextHelp button requests entering the context help mode. The DecorationButton is only
     * visible if the DecoratedClient provides context help.
     **/
    ContextHelp,
    /**
     * The Shade button requests toggling the DecoratedClient's shaded state. The DecoratedButton
     * is only enabled if the DecoratedClient is shadeable.
     **/
    Shade,
    /**
     * The KeepBelow button requests toggling the DecoratedClient's keep below state.
     **/
    KeepBelow,
    /**
     * The KeepAbove button requests toggling the DecoratedClient's keep above state.
     **/
    KeepAbove,
    /**
     * The Custom type allows a Decoration to provide custom DecorationButtons.
     **/
    Custom,
    /**
     * The overflow button is visible when the local integrated application menu
     * doesn't fit. clicking it wil show the rest of the menu.
     **/
    Overflow,
    /**
     * The ApplicationMenuEntry button requests showing one entry of the application's menu on left or right click.
     */
    ApplicationMenuEntry,
};

using BorderSize = KDecoration2::BorderSize;
using ColorGroup = KDecoration2::ColorGroup;
using ColorRole =  KDecoration2::ColorRole;

enum class CaptionFormat
{
    Full,
    Compact,
};

inline KDecoration2::DecorationButtonType toBaseButtonType(DecorationButtonType type)
{
    if (type == DecorationButtonType::ApplicationMenuEntry || type == DecorationButtonType::Overflow) {
        return KDecoration2::DecorationButtonType::Custom;
    }

    return (KDecoration2::DecorationButtonType) type;
}

inline DecorationButtonType toLimButtonType(KDecoration2::DecorationButtonType type)
{
    return (DecorationButtonType) type;
}

static const QLoggingCategory category("kdecoration.breeze-lim");

}
