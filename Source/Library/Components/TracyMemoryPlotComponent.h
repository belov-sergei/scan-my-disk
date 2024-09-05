// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include "System.h"

#include <tracy/Tracy.hpp>

struct TracyMemoryPlotComponent final {
	TracyMemoryPlotComponent() {
		Event<Application::EndFrame>::Receive(this, [](const auto&) {
			TracyPlot("Virtual Memory", System::GetVirtualMemoryUsed());
			TracyPlot("Physical Memory", System::GetPhysicalMemoryUsed());
		});
	}
};
