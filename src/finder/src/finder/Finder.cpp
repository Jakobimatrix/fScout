#include <finder/Finder.h>

#include <filesystem>
#include <functional>
#include <globals/globals.hpp>
#include <globals/macros.hpp>
#include <globals/timer.hpp>
#include <iostream>
#include <thread>
#include <utils/filesystem/filesystem.hpp>

#ifdef _WIN32
#include "fileapi.h"
#endif

Finder::Finder()
    : FinderSettings(Globals::getInstance().getPath2fScoutSettings()) {
  setDefaultSearchExceptions();
  put<bool>(useExactMatchPattern, USE_EXACT_PATTERN, true);
  put<bool>(useFuzzyMatchPattern, USE_FUZZY_PATTERN, true);
  put<bool>(useWildcardPattern, USE_WILDCARD_PATTERN, true);
  put<char>(wildcard, WILDCARD_PATTERN, true);
  put<bool>(useSubsetPattern, USE_SubSet_PATTERN, true);
  put<size_t>(minSubPatternSize, SUBSET_SIZE, true);
  put<bool>(searchForFolderNames, SEACH_FOLDERS, true);
  put<bool>(searchForFileNames, SEACH_FILES, true);
  put<bool>(searchHiddenObjects, SEACH_HIDDEN_OBJECTS, true);
  put<std::set<std::string>>(exceptions, SEACH_EXEPTIONS, true);
}
Finder::~Finder() { save(); }

void Finder::setDefaultSearchExceptions() {
  // Version control directories (Git): Contains repo metadata and history, not useful for search
  exceptions.insert(".git");

  // Version control directories (Subversion): Similar to .git, holds metadata for SVN
  exceptions.insert(".svn");

  // Version control directories (Mercurial): Another version control metadata directory
  exceptions.insert(".hg");

  // macOS-specific metadata file: Stores custom folder view options and icon positions, not useful for search
  exceptions.insert(".DS_Store");

  // Linux/macOS trash folder: Contains deleted files, usually irrelevant for searching
  exceptions.insert(".Trash");

  // Common cache directory: Stores temporary files, caching data not relevant for search results
  exceptions.insert(".cache");

#ifdef _WIN32
  // Windows AppData: Contains user-specific settings, cache, and temporary files, typically irrelevant
  exceptions.insert("AppData");

  // Windows system files: Core OS files, users shouldn't search these for regular files
  exceptions.insert("Program Files");
  exceptions.insert("Program Files (x86)");
  exceptions.insert("Windows");

  // Windows recycle bin: Stores deleted files, irrelevant for searches
  exceptions.insert("$Recycle.Bin");

  // Windows paging file: A system file for virtual memory management
  exceptions.insert("pagefile.sys");

  // Windows hibernation file: Stores the system's state when hibernating
  exceptions.insert("hiberfil.sys");

  // Windows system volume information: Contains restore points and other system-level data
  exceptions.insert("System Volume Information");

  // Windows swap file: Temporary file used for system memory management
  exceptions.insert("swapfile.sys");

  // Windows old system data (post-upgrade): Contains previous versions of Windows, typically irrelevant
  exceptions.insert("Windows.old");

  // Windows temporary internet files: Stores cached files from web browsing, unnecessary for file searches
  exceptions.insert("Temporary Internet Files");

  // Windows local settings: Stores various temporary and cache files
  exceptions.insert("Local Settings");
#else
  // Unix/Linux system directories: Core OS directories not relevant for file search
  exceptions.insert("/bin");
  exceptions.insert("/boot");
  exceptions.insert("/dev");
  exceptions.insert("/etc");
  exceptions.insert("/lib");
  exceptions.insert("/lib32");
  exceptions.insert("/lib64");
  exceptions.insert("/lost+found");
  exceptions.insert("/media");
  exceptions.insert("/mnt");
  exceptions.insert("/proc");
  exceptions.insert("/root");
  exceptions.insert("/run");
  exceptions.insert("/sbin");
  exceptions.insert("/sys");
  exceptions.insert("/tmp");
  exceptions.insert("/usr");
  exceptions.insert("/var");

  // Linux/macOS local folder: Contains data used by programs
  exceptions.insert(".local");

  // Linux/macOS thumbnail cache: Stores image thumbnails for quick preview, not useful for file searches
  exceptions.insert(".thumbnails");

  // Installation Folder for windows executable under linux, not usefull for file search
  exceptions.insert(".wine");
#endif
}

