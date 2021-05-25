#pragma once

#include "KDecoration2Lim/decorationdefines.h"
#include "KDecoration2Lim/decorationbutton.h"
#include "KDecoration2Lim/decorationbuttongroup.h"
#include "KDecoration2Lim/decorationsettings.h"

#include <KDecoration2/Decoration>

//#include <QAbstractNativeEventFilter>
#include <QSharedPointer>


namespace KDecoration2Lim {

class DecorationButton;
class DecoratedClient;

class Decoration : public KDecoration2::Decoration/*, public QAbstractNativeEventFilter*/
{
    Q_OBJECT

public:
    explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~Decoration() override;

    QWeakPointer<DecoratedClient> client() const;
    QVector<QPointer<DecorationButton>> buttons() const;
    QPointer<DecorationButton> findButton(DecorationButtonType type) const;

//    void setSettings(const QSharedPointer<KDecoration2::DecorationSettings > &settings);
    /**
    * @returns The DecorationSettings used for this Decoration.
    **/
    QSharedPointer<DecorationSettings> settings() const;

    bool event(QEvent *event) override;

public Q_SLOTS:
    void showApplicationMenu(int actionId);
    void requestShowApplicationSubMenu(const QRect &rect, int actionId, bool isOverflowMenu = false);

//protected:
//    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
//    void globalKeyEvent(quint32 key);

private:
//    void installKeyLogger();

    QSharedPointer<DecorationSettings> m_settings;
    QSharedPointer<DecoratedClient> m_client;
    void setupWaylandIntegration();
    void printChildren(const QObject *object);
};

}
