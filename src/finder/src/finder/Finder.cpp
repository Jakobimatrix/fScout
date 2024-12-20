#include <finder/Finder.h>

#include <cmath>
#include <filesystem>
#include <functional>
#include <globals/globals.hpp>
#include <globals/macros.hpp>
#include <globals/timer.hpp>
#include <iostream>
#include <sanitizers.hpp>
#include <thread>
#include <utils/filesystem/filesystem.hpp>

#ifdef _WIN32
#include "fileapi.h"
#endif

Finder::Finder()
    : FinderSettings(Globals::getInstance().getPath2fScoutSettings()) {
  setDefaultSearchExceptions();
  put<bool>(useWildcardPattern, USE_WILDCARD_PATTERN, true);
  put<wchar_t>(wildcard, WILDCARD_PATTERN, true);
  put<bool>(searchForFolderNames, SEACH_FOLDERS, true);
  put<bool>(searchForFileNames, SEACH_FILES, true);
  put<bool>(searchHiddenObjects, SEACH_HIDDEN_OBJECTS, true);
  put<float>(fuzzyCoefficient, FUZZY_SEARCH_COEFF, true, util::saneMinMax, MIN_FUZZY_COEFF, MAX_FUZZY_COEFF);
  put<std::unordered_set<std::wstring>>(exceptions, SEACH_EXEPTIONS, true);
}
Finder::~Finder() { save(); }

void Finder::setDefaultSearchExceptions() {
  // Version control directories (Git): Contains repo metadata and history, not useful for search
  exceptions.insert(L".git");

  // Version control directories (Subversion): Similar to .git, holds metadata for SVN
  exceptions.insert(L".svn");

  // Version control directories (Mercurial): Another version control metadata directory
  exceptions.insert(L".hg");

  // macOS-specific metadata file: Stores custom folder view options and icon positions, not useful for search
  exceptions.insert(L".DS_Store");

  // Linux/macOS trash folder: Contains deleted files, usually irrelevant for searching
  exceptions.insert(L".Trash");

  // Common cache directory: Stores temporary files, caching data not relevant for search results
  exceptions.insert(L".cache");

#ifdef _WIN32
  // Windows AppData: Contains user-specific settings, cache, and temporary files, typically irrelevant
  exceptions.insert(L"AppData");

  // Windows system files: Core OS files, users shouldn't search these for regular files
  exceptions.insert(L"Program Files");
  exceptions.insert(L"Program Files (x86)");
  exceptions.insert(L"Windows");

  // Windows recycle bin: Stores deleted files, irrelevant for searches
  exceptions.insert(L"$Recycle.Bin");

  // Windows paging file: A system file for virtual memory management
  exceptions.insert(L"pagefile.sys");

  // Windows hibernation file: Stores the system's state when hibernating
  exceptions.insert(L"hiberfil.sys");

  // Windows system volume information: Contains restore points and other system-level data
  exceptions.insert(L"System Volume Information");

  // Windows swap file: Temporary file used for system memory management
  exceptions.insert(L"swapfile.sys");

  // Windows old system data (post-upgrade): Contains previous versions of Windows, typically irrelevant
  exceptions.insert(L"Windows.old");

  // Windows temporary internet files: Stores cached files from web browsing, unnecessary for file searches
  exceptions.insert(L"Temporary Internet Files");

  // Windows local settings: Stores various temporary and cache files
  exceptions.insert(L"Local Settings");
#else
  // Unix/Linux system directories: Core OS directories not relevant for file search
  exceptions.insert(L"/bin");
  exceptions.insert(L"/boot");
  exceptions.insert(L"/dev");
  exceptions.insert(L"/etc");
  exceptions.insert(L"/lib");
  exceptions.insert(L"/lib32");
  exceptions.insert(L"/lib64");
  exceptions.insert(L"/lost+found");
  exceptions.insert(L"/media");
  exceptions.insert(L"/mnt");
  exceptions.insert(L"/proc");
  exceptions.insert(L"/root");
  exceptions.insert(L"/run");
  exceptions.insert(L"/sbin");
  exceptions.insert(L"/sys");
  exceptions.insert(L"/tmp");
  exceptions.insert(L"/usr");
  exceptions.insert(L"/var");

  // Linux/macOS local folder: Contains data used by programs
  exceptions.insert(L".local");

  // Linux/macOS thumbnail cache: Stores image thumbnails for quick preview, not useful for file searches
  exceptions.insert(L".thumbnails");

  // Installation Folder for windows executable under linux, not usefull for file search
  exceptions.insert(L".wine");
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
    stopWorking.store(true);
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
    constexpr wchar_t SLASH{'/'};
    if (exceptions.find(SLASH + filename) != exceptions.end()) {
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
            callback(false, L"Stopped by User.");
            return;
          }

          if (!shouldIndexEntry(entry)) {
            continue;
          }

          const bool isDirectory = std::filesystem::is_directory(entry.status());

          // dont folow symlinks/junctions, they could create a circle!
          if (!isJunction(entry) && isDirectory &&
              !std::filesystem::is_symlink(entry.status())) {
            directoriesToExplore.push_back(entry.path());
          }
          dictionary->addPath(entry.path(), isDirectory);

          ++numEntries;
          if (t.getPassedTime<std::chrono::milliseconds>() > updateTime) {
            t.start();
            callback(true, std::to_wstring(numEntries) + L" entries found");
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
             std::to_wstring(numEntries) + L" entries found within " +
                 std::to_wstring(t2.getPassedTime<std::chrono::milliseconds>().count()) +
                 L"ms");
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
    dictionary->serialize(filePath, indexingTime);  // Use Dictionary serialization
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
    dictionary->deserialize(filePath, &indexingTime);  // Use Dictionary deserialization
    fullyIndexed = true;
    return true;
  } catch (const std::exception& e) {
    std::cerr << "Failed to load index: " << e.what() << std::endl;
    dictionary.reset();  // Clear dictionary if loading failed
    return false;
  }
}

