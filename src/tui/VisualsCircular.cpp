#include "Visuals.h"
#include "VisualsBase.h"
#include <cmath>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>
#include "../service/ILibraryService.h"

class VisualsCircular : public VisualsBase {
public:
    explicit VisualsCircular(ILibraryService& service) : VisualsBase(service) {}

    ftxui::Element render() override {
        smoothBars();
        int barCount = (int)displayBars_.size();
        if (barCount == 0)
            return ftxui::text("") | ftxui::flex;

        float phase = (float)(frameCount_++) * 0.08f;

        const auto& gradient = palette();
        auto gradLeft = ftxui::LinearGradient();
        auto gradRight = ftxui::LinearGradient();
        for (size_t i = 0; i < gradient.size(); i++) {
            float stop = static_cast<float>(i) / (gradient.size() - 1);
            gradLeft.Stop(gradient[gradient.size() - 1 - i], stop);
            gradRight.Stop(gradient[i], stop);
        }

        ftxui::Elements rows;
        rows.reserve(barCount);
        for (int i = 0; i < barCount; ++i) {
            float envelope = std::abs(std::sin((float)i / (float)barCount * 3.14159f + phase));
            float value = std::min(displayBars_[i] * envelope, 1.0f);

            rows.push_back(ftxui::hbox({
                ftxui::gaugeLeft(value) | ftxui::color(gradLeft) | ftxui::flex,
                ftxui::text(" "),
                ftxui::gaugeRight(value) | ftxui::color(gradRight) | ftxui::flex,
            }) | ftxui::yflex);
        }

        return ftxui::vbox(rows) | ftxui::yflex | ftxui::flex;
    }

private:
    int frameCount_ = 0;
};

ftxui::Component CreateVisualsCircular(ILibraryService& service) {
    auto self = std::make_shared<VisualsCircular>(service);
    return ftxui::Renderer([self] { return self->render(); });
}
