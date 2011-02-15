#ifndef CSYNTHPRO_H
#define CSYNTHPRO_H

#include "abstraction/synthpro.h"
#include "factory/synthprofactory.h"
#include "presentation/psynthpro.h"

class CVirtualPort;
class QGraphicsScene;

class CSynthPro : public SynthPro {
    Q_OBJECT

public:
    CSynthPro(SynthProFactory*);
    virtual ~CSynthPro();

    void setPresentation(PSynthPro*);
    PSynthPro* presentation() const;

    void add(Module*);
    void addModule(SynthProFactory::ModuleType, const QPointF&);

    void showFeedback(CVirtualPort*);
    void hideFeedback();

    void newScheme();

private slots:
    void play(bool);

private:
    PSynthPro* m_presentation;
    QGraphicsScene* m_graphicsScene;
    SynthProFactory* m_factory;
};

#endif // CSYNTHPRO_H
