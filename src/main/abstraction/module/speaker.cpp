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
    , TimeCriticalModule(parent)
    , m_device(device)
    , m_inPort(0)
    , m_audioOutput(audioOutput)
    , m_generationBuffer(0)
    , m_generationBufferIndex(0)
    , m_nbGeneratedBytesRemaining(0)
    , m_clipping(false)
    , m_sequencer(Sequencer::instance())
{
}

Speaker::~Speaker()
{
    // Unregister the module as a fast timer to the Clock.
    Clock& clock = Clock::instance();
    clock.unregisterFastTimer(this);

    // Release the device.
    AudioDeviceProvider& adp = AudioDeviceProvider::instance();
    adp.releaseDevice();

    delete[] m_generationBuffer;
}

void Speaker::initialize(SynthProFactory* factory)
{
    m_generationBuffer = new char[GENERATION_BUFFER_SIZE];

    for (int i = 0; i < GENERATION_BUFFER_SIZE; i++) {
        m_generationBuffer[i] = 0;
    }

    if (factory) {
        // Creation of an Input.
        m_inPort = factory->createInPortReplicable(this, tr("in"));
        m_inports.append(m_inPort);

        // Register to a fast timer to the Clock.
        Clock& clock = Clock::instance();
        clock.registerFastTimer(this);
    }
}


void Speaker::fastTimerExpired()
{
    int fillCounter = 0;
    qint64 sizeWritten = 1;
    int nbBytesNeededByOutput = m_audioOutput->bytesFree();

    while (((fillCounter < FILL_COUNTER_MAX) && (nbBytesNeededByOutput > 0)) && (sizeWritten > 0)) {
        // Check if we have some generated bytes left in order to feed the output.
        if (m_nbGeneratedBytesRemaining > 0) {

            // We have some bytes left in our buffer. Is it enough ?
            if (m_nbGeneratedBytesRemaining >= nbBytesNeededByOutput) {
                // We have more than enough. We send only what is needed.
                sizeWritten = m_device->write(m_generationBuffer + m_generationBufferIndex, nbBytesNeededByOutput);
            } else {
                // We don't have enough. We send what we have for now.
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
            bool previousClipping = m_clipping;
            m_clipping = false;
            for (int i = 0, size = m_inPort->buffer()->length(); i < size; i++) {
                int nb = (int)(data[i] / VCO::SIGNAL_INTENSITY * SIGNAL_OUT_UNSIGNED_INTENSITY);
                // Limit tests.
                if (nb > SIGNAL_OUT_UNSIGNED_INTENSITY) {
                    nb = SIGNAL_OUT_UNSIGNED_INTENSITY;
                    m_clipping = true;
                } else if (nb < -SIGNAL_OUT_UNSIGNED_INTENSITY) {
                    nb = -SIGNAL_OUT_UNSIGNED_INTENSITY;
                    m_clipping = true;
                }
                // Fill the generation buffer with the data from the InPort.
                m_generationBuffer[iG++] = nb & 255;
                m_generationBuffer[iG++] = (nb / 256) & 255;
            }

            m_generationBufferIndex = 0;
            m_nbGeneratedBytesRemaining = Buffer::DEFAULT_LENGTH * 2;

            if (previousClipping != m_clipping) {
                previousClipping = m_clipping;
                emit clippingStateChanged(m_clipping);
            }
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
    // Part of the unsuccessful attempt to manage the soundcard
    // output from the Clock.
    // In order to test it, comment the code inside the timerExpired() method, and uncomment
    // the marked sections in Clock.cpp.

    /*
    // Adds the newly fetched Input buffer to our own, in a circular way.
    qreal* dataIn = m_inPort->buffer()->data();
    int generationBufferCurrentIndex = m_generationBufferIndex;
    int sizeInPortBuffer = m_inPort->buffer()->length();
    for (int i = 0; i < sizeInPortBuffer; i++) {
        int nb = (int)(dataIn[i] / VCO::SIGNAL_INTENSITY * SIGNAL_OUT_UNSIGNED_INTENSITY);
        // Limit tests.
        nb = (nb > SIGNAL_OUT_UNSIGNED_INTENSITY ? SIGNAL_OUT_UNSIGNED_INTENSITY : nb);
        nb = (nb < -SIGNAL_OUT_UNSIGNED_INTENSITY ? -SIGNAL_OUT_UNSIGNED_INTENSITY : nb);
        // Fill the generation buffer with the data from the InPort.
        m_generationBuffer[generationBufferCurrentIndex] = nb & 255;
        generationBufferCurrentIndex = (generationBufferCurrentIndex + 1) % GENERATION_BUFFER_SIZE;
        m_generationBuffer[generationBufferCurrentIndex] = (nb / 256) & 255;
        generationBufferCurrentIndex = (generationBufferCurrentIndex + 1) % GENERATION_BUFFER_SIZE;
    }
    m_nbGeneratedBytesRemaining += sizeInPortBuffer * 2;

    if (m_nbGeneratedBytesRemaining > GENERATION_BUFFER_SIZE) {
        qWarning() << "Generation Buffer size exceeded !";
    }

    // Send the data to the soundcard, according to what it needs.
    // We send all that is needed, unless we don't have enough of course.
    int nbBytesNeededByOutput = m_audioOutput->bytesFree();
    int bytesToSend = (m_nbGeneratedBytesRemaining < nbBytesNeededByOutput) ? m_nbGeneratedBytesRemaining : nbBytesNeededByOutput;

    if (bytesToSend > 0) {
        // Can we send everything is a row (because of the circular buffer) ?
        if ((m_generationBufferIndex + bytesToSend) < GENERATION_BUFFER_SIZE) {
            // Yes.
            int actuallySent = m_device->write(m_generationBuffer + m_generationBufferIndex, bytesToSend);

            m_generationBufferIndex += bytesToSend;
            if (actuallySent != bytesToSend) {
                qDebug() << "PASS ONE : NOT THE SAME AMOUNT WRITTEN !!";
            }
        } else {
            // No. We do it in two passes. First the end of the buffer.
            int firstPassNbBytesSent = GENERATION_BUFFER_SIZE - m_generationBufferIndex;
            int actuallySent;
            actuallySent = m_device->write(m_generationBuffer + m_generationBufferIndex, firstPassNbBytesSent);
            if (actuallySent != firstPassNbBytesSent) {
                qDebug() << "PASS TWO_1: NOT THE SAME AMOUNT WRITTEN !!";
            }

            // Then the beginning of the buffer.
            int secondPassNbBytesSent = bytesToSend - firstPassNbBytesSent;
            if (secondPassNbBytesSent > 0) {
                actuallySent = m_device->write(m_generationBuffer, secondPassNbBytesSent);
                if (actuallySent != secondPassNbBytesSent) {
                    qDebug() << "PASS TWO_2: NOT THE SAME AMOUNT WRITTEN !!";
                }
            }

            m_generationBufferIndex = bytesToSend - firstPassNbBytesSent;
        }
        m_nbGeneratedBytesRemaining -= bytesToSend;
    }
    */
}
