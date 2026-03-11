#pragma once
#include <ftxui/component/component.hpp>
#include <memory>

class ILibraryService;

ftxui::Component CreateFileManager(ILibraryService& service, const std::shared_ptr<bool>& reload_flag);
