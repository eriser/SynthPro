#include "cadsr.h"

#include "control/cdimmer.h"
#include "control/cinport.h"
#include "control/coutport.h"
#include "control/cpushbutton.h"
#include "control/cselector.h"
#include "presentation/padsr.h"

CADSR::CADSR(SynthPro* parent)
    : Module(parent)
    , ADSR(parent)
    , CModule(parent)
{
}

void CADSR::initialize(SynthProFactory* factory)
{
    ADSR::initialize(factory);

    CInPort* gate = dynamic_cast<CInPort*>(m_gate);
    COutPort* out = dynamic_cast<COutPort*>(m_outPort);
    CDimmer* attack = dynamic_cast<CDimmer*>(m_attackDimmer);
    CDimmer* decay = dynamic_cast<CDimmer*>(m_decayDimmer);
    CDimmer* sustain = dynamic_cast<CDimmer*>(m_sustainDimmer);
    CDimmer* release = dynamic_cast<CDimmer*>(m_releaseDimmer);
    CPushButton* manual = dynamic_cast<CPushButton*>(m_manualControl);

    dynamic_cast<PADSR*>(presentation())->initialize(gate->presentation(), out->presentation(),
                                                     attack->presentation(), decay->presentation(),
                                                     sustain->presentation(), release->presentation(),
                                                     manual->presentation());
}