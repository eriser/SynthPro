#ifndef MOCKINOUTMODULE_H
#define MOCKINOUTMODULE_H

#include "abstraction/inport.h"
#include "abstraction/mockmodule.h"
#include "abstraction/outport.h"

/**
 * Mock Module with replicable input and output ports
 */
class MockInOutModule : public MockModule {
public:
    MockInOutModule(SynthPro*, const QString& msg, QTextStream&);

    InPort input;
    OutPort output;
};

#endif // MOCKINOUTMODULE_H
