#pragma once

#include "breezebutton.h"

namespace Breeze {

class ButtonLim : public Button
{
    Q_OBJECT

public:
    //* constructor
    explicit ButtonLim(QObject *parent, const QVariantList &args);

    //* destructor
    virtual ~ButtonLim() = default;

    //* button creation
    static Button *create(KDecoration2Lim::DecorationButtonType type, KDecoration2Lim::Decoration *decoration, QObject *parent);

    //* render
    virtual void paint(QPainter *painter, const QRect &repaintArea) override;

private slots:
    void updateSize();

private:
    ButtonLim(KDecoration2Lim::DecorationButtonType type, Decoration *decoration, QObject *parent);
    void drawText(QPainter *painter) const;
    QColor foregroundColor() const;
    QColor backgroundColor() const;
};

}
