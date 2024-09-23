#pragma once

#include <finder/Dictionary.h>
#include <finder/SearchPattern.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <thread>

class Finder {
  std::filesystem::path root = std::filesystem::path();
  bool fullyIndexed = false;
  std::unique_ptr<Dictionary> dictionary;
  std::unique_ptr<std::thread> workerThread;
  std::atomic<bool> stopWorking;

  using CallbackFinnished = std::function<void(const bool, const std::string &msg)>;
  using CallbackSearchResult =
      std::function<void(const bool, const std::vector<std::filesystem::path> &)>;
  size_t numEntries = 0;

 public:
  Finder()
      : stopWorking(false),
        fullyIndexed(false),
        dictionary(std::make_unique<Dictionary>()) {}
  bool isInitiated() const;
  bool isWorking() const;
  size_t getNumEntries() const;
  std::filesystem::path getRootFolder() const;

  void setRootPath(const std::filesystem::path &, const CallbackFinnished &);

  bool saveCurrentIndex(const std::filesystem::path &);
  bool loadIndexFromFile(const std::filesystem::path &);

  bool usesExactPattern() const { return useExactMatchPattern; }
  bool usesCaseInsensitivePattern() const { return useCaseInsensitivePattern; }
  bool usesFuzzyMatchPattern() const { return useFuzzyMatchPattern; }
  bool usesWildcardPattern() const { return useWildcardPattern; }
  char getWindcard() const { return wildcard; }
  bool usesSubsetPattern() const { return useSubsetPattern; }
  size_t getMinSubPatternSearchSize() const { return minSubPatternSize; }

  void setUseExactMatchPattern(const bool use) { useExactMatchPattern = use; }
  void setUseCaseInsensitivePattern(const bool use) {
    useCaseInsensitivePattern = use;
  }
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
  bool isSetSearchFileNames() const { return searchForFileNames; }
  bool isSetSearchFolderNames() const { return searchForFolderNames; }


  void search(const std::string needle, const CallbackSearchResult &);

  std::string getIndexingDate() const;

 private:
  void startIndexing(const CallbackFinnished &);
  void stopCurrentWorker();
  std::vector<std::unique_ptr<SearchPattern>> getActiveSearchPatterns() const;

  bool useExactMatchPattern = true;
  bool useCaseInsensitivePattern = true;
  bool useFuzzyMatchPattern = true;
  bool useWildcardPattern = false;
  char wildcard = '*';
  bool useSubsetPattern = false;
  size_t minSubPatternSize = 3;
  bool searchForFolderNames = true;
  bool searchForFileNames = true;
  std::chrono::steady_clock::time_point indexingTime;
};
