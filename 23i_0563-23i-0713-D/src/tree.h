#ifndef TREE_H
#define TREE_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

// Node in the parse tree
struct TreeNode {
    string label;
    vector<TreeNode*> children;

    TreeNode(const string& lbl) : label(lbl) {}
    ~TreeNode() { for (auto c:children) delete c; }
};

class ParseTree {
public:
    TreeNode* root;

    ParseTree() : root(nullptr) {}
    ~ParseTree() { delete root; root=nullptr; }

    // Print indented ASCII tree
    void print(ostream& out = cout) const;
    void printNode(ostream& out, TreeNode* node, const string& prefix, bool isLast) const;

    // Preorder traversal (for output file)
    void preorder(ostream& out, TreeNode* node, int depth=0) const;

    void clear();
};

#endif
