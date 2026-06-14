#pragma once
#include "token.hpp"
#include <string>
#include <vector>

namespace jsling {

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    std::string source_;
    size_t pos_ = 0;
    int line_ = 1;

    char current() const;
    char peek() const;
    char peekNext() const;
    char advance();
    bool isAtEnd() const;
    void skipWhitespaceAndComments();
    Token makeToken(TokenType type, const std::string& lexeme);
    Token readNumber();
    Token readString(char quote);
    Token readTemplateLiteral();
    Token readIdentifierOrKeyword();
    Token readOperatorOrPunctuation();
};

} // namespace jsling
