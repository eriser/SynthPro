#include "testwavegeneratorsaw.h"

#include "abstraction/component/connection.h"
#include "abstraction/component/inport.h"
#include "abstraction/component/outport.h"
#include "abstraction/module.h"
#include "abstraction/module/vco.h"
#include "abstraction/module/wavrecorder.h"
#include "abstraction/synthpro.h"
#include "abstraction/wavegen/wavegeneratorsaw.h"
#include "factory/simplefactory.h"


#include <QFile>
#include <QtTest/QTest>

void TestWaveGeneratorSaw::testWaveGeneratorSaw()
{
    SynthPro synthPro(0);

    QString fileName = "testWaveGeneratorSaw.wav";

    SimpleFactory factory;
    WavRecorder* mbr = factory.createWavRecorder(0, NB_ITERATIONS);

    mbr->newFile(fileName);
    mbr->startRecording();
    VCO* vco = factory.createVCO(&synthPro);
    vco->setShape("Saw");
    vco->outports().first()->connect(mbr->inports().first());

    for (int i = 0; i < NB_ITERATIONS; i++) {
        vco->process();
        mbr->process();
    }

    QVERIFY(true);
}
