#include "serializer.h"

#include "abstraction/module.h"
#include "control/cmodule.h"
#include "control/csynthpro.h"

void operator<<(QTextStream& stream, const CSynthPro& synthPro)
{
    stream << synthPro.modules().size() << endl;

    foreach (Module* module, synthPro.modules()) {
        CModule* cmodule = dynamic_cast<CModule*>(module);
        stream << *cmodule;
        stream << endl;
    }
}

void operator<<(QTextStream& stream, const CModule& module)
{
     // Serialize Module real type.
    stream << typeid(module).name() << " ";
    // Serialize Module id.
    stream << &module << " ";
    // Serialize Module position.
    stream << module.presentation()->pos().rx() << " " << module.presentation()->pos().ry();
}
