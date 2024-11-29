#pragma once

#include <finder/Needle.h>
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

  size_t getMaxEntryLength() const;

  void insertWord(const std::wstring &, const std::filesystem::path &, const bool);

  void searchHelper(const TreeNode *nodePtr, Needle &, std::vector<TreeNode::PathInfo> &result) const;

  std::vector<TreeNode::PathInfo> search(Needle) const;

  void serialize(std::ofstream &outFile) const;
  void deserialize(std::ifstream &inFile);

  void print() const;
  void generateDotFile(const std::string &filename) const;

 private:
  void traverse(const TreeNode *, std::vector<TreeNode::PathInfo> &) const;
};
