#pragma once

#include "decorationdefines.h"

#include <KDecoration2/DecorationButton>

namespace KDecoration2Lim {

class Decoration;

class DecorationButton : public KDecoration2::DecorationButton
{
    Q_OBJECT

public:
    ~DecorationButton() override;

    bool event(QEvent *event) override;
    QPointer<Decoration> decoration () const;
    DecorationButtonType type() const;
    int actionId() const;
    void setActionId(int actionId);
    const QString &label() const;
    void setLabel(const QString &label);

Q_SIGNALS:
    void labelChanged(const QString &label);

protected:
    DecorationButton(DecorationButtonType type, const QPointer<Decoration> &decoration, QObject *parent=nullptr);

private Q_SLOTS:
    void updateApplicationMenuState();

private:
    int m_actionId;
    QString m_label;
    DecorationButtonType m_type;
};

}

Q_DECLARE_METATYPE(KDecoration2Lim::DecorationButtonType)
