#pragma once

#include "breeze/breezedecoration.h"

namespace KDecoration2Lim
{
    class DecorationButtonGroup;
}

namespace Breeze {

class AppMenuState;

class DecorationLim : public Decoration
{
    Q_OBJECT


public:
    //* constructor
    explicit DecorationLim(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    // Decoration interface
public:
    virtual void paint(QPainter *painter, const QRect &repaintArea) override;
    QColor captionColor() const;
    void setAppMenuExpanded(bool expanded, bool animated);

public Q_SLOTS:
    void init() override;

private Q_SLOTS:
    void reconfigure();
    void updateAppMenuButtonsGeometry();
    void updateAppMenuButtonsGeometryDelayed();
    void updateAppMenuAnimationState(bool now);

private:
    void createAppMenuButtons();
    QRectF captionTextRect() const;
    QRectF calculateAppMenuGeometry(bool expanded) const;

    qreal m_captionOpacity = 1.0;
    QVariantAnimation *m_captionOpacityAnimation;
    KDecoration2Lim::DecorationButtonGroup *m_appMenuButtons = nullptr;
    QVariantAnimation *m_appMenuButtonsAnimation;
    QRectF m_appMenuButtonsPaintRect;
    QTimer *m_appMenuButtonsHoverTimer;
};

} // namespace Breeze

