#include "display.h"

#include <functional>
#include <globals/globals.hpp>
#include <globals/macros.hpp>

Display::Display()
    : DisplaySettings(Globals::getInstance().getPath2DisplaySettings()) {
  put<int, 4>(disp_pos_size[0], DISP_POS_SIZE, true);
  put<std::string>(split_widget_state, SPLIT_WIDGET_STATE, true);
  put<int>(disp_scale, DISP_SCALE, true);
  put<int>(doubleClickInterval_ms, DOUBLE_CLICK_INTERVAL, true);
}

Display::~Display() {
  if (!exitGracefully()) {
    ERROR("Could not exit gracefully :(");
  }
  try {
    DisplaySettings::save();
    finder.save();
  } catch (...) {
    F_ERROR("Failed to write into Display Settings: %s",
            Globals::getInstance().getPath2DisplaySettings().string().c_str());
  }
}

void Display::visualize() const { finder.visualize(); }

void Display::save() {
  // todo
  // go into cache folder
  // create all folders like the root path
  // in the last folder save the current index
  if (!finder.saveCurrentIndex(finder.getRootFolder() /
                               Globals::getInstance().getBinaryFileIndex())) {
    std::wstring msg(
        L"Failed to save index to " +
        (finder.getRootFolder() / Globals::getInstance().getBinaryFileIndex()).wstring());
    popup_error(msg, L"Error Saveing");
  }
}


void Display::loadOldIndex() {
  // todo
  // go into cache folder
  // see if the root path was aved and an index file is found
}

void Display::open() {
  setStatus(L"choosing root folder");

  const auto choosenPath = openDirChooserDialog();
  if (choosenPath.empty()) {
    if (!finder.isInitiated()) {
      setStatus(L"no root folder selected for search.");
    }
    return;
  }

  if (finder.isWorking()) {
    finder.stopCurrentWorker();
  }

  setStatus(L"Searching through root Folder: " + choosenPath.wstring());
  resetDisplayElements();
  finder.setRootPath(
      choosenPath,
      std::bind(&Display::callbackIndexing, this, std::placeholders::_1, std::placeholders::_2));
}


bool Display::exitGracefully() {
  if (finder.isWorking()) {
    finder.stopCurrentWorker();
  }
  // todo dont block. make a callback and check after 1 second: if still working... kill
  return true;
}

void Display::callbackIndexing(bool success, const std::wstring& msg) {
  if (success) {
    if (!finder.isInitiated()) {
      setStatus(L"Indexing: " + msg);
    } else {
      setStatus(L"Indexing finnished: " + msg);
      updateInfo(finder.getRootFolder().wstring(),
                 finder.getNumEntries(),
                 finder.getIndexingDate());
    }
  } else {
    popup_error(L"Indexing stopped: " + msg, L"Error");
  }
}

void Display::search(const std::wstring& needel) {
  if (needel.size() < 2) {
    return;
  }
  last_search = needel;
  setStatus(L"Search for " + needel);
  finder.search(needel,
                std::bind(&Display::callbackSearch,
                          this,
                          std::placeholders::_1,
                          std::placeholders::_2,
                          std::placeholders::_3));
}

void Display::searchAgain() { search(last_search); }

void Display::callbackSearch(bool finnished,
                             const std::vector<std::filesystem::path>& results,
                             const std::wstring& search) {
  setSearchResults(results, search);
  if (finnished) {
    setStatus(L"Search finnished, found " + std::to_wstring(results.size()) +
              L" matches");
  } else {
    setStatus(L"searching ... " + std::to_wstring(results.size()));
  }
}


void Display::setSearchForFileNames(bool searchFileNames) {
  if (isSetSearchFileNames() == searchFileNames)
    return;

  finder.setSearchForFileNames(searchFileNames);
  search(last_search);
}
void Display::setSearchForFolderNames(bool searchFolderNames) {
  if (isSetSearchFolderNames() == searchFolderNames)
    return;

  finder.setSearchForFolderNames(searchFolderNames);
  search(last_search);
}
