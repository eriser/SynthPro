#ifndef CSYNTHPRO_H
#define CSYNTHPRO_H

#include "abstraction/synthpro.h"
#include "presentation/psynthpro.h"

class QGraphicsScene;

class CSynthPro : virtual public SynthPro {
public:
    CSynthPro();
    virtual ~CSynthPro();

    void setPresentation(PSynthPro*);
    PSynthPro* presentation() const;

    void addModule(QString& moduleType, QPoint);

private:
    PSynthPro* m_presentation;
    QGraphicsScene* m_graphicsScene;
};

#endif // CSYNTHPRO_H
