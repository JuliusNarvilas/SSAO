#include "MeasuringTimer.h"

#include <vector>
#include <stack>


MeasuringTimer MeasuringTimer::Instance(10);

inline MeasuringTimer& MeasuringTimer::operator=(const MeasuringTimer& other) {
	if(this != &other) {
		indentation = other.indentation;
		endLine = other.endLine;
		const_cast<uint&>(maxSize) = other.maxSize;
		m_endIndex = other.m_endIndex;

		if(m_logs) delete[] m_logs;
		m_logs = new record[maxSize];
		memcpy(m_logs, other.m_logs, m_endIndex * sizeof(MeasuringTimer::record));
	}
	return *this;
}

float MeasuringTimer::DurationOf(uint index, float timeResolution) const {
	assert(("out of range", index < m_endIndex));
	assert(("no end timestamp", m_logs[index].endTimestamp.QuadPart != 0));
	return Timer::Duration(m_logs[index].startTimestamp, m_logs[index].endTimestamp, timeResolution);
}


void MeasuringTimer::Print(std::ostream& os, uint startLevel, uint maxDepth, float timeResolution) const {
	os << "Scale: 1/" << timeResolution << " sec" << LINE_SEPARATOR << LINE_SEPARATOR;
	std::stack<record*> endReferences;
	endReferences.push(NULL);
	//levelGroupingCounters.push_back(0);

	//max level to print out based on the depth
	uint maxLevel = startLevel + maxDepth;
	if(maxLevel < maxDepth)
		maxLevel = UINT_MAX;

	//variable to store indentation or relative level
	uint indentCount;
	//counter to keep track of the current nesting level
	uint counter = 0;

	for (uint i = 0; i < m_endIndex; ++i) {
		while (endReferences.top() == (m_logs + i))
		{
			endReferences.pop();
			--counter;
		}
		if (counter >= startLevel && counter <= maxLevel) { //if in range for printing
			indentCount = counter - startLevel;
			for (uint j = 0; j < indentCount; ++j)
				os << indentation;

			os << m_logs[i].text << " : " << DurationOf(i, timeResolution) << endLine;
		}
		++counter; //counting up on start of next periods
		endReferences.push(m_logs[i].end);
	}
}


std::ostream& operator<<(std::ostream& os, const MeasuringTimer& obj) {
	obj.Print(os, 0);
	return os;
}