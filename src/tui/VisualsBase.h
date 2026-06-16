#pragma once
#include <vector>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/screen/color.hpp>

class IVisualizationSource;

class VisualsBase {
public:
    explicit VisualsBase(IVisualizationSource& service);
    virtual ~VisualsBase() = default;

    virtual ftxui::Element render() = 0;

protected:
    IVisualizationSource& service_;
    std::vector<float> displayBars_;

    const std::vector<ftxui::Color>& palette() const;
    void smoothBars();
};
