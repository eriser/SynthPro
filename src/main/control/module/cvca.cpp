#include "cvca.h"

#include "control/component/cdimmer.h"
#include "control/component/cinport.h"
#include "control/component/coutport.h"
#include "control/component/cselector.h"
#include "presentation/module/pvca.h"

#include <QTextStream>
#include <qmath.h>

CVCA::CVCA(SynthPro* parent)
    : Module(parent)
    , VCA(parent)
    , CModule(parent)
{
}

void CVCA::initialize(SynthProFactory* factory)
{
    VCA::initialize(factory);

    CInPort* in = dynamic_cast<CInPort*>(m_inPort);
    COutPort* out = dynamic_cast<COutPort*>(m_outPort);
    CInPort* controlInput = dynamic_cast<CInPort*>(m_controlInput);
    CDimmer* gain = dynamic_cast<CDimmer*>(m_gainDimmer);

    gain->setValueFormat(CDimmer::gainFormat);

    dynamic_cast<PVCA*>(presentation())->initialize(in->presentation(), out->presentation(),
                                                    controlInput->presentation(), gain->presentation());
}

QString CVCA::settings() const
{
    QString result;
    QTextStream(&result) << m_gainDimmer->value();

    return result;
}

void CVCA::setUpSettings(const QString& settings)
{
    QStringList list = settings.split(" ", QString::SkipEmptyParts);

    m_gainDimmer->setValue(list[0].toFloat());
}
