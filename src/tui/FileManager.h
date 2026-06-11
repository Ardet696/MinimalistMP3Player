#pragma once
#include <ftxui/component/component.hpp>
#include <atomic>
#include <memory>

class ILibraryService;

ftxui::Component CreateFileManager(ILibraryService& service, const std::shared_ptr<std::atomic<bool>>& reload_flag);
