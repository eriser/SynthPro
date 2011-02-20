#ifndef PSPEAKER_H
#define PSPEAKER_H

#include "presentation/pmodule.h"

class CSpeaker;
class PVirtualPort;

/**
 * Presentation of Speaker.
 */
class PSpeaker : public PModule {
public:
    explicit PSpeaker(CSpeaker*);

    void initialize(PVirtualPort* in);
};

#endif // PSPEAKER_H
