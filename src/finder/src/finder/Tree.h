#pragma once

#include <finder/Needle.h>
#include <finder/TreeNode.h>

#include <atomic>
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

  void searchHelper(const TreeNode *nodePtr,
                    Needle &,
                    std::vector<TreeNode::PathInfo> &result,
                    std::atomic<bool> &stopSearch) const;

  void search(Needle, std::atomic<bool> &, std::vector<TreeNode::PathInfo> &matches) const;

  void serialize(std::wofstream &outFile) const;
  void deserialize(std::wifstream &inFile);

  void print() const;
  void generateDotFile(const std::string &filename) const;

 private:
  void traverse(const TreeNode *, std::vector<TreeNode::PathInfo> &) const;
};
