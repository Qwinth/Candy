#pragma once
#include "CandyObj/FunctionObj.hpp"
#include "CandyObj/VariableObj.hpp"
#include "CandyObj/ListObj.hpp"
#include "CandyObj/BytesObj.hpp"
using namespace std;

class Parser {
    map<int, vector<Token>> tokens = {};
    int line = 0;
    map<string, vector<string>> objects = {{"FUNCTION", {}}, {"VARIABLE", {}}};
    vector<string> keywords = { "if", "else", "for", "while", "continue", "break", "in", "import", "return", "func" };
    vector<string> booloperators = { "==", "!=", ">=", "<=", ">", "<" };
    vector<string> mathoperators = { "+", "-", "*", "/", "%" };
    vector<string> boolean = { "false", "true" };
    map<char, string> control_characters = { {'0', "\0"}, {'a', "\a"}, {'b', "\b"}, {'t', "\t"}, {'n', "\n"}, {'v', "\v"}, {'f', "\f"}, {'r', "\r"}, {'\\', "\\"} };
    map<string, Variable*> variables = {};
    map<string, Function*> functions = {};
    map<string, Bytes*> bytesArrays = {};
    vector<string> system_functions = { "out", "outln", "input", "len", "type", "parseInt", "parseString", "parseBool", "exec", "getarg", "random", "exit", "quit", "time", "sqrt", "system"};
    vector<string> argv = {};


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
        if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), name) == objects["VARIABLE"].end()) { objects["VARIABLE"].push_back(name); }   
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
        objects["VARIABLE"].erase(find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), name));
    }

    bool tokenInLine(int ln, string token) {
        for (auto i : tokens[ln]) {
            if (i.type == token) {
                return true;
            }
        }
        return false;
    }

    string to_stringWp(long double val, int n) {
        ostringstream out;
        out.precision(n);
        out << fixed << val;
        return out.str();
    }

