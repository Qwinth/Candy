#pragma once
#include "CandyObj/FunctionObj.hpp"
#include "CandyObj/VariableObj.hpp"
#include "CandyObj/ListObj.hpp"
#include "CandyObj/BytesObj.hpp"
using namespace std;

class Parser {
    struct __data {
        string arg;
        string type;
    };

    struct _data {
        string arg;
        string type;
        string retarg;
        int pos;
    };

    struct _argument {
        vector<__data> list;
    };

    map<int, vector<Token>> tokens = {};
    int line = 0;
    map<string, vector<string>> objects = {{"FUNCTION", {}}, {"VARIABLE", {}}};
    vector<string> keywords = { "if", "else", "for", "while", "continue", "break", "in", "import", "return", "func" };
    vector<string> booloperators = { "==", "!=", ">=", "<=", ">", "<" };
    vector<string> mathoperators = { "+", "-", "*", "/", "%" };
    vector<string> boolean = { "false", "true" };
    map<char, string> control_characters = { {'0', "\0" }, {'a', "\a" }, {'b', "\b" }, {'t', "\t" }, {'n', "\n" }, {'v', "\v" }, {'f', "\f" }, {'r', "\r" }, {'\\', "\\" } };
    map<string, Variable*> variables = {};
    map<string, Function*> functions = {};
    map<string, Bytes*> bytesArrays = {};
    vector<string> system_functions = { "out", "outln", "input", "len", "type", "parseInt", "parseString", "parseBool", "exec", "getarg", "random", "exit", "quit", "time", "sqrt", "system" };
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

    bool tokenInLine(int ln, string type) {
        for (auto i : tokens[ln]) {
            if (i.type == type) {
                return true;
            }
        }
        return false;
    }

