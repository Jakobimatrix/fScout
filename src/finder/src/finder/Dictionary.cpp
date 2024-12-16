#include <finder/Dictionary.h>
#include <finder/Needle.h>

#include <atomic>
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


void Dictionary::addPath(const std::filesystem::path& path, const bool isDirectory) {
  std::wstring name = util::getLastPathComponent(path);
  // to save storage and computation time, we save everything lower case.
  // The scoring function at the end will score exact matches better than case insensitive matches.
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  tree->insertWord(name, path, isDirectory);
  ++size;
}


void Dictionary::search(std::atomic<bool>& stopSearch,
                        const std::wstring& needle_in,
                        const size_t num_fuzzy_replacements,
                        const wchar_t wildcard,
                        std::vector<TreeNode::PathInfo>& matches) const {

  // to save storage and computation time, we save everything lower case.
  // The scoring function at the end will score exact matches better than case insensitive matches.
  std::wstring needle = needle_in;
  std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
  Needle n{needle};
  if (num_fuzzy_replacements > 0) {
    n.setFuzzySearch(num_fuzzy_replacements);
  }
  if (wildcard != NO_WILDCARD) {
    n.useWildCard(wildcard);
  }
  tree->search(n, stopSearch, matches);
}


void Dictionary::serialize(const std::filesystem::path& filename,
                           const std::chrono::steady_clock::time_point& timeOfIndexing) const {
  std::wofstream outFile(filename, std::ios::binary);

  if (!outFile.is_open()) {
    throw std::runtime_error("Could not open file for serialization");
  }

  // Write the global header: identifier, version, and indexing time
  const std::wstring identifier = Globals::getInstance().getBinaryTreeFromatIdentifier();
  constexpr uint32_t version = Globals::VERSION;
  outFile.write(identifier.c_str(), identifier.size());
  outFile.write(reinterpret_cast<const wchar_t*>(&version), sizeof(version));

  // Serialize the timeOfIndexing (as seconds since epoch)
  auto timeSinceEpoch =
      std::chrono::duration_cast<std::chrono::seconds>(timeOfIndexing.time_since_epoch())
          .count();
  outFile.write(reinterpret_cast<const wchar_t*>(&timeSinceEpoch), sizeof(timeSinceEpoch));

  tree->serialize(outFile);

  outFile.close();
}

void Dictionary::deserialize(const std::filesystem::path& filename,
                             std::chrono::steady_clock::time_point* timeOfIndexing) {
  std::wifstream inFile(filename, std::ios::binary);

  if (!inFile.is_open()) {
    throw std::runtime_error("Could not open file for deserialization");
  }

  // Read the global header: identifier, version, and indexing time
  const std::wstring identifier = Globals::getInstance().getBinaryTreeFromatIdentifier();
  wchar_t identifierBuffer[identifier.size()];
  uint32_t version = 0;

  inFile.read(identifierBuffer, identifier.size());

  // Check if the identifier matches
  if (std::wstring(identifierBuffer, identifier.size()) != identifier) {
    throw std::runtime_error(
        "File identifier does not match. This file may not be serialized by "
        "this program.");
  }

  inFile.read(reinterpret_cast<wchar_t*>(&version), sizeof(version));

  // Handle different versions if needed (for now we assume version 1)
  if (version != Globals::VERSION) {
    throw std::runtime_error("Unsupported file version.");
  }

  // Deserialize the timeOfIndexing (as seconds since epoch)
  std::time_t timeSinceEpoch;
  inFile.read(reinterpret_cast<wchar_t*>(&timeSinceEpoch), sizeof(timeSinceEpoch));
  *timeOfIndexing =
      std::chrono::steady_clock::time_point(std::chrono::seconds(timeSinceEpoch));

  tree = std::make_unique<Tree>();
  tree->deserialize(inFile);

  inFile.close();
}

int Dictionary::scoreChars(wchar_t a, wchar_t b) {
  // Commonly mixed-up character pairs
  // clang-format off
  static const std::unordered_map<wchar_t, std::vector<wchar_t>> confusedChars = {
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

  if (a == b)
    return 3;  // Exact match
  if (std::tolower(a) == std::tolower(b))
    return 2;  // Case mismatch

  // Commonly confused characters
  if ((confusedChars.count(a) &&
       std::find(confusedChars.at(a).begin(), confusedChars.at(a).end(), b) !=
           confusedChars.at(a).end()) ||
      (confusedChars.count(b) &&
       std::find(confusedChars.at(b).begin(), confusedChars.at(b).end(), a) !=
           confusedChars.at(b).end())) {
    return 1;
  }

  return 0;  // Total mismatch
}

int Dictionary::scoreMatch(const std::wstring& searchString, const std::wstring& match) {

  int bestScore = 0;

  // Loop over all valid translations where searchString and match fully overlap
  for (int offset = -(int)searchString.size() + 1; offset < (int)match.size(); ++offset) {
    int score = 0;
    for (int i = 0; i < (int)searchString.size(); ++i) {
      int matchIdx = i + offset;
      if (matchIdx >= 0 && matchIdx < (int)match.size()) {
        score += scoreChars(searchString[i], match[matchIdx]);
      }
    }
    bestScore = std::max(bestScore, score);
  }
  return bestScore;
}

std::vector<int> Dictionary::getMatchScores(const std::wstring& searchString,
                                            const std::wstring& match) {
  std::vector<int> charScores(match.size());
  std::vector<int> bestScores;
  int bestScoreOffset = -(int)searchString.size() + 1;
  int bestScore = 0;

  for (int offset = -(int)searchString.size() + 1; offset < (int)match.size(); ++offset) {
    std::vector<int> tempScores;
    int score = 0;
    for (int i = 0; i < (int)searchString.size(); ++i) {
      int matchIdx = i + offset;
      if (matchIdx >= 0 && matchIdx < (int)match.size()) {
        tempScores.push_back(scoreChars(searchString[i], match[matchIdx]));
        score += tempScores.back();
      }
    }

    if (bestScore < score) {
      bestScore = score;
      bestScoreOffset = offset;
      bestScores = tempScores;
    }
  }
  for (int i = 0; i < (int)charScores.size(); ++i) {
    int matchIdx = i - bestScoreOffset;
    if (matchIdx < 0 || matchIdx > (int)bestScores.size()) {
      charScores[i] = 0;
    } else {
      charScores[i] = bestScores[matchIdx];
    }
  }
  return charScores;
}

void Dictionary::visualize() const {
  // tree->print();
  if (tree == nullptr) {
    return;
  }
  tree->generateDotFile(
      (Globals::getInstance().getAbsPath2Resources() / "map.dot").string());
}
