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

  void searchHelper(const TreeNode *nodePtr,
                    const std::string &prefix,
                    const std::string &currentPrefix,
                    std::vector<std::filesystem::path> &result) const;
  std::vector<std::filesystem::path> search(const std::string &) const;

  void serialize(std::ofstream &outFile) const;
  void deserialize(std::ifstream &inFile);

  void print() const;
  void generateDotFile(const std::string &filename) const;

 private:
  void traverse(const TreeNode *, const std::string &, std::vector<std::filesystem::path> &) const;
};
