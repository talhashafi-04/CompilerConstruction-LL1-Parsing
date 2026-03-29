#include "grammar.h"

// ─────────────────────────────────────────
//  Utility helpers
// ─────────────────────────────────────────

string Grammar::trim(string s) {
    int start = 0;
    while (start < (int)s.size() && (s[start]==' '||s[start]=='\t')) start++;
    int end = (int)s.size()-1;
    while (end>=0 && (s[end]==' '||s[end]=='\t'||s[end]=='\r'||s[end]=='\n')) end--;
    return (start>end) ? "" : s.substr(start, end-start+1);
}

bool Grammar::addToSet(vector<string>& set, const string& val) {
    for (auto& x : set) if (x==val) return false;
    set.push_back(val);
    return true;
}

int Grammar::getNTIndex(const string& s, const vector<string>& nts) const {
    for (int i=0;i<(int)nts.size();i++) if (nts[i]==s) return i;
    return -1;
}

string Grammar::formatRule(const Production& p) const {
    string s = p.lhs + " -> ";
    for (int i=0;i<(int)p.rhs.size();i++){
        s += p.rhs[i];
        if (i+1<(int)p.rhs.size()) s+=" ";
    }
    return s;
}

// ─────────────────────────────────────────
//  Load grammar from file
// ─────────────────────────────────────────

bool Grammar::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open " << filename << endl;
        return false;
    }
    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
        size_t arrowPos = line.find("->");
        if (arrowPos == string::npos) continue;

        string lhs = trim(line.substr(0, arrowPos));
        string rhsPart = line.substr(arrowPos+2);

        stringstream ss(rhsPart);
        string alternative;
        while (getline(ss, alternative, '|')) {
            Production p;
            p.lhs = lhs;
            stringstream ssAlt(alternative);
            string token;
            while (ssAlt >> token) p.rhs.push_back(token);
            // Allow empty RHS to represent epsilon explicitly
            if (p.rhs.empty()) p.rhs.push_back("epsilon");
            productions.push_back(p);
        }
    }
    file.close();
    return true;
}

// ─────────────────────────────────────────
//  Introspection
// ─────────────────────────────────────────

vector<string> Grammar::getNonTerminals() const {
    vector<string> nts;
    for (auto& p : productions) {
        bool found=false;
        for (auto& n : nts) if (n==p.lhs){found=true;break;}
        if (!found) nts.push_back(p.lhs);
    }
    return nts;
}

vector<string> Grammar::getTerminals(const vector<string>& nts) const {
    vector<string> terms;
    for (auto& p : productions)
        for (auto& sym : p.rhs)
            if (getNTIndex(sym,nts)==-1 && sym!="epsilon") {
                bool found=false;
                for (auto& t:terms) if(t==sym){found=true;break;}
                if (!found) terms.push_back(sym);
            }
    // Add $ at the end
    bool found=false;
    for (auto& t:terms) if(t=="$"){found=true;break;}
    if (!found) terms.push_back("$");
    return terms;
}

string Grammar::getStartSymbol() const {
    if (!productions.empty()) return productions[0].lhs;
    return "";
}

// ─────────────────────────────────────────
//  Left Factoring
// ─────────────────────────────────────────

void Grammar::applyLeftFactoring() {
    bool overallChanged = true;
    while (overallChanged) {
        overallChanged = false;
        vector<string> nts = getNonTerminals();

        for (auto& s : nts) {
            // Find the longest common prefix among productions for 's'
            string prefix = "";
            bool foundPrefix = false;

            // Collect all productions for this NT
            vector<int> myProds;
            for (int i=0;i<(int)productions.size();i++)
                if (productions[i].lhs==s) myProds.push_back(i);

            // Check each pair for a common first token
            for (int a=0;a<(int)myProds.size()&&!foundPrefix;a++)
                for (int b=a+1;b<(int)myProds.size()&&!foundPrefix;b++) {
                    auto& rA = productions[myProds[a]].rhs;
                    auto& rB = productions[myProds[b]].rhs;
                    if (!rA.empty()&&!rB.empty()&&rA[0]==rB[0]) {
                        prefix = rA[0];
                        foundPrefix = true;
                    }
                }

            if (!foundPrefix) continue;
            overallChanged = true;

            // Build new NT name – keep appending ' until unique
            string newNT = s + "'";
            while (getNTIndex(newNT, getNonTerminals())!=-1) newNT += "'";

            vector<Production> nextRound;

            // Keep productions that don't start with prefix
            for (auto& p : productions)
                if (p.lhs!=s || p.rhs.empty() || p.rhs[0]!=prefix)
                    nextRound.push_back(p);

            // A -> prefix newNT
            Production main;
            main.lhs = s;
            main.rhs = {prefix, newNT};
            nextRound.push_back(main);

            // newNT -> rest_of_each_alternative_with_prefix
            for (auto& p : productions) {
                if (p.lhs==s && !p.rhs.empty() && p.rhs[0]==prefix) {
                    Production np;
                    np.lhs = newNT;
                    for (int m=1;m<(int)p.rhs.size();m++) np.rhs.push_back(p.rhs[m]);
                    if (np.rhs.empty()) np.rhs.push_back("epsilon");
                    nextRound.push_back(np);
                }
            }
            productions = nextRound;
            break; // restart outer loop after any factoring
        }
    }
}

