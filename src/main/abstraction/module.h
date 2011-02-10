#ifndef MODULE_H
#define MODULE_H

#include <QObject>

class InPort;
class OutPort;
class SynthPro;

/**
 * A module is a component performing a single function (generating a signal, filtering, etc.)
 */
class Module : public QObject {
    Q_OBJECT

public:
    explicit Module(SynthPro*);
    virtual ~Module();

    /// Get the list of output ports of this module
    inline const QList<OutPort*> outports() const { return m_outports; }

    /// Get the list of input ports of this module FIXME is it needed?
    inline const QList<InPort*> inports() const { return m_inports; }

    /**
     * Operation called by the sequencer.
     * Fetches all input ports then call ownProcess.
     */
    void process();

    /// Subclasses have to implement here their own process
    virtual void ownProcess() = 0;

    /// Get the list of required modules for this module to perform its function
    const QList<Module*> requirements() const;

    /// Get the parent SynthPro.
    SynthPro* synthPro() const;

public slots:
    virtual void timerExpired();

protected:
    /// Fetch input from all this Module input Ports (call this operation should be the first thing to do in a “process” implementation)
    void fetchInput();

    QList<OutPort*> m_outports;
    QList<InPort*> m_inports;
    mutable QList<Module*> m_requirements;
};

#endif // MODULE_H
