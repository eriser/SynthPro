#include "coscilloscope.h"

#include "control/component/cdimmer.h"
#include "control/component/cinport.h"
#include "presentation/module/poscilloscope.h"

COscilloscope::COscilloscope(SynthPro* parent)
    : Module(parent)
    , Oscilloscope(parent)
    , CModule(parent)
{
}

void COscilloscope::initialize(SynthProFactory* factory)
{
    Oscilloscope::initialize(factory);

    CInPort* in = dynamic_cast<CInPort*>(m_inPort);

    dynamic_cast<POscilloscope*>(presentation())->initialize(in->presentation());

    dynamic_cast<POscilloscope*>(presentation())->setVisualizedBuffer(inports().at(0)->buffer());
}

void COscilloscope::ownProcess()
{
    dynamic_cast<POscilloscope*>(presentation())->refreshOscilloscopeView();
}