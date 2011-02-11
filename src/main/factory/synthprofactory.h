#ifndef SYNTHPROFACTORY_H
#define SYNTHPROFACTORY_H

#include <QString>
#include <QtGlobal>

class ADSR;
class Dimmer;
class InPort;
class LFO;
class Module;
class ModuleBufferRecorder;
class ModuleKeyboard;
class ModuleOscilloscope;
class ModuleOut;
class OutPort;
class PushButton;
class Selector;
class Sequencer;
class SynthPro;
class VCA;
class VCF;
class VCO;

class SynthProFactory {
public:
    virtual SynthPro* createSynthPro() = 0;

    virtual InPort* createInPort(Module* parent, const QString& name) = 0;
    virtual InPort* createInPortReplicable(Module* parent, const QString& name) = 0;
    virtual InPort* createInPortGate(Module* parent, const QString& name) = 0;

    virtual OutPort* createOutPort(Module* parent, const QString& name) = 0;
    virtual OutPort* createOutPortReplicable(Module* parent, const QString& name) = 0;
    virtual OutPort* createOutPortGate(Module* parent, const QString& name) = 0;

    virtual VCO* createVCO(SynthPro*) = 0;
    virtual LFO* createLFO(SynthPro*) = 0;
    virtual VCF* createVCF(SynthPro*) = 0;
    virtual VCA* createVCA(SynthPro*) = 0;
    virtual ADSR* createADSR(SynthPro*) = 0;
    virtual ModuleBufferRecorder* createModuleBufferRecorder(SynthPro*, QString fileName = "output.wav", int nbProcessingBeforeSaving = 5) = 0;
    virtual ModuleKeyboard* createModuleKeyboard(SynthPro*) = 0;

    /**
      * Instanciate a ModuleOut, but ONLY if the audio device isn't already used by another instance.
      * @return ModuleOut if instanciation successful, 0 if not.
      */
    virtual ModuleOut* createModuleOut(SynthPro*) = 0;

    virtual ModuleOscilloscope* createModuleOscilloscope(SynthPro*) = 0;

    /*
     * instantiate a dimmer's abstraction
     * @param min min value of the dimer
     * @param max max value
     * @param default_value the value where the dimmer is positionned at creation
     */
    virtual Dimmer* createDimmer(QString name, qreal min, qreal man, qreal defaultValue, Module* parent) = 0;
    virtual Selector* createSelector(QList<int> keys, int defaultKey, QList<QString> values, QString name, Module* parent) = 0;
    virtual PushButton* createPushButton(QString name, Module* parent) = 0;

    /*
     * Enumeration of module types.
     */
    enum ModuleType {
        KeyboardId,
        VCOId,
        VCFId,
        VCAId,
        ADSRId,
        LFOId,
        AudioOuputId,
        FileOutputId,
        OscilloscopeId
    };
};

#endif // SYNTHPROFACTORY_H
