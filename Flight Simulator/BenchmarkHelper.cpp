#include "BenchmarkHelper.h"
#include <algorithm>

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

BenchmarkHelper::BenchmarkHelper()
{
}