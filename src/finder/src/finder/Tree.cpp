#include <finder/Tree.h>

#include <cctype>
#include <memory>
#include <string>

Tree::Tree() : _root(std::make_unique<TreeNode>()) {}


void Tree::insertWord(const std::string &word, const std::filesystem::path &path) {
  TreeNode *nodePtr = _root.get();

  for (const char letter : word) {
    if (!nodePtr->_children.count(letter)) {
      nodePtr->_children[letter] = new TreeNode();
    }

    nodePtr = nodePtr->_children[letter];
  }
  nodePtr->_isLeaf = true;
  nodePtr->paths.push_back(path);
}

void Tree::traverse(const TreeNode *rootSubT,
                    const std::string &word,
                    std::vector<std::filesystem::path> &pathList) const {
  if (rootSubT->_isLeaf) {
    pathList.insert(
        std::end(pathList), std::cbegin(rootSubT->paths), std::cend(rootSubT->paths));
  }

  if (!rootSubT->_children.empty()) {
    for (const auto &[letter, tnPtr] : rootSubT->_children) {
      const std::string temp = word + letter;
      traverse(tnPtr, temp, pathList);
    }
  }
}

void Tree::searchHelper(const TreeNode *nodePtr,
                        const std::string &needle,
                        const std::string &currentPrefix,
                        std::vector<std::filesystem::path> &result) const {
  if (needle.empty()) {
    // Base case: we’ve processed all prefix characters, traverse the remaining tree
    traverse(nodePtr, currentPrefix, result);
    return;
  }

  char letter = needle[0];
  std::string remainingPrefix = needle.substr(1);

  auto it = nodePtr->_children.find(letter);
  if (it != nodePtr->_children.end()) {
    searchHelper(it->second, remainingPrefix, currentPrefix + letter, result);
  }
}

std::vector<std::filesystem::path> Tree::search(const std::string &needle) const {
  std::vector<std::filesystem::path> result;
  const TreeNode *nodePtr = _root.get();
  searchHelper(nodePtr, needle, "", result);
  return result;
}

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
