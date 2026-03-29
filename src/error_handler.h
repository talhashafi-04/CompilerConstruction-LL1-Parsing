#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

enum class ErrorType {
    MISSING_SYMBOL,      // expected terminal not found
    UNEXPECTED_SYMBOL,   // terminal where not expected
    EMPTY_TABLE_ENTRY,   // no production in M[X,a]
    PREMATURE_END        // input ended but stack not empty
};

struct ParseError {
    ErrorType type;
    int       line;
    int       column;
    string    expected;
    string    found;
    string    message;
};

class ErrorHandler {
public:
    vector<ParseError> errors;
    int errorCount;

    ErrorHandler() : errorCount(0) {}

    void reportError(ErrorType t, int line, int col,
                     const string& expected, const string& found,
                     ostream& out = cout);

    void reportToFile(ofstream& out) const;

    bool hasErrors() const { return errorCount > 0; }
    void reset() { errors.clear(); errorCount=0; }
};

#endif
