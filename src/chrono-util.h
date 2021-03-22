#pragma once


/*
   @auther: 디알(dire@afreecatv.com)

*/
#include <chrono>


class ChronoDuration {
    public:
        std::chrono::high_resolution_clock::time_point m_baseTime;
        void setBaseTime();
        double durationWithBaseTime();
        double duration();
};

inline void ChronoDuration::setBaseTime() {
    m_baseTime = std::chrono::high_resolution_clock::now();
}
inline double ChronoDuration::durationWithBaseTime() {
    using namespace std;
    typedef typename chrono::duration<double, milli> doublemillisec;
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
    doublemillisec doubleMilliSec = chrono::duration_cast<doublemillisec>(end - m_baseTime);
    m_baseTime = chrono::high_resolution_clock::now();
    return doubleMilliSec.count();
}
inline double ChronoDuration::duration() {
    using namespace std;
    typedef typename chrono::duration<double, milli> doublemillisec;
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
    doublemillisec doubleMilliSec = chrono::duration_cast<doublemillisec>(end - m_baseTime);
    return doubleMilliSec.count();
}

class Chrono{
    public:
        static uint64_t unixTimeStamp() {
            std::chrono::time_point<std::chrono::system_clock> p1, p2, p3;
            p2 = std::chrono::system_clock::now();
            auto t = std::chrono::duration_cast<std::chrono::milliseconds>(p2.time_since_epoch()).count();
            return t;
        }
        static uint64_t nanoUnixTimeStamp() {
            std::chrono::time_point<std::chrono::system_clock> p1, p2, p3;
            p2 = std::chrono::system_clock::now();
            auto t = std::chrono::duration_cast<std::chrono::nanoseconds>(p2.time_since_epoch()).count();
            return t;
        }
};

