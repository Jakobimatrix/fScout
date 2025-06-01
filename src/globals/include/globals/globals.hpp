#pragma once

#include <string.h>

#include <filesystem>
#include <stdexcept>
#include <timer/collecting_timer.hpp>
#include <timer/frame_timer.hpp>
#include <vector>


class Globals {
 private:
  Globals() { init_paths(); }
  // Stop the compiler generating methods of copy the object
  Globals(Globals const& copy);             // Not Implemented
  Globals& operator=(Globals const& copy);  // Not Implemented

  ~Globals() {
    constexpr char seperator = ';';
    const std::string filename_timers =
      std::to_string(timeSinceEpochMillisec()) + "_timers.csv";
    const std::string path_timers =
      (absolute_path_to_settings / filename_timers).string();
    collecting_timer.measurementsToFile<std::chrono::microseconds>(path_timers, seperator);

    const std::string filename_frame_timers =
      std::to_string(timeSinceEpochMillisec()) + "_frame_timers.csv";
    const std::string path_frame_timers =
      (absolute_path_to_settings / filename_frame_timers).string();

    frame_timer.measurementsToFile<std::chrono::microseconds>(path_frame_timers, seperator);
  }

 public:
  /*!
   * \brief private Get the one instance of the class
   * \return A reference to the one existing instance of this class.
   */
  static const Globals& getInstance() {
    static Globals instance;
    return instance;
  }

  static CollectingTimer& getCollectingTimer() {
    return getInstance().collecting_timer;
  }

  static FrameTimer& getFrameTimer() { return getInstance().frame_timer; }

  static uint64_t timeSinceEpochMillisec() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  }


 private:
  void init_paths() {
    namespace fs = std::filesystem;

    const fs::path current_path = fs::current_path();
    auto path_it                = current_path.begin();
    fs::path abs_path_to_base   = fs::path("");
    for (; path_it != current_path.end(); path_it++) {
      abs_path_to_base.append(path_it->c_str());
      if (*path_it == REPRO_FOLDER_NAME) {
        break;
      }
    }

    absolute_path_to_base       = abs_path_to_base;
    absolute_path_to_executable = current_path;
    absolute_path_to_resources  = absolute_path_to_base / RESOURCES_FOLDER_NAME;
    absolute_path_to_save_files = absolute_path_to_base / SAVE_FOLDER_NAME;
    absolute_path_to_settings   = absolute_path_to_base / SETTINGS_FOLDER_NAME;

    if (!fs::exists(absolute_path_to_resources)) {
      std::runtime_error("The expected Path " +
                         absolute_path_to_resources.string() + " to " +
                         RESOURCES_FOLDER_NAME.string() + " does not exist.");
    }
    // if the above exists, the general path is correct!
    // just create the folders if the below path do not exist.

    if (!fs::exists(absolute_path_to_save_files)) {
      fs::create_directory(absolute_path_to_save_files);
    }
    if (!fs::exists(absolute_path_to_settings)) {
      fs::create_directory(absolute_path_to_settings);
    }
  }

 public:
  std::filesystem::path getPath2DisplaySettings() const {
    return absolute_path_to_settings / FILE_NAME_DISPLAY_SETTINGS;
  }

  std::filesystem::path getPath2fScoutSettings() const {
    return absolute_path_to_settings / FILE_NAME_FSCOUT_SETTINGS;
  }

  const std::filesystem::path& getAbsPath2Resources() const {
    return absolute_path_to_resources;
  }


  const std::wstring& getBinaryTreeFromatIdentifier() const {
    return BINARY_FORMAT_IDENTIFIER;
  }

  const std::wstring& getBinaryFileIndex() const { return BINARY_FILE_INDEX; }

  const std::string& getMainWindowName() const { return MAIN_WINDOW_NAME; }


  void getAllSavedFiles(std::vector<std::filesystem::path>& files) const {
    for (const auto& entry : std::filesystem::directory_iterator(absolute_path_to_save_files))
      files.push_back(entry);
  }

  std::string getVersion() const {
    return std::to_string(Globals::VERSION) + " - " + VERSION_NAME;
  }

  static constexpr uint32_t VERSION = 1;

 private:
  // Absolute paths to folders
  std::filesystem::path absolute_path_to_base;
  std::filesystem::path absolute_path_to_resources;
  std::filesystem::path absolute_path_to_executable;
  std::filesystem::path absolute_path_to_save_files;
  std::filesystem::path absolute_path_to_settings;

  // Folder names
  const std::filesystem::path REPRO_FOLDER_NAME     = "fScout";
  const std::filesystem::path RESOURCES_FOLDER_NAME = "resources";
  const std::filesystem::path SAVE_FOLDER_NAME      = "saved_data";
  const std::filesystem::path SETTINGS_FOLDER_NAME  = "settings";

  // File names
  const std::filesystem::path FILE_NAME_DISPLAY_SETTINGS =
    "display_settings.txt";
  const std::filesystem::path FILE_NAME_FSCOUT_SETTINGS = "fScout_settings.txt";

  // Strings
  const std::string MAIN_WINDOW_NAME          = std::string("fScout");
  const std::string VERSION_NAME              = std::string("Abyssinian");
  const std::wstring BINARY_FORMAT_IDENTIFIER = std::wstring(L"8008135-fScout");
  const std::wstring BINARY_FILE_INDEX        = std::wstring(L"fScout.index");

  // timer
  mutable CollectingTimer collecting_timer;
  mutable FrameTimer frame_timer;
};
