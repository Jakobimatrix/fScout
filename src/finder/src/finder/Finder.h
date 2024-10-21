#pragma once

#include <finder/Dictionary.h>
#include <finder/SearchPattern.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <set>
#include <settings.hpp>
#include <string>
#include <thread>

using FinderSettings =
    util::Settings<std::variant<bool *, size_t *, char *, std::set<std::string> *>>;
class Finder : public FinderSettings {
  std::filesystem::path root = std::filesystem::path();
  bool fullyIndexed = false;
  std::unique_ptr<Dictionary> dictionary;
  std::unique_ptr<std::thread> workerThread;
  std::atomic<bool> stopWorking = false;

  using CallbackFinnished = std::function<void(const bool, const std::string &msg)>;
  using CallbackSearchResult =
      std::function<void(const bool, const std::vector<std::filesystem::path> &)>;
  size_t numEntries = 0;

 public:
  Finder();
  ~Finder();
  bool isInitiated() const;
  bool isWorking() const;
  void stopCurrentWorker();
  size_t getNumEntries() const;
  std::filesystem::path getRootFolder() const;

  void setRootPath(const std::filesystem::path &, const CallbackFinnished &);

  bool saveCurrentIndex(const std::filesystem::path &);
  bool loadIndexFromFile(const std::filesystem::path &);

  bool usesExactPattern() const { return useExactMatchPattern; }
  bool usesFuzzyMatchPattern() const { return useFuzzyMatchPattern; }
  bool usesWildcardPattern() const { return useWildcardPattern; }
  char getWindcard() const { return wildcard; }
  bool usesSubsetPattern() const { return useSubsetPattern; }
  size_t getMinSubPatternSearchSize() const { return minSubPatternSize; }

  void setUseExactMatchPattern(const bool use) { useExactMatchPattern = use; }

  void setUseFuzzyMatchPattern(const bool use) { useFuzzyMatchPattern = use; }
  void setUseWildcardPattern(const bool use) { useWildcardPattern = use; }
  void setWildcard(const char wildcardChar) { wildcard = wildcardChar; }
  void setUseSubsetPattern(const bool use) { useSubsetPattern = use; }
  void setMinSubPatternSize(const size_t size) { minSubPatternSize = size; }

  void setSearchForFileNames(const bool searchFileNames) {
    searchForFileNames = searchFileNames;
  }
  void setSearchForFolderNames(const bool searchFolderNames) {
    searchForFolderNames = searchFolderNames;
  }
  void setSearchHiddenObjects(const bool searchHidden) {
    searchHiddenObjects = searchHidden;
  }
  bool isSetSearchFileNames() const { return searchForFileNames; }
  bool isSetSearchFolderNames() const { return searchForFolderNames; }
  bool isSetSearchHiddenObjects() const { return searchHiddenObjects; }


  void search(const std::string needle, const CallbackSearchResult &);

  std::string getIndexingDate() const;

  void visualize() const { dictionary->visualize(); }

 private:
  void setDefaultSearchExceptions();
  bool shouldIndexEntry(const std::filesystem::directory_entry &entry) const;
  void startIndexing(const CallbackFinnished &);
  std::vector<std::unique_ptr<SearchPattern>> getActiveSearchPatterns() const;

  // SETTINGS
  bool useExactMatchPattern = true;
  const std::string USE_EXACT_PATTERN = "UseExactPattern";
  bool useFuzzyMatchPattern = true;
  const std::string USE_FUZZY_PATTERN = "UseFuzzyPattern";
  bool useWildcardPattern = false;
  const std::string USE_WILDCARD_PATTERN = "UseWildcardPattern";
  char wildcard = '*';
  const std::string WILDCARD_PATTERN = "Wildcard";
  bool useSubsetPattern = false;
  const std::string USE_SubSet_PATTERN = "UseSubSetPattern";
  size_t minSubPatternSize = 3;
  const std::string SUBSET_SIZE = "SubsetSize";
  bool searchForFolderNames = true;
  const std::string SEACH_FOLDERS = "SearchFolders";
  bool searchForFileNames = true;
  const std::string SEACH_FILES = "SearchFiles";
  bool searchHiddenObjects = false;
  const std::string SEACH_HIDDEN_OBJECTS = "SearchHiddenObjects";
  std::set<std::string> exceptions;
  const std::string SEACH_EXEPTIONS = "SearchExceptions";

  std::chrono::steady_clock::time_point indexingTime;
};
