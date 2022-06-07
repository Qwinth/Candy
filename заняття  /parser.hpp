#pragma once
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include "lexer.hpp"
#include "token.hpp"
#include "CandyObj/FunctionObj.hpp"
#include "CandyObj/VariableObj.hpp"

using namespace std;
class Parser {
    map<int, vector<Token>> tokens = {};
    int line = 0;

    map<string, vector<string>> objects = { {"FUNCTION", {"function"}}, {"VARIABLE", {"__file__"}} };
    map<string, string> scope = {};
    vector<string> keywords = {"if", "for", "while", "in", "import", "return", "func"};
    vector<string> booloperators = { "==", ">=", "<=", ">", "<" };
    vector<string> mathoperators = { "+", "-", "*", "/", "%" };
    vector<string> boolean = { "true", "false" };
    map<string, string> variables = {};
    map<string, Function*> functions = {};


    bool match(string type, int _pos) {
        if (_pos < tokens[line].size()) {
            if (tokens[line][_pos + 1].type == type) {
                return true;
            }
        }
        return false;
    }
    public:
        Parser(map<int, vector<Token>> _tokens, string _file) {
        tokens = _tokens;
        variables["__file__"] = _file;
        }

        void var_preprocess() {
            for (int pos = 0; pos < tokens[line].size(); pos++) {
                Token i = tokens[line][pos];
                if (i.type == "UNDEFINED_STRING") {
                    if (objects.find(i.value) == objects.end()) {
                        if (match("ASSIGN", pos)) {
                            objects["VARIABLE"].push_back(i.value);
                        }
                    }
                }
            }
        }

        void string_preprocess() {
            for (int pos = 0; pos < tokens[line].size(); pos++) {
                Token i = tokens[line][pos];

                if (i.type == "QUOTE" && match("UNDEFINED_STRING", pos) && match("QUOTE", pos + 1)) {
                    objects["STRING"].push_back(tokens[line][pos + 1].value);
                }
            }
        }


        vector<string> exec(string fname, vector<string> args) {
            cout << "call function: " << fname << " with arg: " << args[0] << endl;
            cout << args.size() << endl;
            for (auto i : args) {
                cout << i << endl;
            }
            vector<string> ret = {"novalue"};
            return ret;
        }

        string format_numeric(string s) {
            if (s[s.size() - 1] == '0')
                for (size_t i = s.size() - 1; s[i] == '0'; i--)
                    s.erase(i, 1);

            if (s[s.size() - 1] == '.')
                s.erase(s.size() - 1, 1);
            return s;
        }

