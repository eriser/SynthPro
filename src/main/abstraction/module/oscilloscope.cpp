#include "oscilloscope.h"

#include "abstraction/component/pushbutton.h"
#include "factory/synthprofactory.h"

Oscilloscope::Oscilloscope(SynthPro* parent)
    : Module(parent)
    , m_inPort(0)
    , m_stabilizeControl(0)
    , m_stabilized(false)
{
}

Oscilloscope::~Oscilloscope()
{
}

void Oscilloscope::initialize(SynthProFactory* factory)
{
    if (factory) {
        // Creation of an Input.
        m_inPort = factory->createInPortReplicable(this, tr("in"));
        m_inports.append(m_inPort);

        m_stabilizeControl = factory->createPushButton(tr("Stabilize"), this);
        m_stabilizeControl->setCheckable(true);

        connect(m_stabilizeControl, SIGNAL(buttonPushed()), this, SLOT(stabilizedPushed()));
    }
}

void Oscilloscope::ownProcess()
{
}

bool Oscilloscope::stabilized()
{
    return m_stabilized;
}

void Oscilloscope::stabilizedPushed()
{
    m_stabilized = m_stabilizeControl->isChecked();
}
