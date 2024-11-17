#pragma once

#include <finder/TreeNode.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>



class Tree {
  std::unique_ptr<TreeNode> _root;
  char wildcard = '*';
  bool useWildCard = false;

 public:
  Tree();
  Tree(const Tree &) = delete;
  ~Tree() = default;

  size_t getMaxEntryLength() const;

  void insertWord(const std::string &, const std::filesystem::path &);

  void searchHelper(const TreeNode *nodePtr,
                    const std::string &needle,
                    std::vector<std::filesystem::path> &result) const;
  std::vector<std::filesystem::path> search(const std::string &) const;
  void setWildCard(const char wildCard, bool useWildcard);

  void serialize(std::ofstream &outFile) const;
  void deserialize(std::ifstream &inFile);

  void print() const;
  void generateDotFile(const std::string &filename) const;

 private:
  void traverse(const TreeNode *, std::vector<std::filesystem::path> &) const;
};
