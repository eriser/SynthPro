#include "speaker.h"

#include "abstraction/audiodeviceprovider.h"
#include "abstraction/buffer.h"
#include "abstraction/clock.h"
#include "abstraction/component/inport.h"
#include "abstraction/module/vco.h"
#include "abstraction/sequencer.h"
#include "factory/synthprofactory.h"

#include <QAudioOutput>
#include <QDebug>

Speaker::Speaker(SynthPro* parent, QIODevice* device, QAudioOutput* audioOutput)
    : Module(parent)
    , m_device(device)
    , m_audioOutput(audioOutput)
    , m_generationBuffer(0)
    , m_generationBufferIndex(0)
    , m_nbGeneratedBytesRemaining(0)
    , m_sequencer(Sequencer::instance())
{
}

Speaker::~Speaker()
{
    // Unregister the module as a fast timer to the Clock.
    Clock& clock = Clock::instance();
    clock.unregister(this);

    // Release the device.
    AudioDeviceProvider& adp = AudioDeviceProvider::instance();
    adp.releaseDevice();

    delete[] m_generationBuffer;
}

void Speaker::initialize(SynthProFactory* factory)
{
    int size = Buffer::DEFAULT_LENGTH * 2;
    m_generationBuffer = new char[size];

    for (int i = 0; i < size; i++) {
        m_generationBuffer[i] = 0;
    }

    if (factory) {
        // Creation of an Input.
        m_inPort = factory->createInPortReplicable(this, tr("in"));
        m_inports.append(m_inPort);

        // Register to a fast timer to the Clock.
        Clock& clock = Clock::instance();
        clock.registerFastClock(this);
    }
}

void Speaker::timerExpired()
{
    int fillCounter = 0;
    qint64 sizeWritten = 1;
    int nbBytesNeededByOutput = m_audioOutput->bytesFree();

    while (((fillCounter < FILL_COUNTER_MAX) && (nbBytesNeededByOutput > 0)) && (sizeWritten > 0)) {
        // qWarning() << "Needing " << nbBytesNeededByOutput << ". Counter = " << fillCounter;
        // Check if we have some generated bytes left in order to feed the output.
        if (m_nbGeneratedBytesRemaining > 0) {

            // We have some bytes left in our buffer. Is it enough ?
            if (m_nbGeneratedBytesRemaining >= nbBytesNeededByOutput) {
                // We have more than enough. We send only what is needed.
                // qWarning() << "Trying to write : " << nbBytesNeededByOutput;
                sizeWritten = m_device->write(m_generationBuffer + m_generationBufferIndex, nbBytesNeededByOutput);
            } else {
                // We don't have enough. We send what we have for now.
                // qWarning() << "Trying to write : " << m_nbGeneratedBytesRemaining;
                sizeWritten = m_device->write(m_generationBuffer + m_generationBufferIndex, m_nbGeneratedBytesRemaining);
            }

            m_generationBufferIndex += sizeWritten;
            m_nbGeneratedBytesRemaining -= sizeWritten;

            nbBytesNeededByOutput = m_audioOutput->bytesFree();
        } else {
            // We don't have any bytes in our buffer. We need to generate data.
            // We call the sequencer for it to process all the modules.
            m_sequencer.process();

            // Now we copy our InPort to the generationBuffer. A conversion is needed.
            qreal* data = m_inPort->buffer()->data();

            int iG = 0;
            for (int i = 0, size = m_inPort->buffer()->length(); i < size; i++) {
                int nb = (int)(data[i] / VCO::SIGNAL_INTENSITY * SIGNAL_OUT_UNSIGNED_INTENSITY);
                // Limit tests.
                nb = (nb > SIGNAL_OUT_UNSIGNED_INTENSITY ? SIGNAL_OUT_UNSIGNED_INTENSITY : nb);
                nb = (nb < -SIGNAL_OUT_UNSIGNED_INTENSITY ? -SIGNAL_OUT_UNSIGNED_INTENSITY : nb);

                int nb1 = (nb / 256) & 255;
                int nb2 = nb & 255;
                m_generationBuffer[iG++] = nb2;
                m_generationBuffer[iG++] = nb1;
            }

            m_generationBufferIndex = 0;
            m_nbGeneratedBytesRemaining = Buffer::DEFAULT_LENGTH * 2;
        }

        fillCounter++;
    }

    /*
    if (fillCounter >= FILL_COUNTER_MAX) {
        qWarning() << "Unable to feed the sound card enough !";
    }
    if (sizeWritten == 0) {
        qWarning() << "Size written = 0... Abnormal.";
    }
    if (sizeWritten < 0) {
        qWarning() << "Error while writing to the sound card !";
    }
    */
}

void Speaker::ownProcess()
{
    /*
    int nbBytesNeededByOutput = m_audioOutput->bytesFree();
    int sizeWritten = 1;
    //if (nbBytesNeededByOutput > 0) {
    while ((nbBytesNeededByOutput > 0) && (sizeWritten > 0)) {
        if (m_nbGeneratedBytesRemaining > 0) {
            sizeWritten = sendToAudioOutput(nbBytesNeededByOutput);
        } else {
            // We don't have any bytes in our buffer.
            // Now we copy our InPort to the generationBuffer. A conversion is needed.
            qreal* data = m_inPort->buffer()->data();

            int iG = 0;
            for (int i = 0, size = m_inPort->buffer()->length(); i < size; i += 1) {
                int nb = (int)(data[i] / VCO::SIGNAL_INTENSITY * SIGNAL_OUT_UNSIGNED_INTENSITY);
                // Two channels are required, regardless of the actual sound channel used.
                int nb1 = (nb / 256) & 255;
                int nb2 = nb & 255;
                m_generationBuffer[iG++] = nb2;
                m_generationBuffer[iG++] = nb1;
                //m_generationBuffer[iG++] = nb2;
                //m_generationBuffer[iG++] = nb1;
            }

            m_generationBufferIndex = 0;
            m_nbGeneratedBytesRemaining = Buffer::DEFAULT_LENGTH * 2;

            sizeWritten = sendToAudioOutput(nbBytesNeededByOutput);
        }
        nbBytesNeededByOutput = m_audioOutput->bytesFree();
    }
    */
}

/*
qint64 Speaker::sendToAudioOutput(int nbBytesNeededByOutput) {
    qint64 sizeWritten;
    // We have some bytes left in our buffer. Is it enough ?
    if (m_nbGeneratedBytesRemaining >= nbBytesNeededByOutput) {
        // We have more than enough. We send only what is needed.
        // qWarning() << "Trying to write : " << nbBytesNeededByOutput;
        sizeWritten = m_device->write(m_generationBuffer + m_generationBufferIndex, nbBytesNeededByOutput);
        //qDebug() << "Needed = " << nbBytesNeededByOutput << "  Written = " << sizeWritten;
    } else {
        // We don't have enough. We send what we have for now.
        // qWarning() << "Trying to write : " << m_nbGeneratedBytesRemaining;
        sizeWritten = m_device->write(m_generationBuffer + m_generationBufferIndex, m_nbGeneratedBytesRemaining);
        //qDebug() << "Needed = " << nbBytesNeededByOutput << "  No enough data. Written = " << sizeWritten << " out of " << m_nbGeneratedBytesRemaining;
    }

    m_generationBufferIndex += sizeWritten;
    m_nbGeneratedBytesRemaining -= sizeWritten;

    return sizeWritten;
}
*/
