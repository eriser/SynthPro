#ifndef PVCO_H
#define PVCO_H

#include "presentation/pmodule.h"
#include "presentation/pport.h"
#include <QGraphicsProxyWidget>

class PVCO : public PModule {
public:
    PVCO();
    void initialize(PPort* vfm, PPort* out);

private:
    QGraphicsProxyWidget* m_waveSelector;
    QGraphicsProxyWidget* m_kSelector;
};

#endif // PVCO_H
