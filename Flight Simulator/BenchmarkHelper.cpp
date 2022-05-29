#include "BenchmarkHelper.h"
#include <algorithm>

#pragma comment(lib, "winmm.lib")

#include <windows.h>
#include <mmsystem.h>
#include <iostream>

using namespace std;
using namespace std::chrono;

BenchmarkHelper* BenchmarkHelper::g_benchmarkHelper = nullptr;

BenchmarkHelper::~BenchmarkHelper()
{
    m_info.clear();
}

BenchmarkHelper* BenchmarkHelper::GetInstance()
{
    if (!g_benchmarkHelper)
        g_benchmarkHelper = new BenchmarkHelper();

    return g_benchmarkHelper;
}

void BenchmarkHelper::FreeInstance()
{
    if (g_benchmarkHelper)
    {
        delete g_benchmarkHelper;
        g_benchmarkHelper = nullptr;
    }
}

void BenchmarkHelper::Update()
{
    m_count++;

    if (timeGetTime() >= (m_startTime + 1000))
    {
        m_fps = m_count;
        m_count = 0;
        m_startTime = timeGetTime();

        m_recordedFps.push_back(m_fps);

        if (m_recordedFps.size() > FPS_AVERAGE_SAMPLES)
            m_recordedFps.pop_front();

        m_recordedSecondsCount++;
    }

    if (m_recordedSecondsCount >= FPS_AVERAGE_SAMPLES)
    {
        float totalFps = 0.0f;
        for (auto& fps : m_recordedFps)
            totalFps += fps;

        float averageFps = totalFps / m_recordedFps.size();

        cout << "Average FPS: " << averageFps << endl;

        m_recordedSecondsCount = 0;
    }
}

void BenchmarkHelper::AddTimeSample(const string& key, const steady_clock::time_point& beginTime, const steady_clock::time_point& endTime)
{
    long long value = duration_cast<nanoseconds>(endTime - beginTime).count();

    if (m_info.find(key) == m_info.end())
    {
        m_info[key] = TimeStats();
        m_info[key].Nanoseconds.push_back(value);
        m_info[key].MustUpdate         = false;
        m_info[key].MinNanoseconds     = value;
        m_info[key].MaxNanoseconds     = value;
        m_info[key].AverageNanoseconds = value;

        return;
    }

    m_info[key].Nanoseconds.push_back(value);
    m_info[key].MustUpdate     = true;
    m_info[key].MinNanoseconds = min(m_info[key].MinNanoseconds, value);
    m_info[key].MaxNanoseconds = max(m_info[key].MaxNanoseconds, value);
}

const BenchmarkHelper::TimeStats& BenchmarkHelper::GetTimeInfo(const string& key)
{
    if (m_info[key].MustUpdate)
    {
        unsigned long long totalSum = 0;
        for (auto& nanoseconds : m_info[key].Nanoseconds)
            totalSum += nanoseconds;

        m_info[key].AverageNanoseconds = totalSum / m_info[key].Nanoseconds.size();
        m_info[key].MustUpdate = false;
    }

    return m_info[key];
}

BenchmarkHelper::BenchmarkHelper() :
    m_count(0),
    m_fps(0),
    m_startTime(timeGetTime()),
    m_recordedSecondsCount(0)
{
}