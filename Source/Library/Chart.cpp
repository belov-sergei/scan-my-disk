// Copyright ❤️ 2023-2024, Sergei Belov

#include <Chart.h>
#include "imgui.h"

namespace Chart {
	namespace Pie {
		namespace Details {
			ImVec2 BeginPosition;
			ImDrawList* DrawList = nullptr;

			namespace Color {
				ImColor Fill;
				ImColor Stroke;
			} // namespace Color

			// Mathematical constants.
			constexpr float kTwoPI = 6.28318;
			constexpr float kAlmostOne = 0.99975;

			// Number of segments for a full circle.
			constexpr int kSegments = 180;
		} // namespace Details

		void Begin(const ImVec2& position) {
			Details::BeginPosition = position;
			Details::DrawList = ImGui::GetWindowDrawList();

			Details::Color::Fill = Details::Color::Stroke = {};
		}

		void End() {
			Details::DrawList = nullptr;
			Details::BeginPosition = {};
		}

		void Slice(float radius, float start, float end) {
			if (!Details::DrawList) {
				return;
			}

			// Calculate the number of segments based on the angle difference.
			const int segments = std::max<int>(1, Details::kSegments * (end - start));

			if (end - start > Details::kAlmostOne) {
				Details::DrawList->AddCircleFilled(Details::BeginPosition, radius, Details::Color::Fill, segments);
				Details::DrawList->AddCircle(Details::BeginPosition, radius, Details::Color::Stroke, segments);

				return;
			}

			Details::DrawList->PathLineTo(Details::BeginPosition);
			Details::DrawList->PathArcTo(Details::BeginPosition, radius, start * Details::kTwoPI, end * Details::kTwoPI, segments);

			Details::DrawList->PathFillConvex(Details::Color::Fill);

			Details::DrawList->PathLineTo(Details::BeginPosition);
			Details::DrawList->PathArcTo(Details::BeginPosition, radius, start * Details::kTwoPI, end * Details::kTwoPI, segments);
			Details::DrawList->PathLineTo(Details::BeginPosition);

			Details::DrawList->PathStroke(Details::Color::Stroke, 0, 1);
		}

		void Color(ImColor fill) {
			auto stroke = fill.Value;
			stroke.x *= 0.9f;
			stroke.y *= 0.9f;
			stroke.z *= 0.9f;

			Color(fill, stroke);
		}

		void Color(ImColor fill, ImColor stroke) {
			Details::Color::Fill = fill;
			Details::Color::Stroke = stroke;
		}
	} // namespace Pie
} // namespace Chart
