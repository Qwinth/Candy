#pragma once
#include <string>
using namespace std;
class Token {
    public:
    string type;
    string value;
    int pos = 0;
    Token(string _type, string val, int position){
        type = _type;
        value = val;
        pos = position;
    }
};