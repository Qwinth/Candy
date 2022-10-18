#ifdef _WIN32
#include <Windows.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <chrono>
#include <random>
#include "libs/strlib.hpp"
#include "classes/token.hpp"
#include "classes/lexer.hpp"
#include "classes/parser.hpp"
using namespace std;

void finalize(int s) {
    cout.clear();
    cin.clear();
    exit(s);
}

int main(int argc, char * argv[]){
    if (argc > 1) {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        if (split(argv[1], ".").back() == "cdn") { FreeConsole(); }
#endif
        string target = argv[1];
        ifstream file(target);
        if (file.good()) {
            signal(SIGINT, finalize);
            stringstream code;
            code << file.rdbuf();
            file.close();
            map<int, vector<Token>> tokenlist = {};
            auto start = chrono::high_resolution_clock::now();
            vector<string> codelines = split(code.str(), "\n");
            Lexer l;

            for (int t = 0; t < codelines.size(); t++) {
                tokenlist[t] = l.tokenize(l.disassemble(codelines[t]));
            }
            auto stop = chrono::high_resolution_clock::now();
            code.str(std::string());

            cout << chrono::duration_cast<chrono::microseconds>(stop - start).count() << endl;
            Parser p(tokenlist, argv, argc);
            p.runCode();
            finalize(0);
        }
    }

    return 0;
}
