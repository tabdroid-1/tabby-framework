#pragma once

#include "event.h"

namespace Tabby {

class Subsystem {
public:
    Subsystem() { };
    virtual ~Subsystem() { };

    virtual void Launch() = 0;
    virtual void Destroy() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnEvent(Event& e) = 0;
};

}
