#include "breezebuttonlim.h"
#include "KDecoration2Lim/decorationdefines.h"
#include <KDecoration2Lim/decorationsettings.h>
#include "KDecoration2Lim/decorationbutton.h"

#include <KColorScheme>
#include <QApplication>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>


namespace Breeze {

using KDecoration2Lim::DecorationButtonType;
using KDecoration2Lim::category;

//__________________________________________________________________
ButtonLim::ButtonLim(KDecoration2Lim::DecorationButtonType type, Decoration* decoration, QObject* parent)
    : Button(type, decoration, parent)
{
    if (type == DecorationButtonType::ApplicationMenuEntry
            || type == DecorationButtonType::Overflow) {
        connect(this, &ButtonLim::labelChanged, this, &ButtonLim::updateSize);
        connect(decoration, &Decoration::titleBarChanged, this, &ButtonLim::updateSize);
        connect(decoration->settings().data(), &KDecoration2Lim::DecorationSettings::fontChanged, this, &ButtonLim::updateSize);
        updateSize();
    }
}

//__________________________________________________________________
ButtonLim::ButtonLim(QObject *parent, const QVariantList &args)
    : Button(parent, args)
{
}

//__________________________________________________________________
Button *ButtonLim::create(DecorationButtonType type, KDecoration2Lim::Decoration *decoration, QObject *parent)
{
    if (type != DecorationButtonType::ApplicationMenuEntry && type != DecorationButtonType::Overflow) {
        return Button::create(type, decoration, parent);
    }

    if (auto d = qobject_cast<Decoration*>(decoration)) {
        return new ButtonLim(type, d, parent);
    }

    return nullptr;

}

//__________________________________________________________________
void ButtonLim::paint(QPainter *painter, const QRect &repaintArea)
{
    if (type() == DecorationButtonType::ApplicationMenuEntry || type() == DecorationButtonType::Overflow)
    {
        painter->save();
        drawText(painter);
        painter->restore();
    } else {
        Button::paint(painter, repaintArea);
    }
}

//__________________________________________________________________
void ButtonLim::drawText( QPainter *painter ) const
{
    // render background
    const QColor backgroundColor( this->backgroundColor() );
    if( backgroundColor.isValid() )
    {
        painter->setPen( Qt::NoPen );
        painter->setBrush( backgroundColor );
        painter->drawRect( geometry() );
    }

    const auto *decorationPtr = qobject_cast<Breeze::Decoration *>(decoration());

    // draw text
    painter->setFont(qApp->font("QMenuBar"));
    painter->setPen(foregroundColor());
    // get additional align flags
    const auto &cR = decorationPtr->captionRect();
    painter->drawText(geometry(), cR.second | Qt::TextSingleLine | Qt::AlignHCenter, label());

    // draw icon in case of overflow button
    if ( type() == DecorationButtonType::Overflow ) {
        painter->translate( m_offset );

        // render mark
        const QColor foregroundColor( this->foregroundColor());
        if( foregroundColor.isValid() )
        {
            painter->setRenderHints( QPainter::Antialiasing );
            /*
                scale painter so that its window matches QRect( -1, -1, 20, 20 )
                this makes all further rendering and scaling simpler
                all further rendering is preformed inside QRect( 0, 0, 18, 18 )
                */
            painter->translate( geometry().topLeft() );

            const qreal width( m_iconSize.width() );
            painter->scale( width/20, width/20 );
            painter->translate( 1, 1 );

            // setup painter
            QPen pen( foregroundColor );
            pen.setCapStyle( Qt::RoundCap );
            pen.setJoinStyle( Qt::MiterJoin );
            pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

            painter->setPen( pen );
            painter->setBrush( Qt::NoBrush );

            painter->drawRect( QRectF( 3.5, 4.5, 11, 1 ) );
            painter->drawRect( QRectF( 3.5, 8.5, 11, 1 ) );
            painter->drawRect( QRectF( 3.5, 12.5, 11, 1 ) );
        }
    }
}

//__________________________________________________________________
QColor ButtonLim::foregroundColor() const
{
    if (type() != DecorationButtonType::ApplicationMenuEntry
            && type() != DecorationButtonType::Overflow) {
        return Button::foregroundColor();
    }

    const auto *decorationPtr = qobject_cast<Decoration*>( decoration() );
    if( !decorationPtr ) {
        return QColor();
    }

    const auto &clientPtr = decorationPtr->client().toStrongRef();
    QColor menuForeground = KStatefulBrush( KColorScheme::Selection, KColorScheme::NormalText ).brush( clientPtr->palette() ).color();

    if ( isChecked() || isHovered() || isPressed() ) {
        return menuForeground;
    } else {
        return decorationPtr->fontColor();
    }
}

//__________________________________________________________________
QColor ButtonLim::backgroundColor() const
{
    if (type() != DecorationButtonType::ApplicationMenuEntry
            && type() != DecorationButtonType::Overflow) {
        return Button::backgroundColor();
    }

    const auto *decorationPtr = qobject_cast<Decoration*>( decoration() );
    if( !decorationPtr ) {
        return QColor();
    }

    const auto &clientPtr = decorationPtr->client().toStrongRef();
    QColor menuBackground = KStatefulBrush( KColorScheme::View, KColorScheme::FocusColor ).brush( clientPtr->palette() ).color();

    if ( isChecked() || isHovered() || isPressed() ) {
        if (clientPtr->isApplicationMenuActive()
                && clientPtr->activeApplicationSubMenu() != actionId()) {
            return QColor();
        }
        return menuBackground;
    } else {
        return QColor();
    }

}

//__________________________________________________________________
void ButtonLim::updateSize()
{
    const auto &decorationPtr = decoration();

    QSize size {0, 0};
    if (type() == DecorationButtonType::ApplicationMenuEntry) {
        const auto &settings = decorationPtr->settings();
        const auto &fontMetrics = QFontMetrics(qApp->font("QMenuBar"));
        size = fontMetrics.size(Qt::TextShowMnemonic, label());
//        size = qApp->style()->sizeFromContents(QStyle::CT_MenuBarItem, nullptr, size, nullptr);
        size.setWidth(size.width() + settings->menuBarSpacing() * 2);
        size.setHeight(decorationPtr->borderTop());
//        qCDebug(category) << "WIDTH=" << size.width() << fontMetrics.size(Qt::TextShowMnemonic, label()) << label() << "FONT=" << decorationPtr->settings()->font();
    } else if (type() == DecorationButtonType::Overflow) {
        size = QSize(decorationPtr->borderTop(), decorationPtr->borderTop());
    } else {
        return;
    }

    QRectF rect = geometry();
    rect.setSize(size);
    setGeometry(rect);
}

}
