#ifndef FIRST_FOLLOW_H
#define FIRST_FOLLOW_H

#include "grammar.h"

class FirstFollow {
public:
    Grammar& g;
    vector<string> nts;
    vector<string> terms;
    vector<vector<string>> firstSets;
    vector<vector<string>> followSets;

    FirstFollow(Grammar& grammar);

    void compute();                     // compute both FIRST and FOLLOW
    void computeFirst();
    void computeFollow();

    void displayFirst() const;
    void displayFollow() const;
    void displayFirstToFile(ofstream& out) const;
    void displayFollowToFile(ofstream& out) const;

    // Check if a sequence of symbols can derive epsilon
    bool canDeriveEpsilon(const vector<string>& seq) const;

    // Compute FIRST of an arbitrary sequence (used by parser table builder)
    vector<string> firstOfSequence(const vector<string>& seq) const;
};

#endif
