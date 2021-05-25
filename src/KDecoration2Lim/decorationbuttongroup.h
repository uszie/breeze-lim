#pragma once

#include "KDecoration2Lim/decorationdefines.h"

#include <KDecoration2/DecorationButtonGroup>


namespace KDecoration2Lim {

class Decoration;
class DecorationButton;
class AppMenuModel;

class DecorationButtonGroup : public KDecoration2::DecorationButtonGroup
{
    Q_OBJECT

    Q_PROPERTY(qreal overflowWidth READ overflowWidth WRITE setOverflowWidth NOTIFY overflowWidthChanged)

public:
    enum class Position {
        Left,
        Right,
        Menu,
    };

    explicit DecorationButtonGroup(Position type, Decoration *parent, std::function<DecorationButton*(DecorationButtonType, Decoration *, QObject *)> buttonCreator);
    ~DecorationButtonGroup() override;

    QPointer<Decoration> decoration() const;
    QVector<QPointer<DecorationButton>> buttons() const;
    Position type() const;
    bool isHovered() const;
    QSizeF minimumSizeHint() const;
    QSizeF sizeHint() const;
    qreal overflowWidth() const;
    void setOverflowWidth(qreal width);

    // finds the first button of type
    QPointer<DecorationButton> findButton(DecorationButtonType type) const;

Q_SIGNALS:
    void hoveredChanged(bool hovered);
    void overflowWidthChanged(qreal width);

private Q_SLOT:
    void updateWidth();
    void updateButtons();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setHovered(bool hovered);
    void updateSizeHints();
    void addApplicationMenuButtons();
    void addButton(DecorationButtonType type, int actionId, const QString &label);

    QPointer<Decoration> m_decoration;
    Position m_type;
    bool m_isHovered;
    QSizeF m_minimumSizeHint;
    QSizeF m_sizeHint;
    QRectF m_clipRect;
    qreal m_overflowWidth;
    AppMenuModel *m_appMenuModel;
    std::function<DecorationButton *(DecorationButtonType, Decoration *, QObject *)> m_buttonCreator;
};

}

