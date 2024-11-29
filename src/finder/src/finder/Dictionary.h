#pragma once

#include <finder/SearchPattern.h>
#include <finder/Tree.h>

#include <filesystem>
#include <map>
#include <string>

class Dictionary {
 public:
  constexpr static wchar_t NO_WILDCARD{'\n'};
  Dictionary();
  ~Dictionary();

  void addPath(const std::filesystem::path &, const bool);


  std::multimap<int, std::filesystem::path, std::greater<int>> search(
      std::atomic<bool> &stopSearch,
      const std::wstring &needle_in,
      const size_t num_fuzzy_replacements,
      const wchar_t wildcard,
      const bool searchDirectories,
      const bool searchFiles) const;

  void serialize(const std::string &filename,
                 const std::chrono::steady_clock::time_point &timeOfIndexing) const;
  void deserialize(const std::string &filename,
                   std::chrono::steady_clock::time_point *timeOfIndexing);

  void setMinSearchSize(size_t size) { min_search_size = size; }
  size_t getMinSearchSize() const { return min_search_size; }

  void visualize() const;

  static int scoreChars(wchar_t a, wchar_t b);
  static int scoreMatch(const std::wstring &needle, const std::wstring &match);
  static std::vector<int> getMatchScores(const std::wstring &needle,
                                         const std::wstring &match);

 private:
  std::unique_ptr<Tree> tree;
  size_t min_search_size = 3;
};
