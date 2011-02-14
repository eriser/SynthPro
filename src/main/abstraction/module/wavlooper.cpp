#include "wavlooper.h"

#include "abstraction/audiodeviceprovider.h"
#include "abstraction/component/outport.h"
#include "abstraction/synthpro.h"
#include "factory/synthprofactory.h"

#include <QDebug>
#include <QFile>

WavLooper::WavLooper(SynthPro* parent)
    : Module(parent)
    , m_outPort(0)
    , m_fileName("essai.wav")
    , m_inputFile(0)
    , m_internalBuffer(0)
    , m_positionInInternalBuffer(0)
{
}

WavLooper::~WavLooper()
{
    if (m_inputFile) {
        m_inputFile->close();
    }
}

void WavLooper::initialize(SynthProFactory* factory)
{
    m_outPort = factory->createOutPortReplicable(this, "out");
    m_outports.append(m_outPort);
}

void WavLooper::newFile(const QString& filename)
{
    if (m_inputFile) {
        m_inputFile->close();
    }

    // Open a new file.
    m_fileName = filename;
    m_inputFile = new QFile(filename);

    if (!m_inputFile->open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open file. " << m_inputFile->fileName();
    } else if (!readWavFile(m_inputFile)) {
        qWarning("Format of file read unknown.");
    }

    m_inputFile->close();
}

void WavLooper::ownProcess()
{
    if (m_internalBuffer) {
        qreal* dataInPort = m_internalBuffer->data();
        qreal* dataOutPort = m_outPort->buffer()->data();
        if ((m_internalBuffer->length() - m_positionInInternalBuffer) > Buffer::DEFAULT_LENGTH) {
            // We have enough data in our internal buffer.
            for (int i = 0; i < Buffer::DEFAULT_LENGTH; i++) {
                dataOutPort[i] = dataInPort[m_positionInInternalBuffer + i];
            }
            m_positionInInternalBuffer += Buffer::DEFAULT_LENGTH;
        } else {
            // We don't have enough data. We copy what we can...
            int sizeToCopy = m_internalBuffer->length() - m_positionInInternalBuffer;
            int i;
            for (i = 0; i < sizeToCopy; i++) {
                dataOutPort[i] = dataInPort[m_positionInInternalBuffer + i];
            }

            m_positionInInternalBuffer += i;
            int offset = i;
            sizeToCopy = Buffer::DEFAULT_LENGTH - sizeToCopy;
            // Then we copy the rest from the beginning of the internal buffer;
            for (i = 0; i < sizeToCopy; i++) {
                dataOutPort[offset] = dataInPort[m_positionInInternalBuffer + i];
            }
            m_positionInInternalBuffer = 0;
        }
    }
}

bool WavLooper::readWavFile(QFile* file)
{
     // Check the WAV format.
    if ((QString)(file->read(4)) != "RIFF") {
        return false;
    }
    file->read(4); // Skip Riff size.
    if ((QString)(file->read(8)) != "WAVEfmt ") {
        return false;
    }
    if (readLittleEndianInt(file) != AudioDeviceProvider::BIT_RATE) {
        return false;
    }
    if (readLittleEndianShort(file) != 1) {
        return false;
    } // Compression code.
    if (readLittleEndianShort(file) != AudioDeviceProvider::NB_CHANNELS) {
        return false;
    }

    // FIXME
    // if (readLittleEndianInt(file) != AudioDeviceProvider::OUTPUT_FREQUENCY) { return false; }
    readLittleEndianInt(file);

    // FIXME too !!
    int blockAlign = AudioDeviceProvider::NB_CHANNELS * (AudioDeviceProvider::BIT_RATE / 8);
    readLittleEndianInt(file);
    // qDebug() << nb;
    // if (nb != (blockAlign * AudioDeviceProvider::OUTPUT_FREQUENCY)) { return false; }

    if (readLittleEndianShort(file) != blockAlign) {
        return false;
    }
    if (readLittleEndianShort(file) != AudioDeviceProvider::BIT_RATE) {
        return false;
    }

    // Data chunk
    if ((QString)(file->read(4)) != "data") {
        return false;
    }
    file->read(4); // Skip data chunk size.

    // Copy the wave into the buffer.
    int wavSize = (file->size() - file->pos());
    m_internalBuffer = new Buffer(wavSize);

    for (int i = 0; i < wavSize; i += 2) {
        m_internalBuffer->data()[i] = readLittleEndianShort(file);
    }

    return true;
}

int WavLooper::readLittleEndianInt(QFile* file)
{
    char tab[4];
    file->read(tab, 4);
    int nb = tab[3] * 0x1000000;
    nb += tab[2] * 0x10000;
    nb += tab[1] * 0x100;
    nb += tab[0];
    return nb;
}

int WavLooper::readLittleEndianShort(QFile* file)
{
    char tab[2];
    file->read(tab, 2);
    int nb = tab[1] * 0x100;
    nb += tab[0];
    return nb;
}
