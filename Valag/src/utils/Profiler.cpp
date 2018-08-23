#include "Valag/utils/Profiler.h"

#include <sstream>

namespace vlg
{

Profiler::Profiler()
{
    //ctor
}

Profiler::~Profiler()
{
    //dtor
}


void Profiler::resetLoop(bool print)
{
    std::lock_guard<std::mutex> lock(Profiler::instance()->m_profilerMutex);

    Time total_time = Profiler::instance()->m_loopClock.restart();

    if(print)
    {
        std::ostringstream buf;

        std::list<ProfilerTime>::iterator timesIt;
        for(timesIt = Profiler::instance()->m_times.begin() ;
            timesIt != Profiler::instance()->m_times.end() ; ++timesIt)
        {
            buf<<timesIt->text<<": "
               <<timesIt->time.count()/**.asMicroseconds()**/*100.0/total_time.count()/**.asMicroseconds()**/<<"%"
               <<std::endl;
        }

        buf<<"Total loop time: "<<total_time.count()*1000.0/**.asMilliseconds()**/<<" ms"<<std::endl<<std::endl;
        std::cout<<buf.str();
    }

    Profiler::instance()->m_times.clear();
}

void Profiler::pushClock(const std::string &text)
{
    ProfilerClock clock;
    clock.text = text;
    clock.clock.restart();
    Profiler::instance()->m_clocks.push(clock);
}

void Profiler::popClock()
{
    if(!Profiler::instance()->m_clocks.empty())
    {
        ProfilerTime time;
        time.time = Profiler::instance()->m_clocks.top().clock.elapsedTime();
        time.text = Profiler::instance()->m_clocks.top().text;
        Profiler::instance()->m_clocks.pop();
        Profiler::instance()->m_times.push_back(time);
    }
}



}
