#pragma once
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include "../libs/strlib.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include "CandyObj/FunctionObj.hpp"
#include "CandyObj/VariableObj.hpp"
#include "CandyObj/ListObj.hpp"

using namespace std;
class Parser {
    map<int, vector<Token>> tokens = {};
    int line = 0;

    map<string, vector<string>> objects = { {"FUNCTION", {"function"}}, {"VARIABLE", {"__file__"}} };
    map<string, string> scope = {};
    vector<string> keywords = { "if", "for", "while", "in", "import", "return", "func" };
    vector<string> booloperators = { "==", ">=", "<=", ">", "<" };
    vector<string> mathoperators = { "+", "-", "*", "/", "%" };
    vector<string> boolean = { "false", "true" };
    map<string, Variable*> variables = {};
    map<string, Function*> functions = {};


    bool match(string type, int _pos) {
        if (_pos < tokens[line].size()) {
            if (tokens[line][_pos + 1].type == type) {
                return true;
            }
        }
        return false;
    }

    void createVariable(string name, string value, string type) {
        if (variables.find(name) != variables.end()) {
            delete variables[name];
        }
        variables[name] = new Variable(type, value);
    }

    void clearVariable(string name) {
        variables[name]->value = "";
        variables[name]->type = "";
    }

    void eraseVariable(string name) {
        if (variables.find(name) != variables.end()) {
            delete variables[name];
        }
        variables.erase(variables.find(name));
    }

    bool tokenInLine(int ln, string token) {
        for (auto i : tokens[ln]) {
            if (i.type == token) {
                return true;
            }
        }
        return false;
    }

public:
    Parser(map<int, vector<Token>> _tokens, string _file) {
        tokens = _tokens;
        createVariable("__file__", _file, "STRING");
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
        vector<string> ret = { "novalue", "NULL"};
        auto vec = functions[fname]->tokenpos;
        ret = executeCode(vec[0], vec[1], vec[2], true);
        return ret;
    }

    string formatNumber(string s) {
        if (s[s.size() - 1] == '0')
            for (size_t i = s.size() - 1; s[i] == '0'; i--)
                s.erase(i, 1);

        if (s[s.size() - 1] == '.')
            s.erase(s.size() - 1, 1);
        return s;
    }

