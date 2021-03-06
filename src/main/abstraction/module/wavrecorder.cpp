#include "wavrecorder.h"

#include "abstraction/audiodeviceprovider.h"
#include "abstraction/buffer.h"
#include "abstraction/component/inport.h"
#include "abstraction/component/pushbutton.h"
#include "abstraction/module/speaker.h"
#include "abstraction/module/vco.h"
#include "factory/synthprofactory.h"

#include <QFile>

WavRecorder::WavRecorder(SynthPro* parent, int nbProcessingBeforeSaving)
    : Module(parent)
    , m_inPort(0)
    , m_recordButton(0)
    , m_stopButton(0)
    , m_fileName(QString())
    , m_outputFile(0)
    , m_isRecording(false)
    , m_nbProcessingBeforeSaving(nbProcessingBeforeSaving)
    , m_nbProcessingSaved(0)
    , m_riffDataSizePosition(0)
    , m_waveDataSizePosition(0)
    , m_dataLength(0)
    , m_bufferForNumbers(0)
{
    m_bufferForNumbers = new char(4); // The buffer is only used to write int32 or short (16 bits),
                                      // as requested by the WAV format.
}

WavRecorder::~WavRecorder()
{
    if (m_outputFile) {
        closeWAVFile();
    }
}

void WavRecorder::initialize(SynthProFactory* factory)
{
    // Creation of an Input.
    m_inPort = factory->createInPortReplicable(this, tr("in"));
    m_inports.append(m_inPort);

    m_recordButton = factory->createPushButton(tr("record"), this);
    m_stopButton = factory->createPushButton(tr("stop"), this);

    connect(m_recordButton, SIGNAL(buttonPushed()), this, SLOT(startRecording()));
    connect(m_stopButton, SIGNAL(buttonPushed()), this, SLOT(stopRecording()));
}

void WavRecorder::newFile(const QString& fileName)
{
    if (m_outputFile) {
        // Close the currently used file.
        closeWAVFile();

        // And reset the variables.
        m_nbProcessingSaved = 0;
        m_riffDataSizePosition = 0;
        m_waveDataSizePosition = 0;
        m_dataLength = 0;
        m_isRecording = false;
    }

    // Create a new file.
    m_fileName = fileName;
    m_outputFile = new QFile(fileName);

    if (!m_outputFile->open(QIODevice::WriteOnly)) {
        qWarning("Unable to create output file.");
    } else {
        createWAVHeader(m_outputFile);
        m_recordButton->setEnabled(true);
        m_stopButton->setEnabled(false);
    }
}

void WavRecorder::startRecording()
{
    m_isRecording = true;
    m_recordButton->setEnabled(false);
    m_stopButton->setEnabled(true);
}

void WavRecorder::stopRecording()
{
    m_isRecording = false;
    m_recordButton->setEnabled(false);
    m_stopButton->setEnabled(false);
    closeWAVFile();
}

void WavRecorder::ownProcess()
{
    if (m_outputFile && m_isRecording) {
        // Save the buffer from the first In Port found.
        if (m_inports.count() > 0) {
            InPort* port = m_inports.at(0);

            if (port) { // Shouldn't be useful.
                qreal* data = port->buffer()->data();

                for (int i = 0, size = port->buffer()->length(); i < size; i++) {
                    // For efficiency, this is the addLittleEndianShortToFile method copied here.
                    // The stored value is 16 bits, signed, little endian, normalised.
                    int nb = (int)(data[i] / VCO::SIGNAL_INTENSITY * SIGNAL_OUT_SIGNED_INTENSITY);

                    // Limit tests.
                    nb = (nb > SIGNAL_OUT_SIGNED_INTENSITY ? SIGNAL_OUT_SIGNED_INTENSITY : nb);
                    nb = (nb < -SIGNAL_OUT_SIGNED_INTENSITY ? -SIGNAL_OUT_SIGNED_INTENSITY : nb);

                    m_bufferForNumbers[0] = nb & 255;
                    m_bufferForNumbers[1] = (nb / 256) & 255;
                    m_outputFile->write(m_bufferForNumbers, 2);

                    m_dataLength += 2;
                }
            }
        }

        if (m_nbProcessingBeforeSaving != 0 && ++m_nbProcessingSaved >= m_nbProcessingBeforeSaving) {
            // If a limit of processing is set,
            // Close the file as we reached the number of processing wanted.
            closeWAVFile();
        }
    }
}

void WavRecorder::createWAVHeader(QFile* file)
{
    file->write("RIFF");

    // Chunk data size. Save the position in order to fill it later.
    m_riffDataSizePosition = file->pos();
    addLittleEndianIntToFile(file, 0xffffffff); // Use a fake size, for now.
    file->write("WAVE");

    // Format chunk
    file->write("fmt ");
    addLittleEndianIntToFile(file, 16); // Chunk format data size.
    addLittleEndianShortToFile(file, 1); // Compression code.
    addLittleEndianShortToFile(file, AudioDeviceProvider::NB_CHANNELS);
    addLittleEndianIntToFile(file, AudioDeviceProvider::OUTPUT_FREQUENCY);
    int blockAlign = AudioDeviceProvider::NB_CHANNELS * (AudioDeviceProvider::BIT_RATE / 8);
    addLittleEndianIntToFile(file, blockAlign * AudioDeviceProvider::OUTPUT_FREQUENCY); // Average bytes per second.
    addLittleEndianShortToFile(file, blockAlign);
    addLittleEndianShortToFile(file, AudioDeviceProvider::BIT_RATE);

    // Data chunk
    file->write("data");
    m_waveDataSizePosition = file->pos();
    addLittleEndianIntToFile(file, 0xffffffff); // Chunk Format Data Size. Use a fake size, for now.
}

void WavRecorder::closeWAVFile()
{
    if (m_outputFile) {
        // Set the previously skipped size.
        m_outputFile->seek(m_riffDataSizePosition);
        addLittleEndianIntToFile(m_outputFile, m_outputFile->size() - 8); // The header doesn't count.
        m_outputFile->seek(m_waveDataSizePosition);
        addLittleEndianIntToFile(m_outputFile, m_dataLength);

        m_outputFile->close();
        m_outputFile = 0;
    }
}

void WavRecorder::addLittleEndianShortToFile(QFile* file, int nb)
{
    m_bufferForNumbers[0] = nb & 0xff;
    m_bufferForNumbers[1] = (nb / 0x100) & 0xff;
    file->write(m_bufferForNumbers, 2);
}

void WavRecorder::addLittleEndianIntToFile(QFile* file, int nb)
{
    m_bufferForNumbers[0] = nb & 0xff;
    m_bufferForNumbers[1] = (nb / 0x100) & 0xff;
    m_bufferForNumbers[2] = (nb / 0x10000) & 0xff;
    m_bufferForNumbers[3] = (nb / 0x1000000) & 0xff;
    file->write(m_bufferForNumbers, 4);
}