bool Finder::isInitiated() const { return fullyIndexed; }
bool Finder::isWorking() const {
  if (workerThread && workerThread->joinable()) {
    return true;
  }
  return false;
}
std::filesystem::path Finder::getRootFolder() const { return root; }
size_t Finder::getNumEntries() const { return numEntries; }

void Finder::stopCurrentWorker() {
  if (workerThread && workerThread->joinable()) {
    stopWorking = true;
    workerThread->join();
    workerThread.reset();
  }
  stopWorking = false;
}

bool Finder::shouldIndexEntry(const std::filesystem::directory_entry& entry) const {
  // Exclude directories or files with no read permissions
  const auto perm = std::filesystem::status(entry.path()).permissions();
  if ((perm & std::filesystem::perms::owner_read) == std::filesystem::perms::none) {
    return false;
  }

  const auto filename = util::getLastPathComponent(entry);
  // Exclude hidden objects (if not searching hidden objects)
  if (!searchHiddenObjects) {
    if (filename.empty() || filename[0] == '.') {
      return false;
    }
  }

  if (!std::filesystem::is_directory(entry.status())) {
    return true;
  }
  // check exception directories


#ifdef _WIN32
  constexpr bool onUnix(false);
#else
  constexpr bool onUnix(true);
#endif
  // Check if we're on Unix (Linux/macOS) and if entry is a directory at the root level
  if (onUnix && root == std::filesystem::path("/") && entry.path().parent_path() == root) {
    if (exceptions.find("/" + filename) != exceptions.end()) {
      return false;
    }
  } else {
    // Check if the folder is in the exceptions set (O(log n) lookup)
    if (exceptions.find(filename) != exceptions.end()) {
      return false;
    }
  }

  return true;
}

void Finder::startIndexing(const Finder::CallbackFinnished& callback) {



  // Stop any currently running indexing thread
  stopCurrentWorker();

  // Reset state for new indexing
  fullyIndexed = false;
  dictionary = std::make_unique<Dictionary>();

  workerThread = std::make_unique<std::thread>([this, callback]() {

#ifdef _WIN32
    auto isJunction = [](const auto& entry) {
      // Check for junctions on Windows (treat them like symlinks)
      DWORD attributes = GetFileAttributesW(entry.path().c_str());
      return attributes != INVALID_FILE_ATTRIBUTES &&
             (attributes & FILE_ATTRIBUTE_REPARSE_POINT);
    };
#else
    auto isJunction = [](const auto&) { return false; };
#endif

    // List of directories to explore
    std::vector<std::filesystem::path> directoriesToExplore = {this->root};
    const std::chrono::milliseconds updateTime(40);
    Timer t;
    t.start();
    Timer t2;
    t2.start();
    while (!directoriesToExplore.empty() && !stopWorking) {
      std::filesystem::path currentPath = directoriesToExplore.back();
      directoriesToExplore.pop_back();

      try {
        for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {
          if (stopWorking) {
            callback(false, "Stopped by User.");
            return;
          }

          if (!shouldIndexEntry(entry)) {
            continue;
          }

          // dont folow symlinks/junctions, they could create a circle!
          if (!isJunction(entry) && std::filesystem::is_directory(entry.status()) &&
              !std::filesystem::is_symlink(entry.status())) {
            directoriesToExplore.push_back(entry.path());
          }
          dictionary->addPath(entry.path());

          ++numEntries;
          if (t.getPassedTime<std::chrono::milliseconds>() > updateTime) {
            t.start();
            callback(true, std::to_string(numEntries) + " entries found");
          }
        }
      } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Skipping directory due to error: " << e.what() << std::endl;
        continue;
      }
    }

    fullyIndexed = true;
    indexingTime = std::chrono::steady_clock::now();
    callback(true,
             std::to_string(numEntries) + " entries found within " +
                 std::to_string(t2.getPassedTime<std::chrono::milliseconds>().count()) +
                 "ms");
  });
}

