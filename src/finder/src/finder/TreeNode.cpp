#include <finder/TreeNode.h>

#include <fstream>
#include <iostream>


void TreeNode::serialize(std::ofstream& outFile) const {
  // Serialize leaf status
  outFile.write(reinterpret_cast<const char*>(&_isLeaf), sizeof(_isLeaf));

  if (_isLeaf) {
    // Serialize the size of the paths vector
    size_t pathsCount = paths.size();
    outFile.write(reinterpret_cast<const char*>(&pathsCount), sizeof(pathsCount));

    // Serialize each path as a string
    for (const auto& path : paths) {
      std::string pathString = path.string();
      size_t pathLength = pathString.size();

      // Write the length of the string
      outFile.write(reinterpret_cast<const char*>(&pathLength), sizeof(pathLength));

      // Write the string itself
      outFile.write(pathString.c_str(), pathLength);
    }
  }

  // Serialize number of children
  size_t childrenCount = _children.size();
  outFile.write(reinterpret_cast<const char*>(&childrenCount), sizeof(childrenCount));

  // Serialize each child node
  for (const auto& [letter, childNode] : _children) {
    outFile.write(&letter, sizeof(letter));  // Write the character key
    childNode->serialize(outFile);           // Recursively serialize child node
  }
}

TreeNode* TreeNode::deserialize(std::ifstream& inFile) {
  auto* node = new TreeNode();

  // Read leaf status
  inFile.read(reinterpret_cast<char*>(&node->_isLeaf), sizeof(node->_isLeaf));

  if (node->_isLeaf) {
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

      // Convert the string to a filesystem::path and add to the vector
      node->paths.emplace_back(pathString);
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


void TreeNode::print(const TreeNode* node, const std::string& prefix, bool is_last) {
  std::string currentPrefix = prefix + (is_last ? "└" : "├");

  if (node->_isLeaf) {
    // Print the paths for leaf nodes
    for (size_t i = 0; i < node->paths.size(); ++i) {
      std::cout << currentPrefix << node->paths[i].filename().string() << " -> "
                << node->paths[i].string() << "\n";
      if (i != node->paths.size() - 1) {
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

void TreeNode::print2dot(const TreeNode* node, std::ofstream& file) {
  static int nodeId = 0;  // Unique ID for each node
  std::string currentNodeName = "N" + std::to_string(nodeId);
  if (nodeId == 0) {
    currentNodeName = "root";
  }
  ++nodeId;

  // Print the current node
  file << currentNodeName << ";\n";

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
    std::string childNodeName = "N" + std::to_string(nodeId);

    file << childNodeName << " [label=\"" << child.first << "\"];\n";
    file << currentNodeName << "->" << childNodeName << " [dir=none];\n";

    print2dot(child.second, file);  // Recursive call for each child
  }
}
