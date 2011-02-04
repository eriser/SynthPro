#include "module.h"

#include "abstraction/inport.h"
#include "abstraction/outport.h"

Module::Module(QObject* parent)
    : QObject(parent)
{
}

Module::~Module()
{

}

const QList<Module*> Module::getReguirements() const
{
    m_requirements.clear();

    foreach (InPort* port, inports()) {
        foreach (Port *connected, port->connections()) {
            m_requirements.append(connected->module());
        }
    }

    return m_requirements;
}

void Module::fetchInput()
{
    foreach (InPort* input, m_inports) {
        input->fetch();
    }
}
