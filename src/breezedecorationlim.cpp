#include "breezedecorationlim.h"
#include "breezebuttonlim.h"

#include "config-breeze.h"
#include "breeze/config/breezeconfigwidget.h"

#include <KPluginFactory>
#include <KColorUtils>

#include <QDBusConnection>
#include <QObject>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>


K_PLUGIN_FACTORY_WITH_JSON(
    BreezeLimDecoFactory,
    "breeze-lim.json",
    registerPlugin<Breeze::DecorationLim>();
    registerPlugin<Breeze::ButtonLim>(QStringLiteral("button"));
    registerPlugin<Breeze::ConfigWidget>(QStringLiteral("kcmodule"));
)

namespace Breeze {

using KDecoration2Lim::DecorationButtonType;
using KDecoration2Lim::DecorationButtonGroup;
using KDecoration2Lim::category;

//________________________________________________________________
DecorationLim::DecorationLim(QObject *parent, const QVariantList &args)
    : Breeze::Decoration(parent, args)
    , m_captionOpacityAnimation(new QVariantAnimation(this))
    , m_appMenuButtonsAnimation(new QVariantAnimation(this))
    , m_appMenuButtonsPaintRect(0, 0, 5000, 5000)
    , m_appMenuButtonsHoverTimer(new QTimer(this))
{
}

//________________________________________________________________
void Breeze::DecorationLim::paint(QPainter *painter, const QRect &repaintArea)
{
    Breeze::Decoration::paint(painter, repaintArea);

    const auto clientPtr = client().toStrongRef();
    painter->save();

    // Repaint caption rect to erase text painted in base paint method
    // this breaks the gradient for now
    // TODO use gradient when needed
    painter->setPen(Qt::NoPen);
    painter->setBrush(titleBarColor());
    painter->drawRect(captionTextRect().adjusted(0,-1,0,1));

    // draw caption
    painter->setFont(settings()->font());
    painter->setBrush(Qt::NoBrush);
    painter->setPen(captionColor());

    const auto cR = captionRect();
    const QString caption = painter->fontMetrics().elidedText(clientPtr->caption(), Qt::ElideMiddle, cR.first.width());
    painter->drawText(cR.first, cR.second | Qt::TextSingleLine, caption);

    painter->restore();

    QPainterPath clipPath;
    if (settings()->isAlphaChannelSupported()) {
        clipPath.addRoundedRect(rect(), Metrics::Frame_FrameRadius, Metrics::Frame_FrameRadius);
    } else {
        clipPath.addRect(rect());
    }

    QPainterPath paintPath;
    paintPath.addRect(m_appMenuButtonsPaintRect);
    clipPath = clipPath.intersected(paintPath);
    painter->setClipPath(clipPath, Qt::IntersectClip);

    m_appMenuButtons->paint(painter, repaintArea.intersected(m_appMenuButtonsPaintRect.toRect()));
}

//________________________________________________________________
void DecorationLim::init()
{
    Decoration::init();

    m_captionOpacityAnimation->setStartValue(1.0);
    m_captionOpacityAnimation->setEndValue(0.0);
    // Linear to have the same easing as Breeze animations
    m_captionOpacityAnimation->setEasingCurve( QEasingCurve::Linear );
    connect(m_captionOpacityAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_captionOpacity = value.toReal();
        update();
    });


