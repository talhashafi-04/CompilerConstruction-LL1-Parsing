#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
using namespace std;

// Represents a single production rule: lhs -> rhs
struct Production {
    string lhs;
    vector<string> rhs;
};

class Grammar {
public:
    vector<Production> productions;

    // ---- Utility ----
    string trim(string s);
    bool addToSet(vector<string>& set, const string& val);
    int  getNTIndex(const string& s, const vector<string>& nts) const;
    string formatRule(const Production& p) const;

    // ---- Load ----
    bool loadFromFile(const string& filename);

    // ---- Introspection ----
    vector<string> getNonTerminals() const;
    vector<string> getTerminals(const vector<string>& nts) const;
    string getStartSymbol() const;

    // ---- Transformations ----
    void applyLeftFactoring();
    void removeLeftRecursion();
    void removeDirectForSpecificNT(const string& A);

    // ---- Display ----
    void displayGrammar(const string& title) const;
    void displayGrammarToFile(ofstream& out, const string& title) const;
};

#endif
