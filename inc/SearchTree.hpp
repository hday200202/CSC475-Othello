/*
    Name: Harrison Day
    Date: 11/04/25
    Desc: Search tree data structure for use in TreeDisplay.
          Simplifies traversal and includes useful data like
          size and depth.
*/

#pragma once

#include <memory>
#include <vector>
#include <functional>

struct SearchNode {
    int row = -1, col = -1;
    int heuristic = 0;
    int depth = 0;
    bool maximizing = false;
    char turn = ' ';
    int whiteScore = 0;
    int blackScore = 0;
    std::string moveSequence = "";
    std::vector<std::shared_ptr<SearchNode>> children;
};

class SearchTree {
public:
    SearchTree() : m_root(nullptr), m_nodeCount(0) {}
    SearchTree(std::shared_ptr<SearchNode> root) : m_root(root), m_nodeCount(0) {}
    
    std::shared_ptr<SearchNode> getRoot() const { return m_root; }
    
    void setRoot(std::shared_ptr<SearchNode> root) { 
        m_root = root;
    }
    
    void setSize(size_t size) {
        m_nodeCount = size;
    }
    
    size_t getSize() const {
        return m_nodeCount;
    }
    
    void traverse(std::function<void(std::shared_ptr<SearchNode>)> visitor) const {
        if (!m_root) return;
        traverseHelper(m_root, visitor);
    }
    
    std::vector<std::shared_ptr<SearchNode>> getPath(int targetRow, int targetCol) const {
        std::vector<std::shared_ptr<SearchNode>> path;
        if (!m_root) return path;
        findPath(m_root, targetRow, targetCol, path);
        return path;
    }
    
    int getMaxDepth() const {
        if (!m_root) return 0;
        return calculateMaxDepth(m_root, 0);
    }

private:
    std::shared_ptr<SearchNode> m_root;
    size_t m_nodeCount;
    
    size_t countNodes(std::shared_ptr<SearchNode> node) const {
        if (!node) return 0;
        size_t count = 1;
        for (auto& child : node->children)
            count += countNodes(child);
        return count;
    }
    
    void traverseHelper(std::shared_ptr<SearchNode> node, std::function<void(std::shared_ptr<SearchNode>)> visitor) const {
        if (!node) return;
        visitor(node);
        for (auto& child : node->children)
            traverseHelper(child, visitor);
    }
    
    bool findPath(std::shared_ptr<SearchNode> node, int targetRow, int targetCol, std::vector<std::shared_ptr<SearchNode>>& path) const {
        if (!node) return false;
        
        path.push_back(node);
        
        if (node->row == targetRow && node->col == targetCol)
            return true;
        
        for (auto& child : node->children)
            if (findPath(child, targetRow, targetCol, path))
                return true;
        
        path.pop_back();
        return false;
    }
    
    int calculateMaxDepth(std::shared_ptr<SearchNode> node, int currentDepth) const {
        if (!node) return currentDepth - 1;
        
        int maxChildDepth = currentDepth;
        for (auto& child : node->children) {
            int childDepth = calculateMaxDepth(child, currentDepth + 1);
            maxChildDepth = std::max(maxChildDepth, childDepth);
        }
        return maxChildDepth;
    }
};