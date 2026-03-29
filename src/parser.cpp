#include "parser.h"

// ─────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────

Parser::Parser(Grammar& grammar, FirstFollow& firstFollow)
    : g(grammar), ff(firstFollow), isLL1(true) {
    table.assign(ff.nts.size(), vector<int>(ff.terms.size(), -1));
    conflictTable.assign(ff.nts.size(), vector<vector<int>>(ff.terms.size(), vector<int>()));
}

// ─────────────────────────────────────────
//  Build Parsing Table
// ─────────────────────────────────────────

void Parser::buildTable() {
    for (int i=0;i<(int)g.productions.size();i++) {
        auto& prod = g.productions[i];
        int r = g.getNTIndex(prod.lhs, ff.nts);
        if (r==-1) continue;

        // Compute FIRST(rhs)
        vector<string> firstRhs = ff.firstOfSequence(prod.rhs);

        // For each terminal in FIRST(rhs) \ {epsilon}
        for (auto& sym : firstRhs) {
            if (sym=="epsilon") continue;
            int c = g.getNTIndex(sym, ff.terms);
            if (c==-1) continue;
            conflictTable[r][c].push_back(i);
            if (conflictTable[r][c].size()>1) isLL1=false;
            table[r][c] = conflictTable[r][c][0]; // keep first for parsing
        }

        // If epsilon in FIRST(rhs), use FOLLOW(lhs)
        bool hasEps=false;
        for (auto& f:firstRhs) if(f=="epsilon"){hasEps=true;break;}
        if (hasEps) {
            for (auto& sym : ff.followSets[r]) {
                int c = g.getNTIndex(sym, ff.terms);
                if (c==-1) continue;
                conflictTable[r][c].push_back(i);
                if (conflictTable[r][c].size()>1) isLL1=false;
                table[r][c] = conflictTable[r][c][0];
            }
        }
    }
}

// ─────────────────────────────────────────
//  Display Table
// ─────────────────────────────────────────

void Parser::displayTable(ostream& out) const {
    out << "\n╔══════════════════════════════════════════╗\n";
    out << "║           LL(1) PARSING TABLE            ║\n";
    out << "╚══════════════════════════════════════════╝\n";

    if (!isLL1) {
        out << "  *** WARNING: Grammar is NOT LL(1)! Conflicts detected. ***\n";
        // Show conflicts
        for (int r=0;r<(int)ff.nts.size();r++)
            for (int c=0;c<(int)ff.terms.size();c++)
                if (conflictTable[r][c].size()>1) {
                    out << "  CONFLICT at M[" << ff.nts[r] << ", " << ff.terms[c] << "]: ";
                    for (int idx : conflictTable[r][c])
                        out << "[" << g.formatRule(g.productions[idx]) << "] ";
                    out << "\n";
                }
    } else {
        out << "  Grammar IS LL(1) - no conflicts detected.\n";
    }

    int colW = 18;
    int labelW = 20;

    // Header row
    out << "\n" << string(labelW,' ');
    for (auto& t : ff.terms) out << left << setw(colW) << t;
    out << "\n";
    out << string(labelW + colW*(int)ff.terms.size(), '-') << "\n";

    // Data rows
    for (int r=0;r<(int)ff.nts.size();r++) {
        out << left << setw(labelW) << ff.nts[r];
        for (int c=0;c<(int)ff.terms.size();c++) {
            if (table[r][c]==-1) {
                out << left << setw(colW) << "---";
            } else {
                string rule = g.formatRule(g.productions[table[r][c]]);
                if (rule.size()>=(size_t)colW) rule=rule.substr(0,colW-3)+"...";
                out << left << setw(colW) << rule;
            }
        }
        out << "\n";
    }
}

void Parser::displayTableToFile(ofstream& out) const {
    displayTable(out);
}

// ─────────────────────────────────────────
//  Tokenizer
// ─────────────────────────────────────────

