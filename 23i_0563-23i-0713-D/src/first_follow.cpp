#include "first_follow.h"
#include <iomanip>

FirstFollow::FirstFollow(Grammar& grammar) : g(grammar) {
    nts   = g.getNonTerminals();
    terms = g.getTerminals(nts);
    firstSets.assign(nts.size(), {});
    followSets.assign(nts.size(), {});
}

// ─────────────────────────────────────────
//  FIRST
// ─────────────────────────────────────────

void FirstFollow::computeFirst() {
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& prod : g.productions) {
            int lIdx = g.getNTIndex(prod.lhs, nts);
            if (lIdx==-1) continue;

            const vector<string>& rhs = prod.rhs;

            // Special case: A -> epsilon
            if (rhs.size()==1 && rhs[0]=="epsilon") {
                if (g.addToSet(firstSets[lIdx],"epsilon")) changed=true;
                continue;
            }

            // Walk through RHS symbols
            bool allCanBeEpsilon = true;
            for (int j=0;j<(int)rhs.size();j++) {
                string sym = rhs[j];
                int sIdx = g.getNTIndex(sym, nts);

                if (sIdx==-1) {
                    // Terminal: add it, stop
                    if (g.addToSet(firstSets[lIdx], sym)) changed=true;
                    allCanBeEpsilon = false;
                    break;
                } else {
                    // Non-terminal: add FIRST(sym) \ {epsilon}
                    bool symHasEps = false;
                    for (auto& f : firstSets[sIdx]) {
                        if (f=="epsilon") { symHasEps=true; continue; }
                        if (g.addToSet(firstSets[lIdx],f)) changed=true;
                    }
                    if (!symHasEps) { allCanBeEpsilon=false; break; }
                    // If sym can be epsilon and it's the last symbol, add epsilon
                    if (j==(int)rhs.size()-1 && symHasEps) {
                        if (g.addToSet(firstSets[lIdx],"epsilon")) changed=true;
                    }
                }
            }
            if (allCanBeEpsilon) {
                // All symbols could be epsilon → LHS can be epsilon
                if (g.addToSet(firstSets[lIdx],"epsilon")) changed=true;
            }
        }
    }
}

// ─────────────────────────────────────────
//  FOLLOW
// ─────────────────────────────────────────

void FirstFollow::computeFollow() {
    // Add $ to FOLLOW of start symbol
    if (!nts.empty()) g.addToSet(followSets[0],"$");

    bool updated = true;
    while (updated) {
        updated = false;
        for (auto& prod : g.productions) {
            int sIdx = g.getNTIndex(prod.lhs, nts);
            const vector<string>& rhs = prod.rhs;

            for (int j=0;j<(int)rhs.size();j++) {
                int aIdx = g.getNTIndex(rhs[j], nts);
                if (aIdx==-1) continue;

                int sizeBefore = (int)followSets[aIdx].size();

                // Look at everything after rhs[j]
                bool restCanBeEmpty = true;
                for (int k=j+1;k<(int)rhs.size();k++) {
                    string next = rhs[k];
                    int nIdx = g.getNTIndex(next, nts);

                    if (nIdx==-1) {
                        // Terminal: add it to FOLLOW(rhs[j])
                        g.addToSet(followSets[aIdx], next);
                        restCanBeEmpty = false;
                        break;
                    } else {
                        // Add FIRST(next) \ {epsilon}
                        bool hasEps=false;
                        for (auto& f : firstSets[nIdx]) {
                            if (f=="epsilon"){hasEps=true;continue;}
                            g.addToSet(followSets[aIdx],f);
                        }
                        if (!hasEps){restCanBeEmpty=false;break;}
                    }
                }
                // If rest can be empty (or j is the last symbol), add FOLLOW(lhs)
                if (restCanBeEmpty) {
                    for (auto& f : followSets[sIdx])
                        g.addToSet(followSets[aIdx],f);
                }
                if ((int)followSets[aIdx].size()>sizeBefore) updated=true;
            }
        }
    }
}

void FirstFollow::compute() {
    computeFirst();
    computeFollow();
}

// ─────────────────────────────────────────
//  Helpers for parser table
// ─────────────────────────────────────────

bool FirstFollow::canDeriveEpsilon(const vector<string>& seq) const {
    for (auto& sym : seq) {
        int idx = g.getNTIndex(sym, nts);
        if (idx==-1) {
            if (sym!="epsilon") return false;
            // sym is literal epsilon token
        } else {
            bool hasEps=false;
            for (auto& f:firstSets[idx]) if(f=="epsilon"){hasEps=true;break;}
            if (!hasEps) return false;
        }
    }
    return true;
}

vector<string> FirstFollow::firstOfSequence(const vector<string>& seq) const {
    vector<string> result;
    if (seq.empty()||( seq.size()==1&&seq[0]=="epsilon")) {
        g.addToSet(result,"epsilon");
        return result;
    }
    bool allEps=true;
    for (auto& sym : seq) {
        int idx = g.getNTIndex(sym, nts);
        if (idx==-1) {
            if (sym=="epsilon") continue;
            g.addToSet(result,sym);
            allEps=false; break;
        } else {
            bool symHasEps=false;
            for (auto& f:firstSets[idx]) {
                if(f=="epsilon"){symHasEps=true;continue;}
                g.addToSet(result,f);
            }
            if (!symHasEps){allEps=false;break;}
        }
    }
    if (allEps) g.addToSet(result,"epsilon");
    return result;
}

// ─────────────────────────────────────────
//  Display
// ─────────────────────────────────────────

void FirstFollow::displayFirst() const {
    cout << "\n╔══════════════════════════════════════════╗\n";
    cout << "║              FIRST SETS                  ║\n";
    cout << "╚══════════════════════════════════════════╝\n";
    for (int i=0;i<(int)nts.size();i++) {
        cout << "  FIRST(" << left << setw(18) << nts[i] << ") = { ";
        for (int j=0;j<(int)firstSets[i].size();j++)
            cout << firstSets[i][j] << (j+1<(int)firstSets[i].size()?", ":"");
        cout << " }\n";
    }
}

void FirstFollow::displayFollow() const {
    cout << "\n╔══════════════════════════════════════════╗\n";
    cout << "║              FOLLOW SETS                 ║\n";
    cout << "╚══════════════════════════════════════════╝\n";
    for (int i=0;i<(int)nts.size();i++) {
        cout << "  FOLLOW(" << left << setw(17) << nts[i] << ") = { ";
        for (int j=0;j<(int)followSets[i].size();j++)
            cout << followSets[i][j] << (j+1<(int)followSets[i].size()?", ":"");
        cout << " }\n";
    }
}

void FirstFollow::displayFirstToFile(ofstream& out) const {
    out << "\n--- FIRST SETS ---\n";
    for (int i=0;i<(int)nts.size();i++) {
        out << "FIRST(" << nts[i] << ") = { ";
        for (int j=0;j<(int)firstSets[i].size();j++)
            out << firstSets[i][j] << (j+1<(int)firstSets[i].size()?", ":"");
        out << " }\n";
    }
}

void FirstFollow::displayFollowToFile(ofstream& out) const {
    out << "\n--- FOLLOW SETS ---\n";
    for (int i=0;i<(int)nts.size();i++) {
        out << "FOLLOW(" << nts[i] << ") = { ";
        for (int j=0;j<(int)followSets[i].size();j++)
            out << followSets[i][j] << (j+1<(int)followSets[i].size()?", ":"");
        out << " }\n";
    }
}
