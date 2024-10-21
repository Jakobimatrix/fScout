#include <finder/Dictionary.h>

#include <fstream>
#include <globals/globals.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utils/filesystem/filesystem.hpp>

Dictionary::Dictionary() { tree = std::make_unique<Tree>(); }

Dictionary::~Dictionary() = default;


void Dictionary::addPath(const std::filesystem::path& path) {
  std::string name = util::getFileName(path);
  // to save storage and computation time, we save everything lower case.
  // The scoring function at the end will score exact matches better than case insensitive matches.
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  tree->insertWord(name, path);
}


std::vector<std::filesystem::path> Dictionary::search(
    const std::string& needle_in,
    const std::vector<std::unique_ptr<SearchPattern>>& patterns) const {
  std::vector<std::string> allNeedles;
  // to save storage and computation time, we save everything lower case.
  // The scoring function at the end will score exact matches better than case insensitive matches.
  std::string needle = needle_in;
  std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
  // Generate needles using each pattern
  for (const auto& pattern : patterns) {
    auto needles = pattern->generateNeedles(needle);
    allNeedles.insert(allNeedles.end(), needles.begin(), needles.end());
  }

  std::multimap<int, std::filesystem::path> scoredResults;

  for (const auto& newNeedle : allNeedles) {
    if (min_search_size > newNeedle.size()) {
      continue;
    }
    auto matches = tree->search(newNeedle);
    for (const auto& match : matches) {
      scoredResults.emplace(scoreMatch(needle, match.filename().string()), match);
    }
  }
  std::set<std::filesystem::path> seenPaths;
  std::vector<std::filesystem::path> result;
  for (const auto& [score, path] : scoredResults) {
    // If the path has not been added yet, insert it into the result
    if (seenPaths.insert(path).second) {
      result.push_back(path);
    }
  }

  return result;
}

void Dictionary::serialize(const std::string& filename,
                           const std::chrono::steady_clock::time_point& timeOfIndexing) const {
  std::ofstream outFile(filename, std::ios::binary);

  if (!outFile.is_open()) {
    throw std::runtime_error("Could not open file for serialization");
  }

  // Write the global header: identifier, version, and indexing time
  const std::string identifier = Globals::getInstance().getBinaryTreeFromatIdentifier();
  constexpr uint32_t version = Globals::VERSION;
  outFile.write(identifier.c_str(), identifier.size());
  outFile.write(reinterpret_cast<const char*>(&version), sizeof(version));

  // Serialize the timeOfIndexing (as seconds since epoch)
  auto timeSinceEpoch =
      std::chrono::duration_cast<std::chrono::seconds>(timeOfIndexing.time_since_epoch())
          .count();
  outFile.write(reinterpret_cast<const char*>(&timeSinceEpoch), sizeof(timeSinceEpoch));

  tree->serialize(outFile);

  outFile.close();
}

void Dictionary::deserialize(const std::string& filename,
                             std::chrono::steady_clock::time_point* timeOfIndexing) {
  std::ifstream inFile(filename, std::ios::binary);

  if (!inFile.is_open()) {
    throw std::runtime_error("Could not open file for deserialization");
  }

  // Read the global header: identifier, version, and indexing time
  const std::string identifier = Globals::getInstance().getBinaryTreeFromatIdentifier();
  char identifierBuffer[identifier.size()];
  uint32_t version = 0;

  inFile.read(identifierBuffer, identifier.size());

  // Check if the identifier matches
  if (std::string(identifierBuffer, identifier.size()) != identifier) {
    throw std::runtime_error(
        "File identifier does not match. This file may not be serialized by "
        "this program.");
  }

  inFile.read(reinterpret_cast<char*>(&version), sizeof(version));

  // Handle different versions if needed (for now we assume version 1)
  if (version != Globals::VERSION) {
    throw std::runtime_error("Unsupported file version.");
  }

  // Deserialize the timeOfIndexing (as seconds since epoch)
  std::time_t timeSinceEpoch;
  inFile.read(reinterpret_cast<char*>(&timeSinceEpoch), sizeof(timeSinceEpoch));
  *timeOfIndexing =
      std::chrono::steady_clock::time_point(std::chrono::seconds(timeSinceEpoch));

  tree = std::make_unique<Tree>();
  tree->deserialize(inFile);

  inFile.close();
}

