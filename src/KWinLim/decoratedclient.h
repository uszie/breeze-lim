#pragma once

#include "KDecoration2Lim/private/decoratedclientprivate.h"

#include <QObject>
#include <QPointer>

namespace KDecoration2Lim
{
    class DecoratedClient;
    class Decoration;
}

class QMenu;
class QAction;
class AppMenuModel;

namespace KWin {
namespace Decoration {

class DecoratedClientImpl : public QObject, public KDecoration2Lim::ApplicationMenuEnabledDecoratedClientPrivate
{
    Q_OBJECT
public:
    DecoratedClientImpl(KDecoration2::ApplicationMenuEnabledDecoratedClientPrivate *clientPrivate, KDecoration2Lim::DecoratedClient *decoratedClient, KDecoration2Lim::Decoration *decoration);
    virtual ~DecoratedClientImpl() = default;

    // DecoratedClientPrivate interface
public:
    bool isActive() const override;
    QString caption() const override;
    int desktop() const override;
    bool isOnAllDesktops() const override;
    bool isShaded() const override;
    QIcon icon() const override;
    bool isMaximized() const override;
    bool isMaximizedHorizontally() const override;
    bool isMaximizedVertically() const override;
    bool isKeepAbove() const override;
    bool isKeepBelow() const override;
    bool isCloseable() const override;
    bool isMaximizeable() const override;
    bool isMinimizeable() const override;
    bool providesContextHelp() const override;
    bool isModal() const override;
    bool isShadeable() const override;
    bool isMoveable() const override;
    bool isResizeable() const override;
    WId windowId() const override;
    WId decorationId() const override;
    int width() const override;
    int height() const override;
    QSize size() const override;
    QPalette palette() const override;
    Qt::Edges adjacentScreenEdges() const override;
    void requestShowToolTip(const QString &text) override;
    void requestHideToolTip() override;
    void requestClose() override;
    void requestToggleMaximization(Qt::MouseButtons buttons) override;
    void requestMinimize() override;
    void requestContextHelp() override;
    void requestToggleOnAllDesktops() override;
    void requestToggleShade() override;
    void requestToggleKeepAbove() override;
    void requestToggleKeepBelow() override;
    void requestShowWindowMenu(const QRect &rect) override;
    QColor color(KDecoration2::ColorGroup group, KDecoration2::ColorRole role) const override;

    // ApplicationMenuEnabledDecoratedClientPrivate interface
    bool hasApplicationMenu() const override;
    bool isApplicationMenuActive() const override;
    // new interface item
    QVector<QPair<int, QString>> applicationMenuEntries() const override;
    // new interface item
    int activeApplicationSubMenu() const override;
    void showApplicationMenu(int actionId) override;
    void requestShowApplicationMenu(const QRect &rect, int actionId) override;
    // new interface item
    void requestShowApplicationSubMenu(const QRect &rect, int actionId, bool isOverflowMenu) override;

private Q_SLOTS:
    void updateApplicationMenuEntries();
    void updateActiveAction(int actionId, bool active = true);

private:
    KDecoration2::ApplicationMenuEnabledDecoratedClientPrivate *m_clientPrivate;
    KDecoration2Lim::DecoratedClient *m_client;
    KDecoration2Lim::Decoration *m_decoration;

    void initApplicationMenu();
    QAction *applicationMenuEntry(int actionId) const;
    QPoint windowPos() const;
    QPoint mapFromGlobal(const QPoint &pos) const;
    QPoint mapToGlobal(const QPoint &pos) const;
    bool eventFilter(QObject *watched, QEvent *event) override;

    AppMenuModel *m_appMenuModel;
    QPointer<QMenu> m_overflowMenu;
    int m_overflowActionId;
    int m_activeActionId;
    bool m_isApplicationMenuActive;
    QVector<QPair<int, QString>> m_applicationMenuEntries;
};

} // namespace Decoration
} // namespace KWin