public:
    Parser(map<int, vector<Token>> _tokens, char * _argv[], int argc) {
        tokens = _tokens;
        argv = vector<string>(_argv, _argv + argc);
        argv.erase(argv.begin());
        createVariable("__file__", argv[0], "STRING");
        objects["FUNCTION"] = system_functions;
    }

    void var_preprocess() {
        for (int pos = 0; pos < tokens[line].size(); pos++) {
            Token i = tokens[line][pos];
            if (i.type == "UNDEFINED_STRING" && find(keywords.begin(), keywords.end(), i.value) == keywords.end() && find(boolean.begin(), boolean.end(), i.value) == boolean.end()) {
                if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), i.value) == objects["VARIABLE"].end()) {
                    if (match("ASSIGN", pos) && !match("ASSIGN", pos + 1)) {
                        objects["VARIABLE"].push_back(i.value);
                    }
                }
            }
        }
    }

    vector<string> execsf(string fname, vector<string> args) {
        if (args.size() == 0) {
            args = {"", "STRING"};
        }
        vector<string> ret = {"novalue", "NULL"};
        if (fname == "out") {
            string outdata = args[0];
            for (int i = 2; i < args.size(); i += 2) {
                outdata += " " + args[i];
            }
            cout << outdata;
            cout.flush();
        }
        else if (fname == "outln") {
            string outdata = args[0];
            for (int i = 2; i < args.size(); i += 2) {
                outdata += " " + args[i];
            }
            cout << outdata << endl;
        }
        else if (fname == "input") {
            string outdata = args[0];
            string indata;
            cout << outdata;
            cout.flush();
            getline(cin, indata);
            ret = {indata, "STRING"};
        }
        else if (fname == "len") {
            if (args[1] == "STRING") {
                
                ret = {to_string(count_if(args[0].begin(), args[0].end(), [](char c) { return (static_cast<unsigned char>(c) & 0xC0) != 0x80; } )), "NUMBER"};
            }
        }
        else if (fname == "type") {
            ret = {args[1], "TYPENAME"};
        }
        else if (fname == "parseInt" && ((args[1] == "STRING" && args[0].length() > 0) || args[1] == "BOOLEAN")) {
            if (all_of(args[0].begin(), args[0].end(), ::isdigit)) {
                ret = {args[0], "NUMBER"};
            }
            else if (args[0][0] == '-' && all_of(args[0].begin() + 1, args[0].end(), ::isdigit)) {
                ret = {args[0], "NUMBER"};
            }
            else if (find(boolean.begin(), boolean.end(), args[0]) != boolean.end() && args[1] == "BOOLEAN") {
                if (args[0] == "true") {
                    ret = {"1", "NUMBER"};
                }
                else {
                    ret = {"0", "NUMBER"};
                }
            }
        }
        else if (fname == "parseString") {
            ret = {args[0], "STRING"};
        }
        else if (fname == "parseBool") {
            if (args[0] == "true" || args[0] == "false") {
                ret = {args[0], "BOOLEAN"};
            }
            else if (args[1] == "NUMBER" && (args[0] == "1" || args[0] == "0")) {
                if (args[0] == "1") {
                    ret = {"true", "BOOLEAN"};
                }
                else {
                    ret = {"false", "BOOLEAN"};
                }
            }
            
        }
        else if (fname == "getarg") {
            if (args[1] == "NUMBER" && stoi(args[0]) < argv.size()) {
                ret = { argv[stoi(args[0])], "STRING" };
            }
        }
        else if (fname == "random") {
            if (args[1] == "NUMBER") {
                random_device rd;

                mt19937 rng(rd());
                if (args.size() == 2) {
                    uniform_int_distribution<int> uni(0, stoi(args[0]));
                    ret = { to_string(uni(rng)), "NUMBER" };
                }
                else if (stoi(args[0]) < stoi(args[2])) {
                    uniform_int_distribution<int> uni(stoi(args[0]), stoi(args[2]));
                    ret = { to_string(uni(rng)), "NUMBER"};
                }
            }
        }
        else if ((fname == "exit" || fname == "quit") && args[1] == "NUMBER") {
            exit(stoi(args[0]));
        }
        else if (fname == "time") {
            ret = { to_stringWp(chrono::duration_cast<chrono::duration<double>>(chrono::system_clock::now().time_since_epoch()).count(), 15), "NUMBER"};
        }
        else if (fname == "sqrt" && args[1] == "NUMBER") {
            ret = { formatNumber(to_stringWp(sqrt(stold(args[0])), 15)), "NUMBER" };
        }
        else if (fname == "system" && args[1] == "STRING") {
            ret = { to_string(system(args[0].c_str())), "NUMBER" };
        }
        return ret;
    }

    vector<string> exec(string fname, vector<string> args) {
        vector<string> ret = { "novalue", "NULL"};
         /*cout << "start args counting" << endl;
         for ( auto i : args ) {
             cout << fname << " " << i << endl;
         }
         cout << "stop args counting" << endl;*/
        if (find(system_functions.begin(), system_functions.end(), fname) != system_functions.end()) {
            ret = execsf(fname, args);
        }
        else {
            auto vec = functions[fname]->tokenpos;

            int _line = line;
            ret = executeCode(vec[0], vec[1], vec[2], true);
            line = _line;
        }
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

    vector<string> processNumber(string startnum, int pos) {
        vector<string> numtokens;
        vector<string> tmp;
        string __tmp;
        pos++;

        numtokens.push_back(startnum);

        while (pos < tokens[line].size() && (tokens[line][pos].type == "NUMBER" || tokens[line][pos].type == "UNDEFINED_STRING" || find(mathoperators.begin(), mathoperators.end(), tokens[line][pos].value) != mathoperators.end() || tokens[line][pos].type == "LEFT_BRACKET")) {
            if (tokens[line][pos].type == "LEFT_BRACKET") {
                vector<string> _tmp = parseCode(pos);
                __tmp = _tmp[0];
                pos = stoi(_tmp.back());
            }
            else if (tokens[line][pos].type == "NUMBER") {
                auto _tmp = _parseCode(pos);
                __tmp = _tmp[0];
                pos = stoi(_tmp.back());
            }
            else if (tokens[line][pos].type == "MINUS" && find(mathoperators.begin(), mathoperators.end(), tokens[line][pos - 1].value) != mathoperators.end()) {
                auto _tmp = _parseCode(pos);
                __tmp = _tmp[0];
                pos = stoi(_tmp.back());
            }
            else if (tokens[line][pos].type == "MULTIPLICATION" && match("MULTIPLICATION", pos)) {
                __tmp = "**";
                pos++;
            }
            else if (tokens[line][pos].type == "UNDEFINED_STRING" && find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), tokens[line][pos].value) != objects["VARIABLE"].end()) {
                auto _tmp = _parseCode(pos);
                if (_tmp[1] == "NUMBER") {
                    __tmp = _tmp[0];
                }
                else {
                    cout << strformat("Exception on pos: %d:%d\nUnnable to combine NUMBER and %s\n", line + 1, tokens[line][pos].pos, variables[tokens[line][pos].value]->type.c_str());
                    exit(-1);
                }
            }
            else if (tokens[line][pos].type == "UNDEFINED_STRING" && find(objects["FUNCTION"].begin(), objects["FUNCTION"].end(), tokens[line][pos].value) != objects["FUNCTION"].end()) {
                auto _tmp = _parseCode(pos);
                if (_tmp[1] == "NUMBER") {
                    __tmp = _tmp[0];
                }
                pos = stoi(_tmp.back());
                if (tokens[line][pos].type == "COMA") {
                    pos--;
                }
            }
            else {
                __tmp = tokens[line][pos].value;
            }

            numtokens.push_back(__tmp);
            pos++;
            
        }

        if (pos < tokens[line].size() && tokens[line][pos].type == "COMA") {
            pos++;
        }

        for (int i = 0; i < numtokens.size(); i++) {
            if (numtokens[i] == "(" || numtokens[i] == ")") {
                numtokens.erase(numtokens.begin() + i);
            }
        }

        string i = numtokens[0];
        int _pos = 0;

        while (_pos + 1 < numtokens.size()) {
            if (numtokens[_pos + 1] == "+") {
                i = formatNumber(to_stringWp(stold(i) + stold(numtokens[_pos + 2]), 15));
            }

            if (numtokens[_pos + 1] == "-") {
                i = formatNumber(to_stringWp(stold(i) - stold(numtokens[_pos + 2]), 15));
            }

            if (numtokens[_pos + 1] == "*") {
                i = formatNumber(to_stringWp(stold(i) * stold(numtokens[_pos + 2]), 15));
            }

            if (numtokens[_pos + 1] == "**") {
                long double operand = stold(i);
                i = formatNumber(to_stringWp(pow(operand, stold(numtokens[_pos + 2])), 15));
            }

            if (numtokens[_pos + 1] == "/") {
                i = formatNumber(to_stringWp(stold(i) / stold(numtokens[_pos + 2]), 15));
            }

            if (numtokens[_pos + 1] == "%") {
                i = formatNumber(to_stringWp(fmod(stold(i), stold(numtokens[_pos + 2])), 15));
            }
            _pos++;
        }
        tmp.push_back(i);
        tmp.push_back("NUMBER");
        tmp.push_back(to_string(pos - 1));
        return tmp;
    }

    vector<string> parseNumber(int pos) {
        vector<string> floattmp = { "" };

        while (pos < tokens[line].size() && (tokens[line][pos].type == "NUMBER" || tokens[line][pos].type == "DOT" || tolower(tokens[line][pos].value[0]) == 'x')) {
            floattmp[0] += tokens[line][pos].value;
            pos++;
        }
        floattmp.push_back(to_string(pos - 1));
        return floattmp;
    }

    vector<string> processBoolean(vector<string> value, int pos, int pos_ex) {
        if (value[1] == value[3]) {
            if (match("ASSIGN", pos) && match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return {boolean[stold(value[0]) == stold(value[2])], "BOOLEAN", to_string(pos_ex)};
                }
                else {
                    return {boolean[value[0] == value[2]], "BOOLEAN", to_string(pos_ex)};
                }
            }
            else if (match("EXCLAMATION_MARK", pos) && match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return {boolean[stold(value[0]) != stold(value[2])], "BOOLEAN", to_string(pos_ex)};
                }
                else {
                    return {boolean[value[0] != value[2]], "BOOLEAN", to_string(pos_ex)};
                }
            }
            else if (match("RIGHT_INEQUALITY_BRACKET", pos) && match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return {boolean[stold(value[0]) >= stold(value[2])], "BOOLEAN", to_string(pos_ex)};
                }
                else if (value[1] == "STRING") {
                    return {boolean[value[0].length() >= value[2].length()], "BOOLEAN", to_string(pos_ex)};
                }
            }
            else if (match("LEFT_INEQUALITY_BRACKET", pos) && match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return {boolean[stold(value[0]) <= stold(value[2])], "BOOLEAN", to_string(pos_ex)};
                }
                else if (value[1] == "STRING") {
                    return {boolean[value[0].length() <= value[2].length()], "BOOLEAN", to_string(pos_ex)};
                }
            }
            else if (match("RIGHT_INEQUALITY_BRACKET", pos) && !match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return {boolean[stold(value[0]) > stold(value[2])], "BOOLEAN", to_string(pos_ex)};
                }
                else if (value[1] == "STRING") {
                    return {boolean[value[0].length() > value[2].length()], "BOOLEAN", to_string(pos_ex)};
                }
            }
            else if (match("LEFT_INEQUALITY_BRACKET", pos) && !match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return {boolean[stold(value[0]) < stold(value[2])], "BOOLEAN", to_string(pos_ex)};
                }
                else if (value[1] == "STRING") {
                    return {boolean[value[0].length() < value[2].length()], "BOOLEAN", to_string(pos_ex)};
                }
            }
        } 
        else if (match("EXCLAMATION_MARK", pos) && match("ASSIGN", pos + 1)) {
            return {boolean[1], "BOOLEAN", to_string(pos_ex)};
        }

        return {boolean[0], "BOOLEAN", to_string(pos_ex)};
    }

    vector<int> parseBrackets(int pos) {
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

        if (pos + 2 < tokens[line].size()) {
            _startline = _line;
            _starttoken = pos + 2;
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
        return {_startline, _starttoken, _endline};
    }

    vector<string> _parseCode(int pos) {
        vector<string> tmp = {};
        string arg;
        auto i = tokens[line][pos];

        if (i.type == "NUMBER") {
            auto _tmp = parseNumber(pos);
            string ret = _tmp[0];

            if (tolower(_tmp[0][1]) == 'x') {
                ret = to_string(stoll(_tmp[0], 0, 15));
            }

            return {ret, i.type, arg, _tmp.back()};

        }
        else if (i.type == "STRING") {
            auto ret = i.value;
            string rettype = i.type;
            arg = "new";
            tmp.push_back(ret);
            tmp.push_back(rettype);
            tmp.push_back(arg);
            tmp.push_back(to_string(pos));
            return tmp;

        }
        else if (find(objects["FUNCTION"].begin(), objects["FUNCTION"].end(), i.value) != objects["FUNCTION"].end()) {
            vector<string> _tmp = {};
            int brackets = 0;
            if (match("LEFT_BRACKET", pos)) {
                int num = 1;
                string name = tokens[line][pos].value;
                brackets++;
                pos++;
                if (match("UNDEFINED_STRING", pos) || match("NUMBER", pos) || match("MINUS", pos) || match("STRING", pos) || match("LEFT_BRACKET", pos)) {
                    do {
                        _tmp = parseCode(pos + 1);
                        pos = stoi(_tmp.back());
                        _tmp.pop_back();

                        if (_tmp[0] != "null") { 
                            tmp.push_back(_tmp[0]);
                            tmp.push_back(_tmp[1]);
                        }
                        
                        if (tokens[line][pos].type == "RIGHT_BRACKET" && brackets > 0) {
                            brackets--;
                        }
                        
                    } while (pos + 1 < tokens[line].size() && (tokens[line][pos].type != "RIGHT_BRACKET" || brackets > 0));

                    if (match("COMA", pos) || match("RIGHT_BRACKET", pos)) {
                        pos++;
                        arg = "pos+";
                    }
                    
                    tmp = exec(i.value, tmp);
                }
                else {
                    tmp = exec(i.value, { "", "NULL" });
                    pos++;
                    if (match("COMA", pos) || match("RIGHT_BRACKET", pos)) {
                        pos++;
                        arg = "pos+";
                    }
                }

            } else {
                tmp = {i.value, "FUNCTION"};
            }

            tmp.push_back(arg);
            tmp.push_back(to_string(pos));
            return tmp;

        }
        else if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), i.value) != objects["VARIABLE"].end()) {
            tmp.push_back(variables[i.value]->value);
            tmp.push_back(variables[i.value]->type);
            tmp.push_back(arg);
            tmp.push_back(to_string(pos));
            return tmp;

        }
        else if (i.type == "LEFT_BRACKET") {
            vector<string> rttmp;
            if (match("NUMBER", pos) && find(mathoperators.begin(), mathoperators.end(), tokens[line][pos + 1].value) != mathoperators.end()) {
                rttmp = processNumber(tokens[line][pos].value, pos);
            }
            else {
                rttmp = _parseCode(pos + 1);
            }
            tmp.push_back(rttmp[0]);
            tmp.push_back(rttmp[1]);
            tmp.push_back(arg);
            tmp.push_back(rttmp.back());
        }
        else if (i.type == "UNDEFINED_STRING" && find(boolean.begin(), boolean.end(), i.value) != boolean.end()) {
            return {i.value, "BOOLEAN", arg, to_string(pos)};
        }
        else if (i.type == "MINUS" && (tokens[line][pos + 1].type == "UNDEFINED_STRING" || tokens[line][pos + 1].type == "NUMBER" || tokens[line][pos + 1].type == "LEFT_BRACKET" || tokens[line][pos + 1].type == "MINUS")) {
            vector<string> _tmp;
            if (tokens[line][pos + 1].type == "LEFT_BRACKET") {
                _tmp = parseCode(pos + 1);
            }
            else {_tmp = _parseCode(pos + 1);}
            tmp.push_back(formatNumber(to_stringWp(-stold(_tmp[0]), 15)));
            tmp.push_back(_tmp[1]);
            tmp.push_back(arg);
            tmp.push_back(_tmp.back());
        }
        else if (i.type == "UNDEFINED_STRING") {
            cout << strformat("Name '%s' is not defined", i.value.c_str()) << endl;
            exit(-1);
        }
        else {
            tmp.push_back("null");
            tmp.push_back("NULL");
            tmp.push_back(arg);
            tmp.push_back(to_string(pos));
        }
        return tmp;
    }

    vector<string> parseCode(int pos, bool wh = true) {
        int brackets = 0;
        while (tokens[line][pos].type == "LEFT_BRACKET") {
            brackets++;
            pos++;
        }
        auto tmp = _parseCode(pos);
        string name = tokens[line][pos].value;
        Token starttoken = tokens[line][pos];
        pos = stoi(tmp.back());
        
        string ret = tmp[0];

        string rettype = tmp[1];
        string arg;

        if (tmp.size() > 3) {
            arg = tmp[2];
        }

        do {
            if (rettype == "STRING" && arg == "new") {
                for (int _tmp = 0; _tmp < ret.length(); _tmp++) {
                    if (ret[_tmp] == '\\' && control_characters.find(ret[_tmp + 1]) != control_characters.end()) {
                        ret.replace(_tmp, 2, control_characters[ret[_tmp + 1]]);
                    }
                    else if (ret[_tmp] == '\\') {
                        ret.erase(_tmp, 1);
                    }
                }
            }

            if (tokens[line][pos].type == "RIGHT_BRACKET" && brackets > 0 && (find(objects["FUNCTION"].begin(), objects["FUNCTION"].end(), starttoken.value) == objects["FUNCTION"].end() && starttoken.type != "STRING")) {
                brackets--;
            }

            if (pos + 2 < tokens[line].size() && tokens[line][pos].type == "UNDEFINED_STRING" && tokens[line][pos + 1].type != "ASSIGN" && ((find(mathoperators.begin(), mathoperators.end(), tokens[line][pos + 1].value) != mathoperators.end() && match("ASSIGN", pos + 1)) || (match("MULTIPLICATION", pos) && match("MULTIPLICATION", pos + 1) && match("ASSIGN", pos + 2)) || (tokens[line][pos + 1].type == tokens[line][pos + 2].type && (match("PLUS", pos) || match("MINUS", pos))))) {
                if (match("PLUS", pos) && match("PLUS", pos + 1)) {
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(stold(ret) + 1, 15));
                    pos += 2;
                }
                else if (match("MINUS", pos) && match("MINUS", pos + 1)) {
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(stold(ret) - 1, 15));
                    pos += 2;
                }
                else if (match("PLUS", pos) && match("ASSIGN", pos + 1)) {
                    vector<string> operand = parseNumber(pos + 3);
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(stold(ret) + stold(processNumber(operand[0], stoi(operand.back()))[0]), 15));
                    pos = stoi(operand[1]);
                }
                else if (match("MINUS", pos) && match("ASSIGN", pos + 1)) {
                    vector<string> operand = parseNumber(pos + 3);
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(stold(ret) - stold(processNumber(operand[0], stoi(operand.back()))[0]), 15));
                    pos = stoi(operand[1]);
                }
                else if (match("MULTIPLICATION", pos) && match("ASSIGN", pos + 1)) {
                    vector<string> operand = parseNumber(pos + 3);
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(stold(ret) * stold(processNumber(operand[0], stoi(operand.back()))[0]), 15));
                    pos = stoi(operand[1]);
                }
                else if (match("MULTIPLICATION", pos) && match("MULTIPLICATION", pos + 1) && match("ASSIGN", pos + 2)) {
                    vector<string> operand = parseNumber(pos + 4);
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(pow(stold(ret), stold(processNumber(operand[0], stoi(operand.back()))[0])), 15));
                    pos = stoi(operand[1]);
                }
                else if (match("DIVISION", pos) && match("ASSIGN", pos + 1)) {
                    vector<string> operand = parseNumber(pos + 3);
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(stold(ret) / stold(processNumber(operand[0], stoi(operand.back()))[0]), 15));
                    pos = stoi(operand[1]);
                }
                else if (match("PERCENT", pos) && match("ASSIGN", pos + 1)) {
                    vector<string> operand = parseNumber(pos + 3);
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(fmod(stold(ret),  stold(processNumber(operand[0], stoi(operand.back()))[0])), 15));
                    pos = stoi(operand[1]);
                }
            }

            else if (pos + 2 < tokens[line].size() && arg != "pos+" && find(mathoperators.begin(), mathoperators.end(), tokens[line][pos + 1].value) != mathoperators.end() && tokens[line][pos + 1].type != "ASSIGN") {
                if (match("LEFT_BRACKET", pos - 2) && !match("UNDEFINED_STRING", pos - 3)) {
                    pos--;
                }

                auto _tmp = processNumber(ret, pos);
                ret = _tmp[0];
                rettype = _tmp[1];
                pos = stoi(_tmp.back());
            }

            if (pos + 2 < tokens[line].size() && arg != "pos+" && find(booloperators.begin(), booloperators.end(), tokens[line][pos + 1].value + tokens[line][pos + 2].value) != booloperators.end()) {
                auto _tmp = parseCode(pos + 3, false);
                auto __tmp = processBoolean({ ret, rettype, _tmp[0], _tmp[1] }, pos, stoi(_tmp.back()));
                ret = __tmp[0];
                rettype = __tmp[1];
                pos = stoi(__tmp.back());
                
            }

            else if (pos + 1 < tokens[line].size() && arg != "pos+" && find(booloperators.begin(), booloperators.end(), tokens[line][pos + 1].value) != booloperators.end()) {
                auto _tmp = parseCode(pos + 2, false);
                auto __tmp = processBoolean({ ret, rettype, _tmp[0], _tmp[1] }, pos, stoi(_tmp.back()));
                ret = __tmp[0];
                rettype = __tmp[1];
                pos = stoi(__tmp.back());
            }

            if (wh && pos + 1 < tokens[line].size() && (!match("COMA", pos - 1) && !match("RIGHT_BRACKET", pos - 1) && !match("LEFT_FIGURE_BRACKET", pos) || brackets > 0)) {pos++;}

        } while (pos + 1 < tokens[line].size() && wh && (!match("COMA", pos - 1) && !match("RIGHT_BRACKET", pos - 1) && !match("LEFT_FIGURE_BRACKET", pos) || brackets > 0));

        return {ret, rettype, to_string(pos)};
    }

    void runCode() {
        executeCode(0, 0, prev(tokens.end())->first, false);
    }

    vector<string> executeCode(int startline, int starttoken, int endline, bool isfunc) {
        int pos = starttoken;
        vector<string> ret = {"novalue", "NULL"};
        for (line = startline; line <= endline; line++) {
            if (tokens[line].size() == 0 || tokens[line][0].type == "HASHTAG") {
                continue;
            }

            var_preprocess();
            auto i = tokens[line][pos];

            if (i.type == "UNDEFINED_STRING") {
                if (find(keywords.begin(), keywords.end(), i.value) != keywords.end()) {

                    if (i.value == "func" && match("UNDEFINED_STRING", pos)) {
                        objects["FUNCTION"].push_back(tokens[line][pos + 1].value);
                        if (match("LEFT_BRACKET", pos + 1)) {

                            vector<Argument> args = {};
                            string value;
                            string name;
                            string type;
                            int argnum = 0;
                            int _pos = pos + 2;
                            while (_pos < tokens[line].size() && tokens[line][_pos].type != "RIGHT_BRACKET") {

                                if (tokens[line][_pos].type == "STRING" || tokens[line][_pos].type == "UNDEFINED_STRING" || tokens[line][_pos].type == "NUMBER") {
                                    argnum++;
                                    
                                    if (match("ASSIGN", _pos)) {
                                        name = tokens[line][_pos].value;
                                        auto tmp = parseCode(_pos + 2);
                                        value = tmp[0];
                                        type = tmp[1];
                                        _pos = stoi(tmp.back());
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
                            auto _tmp = parseBrackets(_pos);
                            functions[tokens[line][pos + 1].value] = new Function(args, _tmp);
                            line = _tmp[2];
                        }
                    }
                    else if (i.value == "if") {
                        if (pos + 1 < tokens[line].size()) {
                            auto _tmp = parseCode(pos + 1);
                            auto poss = parseBrackets(stoi(_tmp.back()));

                            if (_tmp[0] == "true" && _tmp[1] == "BOOLEAN") {
                                ret = executeCode(poss[0], poss[1], poss[2], isfunc);
                            }
                            line = poss[2];

                            if (ret.back() == "return" || ret.back() == "break") {
                                break;
                            }
                        }
                    }
                    else if (i.value == "while") {
                        if (pos + 1 < tokens[line].size()) {
                            auto _tmp = parseCode(pos + 1);
                            auto poss = parseBrackets(stoi(_tmp.back()));
                            int oldline = line;

                            while (_tmp[0] == "true" && _tmp[1] == "BOOLEAN") {
                                ret = executeCode(poss[0], poss[1], poss[2], isfunc);
                                line = oldline;

                                if (ret.back() == "break") {
                                    ret = {"novalue", "NULL"};
                                    break;
                                }
                                else if (ret.back() == "return") {
                                    break;
                                }
                                _tmp = parseCode(pos + 1);
                            }
                            line = poss[2];
                            if (ret.back() == "return") {
                                break;
                            }
                        }
                    }
                    else if (i.value == "break") {
                        ret = { "break" };
                        break;
                    }
                    else if (i.value == "return") {
                        if (isfunc && pos + 1 < tokens[line].size()) {
                            auto tmp = parseCode(pos + 1);
                            ret = {tmp[0], tmp[1], "return"};
                            break;
                        }
                    }

                }
                else if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), i.value) != objects["VARIABLE"].end()) {
                    if (find(mathoperators.begin(), mathoperators.end(), tokens[line][pos + 2].value) == mathoperators.end() && !match("ASSIGN", pos + 1) || (match("MINUS", pos + 1) && (match("UNDEFINED_STRING", pos + 2) || match("NUMBER", pos + 2) || match("LEFT_BRACKET", pos + 2)))) {
                        auto tmp = parseCode(pos + 2);
                        createVariable(i.value, tmp[0], tmp[1]);
                    }
                    else {
                        parseCode(pos);
                    }
                }
                else if (find(objects["FUNCTION"].begin(), objects["FUNCTION"].end(), i.value) != objects["FUNCTION"].end()) {
                    parseCode(pos);
                }
            }
            pos = 0;
        }
        return ret;
    }
};
