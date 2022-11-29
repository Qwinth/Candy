#pragma once
using namespace std;

class Lexer {
    vector<string> spec_characters = { "(", ")", "'", "\"", "{", "}", "[", "]", "=", "!", "+", "-", "*", "/", "%", ":", ";", ".", ",", "<", ">", "#", "\\" };
    map<string, string> spec_characters_name = { {"(", "LEFT_BRACKET"}, {")", "RIGHT_BRACKET"}, {"'", "QUOTE"}, {"\"", "QUOTE"}, {"{", "LEFT_FIGURE_BRACKET"}, {"}", "RIGHT_FIGURE_BRACKET"}, {"[", "RIGHT_SQUARE_BRACKET"}, {"]", "LEFT_SQUARE_BRACKET"}, {"=", "ASSIGN"}, {"!", "EXCLAMATION_MARK"}, {"+", "PLUS"}, {"-", "MINUS"}, {"*", "MULTIPLICATION"}, {"/", "DIVISION"}, {"%", "PERCENT"}, {":", "DOUBLE_DOT"}, {";", "DOT_COMA"}, {".", "DOT"}, {",", "COMA"}, {"<", "LEFT_INEQUALITY_BRACKET"}, {">", "RIGHT_INEQUALITY_BRACKET"}, {"#", "HASHTAG"}, {"\\", "INVERSE_SLASH"} };
    vector<string> quotes = { "'", "\"" };
    string quotes_opened;
    string comment = "#", space = " ";
public:
    map<int, string> disassemble(string code_string) {
        map<int, string> undefined_elements = {};
        string undefined;
        string i;
        int pos;
        int tmp = 0;
        for (pos = 0, tmp = 0; tmp < code_string.length(); tmp++, pos++) {

            i = code_string[tmp];
            if (find(spec_characters.begin(), spec_characters.end(), i) != spec_characters.end()) {

                if (find(quotes.begin(), quotes.end(), i) != quotes.end()) {
                    if (quotes_opened != "" && quotes_opened == i && code_string[pos - 1] != '\\') {
                        quotes_opened = "";
                    }

                    else if (quotes_opened == "") {
                        quotes_opened = quotes[find(quotes.begin(), quotes.end(), i) - quotes.begin()];
                    }
                }

                if (quotes_opened != "") {
                    if (i != quotes_opened || code_string[pos - 1] == '\\') {
                        undefined += i;
                    }
                    else {

                        undefined_elements[pos] = i;
                    }
                }
                else {
                    if (i == "." && all_of(undefined.begin(), undefined.end(), ::isdigit)) {
                        undefined += i;
                    }
                    else {
                        if (undefined != "") {
                            undefined_elements[pos - undefined.length()] = undefined;
                        }

                        undefined_elements[pos] = i;
                        undefined = "";
                    }
                }
            }
            else {
                if (quotes_opened != "")
                    undefined += i;
                else if (i != space && i != "\t")
                    undefined += i;
                else if (undefined != "") {
                    undefined_elements[pos - undefined.length()] = undefined;
                    undefined = "";
                }
            }
        }
        if (undefined != "") {
            undefined_elements[tmp - undefined.length()] = undefined;
        }
        return undefined_elements;
    }

    vector<Token> tokenize(map<int, string> undefined_elements) {
        vector<Token> tokens = {};
        string i;
        string dectest;
        quotes_opened = "";
        for (auto tmp : undefined_elements) {
            i = tmp.second;
            dectest = i;
            if (dectest.find('.') != dectest.npos) { dectest.erase(dectest.find('.')); }


            if (spec_characters_name.find(i) != spec_characters_name.end() && (quotes_opened == "" || find(quotes.begin(), quotes.end(), i) != quotes.end())) {
                if (find(quotes.begin(), quotes.end(), i) != quotes.end()) {
                    if (quotes_opened != "") {
                        quotes_opened = "";

                        if (find(quotes.begin(), quotes.end(), undefined_elements[tmp.first - 1]) != quotes.end()) {
                            tokens.push_back(Token("STRING", "", tmp.first));
                        }
                    }
                    else {
                        quotes_opened = quotes[find(quotes.begin(), quotes.end(), i) - quotes.begin()];
                    }
                }
                else {
                    tokens.push_back(Token(spec_characters_name[i], i, tmp.first));
                }

            }
            else if (quotes_opened == "" && (all_of(i.begin(), i.end(), ::isdigit) || (i[0] == '0' && tolower(i[1]) == 'x') || all_of(dectest.begin(), dectest.end(), ::isdigit))) {
                tokens.push_back(Token("NUMBER", i, tmp.first));
            }
            else if (quotes_opened != "") {
                tokens.push_back(Token("STRING", i, tmp.first));
            }
            else {
                tokens.push_back(Token("UNDEFINED_TOKEN", i, tmp.first));
            }
        }
        return tokens;
    }
};
