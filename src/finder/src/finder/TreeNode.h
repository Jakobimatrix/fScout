#pragma once

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>

struct TreeNode {
  TreeNode() : _isLeaf(false) {}
  TreeNode(const TreeNode&) = delete;

  ~TreeNode() {
    for (auto& item : _children) {
      if (item.second)
        delete item.second;
    }
  }

  bool _isLeaf;
  std::unordered_map<char, TreeNode*> _children;
  std::vector<std::filesystem::path> paths;

  void serialize(std::ofstream& outFile) const;
  static TreeNode* deserialize(std::ifstream& inFile);

  static void print(const TreeNode* node, const std::string& prefix, bool is_last);

  static void print2dot(const TreeNode* node, std::ofstream& file);
};