void Finder::setRootPath(const std::filesystem::path& path_to_root,
                         const Finder::CallbackFinnished& callback) {
  if (root == path_to_root) {
    return;
  }
  root = path_to_root;
  startIndexing(callback);
}

bool Finder::saveCurrentIndex(const std::filesystem::path& filePath) {
  if (!fullyIndexed || !dictionary) {
    std::cerr << "Index is not fully built, or no dictionary available to save."
              << std::endl;
    return false;
  }

  try {
    dictionary->serialize(filePath.string(), indexingTime);  // Use Dictionary serialization
    return true;
  } catch (const std::exception& e) {
    std::cerr << "Failed to save index: " << e.what() << std::endl;
    return false;
  }
}

bool Finder::loadIndexFromFile(const std::filesystem::path& filePath) {
  if (!std::filesystem::exists(filePath)) {
    std::cerr << "The specified index file does not exist." << std::endl;
    return false;
  }

  try {
    dictionary = std::make_unique<Dictionary>();
    dictionary->deserialize(filePath.string(), &indexingTime);  // Use Dictionary deserialization
    fullyIndexed = true;
    return true;
  } catch (const std::exception& e) {
    std::cerr << "Failed to load index: " << e.what() << std::endl;
    dictionary.reset();  // Clear dictionary if loading failed
    return false;
  }
}

void Finder::search(const std::string needle /*intentional copy*/,
                    const CallbackSearchResult& callback) {
  stopCurrentWorker();
  workerThread = std::make_unique<std::thread>([this, callback, needle]() {
    std::vector<std::filesystem::path> results =
        dictionary->search(needle, getActiveSearchPatterns());
    if (!searchForFileNames) {  // TODO this is very inefficient, maxbe 2 different trees?
      results.erase(std::remove_if(results.begin(),
                                   results.end(),
                                   [](const std::filesystem::path& p) {
                                     return std::filesystem::is_regular_file(p);
                                   }),
                    results.end());
    }

    if (!searchForFolderNames) {
      results.erase(std::remove_if(results.begin(),
                                   results.end(),
                                   [](const std::filesystem::path& p) {
                                     return std::filesystem::is_directory(p);
                                   }),
                    results.end());
    }
    callback(true, results, needle);
  });
}

std::vector<std::unique_ptr<SearchPattern>> Finder::getActiveSearchPatterns() const {
  std::vector<std::unique_ptr<SearchPattern>> patterns;
  if (useExactMatchPattern) {
    patterns.emplace_back(std::make_unique<ExactMatchPattern>());
  }
  if (useFuzzyMatchPattern) {
    patterns.emplace_back(std::make_unique<FuzzyMatchPattern>());
  }
  if (useWildcardPattern) {
    patterns.emplace_back(std::make_unique<WildcardPattern>(wildcard));
  }
  if (useSubsetPattern) {
    patterns.emplace_back(std::make_unique<SubsetPattern>(minSubPatternSize));
  }
  return patterns;
}

std::string Finder::getIndexingDate() const {
  if (!fullyIndexed) {
    return "Not indexed yet";
  }

  auto timeT = std::chrono::system_clock::to_time_t(
      std::chrono::system_clock::now() + (indexingTime - std::chrono::steady_clock::now()));

  std::tm* timeStruct = std::localtime(&timeT);

  std::ostringstream oss;
  oss << (timeStruct->tm_year + 1900) << '.' << std::setw(2)
      << std::setfill('0') << (timeStruct->tm_mon + 1) << '.' << std::setw(2)
      << std::setfill('0') << timeStruct->tm_mday;

  return oss.str();
}
