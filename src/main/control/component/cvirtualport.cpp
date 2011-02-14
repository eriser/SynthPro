#include "cvirtualport.h"

#include "abstraction/component/connection.h"
#include "control/component/cinport.h"
#include "control/component/coutport.h"
#include "control/component/cport.h"
#include "control/component/cwire.h"
#include "control/csynthpro.h"
#include "factory/qtfactory.h"
#include "presentation/component/pport.h"
#include "presentation/component/pvirtualport.h"
#include "presentation/component/pwire.h"

CVirtualPort::CVirtualPort(Module* parent, QtFactory* factory, const QString& name, bool replicable, bool gate)
    : VirtualPort(parent, name, factory, replicable, gate)
    , m_presentation(0)
    , m_factory(factory)
    , m_availablePort(0)
{
}

void CVirtualPort::initialize()
{
    m_availablePort = m_factory->createPort(this);
    presentation()->initialize(m_availablePort->presentation());

    updateAvailableFeedback();
}

void CVirtualPort::updateAvailableFeedback()
{
    if (available()) {
        m_availablePort->showAvailableFeedback();
    } else {
        m_availablePort->hideAvailableFeedback();
    }
}

Connection* CVirtualPort::connect(VirtualPort* other)
{
    CVirtualPort* cOther = dynamic_cast<CVirtualPort*>(other);
    Connection* connection = VirtualPort::connect(cOther);
    if (connection) {
        CPort* source = 0;
        CPort* target = 0;
        if (out()) {
            source = createConnectionPort(connection);
            target = cOther->createConnectionPort(connection);
        } else {
            source = cOther->createConnectionPort(connection);
            target = createConnectionPort(connection);
        }
        CWire* wire = m_factory->createWire(presentation()->scene());
        source->setWire(wire);
        target->setWire(wire);
        wire->setOutPort(source);
        wire->setInPort(target);
        wire->updatePosition();
        updateAvailableFeedback();
    }
    return connection;
}

CPort* CVirtualPort::createConnectionPort(Connection* connection)
{
    int idx = m_connections.indexOf(connection);
    CPort* port = m_factory->createPort(this);
    presentation()->addConnectionPort(port->presentation());
    m_connectedPorts.insert(idx, port);
    return port;
}

void CVirtualPort::disconnect(CPort* port)
{
    int idx = m_connectedPorts.key(port, -1);
    if (idx >= 0) {
        Connection* connection = m_connections.at(idx);
        disconnect(connection);
    }
}

bool CVirtualPort::disconnect(Connection* connection)
{
    CVirtualPort* other = dynamic_cast<CVirtualPort*>(connection->source()) == this
                          ? dynamic_cast<CVirtualPort*>(connection->target())
                          : dynamic_cast<CVirtualPort*>(connection->source());
    int idx = m_connections.indexOf(connection);
    if (VirtualPort::disconnect(connection)) {
        delete m_connectedPorts.value(idx)->wire()->presentation();
        // delete m_connectedPorts.value(idx)->wire();
        deleteConnectionPort(idx);
        other->deleteConnectionPort(idx);
        return true;
    }
    return false;
}

void CVirtualPort::deleteConnectionPort(int idx)
{
    CPort* port = m_connectedPorts.value(idx);
    m_connectedPorts.remove(idx);
    presentation()->removeConnectionPort(port->presentation());
    // delete port->presentation();
    // delete port;
}

void CVirtualPort::setPresentation(PVirtualPort* presentation)
{
    if (m_presentation) {
        delete m_presentation;
        m_presentation = 0;
    }

    m_presentation = presentation;
}

void CVirtualPort::updateWiresPositions()
{
    foreach (CPort* port, m_connectedPorts) {
        port->wire()->updatePosition();
    }
}

void CVirtualPort::showCompatibleFeedback(CVirtualPort* from)
{
    bool isCompatible = compatible(from);
    if (from != this) {
        m_availablePort->showFeedback(isCompatible);
        foreach (CPort* port, m_connectedPorts) {
            port->showFeedback(isCompatible);
        }
    }
}

void CVirtualPort::hideFeedback()
{
    m_availablePort->hideFeedback();
    foreach (CPort* port, m_connectedPorts) {
        port->hideFeedback();
    }
}