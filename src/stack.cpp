#include "stack.h"
#include <stdexcept>

void Stack::push(const string& s) {
    data.push_back(s);
}

void Stack::pop() {
    if (data.empty()) throw runtime_error("Stack underflow");
    data.pop_back();
}

string Stack::top() const {
    if (data.empty()) throw runtime_error("Stack is empty");
    return data.back();
}

bool Stack::isEmpty() const {
    return data.empty();
}

int Stack::size() const {
    return (int)data.size();
}

void Stack::clear() {
    data.clear();
}

// Returns contents bottom→top separated by spaces (matches assignment trace format)
string Stack::toString() const {
    string s;
    for (int i=0;i<(int)data.size();i++){
        if (i) s+=" ";
        s+=data[i];
    }
    return s;
}
