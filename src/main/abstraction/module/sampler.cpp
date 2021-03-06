#include "sampler.h"

#include "abstraction/audiodeviceprovider.h"
#include "abstraction/buffer.h"
#include "abstraction/component/dimmer.h"
#include "abstraction/component/inport.h"
#include "abstraction/component/outport.h"
#include "abstraction/component/pushbutton.h"
#include "abstraction/synthpro.h"
#include "factory/synthprofactory.h"

#include <QFile>
#include <QTextStream>

Sampler::Sampler(SynthPro* synth) 
    : Module(synth)
    , m_inPort(0)
    , m_gate(0)
    , m_outPort(0)
    , m_bpmDimmer(0)
    , m_recordButton(0)
    , m_stopButton(0)
    , m_playButton(0)
    , m_buffer(0)
    , m_bufferIndex(0)
    , m_sampleSize(0)
    , m_gateState(false)
    , m_oldGateState(false)
    , m_state(EMPTY)
    , m_positionInBuffer(0)
    , m_sampleStart(0)
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

    m_gate = factory->createInPortGate(this, tr("gate in"));
    m_inports.append(m_gate);

    m_bpmDimmer = factory->createDialDimmer(tr("Speed"), MIN_BPM, MAX_BPM, DEFAULT_BPM, this);

    m_recordButton = factory->createPushButton(tr("record"), this);
    m_stopButton = factory->createPushButton(tr("stop"), this);
    m_playButton = factory->createPushButton(tr("play"), this);

    m_recordButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_playButton->setEnabled(false);

    connect(m_recordButton, SIGNAL(buttonPushed()), this, SLOT(startRecording()));
    connect(m_stopButton, SIGNAL(buttonPushed()), this, SLOT(stopRecording()));
    connect(m_playButton, SIGNAL(buttonPushed()), this, SLOT(startPlaying()));

    // Emit starting value (0) and maximum recording length.
    emit valueChanged(0);
    emit lengthChanged(SAMPLER_MAX_DURATION * Buffer::DEFAULT_LENGTH - 1);

    int buffer_length = SAMPLER_MAX_DURATION * Buffer::DEFAULT_LENGTH;

    m_buffer = new Buffer(buffer_length);

    for (int i = 0; i < buffer_length; i++) {
        m_buffer->data()[i] = 0;
    }
}

void Sampler::startRecording()
{
    purgeBuffer(m_outPort->buffer());
    m_state = RECORDING;
    initializeBuffer();

    m_recordButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    m_playButton->setEnabled(false);

    emit valueChanged(0);
    emit lengthChanged(SAMPLER_MAX_DURATION * Buffer::DEFAULT_LENGTH - 1);
}

void Sampler::stopRecording()
{
    purgeBuffer(m_outPort->buffer());
    m_sampleStart = 0;
    int i = 0;
    while (m_buffer->data()[++i] == 0) {
        m_sampleStart++;

    }

    m_state = WAITING;

    m_recordButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_playButton->setEnabled(true);
}

void Sampler::startPlaying()
{
    m_state = PLAYING;
    m_bufferIndex = 0;
    m_positionInBuffer = m_sampleStart;
    m_recordButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    m_playButton->setEnabled(false);

    emit valueChanged(0);
    emit lengthChanged(m_sampleSize);
}

void Sampler::ownProcess()
{
    qreal speed = m_bpmDimmer->value();

    int sampleMaxInByte = SAMPLER_MAX_DURATION * Buffer::DEFAULT_LENGTH;

    // do a loop in the buffer, test the state of the gate (and his evolution) and do the reading or writing necessary.
    for (int i = 0; i < Buffer::DEFAULT_LENGTH; i++) {

        m_gateState = m_gate->buffer()->data()[i] > 0;

        bool gateUp = (m_gateState && !m_oldGateState) || (m_oldGateState && !m_gateState);

        if (m_state == RECORDING && (gateUp || m_sampleSize == sampleMaxInByte)) {
            m_state = WAITING;
            stopRecording();
            gateUp = !gateUp;
        }
        if (m_state== RECORDING && m_sampleSize == sampleMaxInByte) {
            startPlaying();
            gateUp = !gateUp;
        }

        if (m_state == EMPTY && gateUp) {
            startRecording();
        }

        if (m_state == WAITING && gateUp) {
            // event gateUp has been used
            gateUp = !gateUp;
            startPlaying();
        }

        if (m_state == PLAYING) {
            if (gateUp) {
                stopRecording();
                // m_state = WAITING;
            }
        }

        switch (m_state) {
        case WAITING : break;

        case EMPTY : break;

        case PLAYING :
            m_positionInBuffer += speed;

            if ((i % NB_VALUES_CHANGED_BEFORE_EMIT_VALUECHANGED) == 0) {
                emit valueChanged(m_positionInBuffer);
            }

            if (m_positionInBuffer > m_sampleSize && speed > 0) {
                m_positionInBuffer = m_sampleStart;
            }
            if (speed <= 0 && m_positionInBuffer <= 0) {
                m_positionInBuffer = m_sampleSize - 1;
            }

            m_outPort->buffer()->data()[i] = m_buffer->data()[(int)m_positionInBuffer];
            break;

        case RECORDING :
            m_buffer->data()[Buffer::DEFAULT_LENGTH * m_bufferIndex + i] = m_inPort->buffer()->data()[i];

            // Needed to update the UI, but no need to over-update it with signals !
            if ((i % NB_VALUES_CHANGED_BEFORE_EMIT_VALUECHANGED) == 0) {
                emit valueChanged(m_sampleSize);
            }

            // If we still record at the end of the buffer : increment m_bufferIndex
            if (i == Buffer::DEFAULT_LENGTH - 1) {
                m_bufferIndex++;
            }
            m_sampleSize++;
            break;

        default : break;
        } // switch

        m_oldGateState = m_gateState;
    } // for
} // ownprocess()

void Sampler::initializeBuffer()
{
    m_bufferIndex = 0;
    m_sampleSize = 0;
}


void Sampler::purgeBuffer(Buffer* buf)
{
    for (int i = 0; i < Buffer::DEFAULT_LENGTH; i++) {
        buf->data()[i] = 0;
    }
}
