#include "jsling/token.hpp"

namespace jsling {

const std::unordered_map<std::string, TokenType>& getKeywords() {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"var", TokenType::Var}, {"let", TokenType::Let}, {"const", TokenType::Const},
        {"function", TokenType::Function}, {"return", TokenType::Return},
        {"if", TokenType::If}, {"else", TokenType::Else},
        {"for", TokenType::For}, {"while", TokenType::While}, {"do", TokenType::Do},
        {"switch", TokenType::Switch}, {"case", TokenType::Case}, {"default", TokenType::Default},
        {"break", TokenType::Break}, {"continue", TokenType::Continue},
        {"throw", TokenType::Throw}, {"try", TokenType::Try}, {"catch", TokenType::Catch},
        {"class", TokenType::Class}, {"extends", TokenType::Extends},
        {"import", TokenType::Import}, {"export", TokenType::Export},
        {"new", TokenType::New}, {"delete", TokenType::Delete},
        {"typeof", TokenType::Typeof}, {"void", TokenType::Void},
        {"in", TokenType::In}, {"of", TokenType::Of}, {"instanceof", TokenType::Instanceof},
        {"yield", TokenType::Yield},
        {"true", TokenType::True}, {"false", TokenType::False},
        {"null", TokenType::Null}, {"undefined", TokenType::Undefined}
    };
    return keywords;
}

std::string tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::Number: return "Number";
        case TokenType::String: return "String";
        case TokenType::TemplateLiteral: return "TemplateLiteral";
        case TokenType::True: return "True";
        case TokenType::False: return "False";
        case TokenType::Null: return "Null";
        case TokenType::Undefined: return "Undefined";
        case TokenType::Identifier: return "Identifier";
        case TokenType::Var: return "Var";
        case TokenType::Let: return "Let";
        case TokenType::Const: return "Const";
        case TokenType::Function: return "Function";
        case TokenType::Return: return "Return";
        case TokenType::If: return "If";
        case TokenType::Else: return "Else";
        case TokenType::For: return "For";
        case TokenType::While: return "While";
        case TokenType::Do: return "Do";
        case TokenType::Switch: return "Switch";
        case TokenType::Case: return "Case";
        case TokenType::Default: return "Default";
        case TokenType::Break: return "Break";
        case TokenType::Continue: return "Continue";
        case TokenType::Throw: return "Throw";
        case TokenType::Try: return "Try";
        case TokenType::Catch: return "Catch";
        case TokenType::Class: return "Class";
        case TokenType::Extends: return "Extends";
        case TokenType::Import: return "Import";
        case TokenType::Export: return "Export";
        case TokenType::New: return "New";
        case TokenType::Delete: return "Delete";
        case TokenType::Typeof: return "Typeof";
        case TokenType::Void: return "Void";
        case TokenType::In: return "In";
        case TokenType::Of: return "Of";
        case TokenType::Instanceof: return "Instanceof";
        case TokenType::Yield: return "Yield";
        case TokenType::Plus: return "Plus";
        case TokenType::Minus: return "Minus";
        case TokenType::Star: return "Star";
        case TokenType::Slash: return "Slash";
        case TokenType::Percent: return "Percent";
        case TokenType::StarStar: return "StarStar";
        case TokenType::PlusPlus: return "PlusPlus";
        case TokenType::MinusMinus: return "MinusMinus";
        case TokenType::Equal: return "Equal";
        case TokenType::PlusEqual: return "PlusEqual";
        case TokenType::MinusEqual: return "MinusEqual";
        case TokenType::StarEqual: return "StarEqual";
        case TokenType::SlashEqual: return "SlashEqual";
        case TokenType::PercentEqual: return "PercentEqual";
        case TokenType::EqualEqual: return "EqualEqual";
        case TokenType::BangEqual: return "BangEqual";
        case TokenType::EqualEqualEqual: return "EqualEqualEqual";
        case TokenType::BangEqualEqual: return "BangEqualEqual";
        case TokenType::Less: return "Less";
        case TokenType::Greater: return "Greater";
        case TokenType::LessEqual: return "LessEqual";
        case TokenType::GreaterEqual: return "GreaterEqual";
        case TokenType::AmpAmp: return "AmpAmp";
        case TokenType::PipePipe: return "PipePipe";
        case TokenType::Bang: return "Bang";
        case TokenType::Question: return "Question";
        case TokenType::Colon: return "Colon";
        case TokenType::Amp: return "Amp";
        case TokenType::Pipe: return "Pipe";
        case TokenType::Caret: return "Caret";
        case TokenType::Tilde: return "Tilde";
        case TokenType::LessLess: return "LessLess";
        case TokenType::GreaterGreater: return "GreaterGreater";
        case TokenType::GreaterGreaterGreater: return "GreaterGreaterGreater";
        case TokenType::AmpEqual: return "AmpEqual";
        case TokenType::PipeEqual: return "PipeEqual";
        case TokenType::CaretEqual: return "CaretEqual";
        case TokenType::LessLessEqual: return "LessLessEqual";
        case TokenType::GreaterGreaterEqual: return "GreaterGreaterEqual";
        case TokenType::LeftParen: return "LeftParen";
        case TokenType::RightParen: return "RightParen";
        case TokenType::LeftBracket: return "LeftBracket";
        case TokenType::RightBracket: return "RightBracket";
        case TokenType::LeftBrace: return "LeftBrace";
        case TokenType::RightBrace: return "RightBrace";
        case TokenType::Comma: return "Comma";
        case TokenType::Dot: return "Dot";
        case TokenType::Semicolon: return "Semicolon";
        case TokenType::Arrow: return "Arrow";
        case TokenType::Spread: return "Spread";
        case TokenType::TemplateStart: return "TemplateStart";
        case TokenType::TemplateEnd: return "TemplateEnd";
        case TokenType::TemplateMiddle: return "TemplateMiddle";
        case TokenType::Eof: return "Eof";
    }
    return "Unknown";
}

} // namespace jsling
