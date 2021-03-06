#include "wavegeneratordummy.h"

#include "abstraction/buffer.h"
#include "abstraction/module/vco.h"

WaveGeneratorDummy::WaveGeneratorDummy()
    : m_intensity(VCO::SIGNAL_INTENSITY)
    , m_currentStep(0)
{
}

void WaveGeneratorDummy::generate(const Buffer*, Buffer* bufferOut)
{
    for (int i = 0, length = bufferOut->length(); i < length; i++) {
        if (++m_currentStep > FIXED_PERIOD) {
            m_intensity = -m_intensity;
            m_currentStep = 0;
        }

        bufferOut->data()[i] = m_intensity;
    }
}
