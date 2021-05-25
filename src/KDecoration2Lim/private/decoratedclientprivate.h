#pragma once

#include <KDecoration2/Private/DecoratedClientPrivate>

namespace KDecoration2Lim
{

class Decoration;
class DecoratedClient;

class ApplicationMenuEnabledDecoratedClientPrivate : public KDecoration2::ApplicationMenuEnabledDecoratedClientPrivate
{
public:
    ~ApplicationMenuEnabledDecoratedClientPrivate() override;

    virtual QVector<QPair<int, QString>> applicationMenuEntries() const = 0;
    virtual int activeApplicationSubMenu() const = 0;
    virtual void requestShowApplicationSubMenu(const QRect &rect, int actionId, bool isOverflowMenu) = 0;

protected:
    explicit ApplicationMenuEnabledDecoratedClientPrivate(DecoratedClient *client, Decoration *decoration);
};

} // namespace
