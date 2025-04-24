#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <regex>
#include <unordered_set>

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

class MathLexer {
private:
    vector<pair<TokenType, string>> tokenSpecs;
    unordered_set<string> functions = {"sin", "cos", "tan", "log", "exp", "sqrt"};
    regex tokenRegex;
    
public:
    MathLexer() {
        // Define token patterns
        tokenSpecs = {
            {TokenType::NUMBER,    R"(\d+(\.\d*)?)"},     // Integer or decimal number
            {TokenType::VARIABLE,  R"([a-zA-Z_]\w*)"},    // Variables and function names
            {TokenType::OPERATOR,  R"([+\-*/^])"},        // Arithmetic operators
            {TokenType::LPAREN,    R"(\()"},              // Left parenthesis
            {TokenType::RPAREN,    R"(\))"},              // Right parenthesis
            {TokenType::COMMA,     R"(,)"},               // Comma for function arguments
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
        sregex_iterator it(text.begin(), text.end(), tokenRegex);
        sregex_iterator end;
        
        for (; it != end; ++it) {
            smatch match = *it;
            size_t pos = match.position();
            string value = match.str();
            
            // Determine which token type was matched
            TokenType type = TokenType::UNKNOWN;
            for (size_t i = 0; i < tokenSpecs.size(); ++i) {
                if (match[i + 1].matched) {
                    type = tokenSpecs[i].first;
                    break;
                }
            }
            
            // Skip whitespace (not in specs, caught by UNKNOWN)
            if (value.find_first_not_of(" \t\n\r") == string::npos) {
                continue;
            }
            
            // Check if variable is a known function
            if (type == TokenType::VARIABLE) {
                if (functions.find(value) != functions.end()) {
                    type = TokenType::FUNCTION;
                }
            }
            
            tokens.push_back({type, value, pos});
        }
        
        return tokens;
    }
};

// Helper function to print token type as string
string tokenTypeToString(TokenType type) {
    switch(type) {
        case TokenType::NUMBER:   return "NUMBER";
        case TokenType::VARIABLE: return "VARIABLE";
        case TokenType::FUNCTION: return "FUNCTION";
        case TokenType::OPERATOR: return "OPERATOR";
        case TokenType::LPAREN:   return "LPAREN";
        case TokenType::RPAREN:   return "RPAREN";
        case TokenType::COMMA:    return "COMMA";
        default:                  return "UNKNOWN";
    }
}

int main() {
    MathLexer lexer;
    string test_expr = "3 + 4.2 * sin(x^2) / (1 - cos(2*theta))";
    
    cout << "Tokenizing expression: " << test_expr << endl;
    vector<Token> tokens = lexer.tokenize(test_expr);
    
    cout << "\nTokens found:" << endl;
    for (const auto& token : tokens) {
        cout << token.position << ": " 
             << tokenTypeToString(token.type) << "\t" 
             << token.value << endl;
    }
    
    return 0;
}