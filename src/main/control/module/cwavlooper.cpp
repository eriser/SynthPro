#include "cwavlooper.h"

#include "control/component/coutport.h"
#include "presentation/module/pwavlooper.h"

CWavLooper::CWavLooper(SynthPro* parent)
    : Module(parent)
    , WavLooper(parent)
    , CModule(parent)
{
}

void CWavLooper::initialize(SynthProFactory* factory)
{
    WavLooper::initialize(factory);

    COutPort* outPort = dynamic_cast<COutPort*>(m_outPort);

    PWavLooper* pre = dynamic_cast<PWavLooper*>(presentation());
    pre->initialize(outPort->presentation());

    connect(pre, SIGNAL(newFileClicked()), this, SLOT(newFile()));

    newFile();
}

void CWavLooper::newFile()
{
    QString fileName = dynamic_cast<PWavLooper*>(presentation())->askForFileName();
    if (!fileName.isNull()) {
        WavLooper::newFile(fileName);
    }
}