#include "tree.h"

void ParseTree::clear() {
    delete root;
    root = nullptr;
}

void ParseTree::printNode(ostream& out, TreeNode* node, const string& prefix, bool isLast) const {
    if (!node) return;
    out << prefix;
    out << (isLast ? "└── " : "├── ");
    out << node->label << "\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    for (int i=0;i<(int)node->children.size();i++)
        printNode(out, node->children[i], childPrefix, i==(int)node->children.size()-1);
}

void ParseTree::print(ostream& out) const {
    if (!root) { out << "(empty tree)\n"; return; }
    out << root->label << "\n";
    for (int i=0;i<(int)root->children.size();i++)
        printNode(out, root->children[i], "", i==(int)root->children.size()-1);
}

void ParseTree::preorder(ostream& out, TreeNode* node, int depth) const {
    if (!node) return;
    for (int i=0;i<depth;i++) out << "  ";
    out << node->label << "\n";
    for (auto c : node->children) preorder(out, c, depth+1);
}
