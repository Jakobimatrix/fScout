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

void Tree::findPrefixMatchesHelper(const TreeNode *nodePtr,
                                   const std::string &prefix,
                                   const std::string &currentPrefix,
                                   std::vector<std::filesystem::path> &result,
                                   bool caseInsensitive) const {
  if (prefix.empty()) {
    // Base case: we’ve processed all prefix characters, traverse the remaining tree
    traverse(nodePtr, currentPrefix, result);
    return;
  }

  char letter = prefix[0];
  std::string remainingPrefix = prefix.substr(1);

  // Handle case-insensitivity by checking both upper and lower case possibilities
  if (caseInsensitive) {
    char lowerLetter = std::tolower(letter);
    char upperLetter = std::toupper(letter);

    // Try with lowercase variant
    auto itLower = nodePtr->_children.find(lowerLetter);
    if (itLower != nodePtr->_children.end()) {
      findPrefixMatchesHelper(
          itLower->second, remainingPrefix, currentPrefix + lowerLetter, result, true);
    }
    if (lowerLetter == upperLetter) {
      return;
    }

    // Try with uppercase variant
    auto itUpper = nodePtr->_children.find(upperLetter);
    if (itUpper != nodePtr->_children.end() && upperLetter != lowerLetter) {
      findPrefixMatchesHelper(
          itUpper->second, remainingPrefix, currentPrefix + upperLetter, result, true);
    }
  } else {
    // Case-sensitive search, use the exact letter
    auto it = nodePtr->_children.find(letter);
    if (it != nodePtr->_children.end()) {
      findPrefixMatchesHelper(
          it->second, remainingPrefix, currentPrefix + letter, result, false);
    }
  }
}

std::vector<std::filesystem::path> Tree::findPrefixMatches(const std::string &prefix,
                                                           bool caseInsensitive) const {
  std::vector<std::filesystem::path> result;
  const TreeNode *nodePtr = _root.get();

  findPrefixMatchesHelper(nodePtr, prefix, "", result, caseInsensitive);
  return result;
}

void Tree::serialize(std::ofstream &outFile) const {
  _root->serialize(outFile);
}

void Tree::deserialize(std::ifstream &inFile) {
  _root = std::unique_ptr<TreeNode>(TreeNode::deserialize(inFile));
}
