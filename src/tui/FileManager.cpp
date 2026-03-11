#include "FileManager.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

#include "../service/ILibraryService.h"
#include "ftxui/component/event.hpp"
#include "Palette.h"

ftxui::Component CreateFileManager(ILibraryService& service,
                                   const std::shared_ptr<bool>& reload_flag) {
  using namespace ftxui;
  auto albums = std::make_shared<std::vector<std::string>>(
    service.getAlbumNames()
  );
  auto songs_per_album = std::make_shared<std::vector<std::vector<std::string>>>(
    service.getAllSongNames()
  );

  auto selected_album = std::make_shared<int>(0);
  auto selected_song  = std::make_shared<int>(0);
  auto viewing_songs  = std::make_shared<bool>(false);
  auto tab_index      = std::make_shared<int>(0);

  auto current_songs = std::make_shared<std::vector<std::string>>(
    albums->empty() ? std::vector<std::string>{} : (*songs_per_album)[0]
  );

  auto album_menu = Menu(albums.get(), selected_album.get());
  auto song_menu  = Menu(current_songs.get(), selected_song.get());

  auto album_with_enter = CatchEvent(album_menu, [=, &service](Event event) {
    if (event == Event::Return && !albums->empty()) {
      *current_songs = (*songs_per_album)[*selected_album];
      *viewing_songs = true;
      *tab_index = 1;
      *selected_song = 0;
      service.playSong((*albums)[*selected_album], 0);
      return true;
    }
    return false;
  });

  auto song_with_events = CatchEvent(song_menu, [=, &service](Event event) {
    if (event == Event::Return && !current_songs->empty()) {
      service.playSong((*albums)[*selected_album], *selected_song);
      return true;
    }
    if (event == Event::Backspace) {
      *viewing_songs = false;
      *tab_index = 0;
      return true;
    }
    return false;
  });

  auto container = Container::Tab({album_with_enter, song_with_events}, tab_index.get());

  return Renderer(container, [=, &service] {
    if (*reload_flag) {
      *albums = service.getAlbumNames();
      *songs_per_album = service.getAllSongNames();
      *current_songs = albums->empty() ? std::vector<std::string>{} : (*songs_per_album)[0];
      *selected_album = 0;
      *selected_song = 0;
      *viewing_songs = false;
      *tab_index = 0;
      *reload_flag = false;
    }

    std::string currentAlbum = service.getCurrentAlbum();
    int playingIndex = -1;
    if (*viewing_songs && !albums->empty() && !currentAlbum.empty()) {
      if (currentAlbum == (*albums)[*selected_album]) {
        playingIndex = service.getCurrentTrackIndex();
      }
    }

    Element content;
    if (albums->empty()) {
      content = vbox({
        text("FileManager") | bold | center,
        separator(),
        text("") | flex,
        text("Waiting for root path,") | center,
        text("type 'RootConfig' in input") | center,
        text("bar to set it up") | center,
        text("") | flex,
      });
    } else if (*viewing_songs) {
      if (playingIndex >= 0) {
        auto playingColor = Palette::getCurrentGradient().at(1);
        content = vbox({
          text(" < " + (*albums)[*selected_album]) | bold,
          separator(),
          text("▶ Now playing: " + (*current_songs)[playingIndex]) | color(playingColor) | bold,
          song_with_events->Render() | flex,
        });
      } else {
        content = vbox({
          text(" < " + (*albums)[*selected_album]) | bold,
          separator(),
          song_with_events->Render() | flex,
        });
      }
    } else {
      content = vbox({
        text("Albums") | bold | center,
        separator(),
        album_with_enter->Render() | flex,
      });
    }
    return content | border | flex;
  });
}
