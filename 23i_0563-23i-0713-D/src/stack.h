#ifndef STACK_H
#define STACK_H

#include <vector>
#include <string>
using namespace std;

// Simple stack wrapper used by the LL(1) parser
class Stack {
private:
    vector<string> data;
public:
    void    push(const string& s);
    void    pop();
    string  top() const;
    bool    isEmpty() const;
    int     size() const;
    void    clear();
    string  toString() const;  // bottom-to-top for display
    const vector<string>& getData() const { return data; }
};

#endif
