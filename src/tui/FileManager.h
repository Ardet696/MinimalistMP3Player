#pragma once
#include <ftxui/component/component.hpp>
#include <atomic>
#include <memory>

class ILibraryQuery;
class IPlaybackControl;

ftxui::Component CreateFileManager(ILibraryQuery& query, IPlaybackControl& control, const std::shared_ptr<std::atomic<bool>>& reload_flag);
