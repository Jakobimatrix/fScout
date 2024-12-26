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

void Tree::searchHelper(const TreeNode *nodePtr, SearchVariables &vars) const {
  if (vars.stopSearch.load()) {
    return;
  }

  if (vars.dontVisitAgain.contains(nodePtr)) {
    return;
  }

  if (vars.needle.found()) {
    // Base case: we’ve processed all prefix characters, traverse the remaining tree
    vars.dontVisitAgain.insert(nodePtr);  // actually we can go back further to the next branch, but this adds more complexity and the time benefit might be small
    traverse(nodePtr, vars.result);
    return;
  }

  // if the needle is longer than the remaining depth, we wont finde anything.
  if (nodePtr->getMaxWordLength() < vars.needle.getMinNecessaryDepth()) {
    return;
  }

  // if needle is "picture"
  // and this branch only offers "greatpicture" than the below methods faile.
  // so search all children with the complete needle
  // this is ok, because if needle "pictures" never gets reduced to an empty needle (see base case) results wont be written.
  // this only makes sense if the depth is big enough
  if (vars.needle.notIncremented()) {
    for (auto it = nodePtr->_children.begin(); it != nodePtr->_children.end(); ++it) {
      if (it->second->getMaxWordLength() < vars.needle.getMinNecessaryDepth()) {
        continue;
      }
      searchHelper(it->second, vars);
    }
  }


  // if we have a wild card, always use that
  if (vars.needle.nextIsWildCard()) {
    vars.needle.nextIndex();
    for (const auto &child : nodePtr->_children) {
      if (child.second->getMaxWordLength() < vars.needle.getMinNecessaryDepth()) {
        continue;
      }
      searchHelper(child.second, vars);
    }
    vars.needle.undo_nextIndex();
    return;
  }

  // if needle is "picture"
  // than search on this tree branch for p
  // and give the "p" child the needle "icture"
  char letter = vars.needle.getCurrentLetter();

  auto it = nodePtr->_children.find(letter);
  if (it != nodePtr->_children.end()) {
    vars.needle.nextIndex();
    searchHelper(it->second, vars);
    vars.needle.undo_nextIndex();
    return;
  }

  // needle not found, lets see, if we still have a fuzzy search left

  if (!vars.needle.canDoFuzzySearch()) {
    return;
  }
  vars.needle.useFuzzySeaerch();
  // this is equal to wild card
  vars.needle.nextIndex();
  for (const auto &child : nodePtr->_children) {
    if (child.second->getMaxWordLength() < vars.needle.getMinNecessaryDepth()) {
      continue;
    }
    searchHelper(child.second, vars);
  }

  // remove the current letter to the search string by incrementing the index
  // and returning to the current node
  searchHelper(nodePtr, vars);
  vars.needle.undo_nextIndex();

  // add any possible letter to the search string by not incrementing the index
  for (const auto &child : nodePtr->_children) {
    if (child.second->getMaxWordLength() < vars.needle.getMinNecessaryDepth()) {
      continue;
    }
    searchHelper(child.second, vars);
  }
  vars.needle.undo_useFuzzySeaerch();
}

void Tree::search(Needle needle,
                  std::atomic<bool> &stopSearch,
                  std::vector<TreeNode::PathInfo> &matches) const {
  const TreeNode *nodePtr = _root.get();
  SearchVariables vars(needle, matches, stopSearch);
  searchHelper(nodePtr, vars);
}

size_t Tree::getMaxEntryLength() const { return _root->_depth; }

void Tree::serialize(std::wofstream &outFile) const {
  _root->serialize(outFile);
}

void Tree::deserialize(std::wifstream &inFile) {
  _root = std::unique_ptr<TreeNode>(TreeNode::deserialize(inFile));
}

void Tree::generateDotFile(const std::string &filename) const {
  std::wofstream file(filename);
  file << L"digraph Tree {\n";
  file << L"rankdir=\"LR\";\n";
  file << L"node [shape=circle];\n";
  TreeNode::print2dot(_root.get(), file);
  file << L"}\n";
  file.close();
}
