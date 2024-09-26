// Copyright ❤️ 2023-2024, Sergei Belov

#include "System.h"
#include <mach/mach.h>

namespace System {
	int64_t GetVirtualMemoryUsed() {
		task_basic_info_data_t taskInfo;
		mach_msg_type_number_t infoCount = TASK_BASIC_INFO_COUNT;

		task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&taskInfo, &infoCount);
		return taskInfo.virtual_size;
	}

	int64_t GetPhysicalMemoryUsed() {
		task_basic_info_data_t taskInfo;
		mach_msg_type_number_t infoCount = TASK_BASIC_INFO_COUNT;

		task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&taskInfo, &infoCount);
		return taskInfo.resident_size;
	}
} // namespace System
