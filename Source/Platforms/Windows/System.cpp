// Copyright ❤️ 2023-2024, Sergei Belov

#include "System.h"

#include <Windows.h>
// clang-format off
#include <psapi.h>
// clang-format on

namespace System {
	int64_t GetVirtualMemoryUsed() {
		PROCESS_MEMORY_COUNTERS_EX processMemoryCounters;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&processMemoryCounters, sizeof(processMemoryCounters));

		return processMemoryCounters.PrivateUsage / 1024;
	}

	int64_t GetPhysicalMemoryUsed() {
		PROCESS_MEMORY_COUNTERS_EX processMemoryCounters;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&processMemoryCounters, sizeof(processMemoryCounters));

		return processMemoryCounters.WorkingSetSize  / 1024;
	}
} // namespace System
