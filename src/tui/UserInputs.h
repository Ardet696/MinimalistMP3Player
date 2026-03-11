#pragma once
#include <ftxui/component/component.hpp>
#include <memory>

class ILibraryService;
class IConfigService;

ftxui::Component CreateUserInputs(ILibraryService& service,IConfigService& config, std::shared_ptr<bool> reload_flag);
