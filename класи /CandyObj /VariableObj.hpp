#pragma once
#include <string>
using namespace std;
class Variable {     
    public:
        string type;
        string value;
        Variable() {}
        Variable(string t, string v) {
            type = t;
            value = v;
        }
};