// ─────────────────────────────────────────
//  Left Recursion Removal
// ─────────────────────────────────────────

void Grammar::removeDirectForSpecificNT(const string& A) {
    vector<Production> recursive, nonRecursive, others;

    for (auto& p : productions) {
        if (p.lhs==A) {
            if (!p.rhs.empty() && p.rhs[0]==A)
                recursive.push_back(p);
            else
                nonRecursive.push_back(p);
        } else {
            others.push_back(p);
        }
    }

    if (recursive.empty()) return; // no direct recursion

    // Build A'
    string Ap = A + "'";
    // Ensure uniqueness
    vector<string> existingNTs = getNonTerminals();
    while (getNTIndex(Ap, existingNTs)!=-1) Ap += "'";

    // A -> beta A'
    for (auto& p : nonRecursive) {
        Production np = p;
        // If beta is epsilon, just produce A'
        if (np.rhs.size()==1 && np.rhs[0]=="epsilon") {
            np.rhs = {Ap};
        } else {
            np.rhs.push_back(Ap);
        }
        others.push_back(np);
    }
    // If no non-recursive productions, add A -> A' (shouldn't happen normally)
    if (nonRecursive.empty()) {
        Production np; np.lhs=A; np.rhs={Ap};
        others.push_back(np);
    }

    // A' -> alpha A'
    for (auto& p : recursive) {
        Production np;
        np.lhs = Ap;
        for (int k=1;k<(int)p.rhs.size();k++) np.rhs.push_back(p.rhs[k]);
        np.rhs.push_back(Ap);
        others.push_back(np);
    }

    // A' -> epsilon
    Production eps; eps.lhs=Ap; eps.rhs={"epsilon"};
    others.push_back(eps);

    productions = others;
}

void Grammar::removeLeftRecursion() {
    vector<string> NTs = getNonTerminals();

    for (int i=0;i<(int)NTs.size();i++) {
        // Indirect: replace Ai -> Aj gamma where j < i
        for (int j=0;j<i;j++) {
            string Ai = NTs[i], Aj = NTs[j];
            vector<Production> updatedAi;
            for (auto& p : productions) {
                if (p.lhs==Ai && !p.rhs.empty() && p.rhs[0]==Aj) {
                    // Expand: replace Aj with each of Aj's productions
                    vector<string> gamma(p.rhs.begin()+1, p.rhs.end());
                    for (auto& q : productions) {
                        if (q.lhs==Aj) {
                            Production np; np.lhs=Ai;
                            // If q.rhs is just epsilon, don't prepend it
                            if (!(q.rhs.size()==1 && q.rhs[0]=="epsilon"))
                                np.rhs = q.rhs;
                            for (auto& g : gamma) np.rhs.push_back(g);
                            if (np.rhs.empty()) np.rhs={"epsilon"};
                            updatedAi.push_back(np);
                        }
                    }
                } else if (p.lhs==Ai) {
                    updatedAi.push_back(p);
                }
            }
            // Replace Ai productions in master list
            vector<Production> temp;
            for (auto& p : productions) if (p.lhs!=Ai) temp.push_back(p);
            for (auto& p : updatedAi) temp.push_back(p);
            productions = temp;
        }
        // Direct recursion for NTs[i]
        removeDirectForSpecificNT(NTs[i]);
    }
}

// ─────────────────────────────────────────
//  Display
// ─────────────────────────────────────────

void Grammar::displayGrammar(const string& title) const {
    cout << "\n╔══════════════════════════════════════════╗\n";
    cout << "║  " << left << setw(41) << title << "║\n";
    cout << "╚══════════════════════════════════════════╝\n";
    // Group by LHS
    vector<string> seen;
    for (auto& p : productions) {
        bool found=false;
        for (auto& s:seen) if(s==p.lhs){found=true;break;}
        if (found) continue;
        seen.push_back(p.lhs);
        cout << "  " << p.lhs << " -> ";
        bool first=true;
        for (auto& q : productions) {
            if (q.lhs!=p.lhs) continue;
            if (!first) cout << " | ";
            for (int k=0;k<(int)q.rhs.size();k++){
                cout<<q.rhs[k];
                if(k+1<(int)q.rhs.size()) cout<<" ";
            }
            first=false;
        }
        cout << "\n";
    }
}

void Grammar::displayGrammarToFile(ofstream& out, const string& title) const {
    out << "\n--- " << title << " ---\n";
    vector<string> seen;
    for (auto& p : productions) {
        bool found=false;
        for (auto& s:seen) if(s==p.lhs){found=true;break;}
        if (found) continue;
        seen.push_back(p.lhs);
        out << p.lhs << " -> ";
        bool first=true;
        for (auto& q : productions) {
            if (q.lhs!=p.lhs) continue;
            if (!first) out << " | ";
            for (int k=0;k<(int)q.rhs.size();k++){
                out<<q.rhs[k];
                if(k+1<(int)q.rhs.size()) out<<" ";
            }
            first=false;
        }
        out << "\n";
    }
}
