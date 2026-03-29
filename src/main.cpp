#include "grammar.h"
#include "first_follow.h"
#include "parser.h"
#include "tree.h"
#include "error_handler.h"
#include <fstream>
#include <sstream>
#include <ctime>

using namespace std;

// ─────────────────────────────────────────
//  Utility: write a separator banner
// ─────────────────────────────────────────
void banner(ostream& out, const string& msg) {
    out << "\n" << string(90,'#') << "\n";
    out << "#  " << msg << "\n";
    out << string(90,'#') << "\n";
}

// ─────────────────────────────────────────
//  Run one grammar + input file pair
// ─────────────────────────────────────────
void runGrammar(const string& grammarFile,
                const string& inputFile,
                const string& outputPrefix,   // e.g. "output/grammar1"
                int grammarNum)
{
    cout << "\n";
    cout << string(90,'=') << "\n";
    cout << "  GRAMMAR " << grammarNum << "  |  " << grammarFile << "\n";
    cout << string(90,'=') << "\n";

    // ── Load ──
    Grammar g;
    if (!g.loadFromFile(grammarFile)) return;
    g.displayGrammar("ORIGINAL GRAMMAR");

    // ── Output files ──
    ofstream outGrammar(outputPrefix+"_grammar_transformed.txt");
    ofstream outFF(outputPrefix+"_first_follow_sets.txt");
    ofstream outTable(outputPrefix+"_parsing_table.txt");
    ofstream outTrace(outputPrefix+"_parsing_trace.txt");
    ofstream outTree(outputPrefix+"_parse_trees.txt");

    // Write original to transformed file
    g.displayGrammarToFile(outGrammar, "ORIGINAL GRAMMAR");

    // ── Left Factoring ──
    g.applyLeftFactoring();
    g.displayGrammar("AFTER LEFT FACTORING");
    g.displayGrammarToFile(outGrammar, "AFTER LEFT FACTORING");

    // ── Left Recursion Removal ──
    g.removeLeftRecursion();
    g.displayGrammar("AFTER LEFT RECURSION REMOVAL");
    g.displayGrammarToFile(outGrammar, "AFTER LEFT RECURSION REMOVAL");
    outGrammar.close();

    // ── FIRST / FOLLOW ──
    FirstFollow ff(g);
    ff.compute();
    ff.displayFirst();
    ff.displayFollow();
    ff.displayFirstToFile(outFF);
    ff.displayFollowToFile(outFF);
    outFF.close();

    // ── Parsing Table ──
    Parser parser(g, ff);
    parser.buildTable();
    parser.displayTable();
    parser.displayTableToFile(outTable);
    outTable.close();

    if (!parser.isLL1) {
        cout << "\n  *** Grammar " << grammarNum << " is NOT LL(1). Parsing may be unreliable. ***\n";
    }

    // ── Parse Input File ──
    ifstream inFile(inputFile);
    if (!inFile.is_open()) {
        cerr << "  Warning: Could not open input file: " << inputFile << "\n";
        return;
    }

    banner(outTrace, "PARSING TRACES - Grammar " + to_string(grammarNum));
    banner(outTree,  "PARSE TREES - Grammar "    + to_string(grammarNum));

    string line;
    int lineNum=0, accepted=0, rejected=0;
    while (getline(inFile, line)) {
        lineNum++;
        line = g.trim(line);
        if (line.empty()) continue;

        ParseTree tree;
        bool ok = parser.parse(line, lineNum, tree, cout);
        // Also write trace to file
        {
            // Re-run to capture into file (redirect would be cleaner, but we keep it simple)
            // We instead duplicate output to file by parsing again with outTrace
            parser.parse(line, lineNum, tree, outTrace);
        }

        if (ok) {
            accepted++;
            outTree << "\n--- Parse Tree for: \"" << line << "\" ---\n";
            tree.print(outTree);
        } else {
            rejected++;
        }
    }
    inFile.close();
    outTrace.close();
    outTree.close();

    cout << "\n  Summary: " << accepted << " accepted, " << rejected << " rejected out of " << lineNum << " strings.\n";
}

// ─────────────────────────────────────────
//  main
// ─────────────────────────────────────────
int main(int argc, char* argv[]) {
    cout << string(90,'*') << "\n";
    cout << "*  LL(1) Parser - CS4031 Assignment 02\n";
    cout << string(90,'*') << "\n";

    // If called with explicit grammar + input args
    if (argc==3) {
        runGrammar(argv[1], argv[2], "output/custom", 0);
        return 0;
    }

    // Default: run all 4 test grammars
    struct TestCase {
        string grammar;
        string input;
        string outPrefix;
        int    num;
    };

    vector<TestCase> tests = {
        {"input/grammar1.txt", "input/grammar1_valid.txt",      "output/grammar1_valid",  1},
        {"input/grammar1.txt", "input/grammar1_errors.txt",     "output/grammar1_errors", 1},
        {"input/grammar2.txt", "input/grammar2_valid.txt",      "output/grammar2_valid",  2},
        {"input/grammar2.txt", "input/grammar2_errors.txt",     "output/grammar2_errors", 2},
        {"input/grammar3.txt", "input/grammar3_valid.txt",      "output/grammar3_valid",  3},
        {"input/grammar3.txt", "input/grammar3_errors.txt",     "output/grammar3_errors", 3},
        {"input/grammar4.txt", "input/grammar4_valid.txt",      "output/grammar4_valid",  4},
        {"input/grammar4.txt", "input/grammar4_errors.txt",     "output/grammar4_errors", 4},
    };

    for (auto& t : tests) {
        runGrammar(t.grammar, t.input, t.outPrefix, t.num);
    }

    cout << "\n" << string(90,'*') << "\n";
    cout << "*  All grammars processed. Check output/ directory for results.\n";
    cout << string(90,'*') << "\n";
    return 0;
}
