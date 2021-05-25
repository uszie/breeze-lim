#pragma once

#include "KDecoration2Lim/decorationdefines.h"

#include <KDecoration2/DecoratedClient>
#include <KDecoration2/Private/DecoratedClientPrivate>

namespace KDecoration2Lim {

class Decoration;

class DecoratedClient : public QObject
{
    Q_OBJECT

public:
    DecoratedClient(KDecoration2::DecoratedClient *client);
    ~DecoratedClient() = default;

    bool isActive() const;
    QString caption() const;
    int desktop() const;
    bool isOnAllDesktops() const;
    bool isShaded() const;
    QIcon icon() const;
    bool isMaximized() const;
    bool isMaximizedHorizontally() const;
    bool isMaximizedVertically() const;
    bool isKeepAbove() const;
    bool isKeepBelow() const;
    bool isCloseable() const;
    bool isMaximizeable() const;
    bool isMinimizeable() const;
    bool providesContextHelp() const;
    bool isModal() const;
    bool isShadeable() const;
    bool isMoveable() const;
    bool isResizeable() const;
    Qt::Edges adjacentScreenEdges() const;
    WId windowId() const;
    WId decorationId() const;
    int width() const;
    int height() const;
    QSize size() const;
    QPointer<Decoration> decoration() const;
    QPalette palette() const;
    QColor color(QPalette::ColorGroup group, QPalette::ColorRole role) const;
    QColor color(ColorGroup group, ColorRole role) const;
    bool hasApplicationMenu() const;
    bool isApplicationMenuActive() const;
    QVector<QPair<int, QString>> applicationMenuEntries() const;
    int activeApplicationSubMenu() const;
    void showApplicationMenu(int actionId);

Q_SIGNALS:
    void activeChanged(bool);
    void captionChanged(QString);
    void desktopChanged(int);
    void onAllDesktopsChanged(bool);
    void shadedChanged(bool);
    void iconChanged(QIcon);
    void maximizedChanged(bool);
    void maximizedHorizontallyChanged(bool);
    void maximizedVerticallyChanged(bool);
    void keepAboveChanged(bool);
    void keepBelowChanged(bool);
    void closeableChanged(bool);
    void maximizeableChanged(bool);
    void minimizeableChanged(bool);
    void providesContextHelpChanged(bool);
    void shadeableChanged(bool);
    void moveableChanged(bool);
    void resizeableChanged(bool);
    void widthChanged(int);
    void heightChanged(int);
    void sizeChanged(const QSize &size);
    void paletteChanged(const QPalette &palette);
    void adjacentScreenEdgesChanged(Qt::Edges edges);
    void hasApplicationMenuChanged(bool);
    void applicationMenuActiveChanged(bool);
    // new interface item
    void applicationSubMenuActiveChanged(bool, int);

private:
    QPoint windowPos() const;
    QPoint mapFromGlobal(const QPoint &pos) const;
    QPoint mapToGlobal(const QPoint &pos) const;
    void showApplicationMenu(const QRect &rect, int actionId, bool isOverflowMenu);

    Decoration *m_decoration;
    friend class ApplicationMenuEnabledDecoratedClientPrivate;
    KDecoration2::DecoratedClient *m_client;

    friend class Decoration;
    std::unique_ptr<KDecoration2::DecoratedClientPrivate> d;
};

} // namespace KDecoration2Lim

