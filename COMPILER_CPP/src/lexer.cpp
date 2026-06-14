#include "jsling/lexer.hpp"
#include "jsling/errors.hpp"
#include <cctype>

namespace jsling {

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

char Lexer::current() const {
    return source_[pos_];
}

char Lexer::peek() const {
    if (pos_ + 1 >= source_.size()) return '\0';
    return source_[pos_ + 1];
}

char Lexer::peekNext() const {
    if (pos_ + 2 >= source_.size()) return '\0';
    return source_[pos_ + 2];
}

char Lexer::advance() {
    char c = source_[pos_++];
    if (c == '\n') line_++;
    return c;
}

bool Lexer::isAtEnd() const {
    return pos_ >= source_.size();
}

Token Lexer::makeToken(TokenType type, const std::string& lexeme) {
    return {type, lexeme, line_};
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = current();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '/' && peek() == '/') {
            while (!isAtEnd() && current() != '\n') advance();
        } else if (c == '/' && peek() == '*') {
            advance(); advance();
            while (!isAtEnd() && !(current() == '*' && peek() == '/')) advance();
            if (!isAtEnd()) { advance(); advance(); }
        } else {
            break;
        }
    }
}

Token Lexer::readNumber() {
    size_t start = pos_;
    if (current() == '0' && (peek() == 'x' || peek() == 'X')) {
        advance(); advance();
        while (!isAtEnd() && std::isxdigit(current())) advance();
        return makeToken(TokenType::Number, source_.substr(start, pos_ - start));
    }
    while (!isAtEnd() && std::isdigit(current())) advance();
    if (!isAtEnd() && current() == '.' && std::isdigit(peek())) {
        advance();
        while (!isAtEnd() && std::isdigit(current())) advance();
    }
    if (!isAtEnd() && (current() == 'e' || current() == 'E')) {
        advance();
        if (!isAtEnd() && (current() == '+' || current() == '-')) advance();
        while (!isAtEnd() && std::isdigit(current())) advance();
    }
    return makeToken(TokenType::Number, source_.substr(start, pos_ - start));
}

Token Lexer::readString(char quote) {
    advance(); // skip opening quote
    std::string value;
    while (!isAtEnd() && current() != quote) {
        if (current() == '\\') {
            advance();
            if (isAtEnd()) break;
            char esc = current();
            switch (esc) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '\'': value += '\''; break;
                case '"': value += '"'; break;
                case '0': value += '\0'; break;
                default: value += esc; break;
            }
        } else {
            value += current();
        }
        advance();
    }
    if (isAtEnd()) throw SyntaxError("Unterminated string literal at line " + std::to_string(line_));
    advance(); // skip closing quote
    return makeToken(TokenType::String, value);
}

Token Lexer::readTemplateLiteral() {
    advance(); // skip opening backtick
    std::string value;
    while (!isAtEnd() && current() != '`') {
        if (current() == '\\') {
            advance(); // skip backslash
            if (isAtEnd()) break;
            char esc = current();
            switch (esc) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case '\\': value += '\\'; break;
                case '`': value += '`'; break;
                case '$': value += '$'; break;
                default: value += esc; break;
            }
            advance(); // skip escaped char
        } else if (current() == '$' && peek() == '{') {
            // Copy ${ into value, then read expression tracking brace depth
            value += advance(); // $
            value += advance(); // {
            int depth = 1;
            while (!isAtEnd() && depth > 0) {
                if (current() == '{') {
                    depth++;
                    value += advance();
                } else if (current() == '}') {
                    depth--;
                    value += advance();
                } else if (current() == '`') {
                    // Nested template literal - recursively read it
                    Token nested = readTemplateLiteral();
                    value += '`' + nested.lexeme + '`';
                } else if (current() == '\'' || current() == '"') {
                    // String literal inside expression - read it whole
                    char q = current();
                    value += advance(); // opening quote
                    while (!isAtEnd() && current() != q) {
                        if (current() == '\\') { value += advance(); }
                        if (!isAtEnd()) value += advance();
                    }
                    if (!isAtEnd()) value += advance(); // closing quote
                } else if (current() == '/' && peek() == '/') {
                    // Line comment inside expression
                    while (!isAtEnd() && current() != '\n') advance();
                } else {
                    value += advance();
                }
            }
        } else {
            value += current();
            advance();
        }
    }
    if (isAtEnd()) throw SyntaxError("Unterminated template literal at line " + std::to_string(line_));
    advance(); // skip closing backtick
    return makeToken(TokenType::TemplateLiteral, value);
}

Token Lexer::readIdentifierOrKeyword() {
    size_t start = pos_;
    while (!isAtEnd() && (std::isalnum(current()) || current() == '_' || current() == '$')) {
        advance();
    }
    std::string word = source_.substr(start, pos_ - start);
    const auto& keywords = getKeywords();
    auto it = keywords.find(word);
    if (it != keywords.end()) {
        return makeToken(it->second, word);
    }
    return makeToken(TokenType::Identifier, word);
}