vector<string> Parser::tokenize(const string& input) const {
    vector<string> tokens;
    int i=0;
    while (i<(int)input.size()) {
        if (isspace(input[i])){i++;continue;}
        if (isalpha(input[i])) {
            string word;
            while (i<(int)input.size()&&(isalnum(input[i])||input[i]=='_'))
                word+=input[i++];
            tokens.push_back(word);
        } else {
            tokens.push_back(string(1,input[i++]));
        }
    }
    tokens.push_back("$");
    return tokens;
}

// ─────────────────────────────────────────
//  Panic-mode error recovery
// ─────────────────────────────────────────

void Parser::panicRecover(Stack& stk, vector<string>& tokens, int& idx,
                           const string& topNT, int /*lineNum*/, int /*col*/,
                           ostream& traceOut) {
    int ntIdx = g.getNTIndex(topNT, ff.nts);
    // Synchronising set = FOLLOW(topNT)
    vector<string> syncSet;
    if (ntIdx!=-1) syncSet = ff.followSets[ntIdx];
    syncSet.push_back("$");

    traceOut << "    [RECOVERY] Sync set for " << topNT << ": { ";
    for (auto& s:syncSet) { traceOut<<s<<" "; }
    traceOut<<"}\n";

    // Pop stack until we find something useful OR stack hits $
    while (!stk.isEmpty() && stk.top()!="$") {
        string t = stk.top();
        int tNT = g.getNTIndex(t, ff.nts);
        if (tNT!=-1) {
            // Check if current input is in FOLLOW(t)
            string cur = (idx<(int)tokens.size()) ? tokens[idx] : "$";
            bool inFollow=false;
            for (auto& f:ff.followSets[tNT]) if(f==cur){inFollow=true;break;}
            if (inFollow) break;
        }
        traceOut << "    [RECOVERY] Popping '" << t << "' from stack\n";
        stk.pop();
    }

    // Skip input until we hit a sync symbol
    while (idx<(int)tokens.size()) {
        string cur = tokens[idx];
        bool isSync=false;
        for (auto& s:syncSet) if(s==cur){isSync=true;break;}
        if (isSync) break;
        traceOut << "    [RECOVERY] Skipping input token '" << cur << "'\n";
        idx++;
    }
}

// ─────────────────────────────────────────
//  Parse
// ─────────────────────────────────────────

