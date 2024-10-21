#include "display.h"

#include <functional>
#include <globals/globals.hpp>
#include <globals/macros.hpp>

Display::Display()
    : DisplaySettings(Globals::getInstance().getPath2DisplaySettings()) {
  put<int, 4>(disp_pos_size[0], DISP_POS_SIZE, true);
  put<std::string>(split_widget_state, SPLIT_WIDGET_STATE, true);
  put<int>(disp_scale, DISP_SCALE, true);
  put<bool>(prefere_open_folder_beneath, PREFERE_OPEN_FOLDER_BENEATH, true);
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
}


void Display::loadOldIndex() {
  // todo
  // go into cache folder
  // see if the root path was aved and an index file is found
}

void Display::open() {
  setStatus("choosing root folder");

  const auto choosenPath = openDirChooserDialog();
  if (choosenPath.empty()) {
    if (!finder.isInitiated()) {
      setStatus("no root folder selected for search.");
    }
    return;
  }

  if (finder.isWorking()) {
    finder.stopCurrentWorker();
  }

  setStatus("Searching through root Folder: " + choosenPath.string());
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

void Display::callbackIndexing(bool success, const std::string& msg) {
  if (success) {
    if (!finder.isInitiated()) {
      setStatus("Indexing: " + msg);
    } else {
      setStatus("Indexing finnished: " + msg);
      updateInfo(finder.getRootFolder().string(), finder.getNumEntries(), finder.getIndexingDate());
    }
  } else {
    popup_error("Indexing stopped: " + msg);
  }
}

void Display::search(const std::string& needel) {
  if (needel.size() < 2) {
    return;
  }
  last_search = needel;
  setStatus("Search for " + needel);
  finder.search(
      needel, std::bind(&Display::callbackSearch, this, std::placeholders::_1, std::placeholders::_2));
}

void Display::callbackSearch(bool finnished,
                             const std::vector<std::filesystem::path>& results) {
  setSearchResults(results);
  if (finnished) {
    setStatus("Search finnished");
  }
}


void Display::setSearchForFileNames(bool searchFileNames) {
  if (isSetSearchFileNames() == searchFileNames)
    return;

  finder.setUseWildcardPattern(searchFileNames);
  search(last_search);
}
void Display::setSearchForFolderNames(bool searchFolderNames) {
  if (isSetSearchFolderNames() == searchFolderNames)
    return;

  finder.setSearchForFolderNames(searchFolderNames);
  search(last_search);
}