int Dictionary::scoreMatch(const std::string& needle, const std::string& match) const {
  // Commonly mixed-up character pairs
  // clang-format off
  static const std::unordered_map<char, std::vector<char>> confusedChars = {
    {'k', {'c'}}, {'c', {'k'}},  // Phonetically similar
    {'K', {'C'}}, {'C', {'K'}},  // Phonetically similar
    {'b', {'p'}}, {'p', {'b'}},  // Phonetically similar
    {'b', {'d'}}, {'d', {'b'}},  // Similar shapes
    {'-', {'_'}}, {'_', {'-'}},  // Similar shapes
    {',', {'.'}}, {'.', {','}},  // Typo or Similar shapes
    {'m', {'n'}}, {'n', {'m'}},  // Similar shapes
    {'v', {'w'}}, {'w', {'v'}},  // Visually similar
    {'i', {'l', '!', '1'}}, {'l', {'i', '1', '!'}},  // Similar shapes
    {'1', {'l', 'i', '!'}}, {'!', {'l', 'i', '1'}},  // Similar shapes
    {'o', {'0'}}, {'0', {'o', 'O'}},  // Letter/digit confusion
    {'O', {'0'}}, {'0', {'O'}},  // Uppercase
    {'s', {'z'}}, {'z', {'s'}},  // Phonetically similar
    {'f', {'t'}}, {'t', {'f'}},  // Typo due to adjacent keys
    {'g', {'q', '9'}}, {'q', {'g'}},  // Typo or Similar shapes
    {'u', {'v'}}, {'v', {'u'}},  // Similar shape
    {'5', {'S'}}, {'S', {'5'}},  // Digit/letter confusion

    // Brackets and similar symbols
    {'(', {'{', '['}}, {'{', {'(', '['}}, {'[', {'(', '{'}},  // Opening brackets
    {')', {'}', ']'}}, {'}', {')', ']'}}, {']', {')', '}'}},  // Closing brackets

    // Special symbols
    {'~', {'-'}}, {'-', {'~'}}  // Similar special characters
  };
  // clang-format on

  // Helper function to score individual character pairs
  auto scoreChars = [&](char a, char b) -> int {
    if (a == b) {
      return 0;  // Exact match
    }
    if (std::tolower(a) == std::tolower(b)) {
      return 1;  // Case mismatch
    }
    // Check if they are commonly mixed up
    if ((confusedChars.count(a) &&
         std::find(confusedChars.at(a).begin(), confusedChars.at(a).end(), b) !=
             confusedChars.at(a).end()) ||
        (confusedChars.count(b) &&
         std::find(confusedChars.at(b).begin(), confusedChars.at(b).end(), a) !=
             confusedChars.at(b).end())) {
      return 2;  // Commonly confused characters
    }
    // Case mismatch + confused characters
    if (std::tolower(a) != std::tolower(b) && confusedChars.count(std::tolower(a)) &&
        std::find(confusedChars.at(std::tolower(a)).begin(),
                  confusedChars.at(std::tolower(a)).end(),
                  std::tolower(b)) != confusedChars.at(std::tolower(a)).end()) {
      return 3;
    }
    return 4;  // Total mismatch
  };

  int bestScore = std::numeric_limits<int>::max();

  // Loop over all valid translations where needle and match fully overlap
  for (int offset = -(int)needle.size() + 1; offset < (int)match.size(); ++offset) {
    int score = 0;
    for (int i = 0; i < (int)needle.size(); ++i) {
      int matchIdx = i + offset;
      if (matchIdx >= 0 && matchIdx < (int)match.size()) {
        score += scoreChars(needle[i], match[matchIdx]);
      }
    }
    bestScore = std::min(bestScore, score);
  }

  return bestScore;
}

void Dictionary::visualize() const {
  // tree->print();
  tree->generateDotFile(
      (Globals::getInstance().getAbsPath2Resources() / "map.dot").string());
}
