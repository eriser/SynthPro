#include "clfo.h"

#include "control/component/cdimmer.h"
#include "control/component/cinport.h"
#include "control/component/coutport.h"
#include "control/component/cselector.h"
#include "presentation/module/plfo.h"

#include <QTextStream>
#include <qmath.h>

CLFO::CLFO(SynthPro* parent)
    : Module(parent)
    , LFO(parent)
    , CModule(parent)
{
}

void CLFO::initialize(SynthProFactory* factory)
{
    LFO::initialize(factory);

    COutPort* out = dynamic_cast<COutPort*>(m_out);
    CSelector* selector = dynamic_cast<CSelector*>(m_shapeSelector);
    CDimmer* k = dynamic_cast<CDimmer*>(m_kDimmer);
    CDimmer* range = dynamic_cast<CDimmer*>(m_rangeDimmer);
    CDimmer* offset= dynamic_cast<CDimmer*>(m_offsetDimmer);

    k->setValueFormat(formatK);
    range->setValueFormat(CDimmer::percentageFormat);
    offset->setValueFormat(formatOffset);

    dynamic_cast<PLFO*>(presentation())->initialize(out->presentation(), k->presentation(),
                                                    range->presentation(), offset->presentation(),
                                                    selector->presentation());
}

QString CLFO::formatK(qreal k)
{
    return QString::number(LFO::F0 * qPow(2, k), 'g', 2) + " Hz";
}

QString CLFO::formatOffset(qreal offset)
{
    return QString::number(offset, 'g', 2) + " V";
}

QString CLFO::settings() const
{
    QString result;
    QTextStream(&result) << m_shapeSelector->choice() << " "
                         << m_kDimmer->value() << " "
                         << m_rangeDimmer->value() << " "
                         << m_offsetDimmer->value();

    return result;
}

void CLFO::setUpSettings(const QString& settings)
{
    QStringList list = settings.split(" ", QString::SkipEmptyParts);

    m_shapeSelector->setChoice(list[0].toInt());
    m_kDimmer->setValue(list[1].toFloat());
    m_rangeDimmer->setValue(list[2].toFloat());
    m_offsetDimmer->setValue(list[3].toFloat());
}
