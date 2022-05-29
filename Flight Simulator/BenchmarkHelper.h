#pragma once

#include <chrono>
#include <vector>
#include <unordered_map>

class BenchmarkHelper
{
private:

	const int FPS_AVERAGE_SAMPLES = 10;

private:

	struct TimeStats
	{
	public:

		std::vector<long long> Nanoseconds;
		bool                   MustUpdate;

		long long              AverageNanoseconds;
		long long              MinNanoseconds;
		long long              MaxNanoseconds;
	};

public:

	BenchmarkHelper(const BenchmarkHelper&) = delete;
	void operator=(const BenchmarkHelper&)  = delete;

	~BenchmarkHelper();

	static BenchmarkHelper*                      GetInstance();
	static void                                  FreeInstance();

	       void                                  Update();
							                     
	       void                                  AddTimeSample(const std::string&, const std::chrono::steady_clock::time_point&, const std::chrono::steady_clock::time_point&);
	const  TimeStats&                            GetTimeInfo(const std::string&);

	inline std::chrono::steady_clock::time_point Now() const { return std::chrono::high_resolution_clock::now(); }

private:

	BenchmarkHelper();

private:

	       std::unordered_map<std::string, TimeStats> m_info;
		   std::list<int>                             m_recordedFps;

		   int                                        m_count;
		   int                                        m_fps;

		   unsigned long                              m_startTime;
		   int                                        m_recordedSecondsCount;

	static BenchmarkHelper*                           g_benchmarkHelper;
};