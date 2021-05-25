#define private public

#include "KDecoration2Lim/decoratedclient.h"
#include "KDecoration2Lim/decoration.h"
#include "KWinLim/decoratedclient.h"

#include <KDecoration2/Private/DecoratedClientPrivate>

namespace KDecoration2Lim
{

DecoratedClient::DecoratedClient(KDecoration2::DecoratedClient *client)
    : QObject()
    , m_decoration(static_cast<Decoration*>(client->decoration().data()))
    , m_client(client)
{
    connect(m_client, &KDecoration2::DecoratedClient::activeChanged, this , &DecoratedClient::activeChanged);
    connect(m_client, &KDecoration2::DecoratedClient::captionChanged, this , &DecoratedClient::captionChanged);
    connect(m_client, &KDecoration2::DecoratedClient::onAllDesktopsChanged, this , &DecoratedClient::onAllDesktopsChanged);
    connect(m_client, &KDecoration2::DecoratedClient::shadedChanged, this , &DecoratedClient::shadedChanged);
    connect(m_client, &KDecoration2::DecoratedClient::iconChanged, this , &DecoratedClient::iconChanged);
    connect(m_client, &KDecoration2::DecoratedClient::maximizedChanged, this , &DecoratedClient::maximizedChanged);
    connect(m_client, &KDecoration2::DecoratedClient::maximizedHorizontallyChanged, this , &DecoratedClient::maximizedHorizontallyChanged);
    connect(m_client, &KDecoration2::DecoratedClient::maximizedVerticallyChanged, this , &DecoratedClient::maximizedVerticallyChanged);
    connect(m_client, &KDecoration2::DecoratedClient::keepAboveChanged, this , &DecoratedClient::keepAboveChanged);
    connect(m_client, &KDecoration2::DecoratedClient::keepBelowChanged, this , &DecoratedClient::keepBelowChanged);
    connect(m_client, &KDecoration2::DecoratedClient::closeableChanged, this , &DecoratedClient::closeableChanged);
    connect(m_client, &KDecoration2::DecoratedClient::maximizeableChanged, this , &DecoratedClient::maximizeableChanged);
    connect(m_client, &KDecoration2::DecoratedClient::minimizeableChanged, this , &DecoratedClient::minimizeableChanged);
    connect(m_client, &KDecoration2::DecoratedClient::providesContextHelpChanged, this , &DecoratedClient::providesContextHelpChanged);
    connect(m_client, &KDecoration2::DecoratedClient::shadeableChanged, this , &DecoratedClient::shadeableChanged);
    connect(m_client, &KDecoration2::DecoratedClient::moveableChanged, this , &DecoratedClient::moveableChanged);
    connect(m_client, &KDecoration2::DecoratedClient::resizeableChanged, this , &DecoratedClient::resizeableChanged);
    connect(m_client, &KDecoration2::DecoratedClient::widthChanged, this , &DecoratedClient::widthChanged);
    connect(m_client, &KDecoration2::DecoratedClient::heightChanged, this , &DecoratedClient::heightChanged);
    connect(m_client, &KDecoration2::DecoratedClient::sizeChanged, this , &DecoratedClient::sizeChanged);
    connect(m_client, &KDecoration2::DecoratedClient::paletteChanged, this , &DecoratedClient::paletteChanged);
    connect(m_client, &KDecoration2::DecoratedClient::adjacentScreenEdgesChanged, this , &DecoratedClient::adjacentScreenEdgesChanged);
    connect(m_client, &KDecoration2::DecoratedClient::hasApplicationMenuChanged, this , &DecoratedClient::hasApplicationMenuChanged);
    connect(m_client, &KDecoration2::DecoratedClient::applicationMenuActiveChanged, this , &DecoratedClient::applicationMenuActiveChanged);
    connect(dynamic_cast<QObject*>(m_client), &QObject::destroyed, this, &QObject::deleteLater);

    auto *clientImpl = dynamic_cast<KDecoration2::ApplicationMenuEnabledDecoratedClientPrivate*>(m_client->d.get());
    if (clientImpl) {
        d = std::unique_ptr<KDecoration2::DecoratedClientPrivate>(new KWin::Decoration::DecoratedClientImpl(clientImpl, this, m_decoration));
    }/* else {
        d = std::unique_ptr<KDecoration2::DecoratedClientPrivate>(m_client->d.get());
    }*/
}

#define DELEGATE(type, method) \
type DecoratedClient::method() const \
{ \
    return m_client->method(); \
}

DELEGATE(bool, isActive)
//DELEGATE(QString, caption)
DELEGATE(int, desktop)
DELEGATE(bool, isOnAllDesktops)
DELEGATE(bool, isShaded)
DELEGATE(QIcon, icon)
DELEGATE(bool, isMaximized)
DELEGATE(bool, isMaximizedHorizontally)
DELEGATE(bool, isMaximizedVertically)
DELEGATE(bool, isKeepAbove)
DELEGATE(bool, isKeepBelow)
DELEGATE(bool, isCloseable)
DELEGATE(bool, isMaximizeable)
DELEGATE(bool, isMinimizeable)
DELEGATE(bool, providesContextHelp)
DELEGATE(bool, isModal)
DELEGATE(bool, isShadeable)
DELEGATE(bool, isMoveable)
DELEGATE(bool, isResizeable)
DELEGATE(WId, windowId)
DELEGATE(WId, decorationId)
DELEGATE(int, width)
DELEGATE(int, height)
DELEGATE(QSize, size)
DELEGATE(QPalette, palette)
DELEGATE(Qt::Edges, adjacentScreenEdges)

#undef DELEGATE

QString DecoratedClient::caption() const
{
    CaptionFormat format = m_decoration->settings()->captionFormat();
    QString caption = m_client->caption();
    if (format == CaptionFormat::Full) {
        return caption;
    } else if (format == CaptionFormat::Compact) {
        int index = caption.lastIndexOf(" — ");
        if (index > 0) {
            caption.remove(index, caption.length() - index);
            return caption;
        }

        index = caption.lastIndexOf(" - ");
        if (index > 0) {
            caption.remove(index, caption.length() - index);
            return caption;
        }
    }

    return caption;
}

bool DecoratedClient::hasApplicationMenu() const
{
    if (const auto *appMenuEnabledPrivate = dynamic_cast<KDecoration2Lim::ApplicationMenuEnabledDecoratedClientPrivate *>(d.get())) {
        return appMenuEnabledPrivate->hasApplicationMenu();
    }
    return false;
}

bool DecoratedClient::isApplicationMenuActive() const
{
    if (const auto *appMenuEnabledPrivate = dynamic_cast<KDecoration2Lim::ApplicationMenuEnabledDecoratedClientPrivate *>(d.get())) {
        return appMenuEnabledPrivate->isApplicationMenuActive();
    }
    return false;
}

QVector<QPair<int, QString> > DecoratedClient::applicationMenuEntries() const
{
    if (const auto *appMenuEnabledPrivate = dynamic_cast<KDecoration2Lim::ApplicationMenuEnabledDecoratedClientPrivate*>(d.get())) {
        return appMenuEnabledPrivate->applicationMenuEntries();
    }

    return {};
}

int DecoratedClient::activeApplicationSubMenu() const
{
    if (const auto *appMenuEnabledPrivate = dynamic_cast<KDecoration2Lim::ApplicationMenuEnabledDecoratedClientPrivate*>(d.get())) {
        return appMenuEnabledPrivate->activeApplicationSubMenu();
    }
    return -1;
}

QPointer<Decoration> DecoratedClient::decoration() const
{
    return QPointer<Decoration>(m_decoration);
}

QColor DecoratedClient::color(QPalette::ColorGroup group, QPalette::ColorRole role) const
{
    return m_client->palette().color(group, role);
}

QColor DecoratedClient::color(ColorGroup group, ColorRole role) const
{
    return m_client->color(group, role);
}

void DecoratedClient::showApplicationMenu(int actionId)
{
    if (auto *appMenuEnabledPrivate = dynamic_cast<KDecoration2Lim::ApplicationMenuEnabledDecoratedClientPrivate *>(d.get())) {
        return appMenuEnabledPrivate->showApplicationMenu(actionId);
    }
}

} // namespace KDecoration2Lim
