#include <finder/Tree.h>

#include <cctype>
#include <memory>
#include <string>

Tree::Tree() : _root(std::make_unique<TreeNode>(0)) {}


void Tree::insertWord(const std::wstring &word,
                      const std::filesystem::path &path,
                      const bool isDirectory) {
  TreeNode *nodePtr = _root.get();
  size_t remaining_depth = word.size();
  nodePtr->_depth = std::max(remaining_depth, nodePtr->_depth);

  for (const char letter : word) {
    --remaining_depth;
    if (!nodePtr->_children.count(letter)) {
      nodePtr->_children[letter] = new TreeNode(remaining_depth);
    }
    nodePtr = nodePtr->_children[letter];
    nodePtr->_depth = std::max(remaining_depth, nodePtr->_depth);
  }
  nodePtr->_paths.emplace_back(path, isDirectory);
}

void Tree::traverse(const TreeNode *rootSubT, std::vector<TreeNode::PathInfo> &pathList) const {
  if (rootSubT->isLeaf()) {
    pathList.insert(
        std::end(pathList), std::cbegin(rootSubT->_paths), std::cend(rootSubT->_paths));
  }

  if (!rootSubT->_children.empty()) {
    for (const auto &[letter, tnPtr] : rootSubT->_children) {
      traverse(tnPtr, pathList);
    }
  }
}

void Tree::searchHelper(const TreeNode *nodePtr,
                        Needle &needle,
                        std::vector<TreeNode::PathInfo> &result) const {
  if (needle.found()) {
    // Base case: we’ve processed all prefix characters, traverse the remaining tree
    traverse(nodePtr, result);
    return;
  }

  // if the needle is longer than the remaining depth, we wont finde anything.
  if (nodePtr->_depth < needle.getMinNecessaryDepth()) {
    return;
  }

  // if needle is "picture"
  // and this branch only offers "greatpicture" than the below methods faile.
  // so search all children with the complete needle
  // this is ok, because if needle "pictures" never gets reduced to an empty needle (see base case) results wont be written.
  // this only makes sense if the depth is big enough
  if (needle.notIncremented()) {
    Needle copy{needle};
    for (auto it = nodePtr->_children.begin(); it != nodePtr->_children.end(); ++it) {
      if (it->second->_depth < needle.getMinNecessaryDepth()) {
        continue;
      }
      searchHelper(it->second, copy, result);
    }
  }


  // if we have a wild card, always use that
  if (needle.nextIsWildCard()) {
    Needle copy{needle};
    copy.nextIndex();
    for (const auto &child : nodePtr->_children) {
      searchHelper(child.second, copy, result);
    }
    return;
  }

  // if needle is "picture"
  // than search on this tree branch for p
  // and give all the "p" children the needle "icture"
  char letter = needle.front();

  auto it = nodePtr->_children.find(letter);
  if (it != nodePtr->_children.end()) {
    needle.nextIndex();
    searchHelper(it->second, needle, result);
    return;
  }

  // needle not found, lets see, if we still have a fuzzy search left

  if (!needle.canDoFuzzySearch()) {
    return;
  }
  Needle copySwapLetter{needle};
  copySwapLetter.useFuzzySeaerch();
  // this is equal to wild card
  copySwapLetter.nextIndex();
  for (const auto &child : nodePtr->_children) {
    searchHelper(child.second, copySwapLetter, result);
  }

  Needle copyaddLetter{needle};
  copyaddLetter.useFuzzySeaerch();
  // add any possible letter to the search string by not incrementing the index
  for (const auto &child : nodePtr->_children) {
    searchHelper(child.second, copyaddLetter, result);
  }

  Needle copyremoveLetter{needle};
  copyremoveLetter.useFuzzySeaerch();
  // remove the current letter to the search string by incrementing the index
  // and returning to the current node
  copyremoveLetter.nextIndex();
  searchHelper(nodePtr, copyaddLetter, result);
}

std::vector<TreeNode::PathInfo> Tree::search(Needle needle) const {
  std::vector<TreeNode::PathInfo> result;
  const TreeNode *nodePtr = _root.get();
  searchHelper(nodePtr, needle, result);
  return result;
}

size_t Tree::getMaxEntryLength() const { return _root->_depth; }

void Tree::serialize(std::ofstream &outFile) const {
  _root->serialize(outFile);
}

void Tree::deserialize(std::ifstream &inFile) {
  _root = std::unique_ptr<TreeNode>(TreeNode::deserialize(inFile));
}

void Tree::print() const { TreeNode::print(_root.get(), "", true); }

void Tree::generateDotFile(const std::string &filename) const {
  std::ofstream file(filename);
  file << "digraph Tree {\n";
  file << "rankdir=\"LR\";\n";
  file << "node [shape=circle];\n";
  TreeNode::print2dot(_root.get(), file);
  file << "}\n";
  file.close();
}
