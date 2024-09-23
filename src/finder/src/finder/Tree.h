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

 public:
  Tree();
  Tree(const Tree &) = delete;
  ~Tree() = default;

  void insertWord(const std::string &, const std::filesystem::path &);

  void findPrefixMatchesHelper(const TreeNode *nodePtr,
                               const std::string &prefix,
                               std::string currentPrefix,
                               std::vector<std::filesystem::path> &result,
                               bool caseInsensitive) const;
  std::vector<std::filesystem::path> findPrefixMatches(const std::string &,
                                                       const bool caseInsensitive) const;

  void serialize(std::ofstream &outFile) const;
  void deserialize(std::ifstream &inFile);

 private:
  void traverse(const TreeNode *, const std::string &, std::vector<std::filesystem::path> &) const;
};
