#include <finder/TreeNode.h>


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
