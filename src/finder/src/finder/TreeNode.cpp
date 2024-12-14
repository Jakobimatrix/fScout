#include <finder/TreeNode.h>

#include <fstream>
#include <iostream>


void TreeNode::serialize(std::ofstream& outFile) const {
  // Serialize leaf status
  outFile.write(reinterpret_cast<const char*>(&_depth), sizeof(_depth));
  bool is_leaf = isLeaf();
  outFile.write(reinterpret_cast<const char*>(&is_leaf), sizeof(is_leaf));

  if (is_leaf) {
    // Serialize the size of the paths vector
    size_t pathsCount = _paths.size();
    outFile.write(reinterpret_cast<const char*>(&pathsCount), sizeof(pathsCount));

    // Serialize each path as a string
    for (const auto& path : _paths) {
      std::string pathString = path.path.string();
      size_t pathLength = pathString.size();

      // Write the length of the string
      outFile.write(reinterpret_cast<const char*>(&pathLength), sizeof(pathLength));

      // Write the string itself
      outFile.write(pathString.c_str(), pathLength);

      const char isDirectory = path.isDirectory ? '1' : '0';
      outFile.write(&isDirectory, sizeof(isDirectory));
    }
  }

  // Serialize number of children
  size_t childrenCount = _children.size();
  outFile.write(reinterpret_cast<const char*>(&childrenCount), sizeof(childrenCount));

  // Serialize each child node
  for (const auto& child : _children) {

    outFile.write(reinterpret_cast<const char*>(&child.first), sizeof(child.first));  // Write the character key
    child.second->serialize(outFile);  // Recursively serialize child node
  }
}

TreeNode* TreeNode::deserialize(std::ifstream& inFile) {
  auto* node = new TreeNode(0);

  // Read leaf status
  inFile.read(reinterpret_cast<char*>(&node->_depth), sizeof(node->_depth));
  bool is_leaf;
  inFile.read(reinterpret_cast<char*>(&is_leaf), sizeof(is_leaf));

  if (is_leaf) {
    // Read the size of the paths vector
    size_t pathsCount;
    inFile.read(reinterpret_cast<char*>(&pathsCount), sizeof(pathsCount));

    // Read each path
    for (size_t i = 0; i < pathsCount; ++i) {
      // Read the length of the path string
      size_t pathLength;
      inFile.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));

      // Read the string itself
      std::string pathString(pathLength, '\0');
      inFile.read(&pathString[0], pathLength);

      char isDirChar;
      inFile.read(&isDirChar, sizeof(isDirChar));
      bool isDir = isDirChar == 1 ? true : false;

      // Convert the Info to a TreeNode::PathInfo and add to the vector
      node->_paths.emplace_back(pathString, isDir);
    }
  }

  // Read number of children
  size_t childrenCount;
  inFile.read(reinterpret_cast<char*>(&childrenCount), sizeof(childrenCount));

  // Read each child node
  for (size_t i = 0; i < childrenCount; ++i) {
    char letter;
    inFile.read(&letter, sizeof(letter));           // Read the character key
    node->_children[letter] = deserialize(inFile);  // Recursively deserialize child node
  }

  return node;
}

bool TreeNode::isLeaf() const { return !_paths.empty(); }

size_t TreeNode::getMaxWordLength() const { return _depth + 1; }


void TreeNode::print(const TreeNode* node, const std::string& prefix, bool is_last) {
  std::string currentPrefix = prefix + (is_last ? "└" : "├");

  if (node->isLeaf()) {
    // Print the paths for leaf nodes
    for (size_t i = 0; i < node->_paths.size(); ++i) {
      std::cout << currentPrefix << node->_paths[i].path.filename().string()
                << " -> " << node->_paths[i].path.string() << "\n";
      if (i != node->_paths.size() - 1) {
        currentPrefix = prefix + (is_last ? " " : "│");  // Align next path entries
      }
    }
  }

  // Iterate over the children
  auto it = node->_children.begin();
  while (it != node->_children.end()) {
    bool lastChild = (std::next(it) == node->_children.end());
    std::cout << prefix << (is_last ? " " : "│") << (lastChild ? "└" : "├")
              << it->first << "\n";
    print(it->second, prefix + (is_last ? " " : "│"), lastChild);
    ++it;
  }
}

void TreeNode::print2dot(const TreeNode* node, std::wofstream& file) {
  static int nodeId = 0;  // Unique ID for each node
  std::wstring currentNodeName = L"N" + std::to_wstring(nodeId);
  if (nodeId == 0) {
    currentNodeName = L"root";
  }
  ++nodeId;

  // Print the current node
  file << currentNodeName << L";\n";

  // For leaf nodes, print their file paths
  /*
  if (node->_isLeaf) {
    for (const auto& path : node->paths) {
      std::string pathNodeName = currentNodeName + "_path_" + std::to_string(nodeId++);
      file << pathNodeName << " [label=\"" << path.string() << "\" URL=\""
           << path.string() << "\", shape=folder];\n";
      file << currentNodeName << " -> " << pathNodeName << ";\n";
    }
  }*/

  // Iterate over the children nodes
  for (const auto& child : node->_children) {
    std::wstring childNodeName = L"N" + std::to_wstring(nodeId);

    file << childNodeName << L" [label=\"" << child.first << L" "
         << child.second->_depth << L"\"];\n";
    file << currentNodeName << L"->" << childNodeName << L" [dir=none];\n";

    print2dot(child.second, file);  // Recursive call for each child
  }
}
