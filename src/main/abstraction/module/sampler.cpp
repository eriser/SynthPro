#include "sampler.h"

#include "abstraction/audiodeviceprovider.h"
#include "abstraction/buffer.h"
#include "abstraction/component/dimmer.h"
#include "abstraction/component/inport.h"
#include "abstraction/component/outport.h"
#include "abstraction/component/pushbutton.h"
#include "abstraction/synthpro.h"
#include "factory/synthprofactory.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>


Sampler::Sampler(SynthPro* synth) 
    : Module(synth)
    , m_bufferIndex(0)
    , m_sampleSize(0)
    , m_gateState(false)
    , m_oldGateState(false)
{

}

Sampler::~Sampler()
{
    delete m_buffer;
}

void Sampler::initialize(SynthProFactory* factory)
{
    m_inPort = factory->createInPort(this, tr("in"));
    m_inports.append(m_inPort);

    m_outPort = factory->createOutPort(this, tr("out"));
    m_outports.append(m_outPort);

    m_gate = factory->createInPortGate(this, tr("gate"));
    m_inports.append(m_gate);

    m_bpmDimmer = factory->createDialDimmer(tr("bpm"), MIN_BPM, MAX_BPM, DEFAULT_BPM, this);

    int buffer_length = SAMPLER_MAX_DURATION * Buffer::DEFAULT_LENGTH;

    m_buffer = new Buffer(buffer_length);
    for (int i = 0; i < buffer_length; i++) {
        m_buffer->data()[i] = 0;
    }

    m_state = EMPTY;
}

void Sampler::startRecording()
{
    purgeBuffer(m_outPort->buffer());
    m_state = RECORDING;
    initializeBuffer();

}

void Sampler::stopRecording()
{
    saveBuffer("buffer.out");
    purgeBuffer(m_outPort->buffer());
    m_state = WAITING;
}

void Sampler::startPlaying()
{
    m_state = PLAYING;
    m_bufferIndex = 0;
}

void Sampler::ownProcess()
{
    int sampleMaxInByte = SAMPLER_MAX_DURATION * Buffer::DEFAULT_LENGTH;

    for (int i = 0; i < Buffer::DEFAULT_LENGTH; i++) {
        if (m_gate->buffer()->data()[i] != 0) {
            m_gateState = true;
        } else {
            m_gateState = false;
        }
        bool gateUp = m_gateState && !m_oldGateState;

        if (m_state == RECORDING && (gateUp || m_sampleSize == sampleMaxInByte)) {
            m_state = WAITING;
        }

        if (m_state == EMPTY && gateUp) {
            startRecording();
        }

        if (m_state == WAITING && gateUp) {
            qDebug() << "start playing ";
            // event gateUp has been used
            gateUp = !gateUp;
            startPlaying();
        }

        if (m_state == PLAYING) {
            if (gateUp) {
                m_state = WAITING;
            }

//            if (m_sampleSize == sampleMaxInByte) {
//                // if the buffer is entirely readed, restart the reading at begin of buffer
//                m_bufferIndex = 0;
//            }
        }

        switch (m_state) {
        case WAITING : break;
        case EMPTY : break;
        case PLAYING :
            m_outPort->buffer()->data()[i] = m_buffer->data()[m_bufferIndex * Buffer::DEFAULT_LENGTH + i];
            if (i == Buffer::DEFAULT_LENGTH - 1) {
                m_bufferIndex++;
            }
            if (m_bufferIndex == m_sampleSize / Buffer::DEFAULT_LENGTH) {
                m_bufferIndex = 0;
            }
            break;
        case RECORDING :
            m_buffer->data()[Buffer::DEFAULT_LENGTH * m_bufferIndex + i] = m_inPort->buffer()->data()[i];
          //  qDebug() << "buffer << " << m_inPort->buffer()->data()[i];
            emit valueChanged(m_sampleSize);
            // if we still record at the end of the buffer : increment m_bufferIndex
            if (i == Buffer::DEFAULT_LENGTH - 1) {
                m_bufferIndex++;
            }
            m_sampleSize++;
            break;
        default : break;
        } // switch
        m_oldGateState = m_gateState;
    } // for

    qDebug() << "state "<< state() << m_bufferIndex;
} // ownprocess()

void Sampler::initializeBuffer()
{
    m_bufferIndex = 0;
    m_sampleSize = 0;
}

QString Sampler::state()
{
    switch (m_state) {
    case WAITING : return "waiting";
    case PLAYING : return "playing";
    case RECORDING : return "recording";
    case EMPTY : return "empty";
    default: return "error";
    }
}
Buffer* Sampler::sampleBuffer()
{
    return m_buffer;
}
void Sampler::purgeBuffer(Buffer* buf)
{
    for (int i = 0 ; i < Buffer::DEFAULT_LENGTH ; i++) {
        buf->data()[i] = 0;
    }
}
void Sampler::saveBuffer(const QString & fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    for (int i = 0 ; i < m_sampleSize ; i++) {
        out << m_buffer->data()[i] << "\n";
    }

    file.close();
}
