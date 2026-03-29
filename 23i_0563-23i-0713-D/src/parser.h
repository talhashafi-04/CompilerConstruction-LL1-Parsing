#ifndef PARSER_H
#define PARSER_H

#include "grammar.h"
#include "first_follow.h"
#include "stack.h"
#include "tree.h"
#include "error_handler.h"
#include <iomanip>

class Parser {
public:
    Grammar&     g;
    FirstFollow& ff;
    ErrorHandler eh;

    // Parsing table: table[ntIdx][termIdx] = production index, or -1 if empty
    // conflictTable: if a cell has multiple entries it's not LL(1)
    vector<vector<int>>              table;
    vector<vector<vector<int>>>      conflictTable; // all production indices per cell
    bool isLL1;

    Parser(Grammar& grammar, FirstFollow& firstFollow);

    // Build the LL(1) parsing table
    void buildTable();
    void displayTable(ostream& out = cout) const;
    void displayTableToFile(ofstream& out) const;

    // Tokenize an input string into terminals
    vector<string> tokenize(const string& input) const;

    // Parse one input string (returns true if accepted)
    // Builds the parse tree into 'tree'. Writes trace to 'traceOut'.
    bool parse(const string& input, int lineNum,
               ParseTree& tree,
               ostream& traceOut = cout);

private:
    // Panic-mode error recovery: skip input/stack until sync symbol found
    void panicRecover(Stack& stk, vector<string>& tokens, int& idx,
                      const string& topNT, int lineNum, int col,
                      ostream& traceOut);
};

#endif
