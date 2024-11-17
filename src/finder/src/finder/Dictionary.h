#pragma once

#include <finder/SearchPattern.h>
#include <finder/Tree.h>

#include <filesystem>
#include <map>
#include <string>

class Dictionary {
 public:
  Dictionary();
  ~Dictionary();

  void addPath(const std::filesystem::path &);


  std::multimap<int, std::filesystem::path, std::greater<int>> search(
      const std::string &needle, const std::shared_ptr<SearchPattern> &pattern) const;

  void serialize(const std::string &filename,
                 const std::chrono::steady_clock::time_point &timeOfIndexing) const;
  void deserialize(const std::string &filename,
                   std::chrono::steady_clock::time_point *timeOfIndexing);

  void setMinSearchSize(size_t size) { min_search_size = size; }
  size_t getMinSearchSize() const { return min_search_size; }

  void visualize() const;

  void setWildCard(const char wildCard, bool useWildcard);

  static int scoreChars(char a, char b);
  static int scoreMatch(const std::string &needle, const std::string &match);
  static std::vector<int> getMatchScores(const std::string &needle, const std::string &match);

 private:
  std::unique_ptr<Tree> tree;
  size_t min_search_size = 3;
};
