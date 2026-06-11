#pragma once
#include <ftxui/component/component.hpp>
#include <atomic>
#include <memory>

class ILibraryService;
class IConfigService;

ftxui::Component CreateUserInputs(ILibraryService& service, IConfigService& config,
                                  std::shared_ptr<std::atomic<bool>> reload_flag,
                                  std::shared_ptr<int> visualIndex);
