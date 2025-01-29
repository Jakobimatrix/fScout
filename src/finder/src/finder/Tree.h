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
#include <unordered_set>
#include <vector>



class Tree {
  std::unique_ptr<TreeNode> _root;

  struct SearchVariables {
    Needle &needle;
    std::vector<std::pair<std::wstring, const std::vector<TreeNode::PathInfo> *>> &result;
    std::atomic<bool> &stopSearch;
    std::unordered_set<const TreeNode *> dontVisitAgain;

    // Constructor
    SearchVariables(Needle &needle_,
                    std::vector<std::pair<std::wstring, const std::vector<TreeNode::PathInfo> *>> &result_,
                    std::atomic<bool> &stopSearch_)
        : needle(needle_), result(result_), stopSearch(stopSearch_), dontVisitAgain() {}
  };

 public:
  Tree();

  Tree(const Tree &) = delete;
  ~Tree() = default;

  size_t getMaxEntryLength() const;

  void insertWord(const std::wstring &, const std::filesystem::path &, const bool);

  void searchHelper(const TreeNode *nodePtr, SearchVariables &) const;

  void search(Needle &,
              std::atomic<bool> &,
              std::vector<std::pair<std::wstring, const std::vector<TreeNode::PathInfo> *>> &matches) const;

  void serialize(std::wofstream &outFile) const;
  void deserialize(std::wifstream &inFile);

  void generateDotFile(const std::string &filename) const;

 private:
  void traverse(const TreeNode *, SearchVariables &) const;
};
