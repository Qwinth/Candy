#pragma once
#include <string>
#include <vector>
#include <map>
#include "VariableObj.hpp"
#include "../token.hpp"
using namespace std;

class Argument: public Variable {
public:
    int pos;
    string name;
    Argument(int p, string n, string v, string t) {
        pos = p;
        name = n;
        value = v;
        type = t;
    }
};


class Function {
    map<string, Variable*> variables = {};
public:
    vector<int> tokenpos = {};
    vector<Argument> args = {};
    Function(vector<Argument> a, vector<int> tpos) {
        args = a;
        tokenpos = tpos;
        for (auto i : args) {
            variables[i.name] = new Variable("STRING", i.value);
        }
    }

};
