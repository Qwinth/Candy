#define _CRT_SECURE_NO_WARNINGS
#define _NO_DEBUG_HEAP
#include <iostream>
#include <vector>
#include <map>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include "classes/token.hpp"
#include "classes/lexer.hpp"
#include "classes/parser.hpp"
using namespace std;

int main(int argc, char * argv[]){
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    // if (argc > 1) {
        ifstream file("functest.cd");
        stringstream code;
        code << file.rdbuf();
        file.close();
        Lexer l;
        vector<Token> i = {};
        map<int, vector<Token>> tokenlist = {};
        vector<string> codelines = split(code.str(), "\n");
        for (int t = 0; t < codelines.size(); t++){
            tokenlist[t] = l.tokenize(l.disassemble(codelines[t]));
        }
        // for (auto i : tokenlist[0]) {
        //     cout << i.value << endl;
        // }
        Parser p(tokenlist, "functest.cd");
        p.runCode();
    // }

    return 0;
}
