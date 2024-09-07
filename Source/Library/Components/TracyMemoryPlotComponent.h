// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include "System.h"

#include <tracy/Tracy.hpp>

struct TracyMemoryPlotComponent final {
	const char* kVirtualMemoryPlot = "Virtual Memory";
	const char* kPhysicalMemoryPlot = "Physical Memory";

	TracyMemoryPlotComponent() {
		TracyPlotConfig(kVirtualMemoryPlot, tracy::PlotFormatType::Memory, false, true, 0);
		TracyPlotConfig(kPhysicalMemoryPlot, tracy::PlotFormatType::Memory, false, true, 0);

		Event<Application::EndFrame>::Receive(this, [this](const auto&) {
			TracyPlot(kVirtualMemoryPlot, System::GetVirtualMemoryUsed());
			TracyPlot(kPhysicalMemoryPlot, System::GetPhysicalMemoryUsed());
		});
	}
};
