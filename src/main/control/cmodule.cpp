#include "cmodule.h"

#include "abstraction/component/inport.h"
#include "abstraction/component/outport.h"
#include "abstraction/synthpro.h"
#include "control/component/cvirtualport.h"
#include "control/component/cwire.h"
#include <QSignalMapper>

CModule::CModule(SynthPro* parent)
    : Module(parent)
    , m_presentation(0)
{
}

CModule::~CModule()
{
    if (!m_presentation.isNull()) {
        // FIXME We need to delete the presentation when we are simply closing the module.
        // But it fails on this same deletion when closing the entire application…
        delete m_presentation;
    }
}

void CModule::setPresentation(PModule* presentation)
{
    if (!m_presentation.isNull()) {
        delete m_presentation;
    }

    m_presentation = presentation;
    QSignalMapper* mapper = new QSignalMapper(this);
    QObject::connect(presentation, SIGNAL(closeBtnClicked()), mapper, SLOT(map()));
    mapper->setMapping(presentation, this);
    QObject::connect(mapper, SIGNAL(mapped(QObject*)), synthPro(), SLOT(remove(QObject*)));
}

void CModule::move()
{
    foreach (InPort* port, m_inports) {
        dynamic_cast<CVirtualPort*>(port)->updateWiresPositions();
    }

    foreach (OutPort* port, m_outports) {
        dynamic_cast<CVirtualPort*>(port)->updateWiresPositions();
    }
}
