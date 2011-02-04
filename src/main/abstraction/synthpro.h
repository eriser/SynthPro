#ifndef SYNTHPRO_H
#define SYNTHPRO_H

#include <QObject>

class Module;

class SynthPro : public QObject {
    Q_OBJECT

public:
    SynthPro(QObject *parent = 0);

    /// @return The list of modules contained by this SynthPro
    QList<Module*> modules();

    /// Add a module to this SynthPro
    void add(Module*);

private:
    QList<Module*> m_modules;
};

#endif // SYNTHPRO_H
