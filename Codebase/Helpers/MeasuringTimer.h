#pragma once

#include "Helpers/common.h"
#include "Timer.h"

#include <cassert>
#include <climits>
#include <iostream>
#include <string>
#include <stack>


#define MEASURING_TIMER_LOG_START(str) MeasuringTimer::Instance.LogStart(str)
#define MEASURING_TIMER_LOG_END() MeasuringTimer::Instance.LogEnd()
#define MEASURING_TIMER_PRINT(stream) MeasuringTimer::Instance.Print(stream, 0, 0)
#define MEASURING_TIMER_CLEAR() MeasuringTimer::Instance.Clear()

class MeasuringTimer : public Timer {
 private:

	struct record {
		timestamp startTimestamp;
		timestamp endTimestamp;
		record* end;
		const char* text;
	};

	record* m_logs;

	uint m_endIndex;

	std::stack<record*> m_recordStack;

public:
	/// <summary>
	/// A static instance of <see cref="MeasuringTimer"/> class.
	/// </summary>
	static MeasuringTimer Instance;

	/// <summary>
	/// Max number of logs that can be taken.
	/// </summary>
	const uint maxSize;

	/// <summary>
	/// String used for indentation when formatting the output.
	/// </summary>
	std::string indentation;

	/// <summary>
	/// String used for line ends when formatting the output.
	/// </summary>
	std::string endLine;

	/// <summary>
	/// Logs a timestamp for the start of a new period.
	/// </summary>
	/// <param name="msg">Message to associate with the new time period.</param>
	/// <returns>Index representation of where the record is placed internally.</returns>
	inline void LogStart(const char* msg = "") {
		assert(("Performance::Time::logStart overran the max size", m_endIndex < maxSize));
		m_recordStack.push(m_logs + m_endIndex);
		m_logs[m_endIndex].text = msg;
		m_logs[m_endIndex++].startTimestamp = Timer::Now();
	}

	/// <summary>
	/// Logs a timestamp for the end of the last period.
	/// </summary>
	/// <param name="msg">Message to associate with the end of a time period.</param>
	inline void LogEnd() {
		assert(("overran the max size", m_endIndex < maxSize));
		record* lastRecord = m_recordStack.top();
		assert(("no open record found", lastRecord != NULL));
		lastRecord->endTimestamp = Timer::Now();
		m_recordStack.pop();
		lastRecord->end = m_logs + m_endIndex;
	}

	inline void LogStart(const std::string& msg) {
		LogStart(msg.c_str());
	}

	/// <summary>
	/// Returns the time measurement of a period, identified internally by the given index.
	/// </summary>
	/// <param name="index">Index representation of where the record is placed internally.</param>
	/// <param name="timeResolution">Time period resolution, 1 = seconds and 1000000 = microseconds.</param>
	/// <returns>Duration of the time period.</returns>
	float DurationOf(uint index, float timeResolution = 1.0f) const;

	/// <summary>
	/// Prints time periods starting at given nested level.
	/// </summary>
	/// <param name="os">Output stream.</param>
	/// <param name="startLevel">The lowest index level of period nesting to output measurements for.</param>
	/// <param name="maxDepth">The max depth of child periods to output.</param>
	/// <param name="timeResolution">Time period resolution, 1 = seconds and 1000000 = microseconds.</param>
	/// <remarks>
	///	If parameter maxDepth is 0, only the matched start level will be printed.
	/// </remarks>
	void Print(std::ostream& os, uint startLevel, uint maxDepth = UINT_MAX, float timeResolution = 1.0f) const;

	/// <summary>
	/// Resets the used logging space to indicate an empty collection of logs.
	/// </summary>
	inline void Clear() {
		//memset(m_logs, 0, sizeof(record) * m_endIndex);
		m_endIndex = 0;
	}

	/// <summary>
	/// Creates a timer for measuring duration of time periods.
	/// </summary>
	/// <param name="maxLogs">Max number of logs that can be made.</param>
	/// <remarks>
	/// Parameter maxLogs has to be at least two times the number of time periods that will be recorded.
	/// </remarks>
	inline MeasuringTimer(uint maxLogs): indentation("-\t"), endLine(LINE_SEPARATOR), m_logs(new record[maxLogs]), m_endIndex(0), maxSize(maxLogs) {
		memset(m_logs, 0, sizeof(record) * maxSize);
	}

	/// <summary>
	/// Creates a copy of a timer for measuring duration of time periods.
	/// </summary>
	/// <param name="obj">Original timer.</param>
	inline MeasuringTimer(const MeasuringTimer& obj): indentation(obj.indentation), endLine(obj.endLine), m_logs(new record[obj.maxSize]), m_endIndex(obj.m_endIndex), maxSize(obj.maxSize) {
		std::memcpy(m_logs, obj.m_logs, obj.m_endIndex * sizeof(record));
	}

	/// <summary>
	/// Creates a timer for measuring duration of time periods, using the existing resources of given object.
	/// </summary>
	/// <param name="obj">Original timer.</param>
	inline MeasuringTimer(MeasuringTimer&& obj): indentation(obj.indentation), endLine(obj.endLine), m_logs(nullptr), m_endIndex(obj.m_endIndex), maxSize(obj.maxSize) {
		std::swap(m_logs, obj.m_logs);
	}

	/// <summary>
	/// Destructor.
	/// </summary>
	inline ~MeasuringTimer() {
		if(m_logs) {
			delete[] m_logs;
			m_logs = nullptr;
		}
	}


	MeasuringTimer& operator=(const MeasuringTimer& other);

	friend std::ostream& operator<<(std::ostream& os, const MeasuringTimer& obj);
};