void Finder::search(const std::wstring needle /*intentional copy*/,
                    const CallbackSearchResult& callback) {
  if (!fullyIndexed) {
    callback(true, {}, needle);
    return;
  }
  constexpr size_t DYNAMIC_LOAD_THRESHOLD = 512;
  constexpr size_t VECTOR_RESERVE_SIZE = 2048;
  stopCurrentWorker();
  workerThread = std::make_unique<std::thread>([this, callback, needle]() {
    std::vector<TreeNode::PathInfo> matches;

    matches.reserve(VECTOR_RESERVE_SIZE);
    bool finnished = false;

    auto collector = std::make_unique<std::thread>([this, &callback, &needle, &matches, &finnished]() {
      size_t num_send_matches = 0;
      std::multimap<int, std::filesystem::path, std::greater<int>> scoredResults;

      auto sendResults = [&scoredResults, &needle, &callback](const bool finished) {
        std::vector<std::filesystem::path> results;
        results.reserve(scoredResults.size());
        const int maxScore = scoredResults.begin()->first;
        const int threshold = maxScore - needle.size();
        std::set<std::filesystem::path> seenPaths;
        for (auto it = scoredResults.begin(); it != scoredResults.end(); ++it) {
          const auto& [score, path] = *it;
          if (score < threshold || !finished && results.size() > 20) {
            break;
          }
          // If the path has not been added yet, insert it into the result
          if (seenPaths.insert(path).second) {
            results.push_back(path);
          }
        }
        /*F_DEBUG("found %lu, unique %lu, send %lu",
                scoredResults.size(),
                seenPaths.size(),
                results.size());*/
        callback(finished, results, needle);
      };

      auto holdDynamicLoading = [this, &matches, &finnished]() {
        if (DYNAMIC_LOAD_THRESHOLD > matches.size()) {
          return;  // dont peak into the vector which is shared between 2 treads, since it can be relocated now.
        }
        while (!finnished && !stopWorking.load()) {
          using namespace std::chrono_literals;
          std::this_thread::sleep_for(100ms);
        }
      };

      bool searchFinnishedAndAllSend = false;
      size_t new_size = 0;
      size_t old_size = 0;
      while (!searchFinnishedAndAllSend) {
        if (stopWorking.load()) {
          break;
        }
        new_size = matches.size();

        for (; num_send_matches < new_size; ++num_send_matches) {
          if (stopWorking.load()) {
            break;
          }
          holdDynamicLoading();
          // copy! in case std vector relocates on resize
          // this could crash if the vector gets relocated while copying.
          // hopefully holdDynamicLoading will prevent this!
          // we could also implement a thread save vector...
          TreeNode::PathInfo match = matches[num_send_matches];
          if (match.isDirectory && searchForFolderNames ||
              !match.isDirectory && searchForFileNames) {
            scoredResults.emplace(
                Dictionary::scoreMatch(needle, util::getLastPathComponent(match.path)),
                match.path);
          }
        }
        searchFinnishedAndAllSend = finnished && new_size == matches.size();
        if (new_size > old_size || searchFinnishedAndAllSend) {
          old_size = new_size;
          sendResults(searchFinnishedAndAllSend);
        }
      }
    });

    wchar_t wildcardChar{useWildcardPattern ? wildcard : Dictionary::NO_WILDCARD};

    constexpr size_t MAX_FUZZY_REPLACEMENTS = 2;
    const size_t numFuzzyReplacements = std::min(
        MAX_FUZZY_REPLACEMENTS, static_cast<size_t>(std::round(fuzzyCoefficient * needle.size())));

    dictionary->search(stopWorking, needle, numFuzzyReplacements, wildcardChar, matches);

    finnished = true;
    if (collector && collector->joinable()) {
      collector->join();
      collector.reset();
    }
  });
}


std::wstring Finder::getIndexingDate() const {
  if (!fullyIndexed) {
    return L"Not indexed yet";
  }

  auto timeT = std::chrono::system_clock::to_time_t(
      std::chrono::system_clock::now() + (indexingTime - std::chrono::steady_clock::now()));

  std::tm* timeStruct = std::localtime(&timeT);

  std::wostringstream oss;
  oss << (timeStruct->tm_year + 1900) << L'.' << std::setw(2)
      << std::setfill(L'0') << (timeStruct->tm_mon + 1) << L'.' << std::setw(2)
      << std::setfill(L'0') << timeStruct->tm_mday;

  return oss.str();
}

bool Finder::usesWildcardPattern() const { return useWildcardPattern; }
wchar_t Finder::getWindcard() const { return wildcard; }

void Finder::setUseWildcardPattern(const bool use) { useWildcardPattern = use; }
void Finder::setWildcard(const wchar_t wildcardChar) {
  wildcard = wildcardChar;
}

void Finder::setSearchForFileNames(const bool searchFileNames) {
  searchForFileNames = searchFileNames;
}
void Finder::setSearchForFolderNames(const bool searchFolderNames) {
  searchForFolderNames = searchFolderNames;
}
void Finder::setSearchHiddenObjects(const bool searchHidden) {
  searchHiddenObjects = searchHidden;
}
bool Finder::isSetSearchFileNames() const { return searchForFileNames; }
bool Finder::isSetSearchFolderNames() const { return searchForFolderNames; }
bool Finder::isSetSearchHiddenObjects() const { return searchHiddenObjects; }

float Finder::getFuzzyCoefficient() const { return fuzzyCoefficient; }
void Finder::setFuzzyCoefficient(const float fuzzy) {
  fuzzyCoefficient = fuzzy;
}
