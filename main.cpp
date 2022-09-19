#define _CRT_SECURE_NO_WARNINGS
#define _NO_DEBUG_HEAP
#ifdef _WIN32
#include <Windows.h>
#endif
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include "libs/strlib.hpp"
#include "classes/token.hpp"
#include "classes/lexer.hpp"
#include "classes/parser.hpp"
using namespace std;

int main(int argc, char * argv[]){
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    if (argc > 1) {
        string target = argv[1];
        ifstream file(target);
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
        code.str(std::string());
        // for (auto i : tokenlist[21]) {
        //     cout << i.type << " " << i.value << endl;
        // }
        Parser p(tokenlist, target);
        p.runCode();
    }

    return 0;
}
