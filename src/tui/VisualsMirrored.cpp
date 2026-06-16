#include "Visuals.h"
#include "VisualsBase.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>
#include "../service/IVisualizationSource.h"

class VisualsMirrored : public VisualsBase {
public:
    explicit VisualsMirrored(IVisualizationSource& service) : VisualsBase(service) {}

    ftxui::Element render() override {
        smoothBars();
        int barCount = (int)displayBars_.size();
        if (barCount == 0)
            return ftxui::text("") | ftxui::flex;

        const auto& gradient = palette();
        auto gradUp = ftxui::LinearGradient();
        auto gradDown = ftxui::LinearGradient();
        for (size_t i = 0; i < gradient.size(); i++) {
            float stop = static_cast<float>(i) / (gradient.size() - 1);
            gradUp.Stop(gradient[i], stop);
            gradDown.Stop(gradient[gradient.size() - 1 - i], stop);
        }

        ftxui::Elements topBars;
        topBars.reserve(barCount * 2 - 1);
        for (int i = 0; i < barCount; ++i) {
            topBars.push_back(ftxui::gaugeDown(displayBars_[i]) | ftxui::xflex_grow | ftxui::yflex);
            if (i < barCount - 1)
                topBars.push_back(ftxui::text(" "));
        }

        ftxui::Elements bottomBars;
        bottomBars.reserve(barCount * 2 - 1);
        for (int i = 0; i < barCount; ++i) {
            bottomBars.push_back(ftxui::gaugeUp(displayBars_[i]) | ftxui::xflex_grow | ftxui::yflex);
            if (i < barCount - 1)
                bottomBars.push_back(ftxui::text(" "));
        }

        return ftxui::vbox({
            ftxui::hbox(topBars) | ftxui::color(gradDown) | ftxui::yflex,
            ftxui::hbox(bottomBars) | ftxui::color(gradUp) | ftxui::yflex,
        }) | ftxui::border | ftxui::flex;
    }
};

ftxui::Component CreateVisualsMirrored(IVisualizationSource& service) {
    auto self = std::make_shared<VisualsMirrored>(service);
    return ftxui::Renderer([self] { return self->render(); });
}
