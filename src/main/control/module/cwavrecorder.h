#ifndef CWAVRECORDER_H
#define CWAVRECORDER_H

#include "abstraction/module/wavrecorder.h"
#include "control/cmodule.h"

class CWavRecorder : public WavRecorder, public CModule {
    Q_OBJECT

public:
    /**
     * CWavRecorder as no processing limit by default. We stop writing to file
     * when the user starts a new file, clicks stop or quits the application.
     */
    CWavRecorder(SynthPro*, int nbProcessingBeforeSaving = 0);

    void initialize(SynthProFactory*);

public slots:
    void newFile();
    void startRecording();
    void stopRecording();
    void closeFile();
};

#endif // CWAVRECORDER_H
