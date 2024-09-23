#include "display.h"

#include <functional>
#include <globals/globals.hpp>
#include <globals/macros.hpp>

Display::Display()
    : Settings(Globals::getInstance().getPath2DisplaySettings()) {
  put<int, 4>(disp_pos_size[0], DISP_POS_SIZE, true);
  put<std::string>(split_widget_state, SPLIT_WIDGET_STATE, true);
}

Display::~Display() {
  try {
    util::Settings::save();
  } catch (...) {
    F_ERROR("Failed to write into Display Settings: %s",
            Globals::getInstance().getPath2DisplaySettings().c_str());
  }
}

bool Display::save() {
  auto save_path = fileSaveDialog(Globals::getInstance().getBinaryFilePostFix());
  if (save_path.empty() || save_path.filename().empty()) {
    popup_warning("Save cancelled.");
    return false;
  }

  if (finder.saveCurrentIndex(save_path)) {
    setStatus("Indexing saved: " + save_path.string());
    return true;
  } else {
    const bool try_again = askYesNoQuestion(
        "Saving to '" + save_path.string() +
        "' Failed. Do you want to try again (maybe in a different Location?");
    if (try_again)
      return save();
    return false;
  }
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

  if (finder.isInitiated()) {
    if (ask4Save()) {
      if (!save()) {
        popup_error("I could not save the last search index. I am sorry ... ");
        return;
      }
    }
  }

  setStatus("Searching through root Folder: " + choosenPath.string());
  resetDisplayElements();
  finder.setRootPath(
      choosenPath,
      std::bind(&Display::callbackIndexing, this, std::placeholders::_1, std::placeholders::_2));
}

void Display::loadOldIndex() {
  setStatus("Opening saved index...");

  auto load_path = filePickerDialog(Globals::getInstance().getBinaryFilePostFix());
  if (load_path.empty()) {
    popup_warning("Open cancelled.");
    return;
  }

  if (finder.loadIndexFromFile(load_path)) {
    resetDisplayElements();
    setStatus("Indexing loaded.");
    updateInfo(
        finder.getRootFolder(), !need_save, finder.getNumEntries(), finder.getIndexingDate());
    return;
  }
  popup_error("Failed to load indexing.");
}

bool Display::ask4Save() {
  if (need_save) {
    const bool save_changes = askYesNoQuestion(
        "The current querry result was not saved. Do you want to save it for "
        "faster startup next time?");
    if (save_changes) {
      return true;
    }
  }
  return false;
}


bool Display::exitGracefully() {
  if (ask4Save()) {
    return save();
  }
  return true;
}

void Display::callbackIndexing(bool success, const std::string& msg) {
  if (success) {
    if (!finder.isInitiated()) {
      setStatus("Indexing: " + msg);
    } else {
      setStatus("Indexing finnished: " + msg);
      updateInfo(
          finder.getRootFolder(), !need_save, finder.getNumEntries(), finder.getIndexingDate());
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