    int getTokenPos(int ln, string type) {
        for (int num = 0; num < tokens[ln].size(); num++) {
            if (tokens[ln][num].type == type) {
                return num;
            }
        }
        return -1;
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
            if (i.type == "UNDEFINED_TOKEN" && find(keywords.begin(), keywords.end(), i.value) == keywords.end() && find(boolean.begin(), boolean.end(), i.value) == boolean.end()) {
                if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), i.value) == objects["VARIABLE"].end()) {
                    if (match("ASSIGN", pos) && !match("ASSIGN", pos + 1)) {
                        objects["VARIABLE"].push_back(i.value);
                    }
                }
            }
        }
    }

    _data execsf(string fname, _argument args) {
        if (args.list.empty()) {
            args.list[0] = {"", "NULL"};
        }
        _data ret = {"", "NULL" };
        if (fname == "out") {
            string outdata = args.list[0].arg;
            for (int i = 1; i < args.list.size(); i += 1) {
                outdata += " " + args.list[i].arg;
            }
            cout << outdata;
            cout.flush();
        }
        else if (fname == "outln") {
            string outdata = args.list[0].arg;
            for (int i = 1; i < args.list.size(); i += 1) {
                outdata += " " + args.list[i].arg;
            }
            cout << outdata << endl;
        }
        else if (fname == "input") {
            string outdata = args.list[0].arg;
            string indata;
            cout << outdata;
            cout.flush();
            getline(cin, indata);
            ret = { indata, "STRING" };
        }
        else if (fname == "len") {
            if (args.list[0].type == "STRING") {
                
                ret = {to_string(count_if(args.list[0].arg.begin(), args.list[0].arg.end(), [](char c) { return (static_cast<unsigned char>(c) & 0xC0) != 0x80; } )), "NUMBER" };
            }
        }
        else if (fname == "type") {
            ret = { args.list[0].type, "TYPENAME" };
        }
        else if (fname == "parseInt" && ((args.list[0].type == "STRING" && args.list[0].arg.length() > 0) || args.list[0].type == "BOOLEAN")) {
            if (all_of(args.list[0].arg.begin(), args.list[0].arg.end(), ::isdigit)) {
                ret = { args.list[0].arg, "NUMBER" };
            }
            else if (args.list[0].arg[0] == '-' && all_of(args.list[0].arg.begin() + 1, args.list[0].arg.end(), ::isdigit)) {
                ret = { args.list[0].arg, "NUMBER" };
            }
            else if (find(boolean.begin(), boolean.end(), args.list[0].arg) != boolean.end() && args.list[0].type == "BOOLEAN") {
                if (args.list[0].arg == "true") {
                    ret = { "1", "NUMBER" };
                }
                else {
                    ret = { "0", "NUMBER" };
                }
            }
        }
        else if (fname == "parseString") {
            ret = { args.list[0].arg, "STRING" };
        }
        else if (fname == "parseBool") {
            if (args.list[0].arg == "true" || args.list[0].arg == "false") {
                ret = { args.list[0].arg, "BOOLEAN" };
            }
            else if (args.list[0].type == "NUMBER" && (args.list[0].arg == "1" || args.list[0].arg == "0")) {
                if (args.list[0].arg == "1") {
                    ret = { "true", "BOOLEAN" };
                }
                else {
                    ret = { "false", "BOOLEAN" };
                }
            }
            
        }
        else if (fname == "getarg") {
            if (args.list[0].type == "NUMBER" && stoi(args.list[0].arg) < argv.size()) {
                ret = { argv[stoi(args.list[0].arg)], "STRING" };
            }
        }
        else if (fname == "random") {
            if (args.list[0].type == "NUMBER") {
                random_device rd;

                mt19937 rng(rd());
                if (args.list.size() == 1) {
                    uniform_int_distribution<int> uni(0, stoi(args.list[0].arg));
                    ret = { to_string(uni(rng)), "NUMBER" };
                }
                else if (stoi(args.list[0].arg) < stoi(args.list[1].arg)) {
                    uniform_int_distribution<int> uni(stoi(args.list[0].arg), stoi(args.list[1].arg));
                    ret = { to_string(uni(rng)), "NUMBER" };
                }
            }
        }
        else if ((fname == "exit" || fname == "quit") && args.list[0].type == "NUMBER") {
            exit(stoi(args.list[0].arg));
        }
        else if (fname == "time") {
            ret = { to_stringWp(chrono::duration_cast<chrono::duration<double>>(chrono::system_clock::now().time_since_epoch()).count(), 15), "NUMBER" };
        }
        else if (fname == "sqrt" && args.list[0].type == "NUMBER") {
            ret = { formatNumber(to_stringWp(sqrt(stold(args.list[0].arg)), 15)), "NUMBER" };
        }
        else if (fname == "system" && args.list[0].type == "STRING") {
            ret = { to_string(system(args.list[0].arg.c_str())), "NUMBER" };
        }
        return ret;
    }

    _data exec(string fname, _argument args) {
        _data ret = { "", "NULL" };

        if (find(system_functions.begin(), system_functions.end(), fname) != system_functions.end()) {
            ret = execsf(fname, args);
        }
        else {
            auto vec = functions[fname]->tokenpos;

            int _line = line;
            ret = executeCode(vec[0], vec[1], vec[2], true, true);
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

    _data processNumber(string startnum, int pos) {
        vector<string> numtokens;
        _data tmp;
        string __tmp;
        pos++;

        numtokens.push_back(startnum);

        while (pos < tokens[line].size() && (tokens[line][pos].type == "NUMBER" || tokens[line][pos].type == "UNDEFINED_TOKEN" || find(mathoperators.begin(), mathoperators.end(), tokens[line][pos].value) != mathoperators.end() || tokens[line][pos].type == "LEFT_BRACKET")) {
            if (tokens[line][pos].type == "LEFT_BRACKET") {
                _data _tmp = parseCode(pos);
                __tmp = _tmp.arg;
                pos = _tmp.pos;
            }
            else if (tokens[line][pos].type == "NUMBER") {
                auto _tmp = _parseCode(pos);
                __tmp = _tmp.arg;
                pos = _tmp.pos;
            }
            else if (tokens[line][pos].type == "MINUS" && find(mathoperators.begin(), mathoperators.end(), tokens[line][pos - 1].value) != mathoperators.end()) {
                auto _tmp = _parseCode(pos);
                __tmp = _tmp.arg;
                pos = _tmp.pos;
            }
            else if (tokens[line][pos].type == "MULTIPLICATION" && match("MULTIPLICATION", pos)) {
                __tmp = "**";
                pos++;
            }
            else if (tokens[line][pos].type == "UNDEFINED_TOKEN" && find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), tokens[line][pos].value) != objects["VARIABLE"].end()) {
                auto _tmp = _parseCode(pos);
                if (_tmp.type == "NUMBER") {
                    __tmp = _tmp.arg;
                }
                else {
                    cout << strformat("Exception on pos: %d:%d\nUnnable to combine NUMBER and %s\n", line + 1, tokens[line][pos].pos, variables[tokens[line][pos].value]->type.c_str());
                    exit(-1);
                }
            }
            else if (tokens[line][pos].type == "UNDEFINED_TOKEN" && find(objects["FUNCTION"].begin(), objects["FUNCTION"].end(), tokens[line][pos].value) != objects["FUNCTION"].end()) {
                auto _tmp = _parseCode(pos);
                if (_tmp.type == "NUMBER") {
                    __tmp = _tmp.arg;
                }
                pos = _tmp.pos;
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
        tmp.arg = i;
        tmp.type = "NUMBER";
        tmp.pos = pos - 1;
        return tmp;
    }

    _data processBoolean(vector<string> value, int pos, int pos_ex) {
        if (value[1] == value[3]) {
            if (match("ASSIGN", pos) && match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return {boolean[stold(value[0]) == stold(value[2])], "BOOLEAN", "", pos_ex };
                }
                else {
                    return {boolean[value[0] == value[2]], "BOOLEAN", "", pos_ex };
                }
            }
            else if (match("EXCLAMATION_MARK", pos) && match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return {boolean[stold(value[0]) != stold(value[2])], "BOOLEAN", "", pos_ex };
                }
                else {
                    return {boolean[value[0] != value[2]], "BOOLEAN", "", pos_ex };
                }
            }
            else if (match("RIGHT_INEQUALITY_BRACKET", pos) && match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return {boolean[stold(value[0]) >= stold(value[2])], "BOOLEAN", "", pos_ex };
                }
                else if (value[1] == "STRING") {
                    return {boolean[value[0].length() >= value[2].length()], "BOOLEAN", "", pos_ex };
                }
            }
            else if (match("LEFT_INEQUALITY_BRACKET", pos) && match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return {boolean[stold(value[0]) <= stold(value[2])], "BOOLEAN", "", pos_ex };
                }
                else if (value[1] == "STRING") {
                    return {boolean[value[0].length() <= value[2].length()], "BOOLEAN", "", pos_ex };
                }
            }
            else if (match("RIGHT_INEQUALITY_BRACKET", pos) && !match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return {boolean[stold(value[0]) > stold(value[2])], "BOOLEAN", "", pos_ex };
                }
                else if (value[1] == "STRING") {
                    return {boolean[value[0].length() > value[2].length()], "BOOLEAN", "", pos_ex };
                }
            }
            else if (match("LEFT_INEQUALITY_BRACKET", pos) && !match("ASSIGN", pos + 1)) {
                if (value[1] == "NUMBER") {
                    return {boolean[stold(value[0]) < stold(value[2])], "BOOLEAN", "", pos_ex };
                }
                else if (value[1] == "STRING") {
                    return {boolean[value[0].length() < value[2].length()], "BOOLEAN", "", pos_ex };
                }
            }
        } 
        else if (match("EXCLAMATION_MARK", pos) && match("ASSIGN", pos + 1)) {
            return {boolean[1], "BOOLEAN", "", pos_ex};
        }

        return {boolean[0], "BOOLEAN", "", pos_ex};
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
        

        if (getTokenPos(_line, "LEFT_FIGURE_BRACKET") + 1 < tokens[_line].size() && tokens[_line][getTokenPos(_line, "LEFT_FIGURE_BRACKET") + 1].type != "HASHTAG") {
            _startline = _line;
            _starttoken = getTokenPos(_line, "LEFT_FIGURE_BRACKET") + 1;
        }
        else {
            _startline = _line + 1;
        }
        

        do {
            for (auto token : tokens[_line]) {
                if (token.type == "RIGHT_FIGURE_BRACKET") {
                    brackets--;
                    if (brackets == 0) { break; }
                } else if (token.type == "LEFT_FIGURE_BRACKET") {
                    brackets++;
                }
            }
            if (brackets != 0) {
                _line++;
            }
            
        } while (brackets != 0);
        _endline = _line;
        return {_startline, _starttoken, _endline, getTokenPos(_endline, "RIGHT_FIGURE_BRACKET")};
    }

    _data _parseCode(int pos) {
        _data ret;
        string arg;
        auto i = tokens[line][pos];

        if (i.type == "NUMBER") {
            auto _tmp = tokens[line][pos].value;
            ret.arg = _tmp;
            ret.type = i.type;
            ret.pos = pos;
            if (tolower(_tmp[1]) == 'x') {
                ret.arg = to_string(stoll(_tmp, 0, 16));
            }

            return ret;

        }
        else if (i.type == "STRING") {
            ret.arg = i.value;
            ret.type = i.type;
            ret.retarg = "new";
            ret.pos = pos;
            return ret;

        }
        else if (find(objects["FUNCTION"].begin(), objects["FUNCTION"].end(), i.value) != objects["FUNCTION"].end()) {
            vector<string> _tmp = {};
            int brackets = 0;
            if (match("LEFT_BRACKET", pos)) {
                int num = 1;
                string name = tokens[line][pos].value;
                _data _ret;
                _argument tmp;
                brackets++;
                pos++;
                if (match("UNDEFINED_TOKEN", pos) || match("NUMBER", pos) || match("MINUS", pos) || match("EXCLAMATION_MARK", pos) || match("STRING", pos) || match("LEFT_BRACKET", pos)) {
                    do {
                        _ret = parseCode(pos + 1);
                        pos = _ret.pos;

                        if (_ret.arg != "null") { 
                            tmp.list.push_back({ _ret.arg, _ret.type });
                        }
                        
                        if (tokens[line][pos].type == "RIGHT_BRACKET" && brackets > 0) {
                            brackets--;
                        }
                        
                    } while (pos + 1 < tokens[line].size() && (tokens[line][pos].type != "RIGHT_BRACKET" || brackets > 0));

                    if (match("COMA", pos) || match("RIGHT_BRACKET", pos)) {
                        pos++;
                        arg = "pos+";
                    }
                    
                    ret = exec(i.value, tmp);

                }
                else {
                    ret = exec(i.value, { { { "", "NULL" } } });
                    pos++;
                    if (match("COMA", pos) || match("RIGHT_BRACKET", pos)) {
                        pos++;
                        arg = "pos+";
                    }
                }

            } else {
                ret.arg = i.value;
                ret.type = "FUNCTION";
            }
            ret.retarg = arg;
            ret.pos = pos;
            return ret;

        }
        else if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), i.value) != objects["VARIABLE"].end()) {
            ret.arg = variables[i.value]->value;
            ret.type = variables[i.value]->type;
            ret.retarg = arg;
            ret.pos = pos;
            return ret;

        }
        else if (i.type == "LEFT_BRACKET") {
            _data rttmp;
            if (match("NUMBER", pos) && find(mathoperators.begin(), mathoperators.end(), tokens[line][pos + 1].value) != mathoperators.end()) {
                rttmp = processNumber(tokens[line][pos].value, pos);
            }
            else {
                rttmp = _parseCode(pos + 1);
            }
            ret = rttmp;
        }
        else if (i.type == "UNDEFINED_TOKEN" && find(boolean.begin(), boolean.end(), i.value) != boolean.end()) {
            return {i.value, "BOOLEAN", arg, pos};
        }
        else if (i.type == "MINUS" && (tokens[line][pos + 1].type == "UNDEFINED_TOKEN" || tokens[line][pos + 1].type == "NUMBER" || tokens[line][pos + 1].type == "LEFT_BRACKET" || tokens[line][pos + 1].type == "MINUS")) {
            _data _tmp;
            if (tokens[line][pos + 1].type == "LEFT_BRACKET") {
                _tmp = parseCode(pos + 1);
            }
            else {_tmp = _parseCode(pos + 1);}
            
            ret.arg = formatNumber(to_stringWp(-stold(_tmp.arg), 15));
            ret.type = _tmp.type;
            ret.retarg = _tmp.retarg;
            ret.pos = _tmp.pos;
        }
        else if (i.type == "EXCLAMATION_MARK" && (tokens[line][pos + 1].type == "UNDEFINED_TOKEN" || tokens[line][pos + 1].type == "NUMBER" || tokens[line][pos + 1].type == "LEFT_BRACKET" || tokens[line][pos + 1].type == "EXCLAMATION_MARK")) {
            _data _tmp;
            if (tokens[line][pos + 1].type == "LEFT_BRACKET") {
                _tmp = parseCode(pos + 1);
            }
            else { _tmp = _parseCode(pos + 1); }
            ret.arg = boolean[(boolean.end() - 1) - find(boolean.begin(), boolean.end(), _tmp.arg)];
            ret.type = _tmp.type;
            ret.retarg = _tmp.retarg;
            ret.pos = _tmp.pos;
        }
        else if (i.type == "UNDEFINED_TOKEN") {
            cout << strformat("Name '%s' is not defined", i.value.c_str()) << endl;
            exit(-1);
        }
        else {
            ret.arg = "null";
            ret.type = "NULL";
            ret.retarg = arg;
            ret.pos = pos;
        }
        return ret;
    }

    _data parseCode(int pos, bool wh = true) {
        int brackets = 0;
        while (tokens[line][pos].type == "LEFT_BRACKET") {
            brackets++;
            pos++;
        }

        auto tmp = _parseCode(pos);
        string name = tokens[line][pos].value;
        Token starttoken = tokens[line][pos];
        pos = tmp.pos;

        _data ret;
        string _ret = tmp.arg;

        string rettype = tmp.type;
        string arg;

        if (tmp.retarg != "") {
            arg = tmp.retarg;
        }

        do {
            if (rettype == "STRING" && arg == "new") {
                for (int _tmp = 0; _tmp < _ret.length(); _tmp++) {
                    if (_ret[_tmp] == '\\' && control_characters.find(_ret[_tmp + 1]) != control_characters.end()) {
                        _ret.replace(_tmp, 2, control_characters[_ret[_tmp + 1]]);
                    }
                    else if (_ret[_tmp] == '\\') {
                        _ret.erase(_tmp, 1);
                    }
                }
            }

            if (tokens[line][pos].type == "RIGHT_BRACKET" && brackets > 0 && (find(objects["FUNCTION"].begin(), objects["FUNCTION"].end(), starttoken.value) == objects["FUNCTION"].end() && starttoken.type != "STRING")) {
                brackets--;
            }

            if (pos + 2 < tokens[line].size() && tokens[line][pos].type == "UNDEFINED_TOKEN" && tokens[line][pos + 1].type != "ASSIGN" && ((find(mathoperators.begin(), mathoperators.end(), tokens[line][pos + 1].value) != mathoperators.end() && match("ASSIGN", pos + 1)) || (match("MULTIPLICATION", pos) && match("MULTIPLICATION", pos + 1) && match("ASSIGN", pos + 2)) || (tokens[line][pos + 1].type == tokens[line][pos + 2].type && (match("PLUS", pos) || match("MINUS", pos))))) {
                if (match("PLUS", pos) && match("PLUS", pos + 1)) {
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(stold(_ret) + 1, 15));
                    pos += 2;
                }
                else if (match("MINUS", pos) && match("MINUS", pos + 1)) {
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(stold(_ret) - 1, 15));
                    pos += 2;
                }
                else if (match("PLUS", pos) && match("ASSIGN", pos + 1)) {
                    _data operand = _parseCode(pos + 3);
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(stold(_ret) + stold(processNumber(operand.arg, operand.pos).arg), 15));
                    pos = operand.pos;
                }
                else if (match("MINUS", pos) && match("ASSIGN", pos + 1)) {
                    _data operand = _parseCode(pos + 3);
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(stold(_ret) - stold(processNumber(operand.arg, operand.pos).arg), 15));
                    pos = operand.pos;
                }
                else if (match("MULTIPLICATION", pos) && match("ASSIGN", pos + 1)) {
                    _data operand = _parseCode(pos + 3);
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(stold(_ret) * stold(processNumber(operand.arg, operand.pos).arg), 15));
                    pos = operand.pos;
                }
                else if (match("MULTIPLICATION", pos) && match("MULTIPLICATION", pos + 1) && match("ASSIGN", pos + 2)) {
                    _data operand = _parseCode(pos + 4);
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(pow(stold(_ret), stold(processNumber(operand.arg, operand.pos).arg)), 15));
                    pos = operand.pos;
                }
                else if (match("DIVISION", pos) && match("ASSIGN", pos + 1)) {
                    _data operand = _parseCode(pos + 3);
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(stold(_ret) / stold(processNumber(operand.arg, operand.pos).arg), 15));
                    pos = operand.pos;
                }
                else if (match("PERCENT", pos) && match("ASSIGN", pos + 1)) {
                    _data operand = _parseCode(pos + 3);
                    variables[tokens[line][pos].value]->value = formatNumber(to_stringWp(fmod(stold(_ret),  stold(processNumber(operand.arg, operand.pos).arg)), 15));
                    pos = operand.pos;
                }
            }

            else if (pos + 2 < tokens[line].size() && arg != "pos+" && find(mathoperators.begin(), mathoperators.end(), tokens[line][pos + 1].value) != mathoperators.end() && tokens[line][pos + 1].type != "ASSIGN") {
                if (match("LEFT_BRACKET", pos - 2) && !match("UNDEFINED_TOKEN", pos - 3)) {
                    pos--;
                }

                auto _tmp = processNumber(_ret, pos);
                _ret = _tmp.arg;
                rettype = _tmp.type;
                pos = _tmp.pos;
            }

            if (pos + 2 < tokens[line].size() && arg != "pos+" && find(booloperators.begin(), booloperators.end(), tokens[line][pos + 1].value + tokens[line][pos + 2].value) != booloperators.end()) {
                auto _tmp = parseCode(pos + 3, false);
                //cout << pos << endl;
                auto __tmp = processBoolean({ _ret, rettype, _tmp.arg, _tmp.type }, pos, _tmp.pos);
                _ret = __tmp.arg;
                rettype = __tmp.type;
                pos = __tmp.pos;
            }

            else if (pos + 1 < tokens[line].size() && arg != "pos+" && find(booloperators.begin(), booloperators.end(), tokens[line][pos + 1].value) != booloperators.end()) {
                auto _tmp = parseCode(pos + 2, false);
                auto __tmp = processBoolean({ _ret, rettype, _tmp.arg, _tmp.type }, pos, _tmp.pos);
                _ret = __tmp.arg;
                rettype = __tmp.type;
                pos = __tmp.pos;
            }

            if (wh && pos + 1 < tokens[line].size() && (!match("COMA", pos - 1) && !match("RIGHT_BRACKET", pos - 1) && !match("LEFT_FIGURE_BRACKET", pos - 1) || brackets > 0)) {pos++;}

        } while (pos + 1 < tokens[line].size() && wh && (!match("COMA", pos - 1) && !match("RIGHT_BRACKET", pos - 1) && !match("LEFT_FIGURE_BRACKET", pos - 1) || brackets > 0));
        
        ret.arg = _ret;
        ret.type = rettype;
        ret.pos = pos;
        return ret;
    }

    void runCode() {
        executeCode(0, 0, prev(tokens.end())->first, false);
    }

    _data executeCode(int startline, int starttoken, int endline, bool isfunc, bool isstfunc = false) {
        int pos = starttoken;
        _data ret = { "", "NULL" };
        for (line = startline; line <= endline; line++) {
            if (tokens[line].size() == 0 || tokens[line][0].type == "HASHTAG") {
                continue;
            }

            var_preprocess();
            auto i = tokens[line][pos];

            if (i.type == "UNDEFINED_TOKEN") {
                if (find(keywords.begin(), keywords.end(), i.value) != keywords.end()) {

                    if (i.value == "func" && match("UNDEFINED_TOKEN", pos)) {
                        objects["FUNCTION"].push_back(tokens[line][pos + 1].value);
                        if (match("LEFT_BRACKET", pos + 1)) {

                            vector<Argument> args = {};
                            string name;
                            string value = "";
                            string type = "NULL";
                            int argnum = 0;
                            int _pos = pos + 2;
                            while (_pos < tokens[line].size() && tokens[line][_pos].type != "RIGHT_BRACKET") {

                                if (tokens[line][_pos].type == "STRING" || tokens[line][_pos].type == "UNDEFINED_TOKEN" || tokens[line][_pos].type == "NUMBER") {
                                    argnum++;
                                    
                                    if (match("ASSIGN", _pos)) {
                                        name = tokens[line][_pos].value;
                                        auto tmp = parseCode(_pos + 2);
                                        value = tmp.arg;
                                        type = tmp.type;
                                        _pos = tmp.pos;
                                    }
                                    else {
                                        name = tokens[line][_pos].value;
                                    }

                                    args.push_back(Argument(argnum, name, value, type));
                                    name = "";
                                    value = "";
                                    type = "NULL";
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
                            auto poss = parseBrackets(_tmp.pos);
                            int _line = poss[2];

                            if (_tmp.arg == "true" && _tmp.type == "BOOLEAN") {
                                ret = executeCode(poss[0], poss[1], poss[2], isfunc);
                            }

                            line = _line;
                            if (ret.arg == "return" || (ret.arg == "break" && !isstfunc)) {
                                break;
                            }

                            
                        }
                    }
                    else if (i.value == "while") {
                        if (pos + 1 < tokens[line].size()) {
                            auto _tmp = parseCode(pos + 1);
                            auto poss = parseBrackets(_tmp.pos);
                            int oldline = line;

                            while (_tmp.arg == "true" && _tmp.type == "BOOLEAN") {
                                ret = executeCode(poss[0], poss[1], poss[2], isfunc);
                                line = oldline;

                                if (ret.arg == "break") {
                                    ret = { "", "NULL" };
                                    break;
                                }
                                else if (ret.arg == "return") {
                                    break;
                                }
                                _tmp = parseCode(pos + 1);
                            }
                            line = poss[2];
                            if (ret.arg == "return") {
                                break;
                            }
                        }
                    }
                    else if (i.value == "break" && !isstfunc) {
                        ret = { "break" };
                        break;
                    }
                    else if (i.value == "return") {
                        if (isfunc && pos + 1 < tokens[line].size()) {
                            auto tmp = parseCode(pos + 1);
                            ret = { tmp.arg, tmp.type, "return" };
                            break;
                        }
                    }

                }
                else if (find(objects["VARIABLE"].begin(), objects["VARIABLE"].end(), i.value) != objects["VARIABLE"].end()) {
                    if (find(mathoperators.begin(), mathoperators.end(), tokens[line][pos + 2].value) == mathoperators.end() && !match("ASSIGN", pos + 1) || (match("MINUS", pos + 1) && (match("UNDEFINED_TOKEN", pos + 2) || match("NUMBER", pos + 2) || match("LEFT_BRACKET", pos + 2)))) {
                        auto tmp = parseCode(pos + 2);
                        createVariable(i.value, tmp.arg, tmp.type);
                    }
                    else {
                        parseCode(pos);
                    }
                }
                else if (find(objects["FUNCTION"].begin(), objects["FUNCTION"].end(), i.value) != objects["FUNCTION"].end()) {
                    parseCode(pos);
                }
                else {
                    cout << strformat("Name '%s' is not defined", i.value.c_str()) << endl;
                    exit(-1);
                }
            }
            pos = 0;
        }
        return ret;
    }
};
