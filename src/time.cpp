#include "tabby/time.h"

namespace Tabby {

double Time::GetTime()
{
    using namespace std::chrono;
    system_clock::time_point currentTimePoint = system_clock::now();
    duration<double> timeSinceEpoch = currentTimePoint.time_since_epoch();

    double result = timeSinceEpoch.count();

    return result;
}

double last_frame_time;

void Time::FrameStart()
{
    m_DeltaTime = GetTime() - last_frame_time;
    last_frame_time = GetTime();
}

}
