#pragma once
#include <string>
#include <unordered_map>

namespace jsling {

enum class TokenType {
    // Literals
    Number, String, TemplateLiteral, True, False, Null, Undefined,
    // Identifier
    Identifier,
    // Keywords
    Var, Let, Const, Function, Return, If, Else, For, While, Do,
    Switch, Case, Default, Break, Continue, Throw, Try, Catch,
    Class, Extends, Import, Export, New, Delete, Typeof, Void,
    In, Of, Instanceof, Yield,
    // Operators
    Plus, Minus, Star, Slash, Percent, StarStar,
    PlusPlus, MinusMinus,
    Equal, PlusEqual, MinusEqual, StarEqual, SlashEqual, PercentEqual,
    EqualEqual, BangEqual, EqualEqualEqual, BangEqualEqual,
    Less, Greater, LessEqual, GreaterEqual,
    AmpAmp, PipePipe, Bang,
    Question, Colon,
    Amp, Pipe, Caret, Tilde, LessLess, GreaterGreater, GreaterGreaterGreater,
    AmpEqual, PipeEqual, CaretEqual, LessLessEqual, GreaterGreaterEqual,
    // Punctuation
    LeftParen, RightParen,
    LeftBracket, RightBracket,
    LeftBrace, RightBrace,
    Comma, Dot, Semicolon,
    Arrow, Spread,
    TemplateStart, TemplateEnd, TemplateMiddle,
    // Special
    Eof
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
};

const std::unordered_map<std::string, TokenType>& getKeywords();
std::string tokenTypeName(TokenType type);

} // namespace jsling
