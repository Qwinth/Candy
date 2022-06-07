#pragma once
#include <string>
#include <vector>
#include <map>
#include "VariableObj.hpp"
#include "../token.hpp"
using namespace std;

class Argument {
    public:
        int pos;
        string value;
        string name;
        Argument(int p, string n, string v) {
            pos = p;
            name = n;
            value = v;
        }
        
};


class Function {
    map<string, Variable*> variables = {};
    map<int, vector<Token>> tokens = {};
    public:
        vector<Argument> args = {};
        Function(vector<Argument> a) {
            args = a;
            for (auto i : args) {
                variables[i.name] = new Variable("string", i.value);
            }
        }

};

