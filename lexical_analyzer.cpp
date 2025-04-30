#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <stdexcept>
#include <iomanip>
#include <exception>
#include <cctype>

using namespace std;

// Token types
enum class TokenType {
    NUMBER,
    VARIABLE,
    FUNCTION,
    OPERATOR,
    LPAREN,
    RPAREN,
    COMMA,
    UNKNOWN
};

// Token structure
struct Token {
    TokenType type;
    string value;
    size_t position;
};

enum class LexerState {
    START,
    IN_NUMBER,
    IN_IDENTIFIER
};

// Symbol Table class
class SymbolTable {
private:
    unordered_set<string> variables;
public:
    void addVariable(const string& varName) { variables.insert(varName); }
    bool exists(const string& varName) const { return variables.find(varName) != variables.end(); }

    void display() const {
        cout << "\nVariables found:\n";
        if (variables.empty()) { cout << "None\n"; return; }
        for (const auto& var : variables)
            cout << "- " << var << endl;
    }
};

// Math Lexer class using FSM
class MathLexer {
private:
    unordered_set<string> functions = { "sin", "cos", "tan", "log", "exp", "sqrt" };

public:
    vector<Token> tokenize(const string& text) {
        vector<Token> tokens;
        LexerState state = LexerState::START;
        string current;
        size_t start = 0;
        vector<size_t> parenStack;

        auto pushToken = [&](TokenType type, size_t pos, const string& val) {
            tokens.push_back({ type, val, pos });
        };

        for (size_t i = 0; i <= text.length(); ++i) {
            char c = (i < text.length()) ? text[i] : '\0';

            switch (state) {
                case LexerState::START:
                    if (isspace(c)) continue;

                    start = i;
                    if (isdigit(c)) {
                        current = c;
                        state = LexerState::IN_NUMBER;
                    }
                    else if (isalpha(c) || c == '_') {
                        current = c;
                        state = LexerState::IN_IDENTIFIER;
                    }
                    else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
                        pushToken(TokenType::OPERATOR, i, string(1, c));
                    }
                    else if (c == '(') {
                        pushToken(TokenType::LPAREN, i, "(");
                        parenStack.push_back(i);
                    }
                    else if (c == ')') {
                        if (parenStack.empty()) throw runtime_error("Unmatched closing parenthesis at position " + to_string(i));
                        pushToken(TokenType::RPAREN, i, ")");
                        parenStack.pop_back();
                    }
                    else if (c == ',') {
                        pushToken(TokenType::COMMA, i, ",");
                    }
                    else if (c == '\0') {
                        break;
                    }
                    else {
                        pushToken(TokenType::UNKNOWN, i, string(1, c));
                    }
                    break;

                case LexerState::IN_NUMBER:
                    if (isdigit(c) || c == '.') {
                        current += c;
                    } else {
                        pushToken(TokenType::NUMBER, start, current);
                        state = LexerState::START;
                        --i; // Reprocess this character
                    }
                    break;

                case LexerState::IN_IDENTIFIER:
                    if (isalnum(c) || c == '_') {
                        current += c;
                    } else {
                        TokenType t = (functions.count(current)) ? TokenType::FUNCTION : TokenType::VARIABLE;
                        pushToken(t, start, current);
                        state = LexerState::START;
                        --i;
                    }
                    break;
            }
        }

        if (!parenStack.empty())
            throw runtime_error("Unmatched opening parenthesis at position " + to_string(parenStack.back()));

        return tokens;
    }
};

// Helper function to convert enum to string
string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::NUMBER:    return "NUMBER";
        case TokenType::VARIABLE:  return "VARIABLE";
        case TokenType::FUNCTION:  return "FUNCTION";
        case TokenType::OPERATOR:  return "OPERATOR";
        case TokenType::LPAREN:    return "LPAREN";
        case TokenType::RPAREN:    return "RPAREN";
        case TokenType::COMMA:     return "COMMA";
        default:                   return "UNKNOWN";
    }
}

// Error display
void display_error_details(const exception& e, const string& exp) {
    cerr << "\nERROR: " << e.what() << endl;
    cerr << "Expression: " << exp << endl;

    try {
        string errorMsg(e.what());
        size_t posMarker = errorMsg.find("at position ");
        if (posMarker != string::npos) {
            size_t errorPos = stoul(errorMsg.substr(posMarker + 12));
            errorPos = min(errorPos, exp.length());
            cerr << string(errorPos, ' ') << "^\n";
        } else {
            cerr << "^\n";
        }
    } catch (...) {
        cerr << "^\n";
    }
}

// Simple syntax validator
bool validate_syntax(const vector<Token>& tokens) {
    if (tokens.empty())
        throw runtime_error("No tokens to parse.");

    TokenType lastType = TokenType::UNKNOWN;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& token = tokens[i];

        if (token.type == TokenType::OPERATOR) {
            if (i == 0 || i == tokens.size() - 1)
                throw runtime_error("Expression cannot start or end with an operator at position " + to_string(token.position));
            if (lastType == TokenType::OPERATOR || lastType == TokenType::LPAREN)
                throw runtime_error("Invalid operator usage at position " + to_string(token.position));
        }

        if (token.type == TokenType::COMMA) {
            if (lastType != TokenType::RPAREN && lastType != TokenType::VARIABLE && lastType != TokenType::NUMBER)
                throw runtime_error("Comma misplacement at position " + to_string(token.position));
        }

        lastType = token.type;
    }

    return true;
}

// User Interface
void user_interface(const string& exp) {
    cout << "\nFor the following expression: " << exp << endl;

    MathLexer lexer;
    SymbolTable symTable;
    cout << "Tokenizing expression..." << endl;

    try {
        vector<Token> tokens = lexer.tokenize(exp);

        cout << "\nTokens found:\n";
        cout << "Position\tType\t\tValue\n";
        cout << string(40, '-') << endl;

        for (const auto& token : tokens) {
            cout << token.position << "\t\t" << left << setw(12) << tokenTypeToString(token.type) << token.value << endl;
            if (token.type == TokenType::VARIABLE)
                symTable.addVariable(token.value);
        }

        symTable.display();

        cout << "\nValidating syntax...\n";
        validate_syntax(tokens);
        cout << "Expression syntax is valid.\n";
    }
    catch (const exception& e) {
        display_error_details(e, exp);
    }
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) throw runtime_error("No expression given.");
        user_interface(argv[1]);
    } catch (...) {
        cout << "Invalid Expression" << endl;
    }

    return 0;
}
