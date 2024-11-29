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
    util::Settings<std::variant<bool *, float *, size_t *, wchar_t *, std::unordered_set<std::wstring> *>>;
class Finder : public FinderSettings {
  std::filesystem::path root = std::filesystem::path();
  bool fullyIndexed = false;
  std::unique_ptr<Dictionary> dictionary;
  std::unique_ptr<std::thread> workerThread;
  std::atomic<bool> stopWorking = false;

  using CallbackFinnished = std::function<void(const bool, const std::wstring &msg)>;
  using CallbackSearchResult =
      std::function<void(const bool, const std::vector<std::filesystem::path> &, const std::wstring &)>;
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

  bool usesWildcardPattern() const;
  wchar_t getWindcard() const;
  void setUseWildcardPattern(const bool use);
  void setWildcard(const wchar_t wildcardChar);

  void setSearchForFileNames(const bool searchFileNames);
  void setSearchForFolderNames(const bool searchFolderNames);
  void setSearchHiddenObjects(const bool searchHidden);
  void setFuzzyCoefficient(const float fuzzy);
  bool isSetSearchFileNames() const;
  bool isSetSearchFolderNames() const;
  bool isSetSearchHiddenObjects() const;
  float getFuzzyCoefficient() const;


  void search(const std::wstring needle, const CallbackSearchResult &);

  std::wstring getIndexingDate() const;

  void visualize() const { dictionary->visualize(); }

 private:
  void setDefaultSearchExceptions();
  bool shouldIndexEntry(const std::filesystem::directory_entry &entry) const;
  void startIndexing(const CallbackFinnished &);

  // SETTINGS
  float fuzzyCoefficient = 0.25f;
  const std::string FUZZY_SEARCH_COEFF = "fuzzySearch";
  bool useWildcardPattern = false;
  const std::string USE_WILDCARD_PATTERN = "UseWildcardPattern";
  wchar_t wildcard = L'*';
  const std::string WILDCARD_PATTERN = "Wildcard";
  bool searchForFolderNames = true;
  const std::string SEACH_FOLDERS = "SearchFolders";
  bool searchForFileNames = true;
  const std::string SEACH_FILES = "SearchFiles";
  bool searchHiddenObjects = false;
  const std::string SEACH_HIDDEN_OBJECTS = "SearchHiddenObjects";
  std::unordered_set<std::wstring> exceptions;
  const std::string SEACH_EXEPTIONS = "SearchExceptions";

  std::chrono::steady_clock::time_point indexingTime;

 public:
  static constexpr float MAX_FUZZY_COEFF = 1.f;
  static constexpr float MIN_FUZZY_COEFF = 0.f;
};
