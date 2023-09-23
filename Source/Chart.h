#pragma once

struct ImVec2;
struct ImColor;

namespace Chart {
	namespace Pie {
		// Begin drawing the pie chart.
		void Begin(const ImVec2& position);

		// End drawing the pie chart.
		void End();

		// Draw a slice of the pie chart.
		void Slice(float radius, float start, float end);

		void Color(ImColor fill);
		void Color(ImColor fill, ImColor stroke);
	} // namespace Pie
} // namespace Chart
