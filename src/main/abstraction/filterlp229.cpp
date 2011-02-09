#include "filterlp229.h"

#include "abstraction/buffer.h"
#include "abstraction/vcf.h"
#include "abstraction/vco.h"

#include <QtCore>

FilterLP229::FilterLP229()
    : m_valueInM1(0)
    , m_valueInM2(0)
    , m_valueOutM1(0)
    , m_valueOutM2(0)
    , m_a1(0)
    , m_a2(0)
    , m_a3(0)
    , m_b1(0)
    , m_b2(0)
    , m_currentCutOffValue(-1000)
    , m_currentResonance(-1)
    , m_currentResonanceNormalized(RESONANCE_MIN)
{
}

void FilterLP229::apply(Buffer* bufferIn, Buffer* bufferInCutOff, qreal cutOffBase, qreal resonance, Buffer* bufferOut)
{
    /*
    r  = rez amount, from sqrt(2) to ~ 0.1
    f  = cutoff frequency

    out(n) = a1 * in + a2 * in(n-1) + a3 * in(n-2) - b1*out(n-1) - b2*out(n-2)

    Lowpass:
      c = 1.0 / tan(pi * f / sample_rate);

      a1 = 1.0 / ( 1.0 + r * c + c * c);
      a2 = 2* a1;
      a3 = a1;
      b1 = 2.0 * ( 1.0 - c*c) * a1;
      b2 = ( 1.0 - r * c + c * c) * a1;
    */

    qreal* dataOut = bufferOut->data();
    qreal* dataIn = bufferIn->data();
    qreal* dataInCutOff = bufferInCutOff->data();

    // Convert the resonance given, if it has changed.
    if (m_currentResonance != resonance) {
        m_currentResonance = resonance;
        // Normalize the resonance, from the values given by the Resonance to the values
        m_currentResonanceNormalized = ((m_currentResonance - VCF::R_MIN) / VCF::R_MAX)
                                       * (RESONANCE_MAX - RESONANCE_MIN) + RESONANCE_MIN;
        m_currentCutOffValue = -1000; // Force recalculation of the filter.
    }

    for (int i = 0, size = bufferIn->length(); i < size; i++) {
        // Check if the input CutOffBuffer delivers a different CutOff frequency.
        // If yes, must recalculate the filter parameters.
        if (dataInCutOff[i] != m_currentCutOffValue) {
            m_currentCutOffValue = dataInCutOff[i];
            // Convert a signal into a frequency from 0 to MAX_FREQUENCY.
            qreal f = (dataInCutOff[i] + VCO::SIGNAL_INTENSITY) * (MAX_FREQUENCY / (VCO::SIGNAL_INTENSITY * 2));
            f += cutOffBase;
            if (f < 0) {
                f = 0;
            } else if (f > MAX_FREQUENCY) {
                f = MAX_FREQUENCY;
            }

            qreal c = 1.0 / tan(M_PI * f / VCO::REPLAY_FREQUENCY);
            m_a1 = 1.0 / (1.0 + m_currentResonanceNormalized * c + c * c);
            m_a2 = 2 * m_a1;
            m_a3 = m_a1;
            m_b1 = 2.0 * (1.0 - c * c) * m_a1;
            m_b2 = (1.0 - m_currentResonanceNormalized * c + c * c) * m_a1;
        }

        // Process the filter.
        qreal in = dataIn[i];
        qreal out = m_a1 * in + m_a2 * m_valueInM1 + m_a3 * m_valueInM2 - m_b1 * m_valueOutM1 - m_b2 * m_valueOutM2;
        dataOut[i] = out;

        m_valueInM2 = m_valueInM1;
        m_valueInM1 = in;

        m_valueOutM2 = m_valueOutM1;
        m_valueOutM1 = out;
    }
}