    m_appMenuButtonsAnimation->setEasingCurve(QEasingCurve::Linear);
    connect (m_appMenuButtonsAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        if  (m_appMenuButtonsAnimation->state() == QAbstractAnimation::Running ) {
            m_appMenuButtonsPaintRect = value.toRectF();
            update();
        }
    });

    connect(m_appMenuButtonsAnimation, &QVariantAnimation::finished, this, [this]() {
        if (!m_appMenuButtons->isHovered() && m_appMenuButtonsAnimation->direction() == QAbstractAnimation::Backward) {
            const qreal overflowWidth = qMin(m_appMenuButtons->sizeHint().width(), captionTextRect().left() - m_appMenuButtons->geometry().left() );
            m_appMenuButtons->setOverflowWidth(overflowWidth);
            m_captionOpacityAnimation->start();
        }
    });

    connect(m_appMenuButtonsAnimation, &QVariantAnimation::stateChanged, this, [this](QAbstractAnimation::State newState, QAbstractAnimation::State oldState) {
        Q_UNUSED(oldState)
        if (newState == QAbstractAnimation::Running) {
            m_captionOpacityAnimation->stop();
        }
    });

    auto clientPtr = client().toStrongRef().data();

    m_appMenuButtonsHoverTimer->setInterval(1000);
    connect(m_appMenuButtonsHoverTimer, &QTimer::timeout, this, [this, clientPtr]()
    {
        // disable the button group from collapsing when the application menu is active
        if (clientPtr->isApplicationMenuActive()) {
            m_appMenuButtonsHoverTimer->start();
            return;
        }

        updateAppMenuAnimationState(true);
    });

    // use DBus connection to update on breeze configuration change
    auto dbus = QDBusConnection::sessionBus();
    dbus.connect(QString(),
        QStringLiteral( "/KGlobalSettings" ),
        QStringLiteral( "org.kde.KGlobalSettings" ),
        QStringLiteral( "notifyChange" ), this, SLOT(reconfigure()));

    reconfigure();

    const auto *settingsPtr = settings().data();

    connect(settingsPtr, &KDecoration2Lim::DecorationSettings::spacingChanged, this, &DecorationLim::updateAppMenuButtonsGeometryDelayed);
    connect(settingsPtr, &KDecoration2Lim::DecorationSettings::decorationButtonsLeftChanged, this, &DecorationLim::updateAppMenuButtonsGeometryDelayed);
    connect(settingsPtr, &KDecoration2Lim::DecorationSettings::decorationButtonsRightChanged, this, &DecorationLim::updateAppMenuButtonsGeometryDelayed);
    connect(settingsPtr, &KDecoration2Lim::DecorationSettings::reconfigured, this, &DecorationLim::updateAppMenuButtonsGeometryDelayed);


    connect(clientPtr, &KDecoration2Lim::DecoratedClient::widthChanged, this, &DecorationLim::updateAppMenuButtonsGeometry);
    connect(clientPtr, &KDecoration2Lim::DecoratedClient::maximizedChanged, this, &DecorationLim::updateAppMenuButtonsGeometry);
    connect(clientPtr, &KDecoration2Lim::DecoratedClient::adjacentScreenEdgesChanged, this, &DecorationLim::updateAppMenuButtonsGeometry);
    connect(clientPtr, &KDecoration2Lim::DecoratedClient::shadedChanged, this, &DecorationLim::updateAppMenuButtonsGeometry);
    connect(clientPtr, &KDecoration2Lim::DecoratedClient::hasApplicationMenuChanged, this, &DecorationLim::updateAppMenuButtonsGeometryDelayed);

    createAppMenuButtons();
}

//________________________________________________________________
void DecorationLim::reconfigure()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    const KConfigGroup cg(config, QStringLiteral("KDE"));

    m_appMenuButtonsAnimation->setDuration(cg.readEntry("AnimationDurationFactor", 1.0f) * 100.0f * 2.0f);
    m_captionOpacityAnimation->setDuration(cg.readEntry("AnimationDurationFactor", 1.0f) * 100.0f * 2.0f);
}

//________________________________________________________________
void DecorationLim::updateAppMenuButtonsGeometryDelayed()
{ QTimer::singleShot( 0, this, &DecorationLim::updateAppMenuButtonsGeometry ); }

//________________________________________________________________
void DecorationLim::updateAppMenuButtonsGeometry()
{
    if (m_appMenuButtons->buttons().isEmpty()) {
        return;
    }

    const auto settingsPtr = settings();

    // adjust button position
    const int bWidth = buttonHeight();
    const int verticalOffset = (isTopEdge() ? settingsPtr->smallSpacing()*Metrics::TitleBar_TopMargin:0) + (captionHeight()-buttonHeight())/2;

    // menu buttons
    const int vPadding = isTopEdge() ? 0 : settingsPtr->smallSpacing()*Metrics::TitleBar_TopMargin;
    const auto &button = m_appMenuButtons->findButton(DecorationButtonType::Overflow);
    static_cast<Button*>(button.data())->setOffset( QPointF( verticalOffset + vPadding, verticalOffset + vPadding ));
    static_cast<Button*>(button.data())->setIconSize( QSize( bWidth, bWidth));
    const auto &leftButtons = m_leftButtons->buttons();
    bool leftButtonsVisible = std::any_of(leftButtons.cbegin(),
                                          leftButtons.cend(),
                                          [](const QPointer<KDecoration2Lim::DecorationButton> &button) {
        return button->isVisible();
    });

    qreal x = leftButtonsVisible ? m_leftButtons->geometry().right() : 0.0;
    m_appMenuButtons->setPos(QPointF(x, 0));

    QRectF collapsed = calculateAppMenuGeometry(false);
    m_appMenuButtonsAnimation->setStartValue(collapsed);
    QRectF expanded = calculateAppMenuGeometry(true);
    m_appMenuButtonsAnimation->setEndValue(expanded);

    setAppMenuExpanded(false, false);

    update();
}

