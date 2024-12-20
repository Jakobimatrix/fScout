#pragma once

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>

struct TreeNode {
  TreeNode(size_t depth) : _depth(depth) {}
  TreeNode(const TreeNode&) = delete;

  ~TreeNode() {
    for (auto& item : _children) {
      if (item.second)
        delete item.second;
    }
  }

  size_t _depth;
  std::unordered_map<wchar_t, TreeNode*> _children;
  struct PathInfo {
    PathInfo(const std::filesystem::path& path, const bool isDir)
        : path(path), isDirectory(isDir) {}
    std::filesystem::path path;
    bool isDirectory;
  };

  std::vector<PathInfo> _paths;

  size_t getMaxWordLength() const;

  bool isLeaf() const;

  void serialize(std::wofstream& outFile) const;
  static TreeNode* deserialize(std::wifstream& inFile);

  static void print2dot(const TreeNode* node, std::wofstream& file);
};