Token Lexer::readOperatorOrPunctuation() {
    char c = advance();
    switch (c) {
        case '(': return makeToken(TokenType::LeftParen, "(");
        case ')': return makeToken(TokenType::RightParen, ")");
        case '[': return makeToken(TokenType::LeftBracket, "[");
        case ']': return makeToken(TokenType::RightBracket, "]");
        case '{': return makeToken(TokenType::LeftBrace, "{");
        case '}': return makeToken(TokenType::RightBrace, "}");
        case ',': return makeToken(TokenType::Comma, ",");
        case '.': 
            if (!isAtEnd() && current() == '.' && peek() == '.') {
                advance(); advance();
                return makeToken(TokenType::Spread, "...");
            }
            return makeToken(TokenType::Dot, ".");
        case ';': return makeToken(TokenType::Semicolon, ";");
        case '?': return makeToken(TokenType::Question, "?");
        case ':': return makeToken(TokenType::Colon, ":");
        case '~': return makeToken(TokenType::Tilde, "~");
        case '+':
            if (!isAtEnd() && current() == '+') { advance(); return makeToken(TokenType::PlusPlus, "++"); }
            if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::PlusEqual, "+="); }
            return makeToken(TokenType::Plus, "+");
        case '-':
            if (!isAtEnd() && current() == '-') { advance(); return makeToken(TokenType::MinusMinus, "--"); }
            if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::MinusEqual, "-="); }
            return makeToken(TokenType::Minus, "-");
        case '*':
            if (!isAtEnd() && current() == '*') { advance(); return makeToken(TokenType::StarStar, "**"); }
            if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::StarEqual, "*="); }
            return makeToken(TokenType::Star, "*");
        case '/':
            if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::SlashEqual, "/="); }
            return makeToken(TokenType::Slash, "/");
        case '%':
            if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::PercentEqual, "%="); }
            return makeToken(TokenType::Percent, "%");
        case '=':
            if (!isAtEnd() && current() == '=') {
                advance();
                if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::EqualEqualEqual, "==="); }
                return makeToken(TokenType::EqualEqual, "==");
            }
            if (!isAtEnd() && current() == '>') { advance(); return makeToken(TokenType::Arrow, "=>"); }
            return makeToken(TokenType::Equal, "=");
        case '!':
            if (!isAtEnd() && current() == '=') {
                advance();
                if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::BangEqualEqual, "!=="); }
                return makeToken(TokenType::BangEqual, "!=");
            }
            return makeToken(TokenType::Bang, "!");
        case '<':
            if (!isAtEnd() && current() == '<') {
                advance();
                if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::LessLessEqual, "<<="); }
                return makeToken(TokenType::LessLess, "<<");
            }
            if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::LessEqual, "<="); }
            return makeToken(TokenType::Less, "<");
        case '>':
            if (!isAtEnd() && current() == '>') {
                advance();
                if (!isAtEnd() && current() == '>') {
                    advance();
                    if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::GreaterGreaterEqual, ">>="); }
                    return makeToken(TokenType::GreaterGreaterGreater, ">>>");
                }
                if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::GreaterGreaterEqual, ">>="); }
                return makeToken(TokenType::GreaterGreater, ">>");
            }
            if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::GreaterEqual, ">="); }
            return makeToken(TokenType::Greater, ">");
        case '&':
            if (!isAtEnd() && current() == '&') { advance(); return makeToken(TokenType::AmpAmp, "&&"); }
            if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::AmpEqual, "&="); }
            return makeToken(TokenType::Amp, "&");
        case '|':
            if (!isAtEnd() && current() == '|') { advance(); return makeToken(TokenType::PipePipe, "||"); }
            if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::PipeEqual, "|="); }
            return makeToken(TokenType::Pipe, "|");
        case '^':
            if (!isAtEnd() && current() == '=') { advance(); return makeToken(TokenType::CaretEqual, "^="); }
            return makeToken(TokenType::Caret, "^");
    }
    throw SyntaxError(std::string("Unexpected character '") + c + "' at line " + std::to_string(line_));
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (!isAtEnd()) {
        skipWhitespaceAndComments();
        if (isAtEnd()) break;
        char c = current();
        if (std::isdigit(c)) {
            tokens.push_back(readNumber());
        } else if (c == '\'' || c == '"') {
            tokens.push_back(readString(c));
        } else if (c == '`') {
            tokens.push_back(readTemplateLiteral());
        } else if (std::isalpha(c) || c == '_' || c == '$') {
            tokens.push_back(readIdentifierOrKeyword());
        } else {
            tokens.push_back(readOperatorOrPunctuation());
        }
    }
    tokens.push_back(makeToken(TokenType::Eof, ""));
    return tokens;
}

} // namespace jsling
