#pragma once

namespace Tabby {

class Time {
public:
    static double GetTime();
    static double GetDeltaTime() { return m_DeltaTime; }

private:
    static void FrameStart();

private:
    inline static double m_DeltaTime;

    friend class Application;
};

}
