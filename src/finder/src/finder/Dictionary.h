#pragma once

#include <finder/SearchPattern.h>
#include <finder/Tree.h>

#include <filesystem>
#include <string>
#include <vector>

class Dictionary {
 public:
  constexpr static wchar_t NO_WILDCARD{'\n'};
  Dictionary();
  ~Dictionary();

  void addPath(const std::filesystem::path &, const bool);


  void search(std::atomic<bool> &stopSearch,
              const std::wstring &needle_in,
              const size_t num_fuzzy_replacements,
              const wchar_t wildcard,
              std::vector<std::pair<std::wstring, const std::vector<TreeNode::PathInfo> *>> &matches) const;

  void serialize(const std::filesystem::path &filename,
                 const std::chrono::steady_clock::time_point &timeOfIndexing) const;
  void deserialize(const std::filesystem::path &filename,
                   std::chrono::steady_clock::time_point *timeOfIndexing);


  void visualize() const;

  size_t getSize() const { return size; }

  static int scoreChars(wchar_t a, wchar_t b);
  static int scoreMatch(const std::wstring &needle, const std::wstring &match);
  static std::vector<int> getMatchScores(const std::wstring &needle,
                                         const std::wstring &match);

 private:
  std::unique_ptr<Tree> tree;
  size_t size = 0;
};
