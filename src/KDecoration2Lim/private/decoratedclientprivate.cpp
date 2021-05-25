#include "KDecoration2Lim/private/decoratedclientprivate.h"
#include "KDecoration2Lim/decoratedclient.h"
#include "KDecoration2Lim/decoration.h"

namespace KDecoration2Lim
{

ApplicationMenuEnabledDecoratedClientPrivate::ApplicationMenuEnabledDecoratedClientPrivate(DecoratedClient *client, Decoration *decoration)
    : KDecoration2::ApplicationMenuEnabledDecoratedClientPrivate(client->m_client, decoration)
{

}

ApplicationMenuEnabledDecoratedClientPrivate::~ApplicationMenuEnabledDecoratedClientPrivate() = default;

}
