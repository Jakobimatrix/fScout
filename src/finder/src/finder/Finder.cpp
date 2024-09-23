#include <finder/Finder.h>

#include <filesystem>
#include <functional>
#include <globals/timer.hpp>
#include <iostream>
#include <thread>

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

void Finder::startIndexing(const Finder::CallbackFinnished& callback) {
  // Stop any currently running indexing thread
  stopCurrentWorker();

  // Reset state for new indexing
  fullyIndexed = false;
  dictionary = std::make_unique<Dictionary>();

  workerThread = std::make_unique<std::thread>([this, callback]() {
    try {
      const std::chrono::milliseconds updateTime(40);
      Timer t;
      t.start();
      numEntries = 0;
      for (const auto& entry : std::filesystem::recursive_directory_iterator(this->root)) {
        ++numEntries;
        if (t.getPassedTime<std::chrono::milliseconds>() > updateTime) {
          if (stopWorking) {
            callback(false, "Stopped by User.");
            return;
          }
          t.start();
          callback(true, std::to_string(numEntries) + " entries found");
        }
        dictionary->addPath(entry.path());
      }

      fullyIndexed = true;
      indexingTime = std::chrono::steady_clock::now();
      callback(true, std::to_string(numEntries) + " entries found");

    } catch (const std::filesystem::filesystem_error& e) {
      callback(false, std::string("Filesystem error: " + std::string(e.what())));
    }
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

void Finder::search(const std::string needle, const CallbackSearchResult& callback) {
  stopCurrentWorker();
  workerThread = std::make_unique<std::thread>([this, callback, needle]() {
    std::vector<std::filesystem::path> results = dictionary->advancedSearch(
        needle, getActiveSearchPatterns(), useCaseInsensitivePattern);
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
    callback(true, results);
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
