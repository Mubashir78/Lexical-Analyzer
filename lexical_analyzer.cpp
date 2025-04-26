#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <regex>
#include <unordered_set>
#include <stdexcept>
#include <iomanip>
#include <exception>
#include <stack>

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
    UNKNOWN,
    WHITESPACE
};

// Token structure
struct Token {
    TokenType type;
    string value;
    size_t position;
};


class MathLexer {
    private:
        vector<pair<TokenType, string>> tokenSpecs;
        unordered_set<string> functions = {"sin", "cos", "tan", "log", "exp", "sqrt"};
        regex tokenRegex;
        
    public:
        MathLexer() {
            // Define token patterns - ORDER MATTERS!
            // More specific patterns should come first
            tokenSpecs = {
                {TokenType::LPAREN,    R"(\()"},              // Left parenthesis (must come before operator)
                {TokenType::RPAREN,    R"(\))"},              // Right parenthesis (must come before comma)
                {TokenType::COMMA,     R"(,)"},               // Comma for function arguments
                {TokenType::NUMBER,    R"(\d+(\.\d*)?)"},     // Integer or decimal number
                {TokenType::FUNCTION,  R"(sin|cos|tan|log|exp|sqrt)"}, // Known functions
                {TokenType::OPERATOR,  R"([+\-*/^])"},        // Arithmetic operators
                {TokenType::VARIABLE,  R"([a-zA-Z_]\w*)"},    // Variables
                {TokenType::UNKNOWN,   R"(.)"}                // Any other character
            };
            
            // Combine all patterns into one regular expression
            string pattern;
            for (const auto& spec : tokenSpecs) {
                if (!pattern.empty()) pattern += "|";
                pattern += "(" + spec.second + ")";
            }
            tokenRegex = regex(pattern);
        
        }
        
        vector<Token> tokenize(const string& text) {
            vector<Token> tokens;
            stack<size_t> parenStack;  // To track opening parenthesis positions

            // Debug output
            // cout << "\nDebug - Input text: '" << text << "'\n";
            // cout << "Debug - Regex pattern has " << tokenRegex.mark_count() << " groups\n";
            
            sregex_iterator it(text.begin(), text.end(), tokenRegex);
            sregex_iterator end;
            
            for (; it != end; ++it) {
                smatch match = *it;
                size_t pos = match.position();
                string value = match.str();
                
                // Determine token type
                TokenType type = TokenType::UNKNOWN;
                for (size_t i = 0; i < tokenSpecs.size(); ++i) {
                    if (match[i + 1].matched) {
                        type = tokenSpecs[i].first;
                        break;
                    }
                }
                
                if (type == TokenType::WHITESPACE) continue;
                
                // Track parentheses
                if (type == TokenType::LPAREN) {
                    parenStack.push(pos);
                } 
                else if (type == TokenType::RPAREN) {
                    if (parenStack.empty()) {
                        throw runtime_error("Unmatched closing parenthesis at position " + to_string(pos));
                    }
                    parenStack.pop();
                }
                
                tokens.push_back({type, value, pos});
            }
            
            // Check for unmatched opening parentheses
            if (!parenStack.empty()) {
                throw runtime_error("Unmatched opening parenthesis at position " + to_string(parenStack.top()));
            }
            
            return tokens;
        }
    };



string tokenTypeToString(TokenType type) {
    switch(type) {
        case TokenType::NUMBER:    return "NUMBER";
        case TokenType::VARIABLE:  return "VARIABLE";
        case TokenType::FUNCTION:  return "FUNCTION";
        case TokenType::OPERATOR:  return "OPERATOR";
        case TokenType::LPAREN:    return "LPAREN";
        case TokenType::RPAREN:    return "RPAREN";
        case TokenType::COMMA:     return "COMMA";
        case TokenType::WHITESPACE:return "WHITESPACE";
        default:                   return "UNKNOWN";
    }
}

string getExpression() {
    string exp;
    if (!getline(cin, exp)) {
        throw runtime_error("Failed to read input");
    }
    return exp;
}

// Helper function to display detailed error information
void display_error_details(const exception& e, const string& exp) {
    cerr << "\nERROR: " << e.what() << endl;
    cerr << "Expression: " << exp << endl;
    
    try {
        string errorMsg(e.what());
        size_t posMarker = errorMsg.find("at position ");
        
        if (posMarker != string::npos) {
            size_t errorPos = stoul(errorMsg.substr(posMarker + 12));
            errorPos = min(errorPos, exp.length());  // Ensure position is within bounds
            cerr << string(errorPos, ' ') << "^\n";
        } else {
            cerr << "^\n";  // Generic error position indicator
        }
    } 
    catch (...) {
        cerr << "^\n";  // Fallback error position indicator
    }
}

void user_interface() {
    try {
        cout << "Enter Expression: ";
        string exp = getExpression();
        if (exp.empty()) {
            throw runtime_error("Empty expression");
        }

        cout << "\nFor the following expression: " << exp << endl;

        MathLexer lexer;
        cout << "Tokenizing expression..." << endl;
        
        try {
            vector<Token> tokens = lexer.tokenize(exp);

            // Display tokens in a clean table format
            cout << "\nTokens found:" << endl;
            cout << "Position\tType\t\tValue" << endl;
            cout << string(40, '-') << endl;
            
            for (const auto& token : tokens) {
                cout << token.position << "\t\t" 
                     << left << setw(12) << tokenTypeToString(token.type) 
                     << token.value << endl;
            }
        }
        catch (const exception& e) {
            display_error_details(e, exp);
        }
    } 
    catch (const exception& e) {
        cerr << "\nError: " << e.what() << endl;
    }
}

int main() {
    user_interface();
    return 0;
}