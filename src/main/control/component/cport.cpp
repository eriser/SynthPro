#include "cport.h"

#include "control/cmodule.h"
#include "control/component/cvirtualport.h"
#include "control/component/cwire.h"
#include "control/csynthpro.h"
#include "factory/qtfactory.h"
#include <QGraphicsScene>

CPort::CPort(CVirtualPort* parent, QtFactory* factory)
    : QObject(parent)
    , m_vPort(parent)
    , m_presentation(0)
    , m_factory(factory)
    , m_wire(0)
    , m_tmpWire(0)
    , m_dropablePort(0)
    , m_reconnecting(false)
    , m_oldConnection(0)
{
}

CPort::~CPort()
{
    if (!m_presentation.isNull()) {
        delete m_presentation;
    }
}

void CPort::setPresentation(PPort* presentation)
{
    if (!m_presentation.isNull()) {
        delete m_presentation;
    }

    m_presentation = presentation;
}

void CPort::setWire(CWire* wire)
{
    m_wire = wire;
}

void CPort::disconnect()
{
    vPort()->disconnect(this);
}

void CPort::reconnect(const QPointF& pos)
{
    CPort* other = m_wire->inPort() == this ? m_wire->outPort() : m_wire->inPort();
    m_reconnecting = true;
    m_oldConnection = other;
    other->m_reconnecting = true;
    other->m_oldConnection = this;
    disconnect();
    createTmpWire(other, pos);
}

void CPort::createTmpWire(CPort* from, const QPointF& to)
{
    // Create a temporary wire
    m_tmpWire = m_factory->createWire(presentation()->scene());

    // Don't forget to register ourself as one of the port
    m_tmpWire->setInPort(from);
    m_tmpWire->showMoveFeedback();
    m_tmpWire->updatePosition(to);

    dynamic_cast<CSynthPro*>(vPort()->module()->synthPro())->showFeedback(from->vPort());
}

void CPort::drag(const QPointF& pos)
{
    if (m_wire) {
        reconnect(pos);
    } else {
        createTmpWire(this, pos);
    }
}

void CPort::dragMove(const QPointF& pos)
{
    CPort* source = m_oldConnection ? m_oldConnection : this;
    PPort* pport  = dynamic_cast<PPort*>(presentation()->scene()->itemAt(pos));
    if (pport != this->presentation()) {
        if (pport) {
            // There’s a port under the cursor position

            if (!m_dropablePort) {
                // We just entered in the port
                m_dropablePort = pport;
                if (source->vPort()->connectable(pport->control()->vPort())) {
                    pport->showDropFeedback();
                } else {
                    pport->showUnDropFeedback();
                }
            }
        } else {
            // There’s no port under the cursor position
            if (m_dropablePort) {
                m_dropablePort->hideDropFeedback();
                m_dropablePort = 0;
            }
        }
    }
    if (m_tmpWire) {
        m_tmpWire->updatePosition(pos);
    }
}

void CPort::drop(CPort* target)
{
    m_dropablePort = 0; // Clean-up
    dynamic_cast<CSynthPro*>(vPort()->module()->synthPro())->hideFeedback();

    // Delete the temporary wire
    delete m_tmpWire;
    m_tmpWire = 0;

    CPort* source = reconnecting() ? m_oldConnection : this;
    // If the user dropped on a target, try to connect to it
    if (target) {
        if (reconnecting()) {
            m_oldConnection->vPort()->connect(m_oldConnection, target);
            if (target != this) {
                vPort()->removeConnectionPort(this);
            } else {
                // The drop was over this port
                m_reconnecting = false;
            }
            m_oldConnection->setReconnecting(false);
        } else {
            source->vPort()->connect(source, target);
        }
    } else if (reconnecting()) { // The reconnection is finished, remove the connection ports
        m_oldConnection->vPort()->removeConnectionPort(m_oldConnection);
        vPort()->removeConnectionPort(this);
    }
}

void CPort::mouseEnter()
{
    presentation()->showClickFeedback();
}

void CPort::mouseLeave()
{
    presentation()->hideClickFeedback();
}

void CPort::showCompatibleFeedback()
{
    presentation()->showCompatibleFeedback();
}

void CPort::showConnectableFeedback()
{
    presentation()->showConnectableFeedback();
}

void CPort::showUnconnectableFeedback()
{
    presentation()->showUnconnectableFeedback();
}

void CPort::hideFeedback()
{
    presentation()->hideFeedback();
}

void CPort::showAvailableFeedback()
{
    presentation()->show();
}


void CPort::hideAvailableFeedback()
{
    presentation()->hide();
}