    vector<string> processNumber(int pos) {
        vector<string> numtokens;
        vector<string> tmp;
        string __tmp;
        int rb = 0;

        while (pos < tokens[line].size() && (tokens[line][pos].type == "NUMBER" || tokens[line][pos].type == "UNDEFINED_STRING" || find(mathoperators.begin(), mathoperators.end(), tokens[line][pos].value) != mathoperators.end() || (tokens[line][pos].type == "LEFT_BRACKET" || (rb != 0 && tokens[line][pos].type == "RIGHT_BRACKET")))) {
            if (tokens[line][pos].type == "LEFT_BRACKET") {
                __tmp = tokens[line][pos].value;
                rb++;
            }
            else if (tokens[line][pos].type == "RIGHT_BRACKET") {
                __tmp = tokens[line][pos].value;
                rb--;
            }
            else if (tokens[line][pos].type == "NUMBER") {
                auto _tmp = parseNumber(pos);
                __tmp = _tmp[0];
                pos = stoi(_tmp.back());
            }
            else if ((tokens[line][pos].type == "UNDEFINED_STRING" && find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), tokens[line][pos].value) != objects["VARIABLE"].end())) {
                if (variables[tokens[line][pos].value]->type == "NUMBER") {
                    __tmp = variables[tokens[line][pos].value]->value;
                }
                else {
                    cout << format("Exception on pos: %d:%d\nUnnable to combine NUMBER and %s", line + 1, pos, variables[tokens[line][pos].value]->type);
                    exit(-1);
                }
            }
            else {
                __tmp = tokens[line][pos].value;
            }
            numtokens.push_back(__tmp);
            pos++;
        }
        string i = numtokens[0];
        int _pos = 0;

        while (_pos + 1 < numtokens.size()) {
            if (numtokens[_pos + 1] == "+") {
                i = formatNumber(to_string(stold(i) + stold(numtokens[_pos + 2])));
            }

            if (numtokens[_pos + 1] == "-") {
                i = formatNumber(to_string(stold(i) - stold(numtokens[_pos + 2])));
            }

            if (numtokens[_pos + 1] == "*") {
                i = formatNumber(to_string(stold(i) * stold(numtokens[_pos + 2])));
            }

            if (numtokens[_pos + 1] == "/") {
                i = formatNumber(to_string(stold(i) / stold(numtokens[_pos + 2])));
            }
            _pos++;
        }
        tmp.push_back(i);
        tmp.push_back("NUMBER");
        tmp.push_back(to_string(pos));
        return tmp;
    }

    vector<string> parseNumber(int pos) {
        vector<string> floattmp = { "" };

        while (pos < tokens[line].size() && (tokens[line][pos].type == "NUMBER" || tokens[line][pos].type == "DOT")) {
            floattmp[0] += tokens[line][pos].value;
            pos++;
        }
        floattmp.push_back(to_string(pos - 1));
        return floattmp;
    }

    string processBoolean(vector<string> value, int pos, int pos_ex) {
        if (value[1] == value[3]) {
            if (match("ASSIGN", pos) && match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return boolean[stold(value[0]) == stold(value[2])];
                }
                else {
                    return boolean[value[0] == value[2]];
                }
            }
            else if (match("RIGHT_INEQUALITY_BRACKET", pos) && match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return boolean[stold(value[0]) >= stold(value[2])];
                }
                else if (value[1] == "STRING") {
                    return boolean[value[0].length() >= value[2].length()];
                }
            }
            else if (match("LEFT_INEQUALITY_BRACKET", pos) && match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return boolean[stold(value[0]) <= stold(value[2])];
                }
                else if (value[1] == "STRING") {
                    return boolean[value[0].length() <= value[2].length()];
                }
            }
            else if (match("RIGHT_INEQUALITY_BRACKET", pos) && !match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return boolean[stold(value[0]) > stold(value[2])];
                }
                else if (value[1] == "STRING") {
                    return boolean[value[0].length() > value[2].length()];
                }
            }
            else if (match("LEFT_INEQUALITY_BRACKET", pos) && !match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return boolean[stold(value[0]) < stold(value[2])];
                }
                else if (value[1] == "STRING") {
                    return boolean[value[0].length() < value[2].length()];
                }
            }
        }
        else {
            cout << "Exception on pos: " << line + 1 << ":" << pos_ex << "\nThere is no such comparison operator for: " << value[1] << " and " << value[3] << "." << endl;
            exit(-1);
        }
        return boolean[0];
    }

    vector<string> parseCode(int pos) {
        if (tokens[line][pos].type == "QUOTE" && match("UNDEFINED_STRING", pos) && match("QUOTE", pos + 1)) {
            pos += 1;
        }


        auto i = tokens[line][pos];
        vector<string> tmp = {};
        if (i.type == "NUMBER") {
            auto rettype = i.type;
            auto _tmp = parseNumber(pos);
            auto ret = _tmp[0];
            pos = stoi(_tmp[1]);

            if (pos + 2 < tokens[line].size() && find(booloperators.begin(), booloperators.end(), tokens[line][pos + 1].value + tokens[line][pos + 2].value) != booloperators.end()) {
                ret = processBoolean({ ret, rettype, parseCode(pos + 3)[0], parseCode(pos + 3)[1] }, pos, pos + 3);
                rettype = "BOOLEAN";
            }

            else if (pos + 1 < tokens[line].size() && find(booloperators.begin(), booloperators.end(), tokens[line][pos + 1].value) != booloperators.end()) {
                ret = processBoolean({ ret, rettype, parseCode(pos + 2)[0], parseCode(pos + 2)[1] }, pos, pos + 2);
                rettype = "BOOLEAN";
            }

            tmp.push_back(ret);
            tmp.push_back(rettype);
            tmp.push_back(to_string(pos));
            if (pos + 1 < tokens[line].size()) {
                if (find(mathoperators.begin(), mathoperators.end(), tokens[line][pos + 1].value) != mathoperators.end()) {
                    tmp = processNumber(pos);
                }
            }
            
            return tmp;

        }
        else if (match("QUOTE", pos - 2) && find(objects["STRING"].begin(), objects["STRING"].end(), i.value) != objects["STRING"].end() && match("QUOTE", pos)) {
            auto ret = i.value;
            string rettype = "STRING";

            if (pos + 3 < tokens[line].size() && find(booloperators.begin(), booloperators.end(), tokens[line][pos + 2].value + tokens[line][pos + 3].value) != booloperators.end()) {
                ret = processBoolean({ ret, rettype, parseCode(pos + 4)[0], parseCode(pos + 4)[1] }, pos + 1, pos + 4);
                rettype = "BOOLEAN";
            }

            else if (pos + 1 < tokens[line].size() && find(booloperators.begin(), booloperators.end(), tokens[line][pos + 2].value) != booloperators.end()) {
                ret = processBoolean({ ret, rettype, parseCode(pos + 3)[0], parseCode(pos + 3)[1] }, pos + 1, pos + 3);
                rettype = "BOOLEAN";
            }
            tmp.push_back(ret);
            tmp.push_back(rettype);
            tmp.push_back(to_string(pos));
            return tmp;

        }
        else if (find(objects["FUNCTION"].begin(), objects["FUNCTION"].end(), i.value) != objects["FUNCTION"].end()) {
            vector<string> _tmp = {};

            if (match("LEFT_BRACKET", pos)) {

                if (match("UNDEFINED_STRING", pos + 1) || match("NUMBER", pos + 1) || match("QUOTE", pos + 1) || match("LEFT_BRACKET", pos + 1)) {
                    pos++;
                    while (pos + 1 < tokens[line].size() && tokens[line][pos].type != "RIGHT_BRACKET") {
                        _tmp = parseCode(pos + 1);
                        pos = stoll(_tmp.back());
                        _tmp.pop_back();
                        if (_tmp[0] != "null") { 
                            tmp.push_back(_tmp[0]);
                            tmp.push_back(_tmp[1]);
                             }

                    }
                    if (match("COMA", pos)) {
                        pos++;
                    }

                    if (tmp.size() == 0) { tmp.push_back("novalue"); }
                    tmp = exec(i.value, tmp);
                }
                else { tmp = exec(i.value, { "novalue" }); }

            }
            if (tmp.size() == 0) { tmp.push_back("novalue"); }
            tmp.push_back(to_string(pos));
            return tmp;

        }
        else if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), i.value) != objects["VARIABLE"].end()) {
            if (pos + 1 < tokens[line].size() && find(mathoperators.begin(), mathoperators.end(), tokens[line][pos + 1].value) != mathoperators.end() && match("NUMBER", pos + 1)) {
                if (match("LEFT_BRACKET", pos - 2) && !match("UNDEFINED_STRING", pos - 3)) {
                    pos--;
                }
                auto rttmp = processNumber(pos);
                tmp.push_back(rttmp[0]);
                tmp.push_back(rttmp.back());
            }
            else {
                tmp.push_back(variables[i.value]->value);
                tmp.push_back(variables[i.value]->type);
                tmp.push_back(to_string(pos));
            }
            return tmp;

        }
        else if (i.type == "LEFT_BRACKET") {
            vector<string> rttmp;
            if (match("NUMBER", pos)) {
                rttmp = processNumber((pos));
            }
            else {
                rttmp = parseCode(pos + 1);
            }
            tmp.push_back(rttmp[0]);
            tmp.push_back(rttmp.back());
        }
        else {
            tmp.push_back("null");
            tmp.push_back("NULL");
            tmp.push_back(to_string(pos));
        }
        return tmp;
    }

    void runCode() {
        executeCode(0, 0, prev(tokens.end())->first, false);
    }

    vector<string> executeCode(int startline, int starttoken, int endline, bool isfunc) {
        int pos = starttoken;
        vector<string> ret = {"novalue", "NULL"};
        for (line = startline; line < endline + 1; line++) {
            if (tokens[line].size() == 0) {
                continue;
            }
            else if (tokens[line].size() == 1 && tokens[line][0].type == "HASHTAG") {
                continue;
            }
            var_preprocess();
            string_preprocess();
            auto i = tokens[line][pos];
            if (i.type == "UNDEFINED_STRING") {
                if (find(keywords.begin(), keywords.end(), i.value) != keywords.end()) {

                    if (i.value == "func" && match("UNDEFINED_STRING", pos)) {
                        objects["FUNCTION"].push_back(tokens[line][pos + 1].value);
                        if (match("LEFT_BRACKET", pos + 1)) {
                            cout << "Create function: " << tokens[line][pos + 1].value << endl;
                            vector<Argument> args = {};
                            string value;
                            string name;
                            string type;
                            int argnum = 0;
                            int _pos = pos + 2;
                            while (_pos < tokens[line].size() && tokens[line][_pos].type != "RIGHT_BRACKET") {

                                if (tokens[line][_pos].type == "UNDEFINED_STRING" || tokens[line][_pos].type == "NUMBER") {
                                    argnum++;
                                    
                                    if (match("ASSIGN", _pos)) {
                                        name = tokens[line][_pos].value;
                                        auto tmp = parseCode(_pos + 2);
                                        value = tmp[0];
                                        type = tmp[1];
                                        _pos = stoi(tmp.back());
                                        cout << "pos: " << _pos << endl;
                                    }
                                    else {
                                        auto tmp = parseCode(_pos);
                                        value = tmp[0];
                                        type = tmp[1];
                                        _pos = stoi(tmp.back());
                                    }

                                    args.push_back(Argument(argnum, name, value, type));
                                    value = "";
                                    name = "";
                                }
                                _pos++;
                            }
                            
                            int _line = line;
                            if (!tokenInLine(line, "LEFT_FIGURE_BRACKET")) {
                                _line++;
                                while (true) {
                                    if (tokens[_line].size() > 0) {
                                        if (tokens[_line][0].type == "LEFT_FIGURE_BRACKET" && tokens[_line][0].type != "HASHTAG") {
                                            break;
                                        }
                                    }
                                    _line++;
                                }
                            }
                            int brackets = 0;
                            int _startline = 0;
                            int _starttoken = 0;
                            int _endline = 0;
                            
                            if (_pos + 2 < tokens[line].size()) {
                                _startline = _line;
                                _starttoken = _pos + 2;
                                cout << "yes" << endl;
                            } else {
                                _startline = _line;
                                if (_startline == line) {
                                    _startline++;
                                }
                                if (tokens[_startline][0].type == "LEFT_FIGURE_BRACKET" && tokens[_startline].size() > 1) {
                                    _starttoken++;
                                }
                            }
                            
                            do {
                                for (auto token : tokens[_line]) {
                                    if (token.type == "RIGHT_FIGURE_BRACKET") {
                                        brackets--;
                                    } else if (token.type == "LEFT_FIGURE_BRACKET") {
                                        brackets++;
                                    }
                                }
                                if (brackets != 0) {
                                    _line++;
                                }
                                
                            } while (brackets != 0);
                            _endline = _line;
                            cout << format("%d %d %d", _startline, _starttoken, _endline) << endl;

                            functions[tokens[line][pos + 1].value] = new Function(args, {_startline, _starttoken, _endline});
                            line = _line;
                            
                            cout << args.size() << endl;
                            for (auto i : args) {
                                cout << i.pos << " " << i.name << " " << i.value << endl;
                            }
                        }
                    }
                    else if (i.value == "return") {
                        cout << "ret yes" << endl;
                        if (isfunc && tokens[line].size() > 1) {
                            auto tmp = parseCode(pos + 1);
                            ret = {tmp[0], tmp[1]};
                        }
                    }

                }
                else if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), i.value) != objects["VARIABLE"].end()) {
                    auto tmp = parseCode(pos + 2);
                    createVariable(i.value, tmp[0], tmp[1]);
                }
                else if (find(objects["FUNCTION"].begin(), objects["FUNCTION"].end(), i.value) != objects["FUNCTION"].end()) {
                    parseCode(pos);
                }
                else {
                    
                }

                if (variables.find("out") != variables.end()) {
                cout << variables["out"]->value << endl;
                eraseVariable("out");
                }
            }
            pos = 0;
        }
        return ret;
    }
};
