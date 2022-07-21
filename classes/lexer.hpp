#pragma once
#include <vector>
#include <cctype>
#include <string>
#include <algorithm>
#include <map>
#include "token.hpp"
using namespace std;

class Lexer {
    vector<string> spec_symbols = {"(", ")", "'", "\"", "{", "}", "[", "]", "=", "!", "+", "-", "*", "/", "%", ":", ";", ".", ",", "<", ">", "#", "\\"};
    map<string, string> spec_symbols_name = {{"(", "LEFT_BRACKET"}, {")", "RIGHT_BRACKET"}, {"'", "QUOTE"}, {"\"", "QUOTE"}, {"{", "LEFT_FIGURE_BRACKET"}, {"}", "RIGHT_FIGURE_BRACKET"}, {"[", "RIGHT_SQUARE_BRACKET"}, {"]", "LEFT_SQUARE_BRACKET"}, {"=", "ASSIGN"}, {"!", "EXCLAMATION_MARK"}, {"+", "PLUS"}, {"-", "MINUS"}, {"*", "MULTIPLICATION"}, {"/", "DIVISION"}, {"%", "INTEREST"}, {":", "DOUBLE_DOT"}, {";", "DOT_COMA"}, {".", "DOT"}, {",", "COMA"}, {"<", "LEFT_INEQUALITY_BRACKET"}, {">", "RIGHT_INEQUALITY_BRACKET"}, {"#", "HASHTAG"}, {"\\", "INVERSE_SLASH"}};
    vector<string> quotes = {"'", "\""};
    string quotes_opened;
    string comment = "#", space = " ";
    public:
        map<int, string> disassemble(string code_string) {
            map<int, string> undefined_elements = {};
            string undefined = "";
            string i = "";
            int pos = 0;
            int tmp = 0;
            for (tmp = 0; tmp != code_string.length(); tmp++) {
                pos = tmp;
                i = code_string[tmp];
                if (find(spec_symbols.begin(), spec_symbols.end(), i) != spec_symbols.end()) {
                    if (find(quotes.begin(), quotes.end(), i) != quotes.end()){
                        if (quotes_opened != "" && quotes_opened == i) {
                            quotes_opened = "";
                        }

                        else if (quotes_opened == "") {
                            quotes_opened = quotes[find(quotes.begin(), quotes.end(), i) - quotes.begin()];
                        }
                    }

                    if (quotes_opened != "") {
                        if (i != quotes_opened) {
                            undefined += i;
                        }
                        else {
                            undefined_elements[pos] = i;
                        }
                    } else {
                        if (undefined != "") {
                            undefined_elements[pos - undefined.length()] = undefined;
                        }
                        undefined_elements[pos] = i;
                        undefined = "";
                    }
                } else {
                if (quotes_opened != "")
                    undefined += i;
                else if (i != space)
                    undefined += i;
                else {
                    if (undefined != ""){
                    undefined_elements[pos - undefined.length()] = undefined;
                    undefined = "";
                    }
                }
                }
            }
            if (undefined != ""){
                undefined_elements[tmp - undefined.length()] = undefined;
            }
        return undefined_elements;
        }

        vector<Token> tokenize(map<int, string> undefined_elements){
            vector<Token> tokens = {};
            string i = "";
            quotes_opened = "";
            for (auto tmp : undefined_elements){
                i = tmp.second;
                if (spec_symbols_name.find(i) != spec_symbols_name.end()){
                    if (find(quotes.begin(), quotes.end(), i) != quotes.end()){
                        if (quotes_opened != "")
                            quotes_opened = "";
                        else
                            quotes_opened = quotes[find(quotes.begin(), quotes.end(), i) - quotes.begin()];
                    }

                    tokens.push_back(Token(spec_symbols_name[i], i, tmp.first));
                
                } else if (i.find_first_not_of("0123456789") == string::npos && quotes_opened == "") {
                    tokens.push_back(Token("NUMBER", i, tmp.first));

                } else {
                    tokens.push_back(Token("UNDEFINED_STRING", i, tmp.first));                    
                }
            }
            return tokens;
        }
};