        vector<string> process_numeric(vector<string> floattmp) {
            string i = floattmp[0];
            vector<string> tmp;
            int pos = stoi(floattmp.back());
            while (pos < tokens[line].size() && (tokens[line][pos].type == "NUMERIC" || tokens[line][pos].type == "UNDEFINED_STRING" || find(mathoperators.begin(), mathoperators.end(), tokens[line][pos].value) != mathoperators.end())) {
                if (match("PLUS", pos)) {
                    if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), tokens[line][pos + 2].value) != objects["VARIABLE"].end()) {
                        i = (format_numeric(to_string(stold(i) + stold(variables[tokens[line][pos + 2].value]))));
                    }
                    else {
                        i = (format_numeric(to_string(stold(i) + stold(parseNumeric(pos + 2)[0]))));
                        pos = stoi(parseNumeric(pos + 2)[1]);
                    }
                }
                else if (match("MINUS", pos)) {
                    if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), tokens[line][pos + 2].value) != objects["VARIABLE"].end()) {
                        i = (format_numeric(to_string(stold(i) - stold(variables[tokens[line][pos + 2].value]))));
                    }
                    else {
                        i = (format_numeric(to_string(stold(i)-+ stold(parseNumeric(pos + 2)[0]))));
                        pos = stoi(parseNumeric(pos + 2)[1]);
                    }

                }
                else if (match("MULTIPLICATION", pos)) {
                    if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), tokens[line][pos + 2].value) != objects["VARIABLE"].end()) {
                        i = (format_numeric(to_string(stold(i) * stold(variables[tokens[line][pos + 2].value]))));
                    }
                    else {
                        i = (format_numeric(to_string(stold(i) * stold(parseNumeric(pos + 2)[0]))));
                        pos = stoi(parseNumeric(pos + 2)[1]);
                    }

                }
                else if (match("DIVISION", pos)) {
                    if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), tokens[line][pos + 2].value) != objects["VARIABLE"].end()) {
                        i = (format_numeric(to_string(stold(i) / stold(variables[tokens[line][pos + 2].value]))));
                    }
                    else {
                        i = (format_numeric(to_string(stold(i) / stold(parseNumeric(pos + 2)[0]))));
                        pos = stoi(parseNumeric(pos + 2)[1]);
                    }

                }

                pos++;
            }
            tmp.push_back(i);
            tmp.push_back(to_string(pos));
            return tmp;
        }

        vector<string> parseNumeric(int pos) {
            vector<string> floattmp = {""};

            while (tokens[line][pos].type == "NUMERIC" || tokens[line][pos].type == "DOT") {
                floattmp[0] += tokens[line][pos].value;
                pos++;
            }
            floattmp.push_back(to_string(pos - 1));
            return floattmp;
        }

        vector<string> parseCode(int pos) {
            if (tokens[line][pos].type == "QUOTE" && match("UNDEFINED_STRING", pos) && match("QUOTE", pos + 1)) {
                pos += 1;
            }

            
            auto i = tokens[line][pos];
            vector<string> tmp = {};
            if (i.type == "NUMERIC") {
                tmp = process_numeric(parseNumeric(pos));
                return tmp;

            } else if (match("QUOTE", pos - 2) && find(objects["STRING"].begin(), objects["STRING"].end(), i.value) != objects["STRING"].end() && match("QUOTE", pos)) {
                tmp.push_back(i.value);
                tmp.push_back(to_string(pos));
                return tmp;

            } else if (find(objects["FUNCTION"].begin(), objects["FUNCTION"].end(), i.value) != objects["FUNCTION"].end()) {
                vector<string> _tmp = {};

                if (match("LEFT_BRACKET", pos)) {
                    
                    if (match("UNDEFINED_STRING", pos + 1) || match("NUMERIC", pos + 1) || match("QUOTE", pos + 1)) {
                        while (pos + 1 < tokens[line].size() && tokens[line][pos].type != "RIGHT_BRACKET") {
                        _tmp = parseCode(pos + 1);
                        pos = stoll(_tmp.back());
                        _tmp.pop_back();
                        if (_tmp[0] != "null"){ tmp.push_back(_tmp[0]); }
                        
                        }
                        if (match("COMA", pos)){
                            pos++;
                        }

                        if (tmp.size() == 0) {tmp.push_back("novalue");}
                        tmp = exec(i.value, tmp);
                    }
                    else { tmp = exec(i.value, {"novalue"}); }

                }
                if (tmp.size() == 0) {tmp.push_back("novalue");}
                tmp.push_back(to_string(pos));
                return tmp;

            } else if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), i.value) != objects["VARIABLE"].end()) {
                if (pos + 1 < tokens[line].size() && find(mathoperators.begin(), mathoperators.end(), tokens[line][pos + 1].value) != mathoperators.end()) {
                    tmp.push_back(process_numeric({ variables[i.value], to_string(pos) })[0]);
                    tmp.push_back(process_numeric({ variables[i.value], to_string(pos) }).back());
                }
                else {
                    tmp.push_back(variables[i.value]);
                    tmp.push_back(to_string(pos));
                }
                
                return tmp;
            
            } else {
                tmp.push_back("null");
                tmp.push_back(to_string(pos));
            }
            return tmp;
        }
        
        int runCode() {
            for (auto ln : tokens) {
            line = ln.first;
            if (tokens[line].size() == 0){
                continue;
            }

            var_preprocess();
            string_preprocess();
            int pos = 0;
            auto i = tokens[line][0];
            if (i.type == "UNDEFINED_STRING") {
                if (find(keywords.begin(), keywords.end(), i.value) != keywords.end()) {

                    if (i.value == "func" && match("UNDEFINED_STRING", pos)) {
                        objects["FUNCTION"].push_back(tokens[line][pos + 1].value);
                        if (match("LEFT_BRACKET", pos + 1)) {
                            vector<Argument> args = {};
                            string value;
                            string name;
                            int argnum = 0;
                            int _pos = pos + 2;
                            while (_pos < tokens[line].size() && tokens[line][_pos].type != "RIGHT_BRACKET") {

                                if (tokens[line][_pos].type == "UNDEFINED_STRING") {
                                    if (match("QUOTE", _pos - 2)) {
                                        _pos++;
                                        continue;
                                    }
                                    argnum++;
                                    name = tokens[line][_pos].value;
                                    if (match("ASSIGN", _pos)) {
                                        _pos += 2;
                                        auto tmp = parseCode(_pos);
                                        value = tmp[0];
                                        _pos = stoi(tmp.back());
                                        cout << "pos: " << _pos << endl;
                                    }
                                    
                                    args.push_back(Argument(argnum, name, value));
                                    value = "";
                                }
                                _pos++;
                            }
                            functions[tokens[line][pos + 1].value] = new Function(args);
                            for (auto i : args) {
                                cout << i.pos << " " << i.name << " " << i.value << endl;
                            }
                        }
                        cout << "Create function: " << tokens[line][pos + 1].value << endl;
                    }

                } else if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), i.value) != objects["VARIABLE"].end()) {
                    variables[i.value] = parseCode(2)[0];
                } else if (find(objects["FUNCTION"].begin(), objects["FUNCTION"].end(), i.value) != objects["FUNCTION"].end()) {
                    parseCode(0);
                }
                
            }
            }
            cout << variables["out1"] << endl;
            cout << variables["out2"] << endl;
            return 0;
        }
        
};