bool Parser::parse(const string& input, int lineNum,
                   ParseTree& tree, ostream& traceOut) {
    eh.reset();
    vector<string> tokens = tokenize(input);

    // Print header
    traceOut << "\n";
    traceOut << string(90,'=') << "\n";
    traceOut << "  Parsing line " << lineNum << ": \"" << input << "\"\n";
    traceOut << string(90,'=') << "\n";
    traceOut << left << setw(4) << "Step"
             << setw(35) << "Stack (bottom->top)"
             << setw(30) << "Input Remaining"
             << "Action\n";
    traceOut << string(90,'-') << "\n";

    Stack stk;
    stk.push("$");
    if (!ff.nts.empty()) stk.push(ff.nts[0]);

    // Build parse tree root
    tree.clear();
    if (!ff.nts.empty()) {
        tree.root = new TreeNode(ff.nts[0]);
    }
    // Stack of tree nodes parallel to parse stack (excluding $)
    vector<TreeNode*> nodeStack;
    if (tree.root) nodeStack.push_back(tree.root);

    int idx=0, step=0;
    bool accepted=false, fatalError=false;

    while (!stk.isEmpty() && !fatalError) {
        string top = stk.top();
        string curr = (idx<(int)tokens.size()) ? tokens[idx] : "$";

        // Build display strings
        string stackStr = stk.toString();
        string inputStr;
        for (int k=idx;k<(int)tokens.size();k++) inputStr+=tokens[k]+" ";

        if (stackStr.size()>33) stackStr=stackStr.substr(stackStr.size()-33);
        if (inputStr.size()>28) inputStr=inputStr.substr(0,28)+"...";

        step++;
        traceOut << left << setw(4) << step
                 << setw(35) << stackStr
                 << setw(30) << inputStr;

        // ── Case 1: both are $  ──
        if (top=="$" && curr=="$") {
            traceOut << "Accept\n";
            accepted=true;
            break;
        }

        // ── Case 2: top == curr (terminal match) ──
        if (top==curr && top!="$") {
            traceOut << "Match '" << top << "'\n";
            stk.pop();
            if (!nodeStack.empty()) nodeStack.pop_back();
            idx++;
            continue;
        }

        // ── Case 3: top is a non-terminal ──
        int r = g.getNTIndex(top, ff.nts);
        if (r!=-1) {
            int c = g.getNTIndex(curr, ff.terms);
            if (c==-1) {
                // Unknown terminal
                traceOut << "\n";
                eh.reportError(ErrorType::UNEXPECTED_SYMBOL, lineNum, idx+1,
                               "<valid token>", curr, traceOut);
                // Skip the unknown token and continue
                idx++;
                continue;
            }
            if (table[r][c]==-1) {
                // Empty cell → error recovery
                traceOut << "\n";
                eh.reportError(ErrorType::EMPTY_TABLE_ENTRY, lineNum, idx+1,
                               top, curr, traceOut);

                int stackSizeBefore = stk.size();
                int idxBefore = idx;
                panicRecover(stk, tokens, idx, top, lineNum, idx+1, traceOut);

                // If recovery made zero progress, force-pop to guarantee termination
                if (stk.size() == stackSizeBefore && idx == idxBefore) {
                    traceOut << "    [RECOVERY] Force-popping '" << stk.top() << "' to avoid loop\n";
                    stk.pop();
                    if (!nodeStack.empty()) nodeStack.pop_back();
                }

                // If only $ left or empty, done
                if (stk.isEmpty() || stk.top()=="$") {
                    fatalError = true;
                    break;
                }
                // Resync nodeStack
                while ((int)nodeStack.size() > (stk.size()-1))
                    nodeStack.pop_back();
                continue;
            }

            // Valid entry: expand
            int prodIdx = table[r][c];
            Production& prod = g.productions[prodIdx];
            traceOut << "Predict: " << g.formatRule(prod) << "\n";

            stk.pop();
            TreeNode* curNode = nodeStack.empty() ? nullptr : nodeStack.back();
            if (!nodeStack.empty()) nodeStack.pop_back();

            // Add children to tree node
            if (curNode) {
                if (!(prod.rhs.size()==1&&prod.rhs[0]=="epsilon")) {
                    for (auto& sym : prod.rhs)
                        curNode->children.push_back(new TreeNode(sym));
                } else {
                    curNode->children.push_back(new TreeNode("ε"));
                }
            }

            // Push RHS in reverse order onto stack and nodeStack
            if (!(prod.rhs.size()==1 && prod.rhs[0]=="epsilon")) {
                for (int k=(int)prod.rhs.size()-1;k>=0;k--) {
                    stk.push(prod.rhs[k]);
                    if (curNode && k<(int)curNode->children.size())
                        nodeStack.push_back(curNode->children[k]);
                }
            }
            continue;
        }

        // ── Case 4: terminal mismatch ──
        if (curr=="$") {
            traceOut << "\n";
            eh.reportError(ErrorType::PREMATURE_END, lineNum, idx+1, top, curr, traceOut);
            fatalError=true;
        } else {
            traceOut << "\n";
            eh.reportError(ErrorType::MISSING_SYMBOL, lineNum, idx+1, top, curr, traceOut);
            // Skip unexpected token (error recovery: advance input)
            idx++;
            if (!nodeStack.empty()) nodeStack.pop_back();
            stk.pop();
        }
    }

    traceOut << string(90,'-') << "\n";
    if (accepted && !eh.hasErrors()) {
        traceOut << "  Result: *** STRING ACCEPTED ***\n";
    } else if (accepted || (!fatalError && !eh.hasErrors())) {
        traceOut << "  Result: STRING ACCEPTED (with " << eh.errorCount << " error(s) recovered)\n";
    } else {
        traceOut << "  Result: *** STRING REJECTED *** (" << eh.errorCount << " error(s))\n";
    }
    traceOut << string(90,'=') << "\n";

    return accepted && !eh.hasErrors();
}
