#pragma once
#include <string>
using namespace std;
class Variable {
    protected:
        string type;
        string value;
    public:
        Variable(string t, string v) {
            type = t;
            value = v;
        }
};