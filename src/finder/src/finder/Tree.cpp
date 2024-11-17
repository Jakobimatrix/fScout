#include <finder/Tree.h>

#include <cctype>
#include <memory>
#include <string>

Tree::Tree() : _root(std::make_unique<TreeNode>(0)) {}


void Tree::insertWord(const std::string &word, const std::filesystem::path &path) {
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
  nodePtr->paths.push_back(path);
}

void Tree::traverse(const TreeNode *rootSubT,
                    std::vector<std::filesystem::path> &pathList) const {
  if (rootSubT->isLeaf()) {
    pathList.insert(
        std::end(pathList), std::cbegin(rootSubT->paths), std::cend(rootSubT->paths));
  }

  if (!rootSubT->_children.empty()) {
    for (const auto &[letter, tnPtr] : rootSubT->_children) {
      traverse(tnPtr, pathList);
    }
  }
}

void Tree::searchHelper(const TreeNode *nodePtr,
                        const std::string &needle,
                        std::vector<std::filesystem::path> &result) const {
  if (needle.empty()) {
    // Base case: we’ve processed all prefix characters, traverse the remaining tree
    traverse(nodePtr, result);
    return;
  }

  // if needle is "picture"
  // than search on this tree branch for p
  // and give all the "p" children the needle "icture"
  char letter = needle[0];
  std::string remainingNeedle = needle.substr(1);

  if (useWildCard && wildcard == letter) {
    for (const auto &child : nodePtr->_children) {
      searchHelper(child.second, remainingNeedle, result);
    }
  } else {
    auto it = nodePtr->_children.find(letter);
    if (it != nodePtr->_children.end()) {
      searchHelper(it->second, remainingNeedle, result);
    }
  }

  // if needle is "picture"
  // and this branch only offers "greatpicture" than the above failes
  // so search all children with the complete needle
  // this is ok, because if needle "pictures" never gets reduced to an empty needle (see base case) results wont be written.
  // this only makes sense if the depth is big enough
  if (nodePtr->_depth < needle.size()) {
    return;
  }
  for (auto it = nodePtr->_children.begin(); it != nodePtr->_children.end(); ++it) {
    if (it->second->_depth < needle.size() - 1) {
      continue;
    }
    searchHelper(it->second, needle, result);
  }
}

std::vector<std::filesystem::path> Tree::search(const std::string &needle) const {
  std::vector<std::filesystem::path> result;
  const TreeNode *nodePtr = _root.get();
  searchHelper(nodePtr, needle, result);
  return result;
}

void Tree::setWildCard(const char wildCard, bool useWildcard) {
  this->wildcard = wildCard;
  this->useWildCard = useWildcard;
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
