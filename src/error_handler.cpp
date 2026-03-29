#include "error_handler.h"

void ErrorHandler::reportError(ErrorType t, int line, int col,
                                const string& expected, const string& found,
                                ostream& out) {
    ParseError e;
    e.type = t; e.line=line; e.column=col;
    e.expected=expected; e.found=found;
    errorCount++;

    switch(t) {
        case ErrorType::MISSING_SYMBOL:
            e.message = "[ERROR] Line "+to_string(line)+", Col "+to_string(col)
                       +": Missing symbol. Expected '"+expected+"', found '"+found+"'.";
            break;
        case ErrorType::UNEXPECTED_SYMBOL:
            e.message = "[ERROR] Line "+to_string(line)+", Col "+to_string(col)
                       +": Unexpected symbol '"+found+"'. Expected '"+expected+"'.";
            break;
        case ErrorType::EMPTY_TABLE_ENTRY:
            e.message = "[ERROR] Line "+to_string(line)+", Col "+to_string(col)
                       +": No rule for non-terminal '"+expected+"' with lookahead '"+found+"'.";
            break;
        case ErrorType::PREMATURE_END:
            e.message = "[ERROR] Line "+to_string(line)+", Col "+to_string(col)
                       +": Premature end of input. Still expecting '"+expected+"'.";
            break;
    }
    errors.push_back(e);
    out << "  " << e.message << "\n";
}

void ErrorHandler::reportToFile(ofstream& out) const {
    out << "\n--- PARSE ERRORS ---\n";
    if (errors.empty()) { out << "  No errors.\n"; return; }
    for (auto& e : errors) out << "  " << e.message << "\n";
    out << "  Total errors: " << errorCount << "\n";
}
