#include "csynthpro.h"

#include "control/cadsr.h"
#include "control/cdelay.h"
#include "control/cinport.h"
#include "control/ckeyboard.h"
#include "control/clfo.h"
#include "control/cmodule.h"
#include "control/coscilloscope.h"
#include "control/coutport.h"
#include "control/cspeaker.h"
#include "control/cvca.h"
#include "control/cvcf.h"
#include "control/cvco.h"
#include "control/cvirtualport.h"
#include "control/cwavrecorder.h"
#include <QGraphicsScene>

CSynthPro::CSynthPro(SynthProFactory* factory)
    : SynthPro()
    , m_presentation(0)
    , m_graphicsScene(new QGraphicsScene)
    , m_factory(factory)
{
}

CSynthPro::~CSynthPro()
{
    if (m_presentation) {
        delete m_presentation;
    }

    // Delete each module.
    foreach (Module* module, modules()) {
        delete module;
    }

    // m_graphicsScene has no parent so we must destroy it ourselves.
    delete m_graphicsScene;
}

void CSynthPro::setPresentation(PSynthPro* presentation)
{
    if (m_presentation) {
        delete m_presentation;
    }

    m_presentation = presentation;
    m_presentation->setGraphicsScene(m_graphicsScene);
}

PSynthPro* CSynthPro::presentation() const
{
    return m_presentation;
}

void CSynthPro::add(Module* module)
{
    CModule* cModule = dynamic_cast<CModule*>(module);

    if (cModule) {
        SynthPro::add(cModule);

        if (modules().contains(cModule)) {
            m_graphicsScene->addItem(cModule->presentation());
        }
    }
}

void CSynthPro::addModule(SynthProFactory::ModuleType moduleType, const QPointF& pos)
{
    Module* module = 0;

    switch (moduleType) {
    case SynthProFactory::KeyboardId:
        module = dynamic_cast<Module*>(m_factory->createKeyboard(this));
        break;
    case SynthProFactory::VCOId:
        module = dynamic_cast<Module*>(m_factory->createVCO(this));
        break;
    case SynthProFactory::VCFId:
        module = dynamic_cast<Module*>(m_factory->createVCF(this));
        break;
    case SynthProFactory::VCAId:
        module = dynamic_cast<Module*>(m_factory->createVCA(this));
        break;
    case SynthProFactory::ADSRId:
        module = dynamic_cast<Module*>(m_factory->createADSR(this));
        break;
    case SynthProFactory::DelayId:
        module = dynamic_cast<Module*>(m_factory->createDelay(this));
        break;
    case SynthProFactory::LFOId:
        module = dynamic_cast<Module*>(m_factory->createLFO(this));
        break;
    case SynthProFactory::AudioOuputId:
        module = dynamic_cast<Module*>(m_factory->createSpeaker(this));
        break;
    case SynthProFactory::FileOutputId:
        module = dynamic_cast<Module*>(m_factory->createWavRecorder(this, "output.wav", 480));
        break;
    case SynthProFactory::OscilloscopeId:
        module = dynamic_cast<Module*>(m_factory->createOscilloscope(this));
        break;
    default:
        break;
    }

    if (module) {
        add(module);
        dynamic_cast<CModule*>(module)->presentation()->setPos(pos);
    }
}

void CSynthPro::showFeedback(CVirtualPort* from)
{
    foreach (Module* module, modules()) {
        foreach (VirtualPort* port, module->inports()) {
            dynamic_cast<CInPort*>(port)->showCompatibleFeedback(from);
        }
        foreach (VirtualPort* port, module->outports()) {
            dynamic_cast<COutPort*>(port)->showCompatibleFeedback(from);
        }
    }
}

void CSynthPro::hideFeedback()
{
    foreach (Module* module, modules()) {
        foreach (VirtualPort* port, module->inports()) {
            dynamic_cast<CInPort*>(port)->hideFeedback();
        }
        foreach (VirtualPort* port, module->outports()) {
            dynamic_cast<COutPort*>(port)->hideFeedback();
        }
    }
}
