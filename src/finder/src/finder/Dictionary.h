#pragma once

#include <finder/SearchPattern.h>
#include <finder/Tree.h>

#include <filesystem>
#include <string>

class Dictionary {
 public:
  Dictionary();
  ~Dictionary();

  void addPath(const std::filesystem::path &);


  std::vector<std::filesystem::path> search(
      const std::string &needle,
      const std::vector<std::unique_ptr<SearchPattern>> &patterns) const;

  void serialize(const std::string &filename,
                 const std::chrono::steady_clock::time_point &timeOfIndexing) const;
  void deserialize(const std::string &filename,
                   std::chrono::steady_clock::time_point *timeOfIndexing);

  void setMinSearchSize(size_t size) { min_search_size = size; }
  size_t getMinSearchSize() const { return min_search_size; }

  void visualize() const;

 private:
  std::unique_ptr<Tree> tree;
  size_t min_search_size = 3;

  int scoreMatch(const std::string &needle, const std::string &match) const;
};
