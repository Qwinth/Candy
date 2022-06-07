#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <map>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <fstream>
#include <sstream>
#include "classes/token.hpp"
#include "classes/lexer.hpp"
#include "classes/parser.hpp"
#include "libs/strlib.hpp"
using namespace std;

int main(int argc, char * argv[]){
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif
    if (argc > 1) {
    ifstream file(argv[1]);
    stringstream code;
    code << file.rdbuf();
    file.close();
    Lexer l;
    vector<Token> i = {};
    map<int, vector<Token>> tokenlist = {};
    for (int t = 0; t < split(code.str(), "\n").size(); t++){

    tokenlist[t] = l.tokenize(l.disassemble(split(code.str(), "\n")[t]));

    }
    Parser p(tokenlist, argv[1]);
    p.runCode();
    }
    
    return 0;
}