//________________________________________________________________
void DecorationLim::createAppMenuButtons()
{
    m_appMenuButtons = new DecorationButtonGroup(DecorationButtonGroup::Position::Menu, this, &ButtonLim::create);
    connect(m_appMenuButtons, &DecorationButtonGroup::overflowWidthChanged, this, &DecorationLim::updateButtonsGeometryDelayed);
    connect(m_appMenuButtons, &DecorationButtonGroup::hoveredChanged, this, &DecorationLim::updateAppMenuAnimationState);
    updateAppMenuButtonsGeometry();
}

//________________________________________________________________
void DecorationLim::updateAppMenuAnimationState(bool now)
{
    if (now) {
        m_appMenuButtonsHoverTimer->stop();
        setAppMenuExpanded(m_appMenuButtons->isHovered(), true);
    } else {
        m_appMenuButtonsHoverTimer->start();
    }
}

//________________________________________________________________
void DecorationLim::setAppMenuExpanded(bool expanded, bool animated)
{
    // check if the requested state matches the current state
    if ((expanded && m_appMenuButtonsPaintRect == m_appMenuButtonsAnimation->endValue().toRectF())
            || (!expanded && m_appMenuButtonsPaintRect == m_appMenuButtonsAnimation->startValue().toRectF())) {
        return;
    }

    // select the matching geometry
    const QRectF overflowRect = expanded
            ? m_appMenuButtonsAnimation->endValue().toRectF()
            : m_appMenuButtonsAnimation->startValue().toRectF();

    if (animated && m_appMenuButtonsAnimation->duration() > 0) {
        m_appMenuButtonsAnimation->setDirection(expanded ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
        if (expanded) {
            // set the button group to the expanded size immediately
            // use the animation to make it look like the size is smoothly growing
            m_appMenuButtons->setOverflowWidth(overflowRect.width());
        }

        if( m_appMenuButtonsAnimation->state() != QAbstractAnimation::Running ) {
            m_appMenuButtonsAnimation->start();
        }
    } else {
        // just set it to the requested gemeotry without animation
        m_appMenuButtonsPaintRect = overflowRect;
        m_appMenuButtonsAnimation->setCurrentTime(0);
        m_appMenuButtonsAnimation->stop();
        m_appMenuButtonsHoverTimer->stop();
        m_appMenuButtons->setOverflowWidth(overflowRect.width());
        update();
    }
}

//________________________________________________________________
QRectF DecorationLim::calculateAppMenuGeometry(bool expanded) const
{
    if (expanded) {
        const qreal expandedWidth = qMin(m_appMenuButtons->sizeHint().width(),
                                         m_rightButtons->geometry().right()
                                         - m_rightButtons->geometry().width()
                                         - m_appMenuButtons->geometry().left());

        const QRectF expandedRect = QRectF(m_appMenuButtons->geometry().topLeft(),
                                           QSizeF(expandedWidth, m_appMenuButtons->geometry().height()));
        return expandedRect;
    } else {
        const qreal collapsedWidth = qMin(m_appMenuButtons->sizeHint().width(),
                                          captionTextRect().left() - m_appMenuButtons->geometry().left());

        const QRectF collapsedRect = QRectF(m_appMenuButtons->geometry().topLeft(),
                                            QSizeF(collapsedWidth, m_appMenuButtons->geometry().height()));
        return collapsedRect;
    }
}

//________________________________________________________________
QColor DecorationLim::captionColor() const
{
    if(m_captionOpacityAnimation->state() == QAbstractAnimation::Running) {
        return KColorUtils::mix(fontColor(), titleBarColor(), m_captionOpacity);
    } else {
        return m_appMenuButtons->geometry().intersects(captionTextRect()) ? titleBarColor() : fontColor();
    }
}

//________________________________________________________________
QRectF DecorationLim::captionTextRect() const
{
    const auto clientPtr = client().toStrongRef();
    const QFontMetricsF &fontMetrics = settings()->fontMetrics();
    const QString &captionText = clientPtr->caption();
    const QString &elidedCaptionText = fontMetrics.elidedText(captionText, Qt::ElideMiddle, captionRect().first.width());

    QRectF textRect = fontMetrics.boundingRect(captionRect().first, captionRect().second, elidedCaptionText);
    textRect.adjust(Metrics::TitleBar_SideMargin * -settings()->largeSpacing(), 0, Metrics::TitleBar_SideMargin * settings()->largeSpacing(), 0);
    return textRect;
}

} // namespace Breeze

#include "breezedecorationlim.moc"
