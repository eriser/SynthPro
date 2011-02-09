#ifndef PWIRE_H
#define PWIRE_H

#include <QGraphicsLineItem>

class CWire;

class PWire : public QGraphicsLineItem {
public:
    PWire(CWire*, QGraphicsScene*);

    QRectF boundingRect() const;

public slots:
    void updatePosition(QPointF);

private:
    CWire* m_control;
};

#endif // PWIRE_H