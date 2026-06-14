#include "jsling/parser.hpp"
#include "jsling/lexer.hpp"
#include "jsling/errors.hpp"
#include <cstdlib>

namespace jsling {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

const Token& Parser::current() const { return tokens_[pos_]; }
const Token& Parser::peek() const { return tokens_[pos_ + 1 < tokens_.size() ? pos_ + 1 : pos_]; }
bool Parser::check(TokenType type) const { return current().type == type; }
bool Parser::isAtEnd() const { return current().type == TokenType::Eof; }

const Token& Parser::advance() {
    const Token& tok = tokens_[pos_];
    if (!isAtEnd()) pos_++;
    return tok;
}

bool Parser::match(TokenType type) {
    if (check(type)) { advance(); return true; }
    return false;
}

const Token& Parser::consume(TokenType type, const std::string& msg) {
    if (check(type)) return advance();
    throw SyntaxError(msg + " at line " + std::to_string(current().line));
}

std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();
    while (!isAtEnd()) {
        program->body.push_back(parseStatement());
    }
    return program;
}

// ==================== STATEMENTS ====================

std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (check(TokenType::LeftBrace)) return parseBlock();
    if (check(TokenType::Var) || check(TokenType::Let) || check(TokenType::Const)) return parseVarDecl();
    if (check(TokenType::Function)) return parseFunctionDecl();
    if (check(TokenType::If)) return parseIf();
    if (check(TokenType::For)) return parseFor();
    if (check(TokenType::While)) return parseWhile();
    if (check(TokenType::Do)) return parseDoWhile();
    if (check(TokenType::Switch)) return parseSwitch();
    if (check(TokenType::Return)) return parseReturn();
    if (check(TokenType::Break)) return parseBreak();
    if (check(TokenType::Continue)) return parseContinue();
    if (check(TokenType::Semicolon)) { advance(); return std::make_unique<ExprStmt>(); }
    return parseExprStmt();
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    consume(TokenType::LeftBrace, "Expected '{'");
    auto block = std::make_unique<BlockStmt>();
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        block->body.push_back(parseStatement());
    }
    consume(TokenType::RightBrace, "Expected '}'");
    return block;
}

std::unique_ptr<ASTNode> Parser::parseVarDecl() {
    std::string kind = advance().lexeme; // var/let/const

    auto firstDecl = std::make_unique<VarDecl>();
    firstDecl->kind = kind;
    if (check(TokenType::Identifier)) {
        firstDecl->name = advance().lexeme;
    } else {
        throw SyntaxError("Expected identifier after " + kind + " at line " + std::to_string(current().line));
    }
    if (match(TokenType::Equal)) {
        firstDecl->initializer = parseAssignmentExpression();
    }

    // Check for comma-separated declarations: let a = 1, b = 2;
    if (!check(TokenType::Comma)) {
        match(TokenType::Semicolon);
        return firstDecl;
    }

    // Multiple declarations - wrap in a statement list (no scope)
    auto list = std::make_unique<StmtList>();
    list->stmts.push_back(std::move(firstDecl));
    while (match(TokenType::Comma)) {
        auto decl = std::make_unique<VarDecl>();
        decl->kind = kind;
        if (check(TokenType::Identifier)) {
            decl->name = advance().lexeme;
        } else {
            throw SyntaxError("Expected identifier after ',' at line " + std::to_string(current().line));
        }
        if (match(TokenType::Equal)) {
            decl->initializer = parseAssignmentExpression();
        }
        list->stmts.push_back(std::move(decl));
    }
    match(TokenType::Semicolon);
    return list;
}

std::unique_ptr<ASTNode> Parser::parseFunctionDecl() {
    consume(TokenType::Function, "Expected 'function'");
    auto fn = std::make_unique<FunctionDecl>();
    if (check(TokenType::Identifier)) fn->name = advance().lexeme;
    consume(TokenType::LeftParen, "Expected '('");
    parseFunctionParams(fn->params, fn->defaults, fn->hasRest, fn->restParam);
    consume(TokenType::RightParen, "Expected ')'");
    fn->body = parseBlock();
    return fn;
}

void Parser::parseFunctionParams(std::vector<std::string>& params,
                                  std::vector<std::unique_ptr<ASTNode>>& defaults,
                                  bool& hasRest, std::string& restParam) {
    while (!check(TokenType::RightParen) && !isAtEnd()) {
        if (check(TokenType::Spread)) {
            advance(); // consume ...
            restParam = consume(TokenType::Identifier, "Expected rest parameter name").lexeme;
            hasRest = true;
            break; // rest must be last
        }
        params.push_back(consume(TokenType::Identifier, "Expected parameter name").lexeme);
        if (match(TokenType::Equal)) {
            defaults.push_back(parseAssignmentExpression());
        } else {
            defaults.push_back(nullptr);
        }
        if (!check(TokenType::RightParen)) consume(TokenType::Comma, "Expected ','");
    }
}

std::unique_ptr<ASTNode> Parser::parseArrowFunctionBody(
    std::vector<std::string> params, std::vector<std::unique_ptr<ASTNode>> defaults,
    bool hasRest, const std::string& restParam) {
    auto fn = std::make_unique<FunctionDecl>();
    fn->params = std::move(params);
    fn->defaults = std::move(defaults);
    fn->hasRest = hasRest;
    fn->restParam = restParam;
    if (check(TokenType::LeftBrace)) {
        fn->body = parseBlock();
    } else {
        auto ret = std::make_unique<ReturnStmt>();
        ret->argument = parseAssignmentExpression();
        fn->body = std::make_unique<BlockStmt>();
        fn->body->body.push_back(std::move(ret));
    }
    return fn;
}

std::unique_ptr<ASTNode> Parser::parseIf() {
    consume(TokenType::If, "Expected 'if'");
    consume(TokenType::LeftParen, "Expected '('");
    auto stmt = std::make_unique<IfStmt>();
    stmt->test = parseExpression();
    consume(TokenType::RightParen, "Expected ')'");
    stmt->consequent = parseStatement();
    if (match(TokenType::Else)) {
        stmt->alternate = parseStatement();
    }
    return stmt;
}

std::unique_ptr<ASTNode> Parser::parseFor() {
    consume(TokenType::For, "Expected 'for'");
    consume(TokenType::LeftParen, "Expected '('");
    auto stmt = std::make_unique<ForStmt>();
    if (!check(TokenType::Semicolon)) {
        if (check(TokenType::Var) || check(TokenType::Let) || check(TokenType::Const)) {
            stmt->init = parseVarDecl();
        } else {
            stmt->init = parseExpression();
            match(TokenType::Semicolon);
        }
    } else {
        advance();
    }
    if (!check(TokenType::Semicolon)) stmt->test = parseExpression();
    consume(TokenType::Semicolon, "Expected ';'");
    if (!check(TokenType::RightParen)) stmt->update = parseExpression();
    consume(TokenType::RightParen, "Expected ')'");
    stmt->body = parseStatement();
    return stmt;
}

std::unique_ptr<ASTNode> Parser::parseWhile() {
    consume(TokenType::While, "Expected 'while'");
    consume(TokenType::LeftParen, "Expected '('");
    auto stmt = std::make_unique<WhileStmt>();
    stmt->test = parseExpression();
    consume(TokenType::RightParen, "Expected ')'");
    stmt->body = parseStatement();
    return stmt;
}

std::unique_ptr<ASTNode> Parser::parseDoWhile() {
    consume(TokenType::Do, "Expected 'do'");
    auto stmt = std::make_unique<DoWhileStmt>();
    stmt->body = parseStatement();
    consume(TokenType::While, "Expected 'while'");
    consume(TokenType::LeftParen, "Expected '('");
    stmt->test = parseExpression();
    consume(TokenType::RightParen, "Expected ')'");
    match(TokenType::Semicolon);
    return stmt;
}

std::unique_ptr<ASTNode> Parser::parseSwitch() {
    consume(TokenType::Switch, "Expected 'switch'");
    consume(TokenType::LeftParen, "Expected '('");
    auto stmt = std::make_unique<SwitchStmt>();
    stmt->discriminant = parseExpression();
    consume(TokenType::RightParen, "Expected ')'");
    consume(TokenType::LeftBrace, "Expected '{'");
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        SwitchCase sc;
        if (match(TokenType::Case)) {
            sc.test = parseExpression();
            consume(TokenType::Colon, "Expected ':'");
        } else if (match(TokenType::Default)) {
            consume(TokenType::Colon, "Expected ':'");
        } else {
            throw SyntaxError("Expected 'case' or 'default' at line " + std::to_string(current().line));
        }
        while (!check(TokenType::Case) && !check(TokenType::Default) && !check(TokenType::RightBrace) && !isAtEnd()) {
            sc.consequent.push_back(parseStatement());
        }
        stmt->cases.push_back(std::move(sc));
    }
    consume(TokenType::RightBrace, "Expected '}'");
    return stmt;
}

std::unique_ptr<ASTNode> Parser::parseReturn() {
    consume(TokenType::Return, "Expected 'return'");
    auto stmt = std::make_unique<ReturnStmt>();
    if (!check(TokenType::Semicolon) && !check(TokenType::RightBrace) && !isAtEnd()) {
        stmt->argument = parseExpression();
    }
    match(TokenType::Semicolon);
    return stmt;
}

std::unique_ptr<ASTNode> Parser::parseBreak() {
    consume(TokenType::Break, "Expected 'break'");
    match(TokenType::Semicolon);
    return std::make_unique<BreakStmt>();
}

std::unique_ptr<ASTNode> Parser::parseContinue() {
    consume(TokenType::Continue, "Expected 'continue'");
    match(TokenType::Semicolon);
    return std::make_unique<ContinueStmt>();
}

std::unique_ptr<ASTNode> Parser::parseExprStmt() {
    auto stmt = std::make_unique<ExprStmt>();
    stmt->expression = parseExpression();
    match(TokenType::Semicolon);
    return stmt;
}

// ==================== EXPRESSIONS ====================

std::unique_ptr<ASTNode> Parser::parseExpression() {
    return parseAssignmentExpression();
}

std::unique_ptr<ASTNode> Parser::parseAssignmentExpression() {
    auto left = parseConditionalExpression();
    if (check(TokenType::Equal) || check(TokenType::PlusEqual) || check(TokenType::MinusEqual) ||
        check(TokenType::StarEqual) || check(TokenType::SlashEqual) || check(TokenType::PercentEqual)) {
        auto assign = std::make_unique<AssignExpr>();
        assign->op = advance().lexeme;
        assign->target = std::move(left);
        assign->value = parseAssignmentExpression();
        return assign;
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseConditionalExpression() {
    auto test = parseLogicalOrExpression();
    if (match(TokenType::Question)) {
        auto cond = std::make_unique<ConditionalExpr>();
        cond->test = std::move(test);
        cond->consequent = parseAssignmentExpression();
        consume(TokenType::Colon, "Expected ':'");
        cond->alternate = parseAssignmentExpression();
        return cond;
    }
    return test;
}

std::unique_ptr<ASTNode> Parser::parseLogicalOrExpression() {
    auto left = parseLogicalAndExpression();
    while (check(TokenType::PipePipe)) {
        auto logical = std::make_unique<LogicalExpr>();
        logical->op = advance().lexeme;
        logical->left = std::move(left);
        logical->right = parseLogicalAndExpression();
        left = std::move(logical);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseLogicalAndExpression() {
    auto left = parseBitwiseOrExpression();
    while (check(TokenType::AmpAmp)) {
        auto logical = std::make_unique<LogicalExpr>();
        logical->op = advance().lexeme;
        logical->left = std::move(left);
        logical->right = parseBitwiseOrExpression();
        left = std::move(logical);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseBitwiseOrExpression() {
    auto left = parseBitwiseXorExpression();
    while (check(TokenType::Pipe)) {
        auto bin = std::make_unique<BinaryExpr>();
        bin->op = advance().lexeme;
        bin->left = std::move(left);
        bin->right = parseBitwiseXorExpression();
        left = std::move(bin);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseBitwiseXorExpression() {
    auto left = parseBitwiseAndExpression();
    while (check(TokenType::Caret)) {
        auto bin = std::make_unique<BinaryExpr>();
        bin->op = advance().lexeme;
        bin->left = std::move(left);
        bin->right = parseBitwiseAndExpression();
        left = std::move(bin);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseBitwiseAndExpression() {
    auto left = parseEqualityExpression();
    while (check(TokenType::Amp)) {
        auto bin = std::make_unique<BinaryExpr>();
        bin->op = advance().lexeme;
        bin->left = std::move(left);
        bin->right = parseEqualityExpression();
        left = std::move(bin);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseEqualityExpression() {
    auto left = parseRelationalExpression();
    while (check(TokenType::EqualEqual) || check(TokenType::BangEqual) ||
           check(TokenType::EqualEqualEqual) || check(TokenType::BangEqualEqual)) {
        auto bin = std::make_unique<BinaryExpr>();
        bin->op = advance().lexeme;
        bin->left = std::move(left);
        bin->right = parseRelationalExpression();
        left = std::move(bin);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseRelationalExpression() {
    auto left = parseShiftExpression();
    while (check(TokenType::Less) || check(TokenType::Greater) ||
           check(TokenType::LessEqual) || check(TokenType::GreaterEqual) ||
           check(TokenType::Instanceof) || check(TokenType::In)) {
        auto bin = std::make_unique<BinaryExpr>();
        bin->op = advance().lexeme;
        bin->left = std::move(left);
        bin->right = parseShiftExpression();
        left = std::move(bin);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseShiftExpression() {
    auto left = parseAdditiveExpression();
    while (check(TokenType::LessLess) || check(TokenType::GreaterGreater) || check(TokenType::GreaterGreaterGreater)) {
        auto bin = std::make_unique<BinaryExpr>();
        bin->op = advance().lexeme;
        bin->left = std::move(left);
        bin->right = parseAdditiveExpression();
        left = std::move(bin);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseAdditiveExpression() {
    auto left = parseMultiplicativeExpression();
    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        auto bin = std::make_unique<BinaryExpr>();
        bin->op = advance().lexeme;
        bin->left = std::move(left);
        bin->right = parseMultiplicativeExpression();
        left = std::move(bin);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseMultiplicativeExpression() {
    auto left = parseExponentiationExpression();
    while (check(TokenType::Star) || check(TokenType::Slash) || check(TokenType::Percent)) {
        auto bin = std::make_unique<BinaryExpr>();
        bin->op = advance().lexeme;
        bin->left = std::move(left);
        bin->right = parseExponentiationExpression();
        left = std::move(bin);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseExponentiationExpression() {
    auto left = parseUnaryExpression();
    if (check(TokenType::StarStar)) {
        auto bin = std::make_unique<BinaryExpr>();
        bin->op = advance().lexeme;
        bin->left = std::move(left);
        bin->right = parseExponentiationExpression();
        return bin;
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseUnaryExpression() {
    if (check(TokenType::Minus) || check(TokenType::Plus) || check(TokenType::Bang) ||
        check(TokenType::Tilde) || check(TokenType::Typeof) || check(TokenType::Void)) {
        auto unary = std::make_unique<UnaryExpr>();
        unary->op = advance().lexeme;
        unary->argument = parseUnaryExpression();
        unary->prefix = true;
        return unary;
    }
    if (check(TokenType::PlusPlus) || check(TokenType::MinusMinus)) {
        auto update = std::make_unique<UpdateExpr>();
        update->op = advance().lexeme;
        update->argument = parseUnaryExpression();
        update->prefix = true;
        return update;
    }
    return parsePostfixExpression();
}

std::unique_ptr<ASTNode> Parser::parsePostfixExpression() {
    auto expr = parseCallExpression();
    if (check(TokenType::PlusPlus) || check(TokenType::MinusMinus)) {
        auto update = std::make_unique<UpdateExpr>();
        update->op = advance().lexeme;
        update->argument = std::move(expr);
        update->prefix = false;
        return update;
    }
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseCallExpression() {
    auto expr = parsePrimaryExpression();
    while (true) {
        if (match(TokenType::LeftParen)) {
            auto call = std::make_unique<CallExpr>();
            call->callee = std::move(expr);
            while (!check(TokenType::RightParen) && !isAtEnd()) {
                if (check(TokenType::Spread)) {
                    advance();
                    auto spread = std::make_unique<SpreadElement>();
                    spread->argument = parseAssignmentExpression();
                    call->arguments.push_back(std::move(spread));
                } else {
                    call->arguments.push_back(parseAssignmentExpression());
                }
                if (!check(TokenType::RightParen)) consume(TokenType::Comma, "Expected ','");
            }
            consume(TokenType::RightParen, "Expected ')'");
            expr = std::move(call);
        } else if (match(TokenType::Dot)) {
            auto member = std::make_unique<MemberExpr>();
            member->object = std::move(expr);
            auto prop = std::make_unique<Identifier>();
            prop->name = consume(TokenType::Identifier, "Expected property name").lexeme;
            member->property = std::move(prop);
            member->computed = false;
            expr = std::move(member);
        } else if (match(TokenType::LeftBracket)) {
            auto member = std::make_unique<MemberExpr>();
            member->object = std::move(expr);
            member->property = parseExpression();
            member->computed = true;
            consume(TokenType::RightBracket, "Expected ']'");
            expr = std::move(member);
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<ASTNode> Parser::parsePrimaryExpression() {
    if (check(TokenType::Number)) {
        auto lit = std::make_unique<Literal>();
        lit->literalType = LiteralType::Number;
        lit->numberValue = std::strtod(advance().lexeme.c_str(), nullptr);
        return lit;
    }
    if (check(TokenType::String)) {
        auto lit = std::make_unique<Literal>();
        lit->literalType = LiteralType::String;
        lit->stringValue = advance().lexeme;
        return lit;
    }
    if (check(TokenType::TemplateLiteral)) {
        return parseTemplateLiteral();
    }
    if (check(TokenType::True)) {
        advance();
        auto lit = std::make_unique<Literal>();
        lit->literalType = LiteralType::Boolean;
        lit->boolValue = true;
        return lit;
    }
    if (check(TokenType::False)) {
        advance();
        auto lit = std::make_unique<Literal>();
        lit->literalType = LiteralType::Boolean;
        lit->boolValue = false;
        return lit;
    }
    if (check(TokenType::Null)) {
        advance();
        auto lit = std::make_unique<Literal>();
        lit->literalType = LiteralType::Null;
        return lit;
    }
    if (check(TokenType::Undefined)) {
        advance();
        auto lit = std::make_unique<Literal>();
        lit->literalType = LiteralType::Undefined;
        return lit;
    }
    if (check(TokenType::Identifier)) {
        auto id = std::make_unique<Identifier>();
        id->name = advance().lexeme;
        // Check for single-parameter arrow function: x => ...
        if (check(TokenType::Arrow)) {
            advance();
            std::vector<std::string> params;
            params.push_back(id->name);
            std::vector<std::unique_ptr<ASTNode>> defaults;
            defaults.push_back(nullptr);
            return parseArrowFunctionBody(std::move(params), std::move(defaults), false, "");
        }
        return id;
    }
    if (check(TokenType::New)) {
        advance();
        auto newExpr = std::make_unique<NewExpr>();
        newExpr->callee = parsePrimaryExpression();
        if (match(TokenType::LeftParen)) {
            while (!check(TokenType::RightParen) && !isAtEnd()) {
                newExpr->arguments.push_back(parseAssignmentExpression());
                if (!check(TokenType::RightParen)) consume(TokenType::Comma, "Expected ','");
            }
            consume(TokenType::RightParen, "Expected ')'");
        }
        return newExpr;
    }
    if (check(TokenType::LeftParen)) {
        advance();
        // Check for arrow function: () =>, (x, y) =>, or (x = 1, ...rest) =>
        size_t savedPos = pos_;
        std::vector<std::string> arrowParams;
        std::vector<std::unique_ptr<ASTNode>> arrowDefaults;
        bool arrowHasRest = false;
        std::string arrowRestParam;

        bool validParams = true;
        try {
            parseFunctionParams(arrowParams, arrowDefaults, arrowHasRest, arrowRestParam);
        } catch (const SyntaxError&) {
            validParams = false;
        }
        if (validParams && check(TokenType::RightParen) && tokens_[pos_ + 1].type == TokenType::Arrow) {
            advance(); advance(); // skip ) and =>
            return parseArrowFunctionBody(std::move(arrowParams), std::move(arrowDefaults), arrowHasRest, arrowRestParam);
        }

        // Not an arrow function - restore position and parse as grouped expression
        pos_ = savedPos;
        auto expr = parseExpression();
        consume(TokenType::RightParen, "Expected ')'");
        return expr;
    }
    if (check(TokenType::LeftBracket)) return parseArrayLiteral();
    if (check(TokenType::LeftBrace)) return parseObjectLiteral();
    if (check(TokenType::Function)) {
        advance();
        auto fn = std::make_unique<FunctionDecl>();
        if (check(TokenType::Identifier)) fn->name = advance().lexeme;
        consume(TokenType::LeftParen, "Expected '('");
        parseFunctionParams(fn->params, fn->defaults, fn->hasRest, fn->restParam);
        consume(TokenType::RightParen, "Expected ')'");
        fn->body = parseBlock();
        return fn;
    }
    throw SyntaxError("Unexpected token '" + current().lexeme + "' at line " + std::to_string(current().line));
}

std::unique_ptr<ASTNode> Parser::parseArrayLiteral() {
    consume(TokenType::LeftBracket, "Expected '['");
    auto arr = std::make_unique<ArrayLiteral>();
    while (!check(TokenType::RightBracket) && !isAtEnd()) {
        if (check(TokenType::Spread)) {
            advance();
            auto spread = std::make_unique<SpreadElement>();
            spread->argument = parseAssignmentExpression();
            arr->elements.push_back(std::move(spread));
        } else {
            arr->elements.push_back(parseAssignmentExpression());
        }
        if (!check(TokenType::RightBracket)) consume(TokenType::Comma, "Expected ','");
    }
    consume(TokenType::RightBracket, "Expected ']'");
    return arr;
}

std::unique_ptr<ASTNode> Parser::parseObjectLiteral() {
    consume(TokenType::LeftBrace, "Expected '{'");
    auto obj = std::make_unique<ObjectLiteral>();
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        ObjectProperty prop;
        if (check(TokenType::Spread)) {
            advance();
            auto spread = std::make_unique<SpreadElement>();
            spread->argument = parseAssignmentExpression();
            prop.value = std::move(spread);
            obj->properties.push_back(std::move(prop));
            if (!check(TokenType::RightBrace)) consume(TokenType::Comma, "Expected ','");
            continue;
        } else if (check(TokenType::String)) {
            auto key = std::make_unique<Literal>();
            key->literalType = LiteralType::String;
            key->stringValue = advance().lexeme;
            prop.key = std::move(key);
        } else if (check(TokenType::Identifier)) {
            auto key = std::make_unique<Identifier>();
            key->name = advance().lexeme;
            prop.key = std::move(key);
        } else if (check(TokenType::Number)) {
            auto key = std::make_unique<Literal>();
            key->literalType = LiteralType::String;
            key->stringValue = advance().lexeme;
            prop.key = std::move(key);
        } else {
            throw SyntaxError("Expected property name at line " + std::to_string(current().line));
        }
        if (match(TokenType::Colon)) {
            prop.value = parseAssignmentExpression();
        } else {
            // Shorthand: { x } means { x: x }
            if (auto* id = dynamic_cast<Identifier*>(prop.key.get())) {
                auto val = std::make_unique<Identifier>();
                val->name = id->name;
                prop.value = std::move(val);
            }
        }
        obj->properties.push_back(std::move(prop));
        if (!check(TokenType::RightBrace)) consume(TokenType::Comma, "Expected ','");
    }
    consume(TokenType::RightBrace, "Expected '}'");
    return obj;
}

std::unique_ptr<ASTNode> Parser::parseTemplateLiteral() {
    std::string raw = advance().lexeme;
    auto tpl = std::make_unique<TemplateLiteral>();

    size_t i = 0;
    std::string currentStr;
    while (i < raw.size()) {
        if (i + 1 < raw.size() && raw[i] == '$' && raw[i + 1] == '{') {
            // Push the string part before this expression
            tpl->strings.push_back(currentStr);
            currentStr.clear();
            i += 2; // skip ${

            // Extract expression string, tracking brace depth
            std::string exprStr;
            int depth = 1;
            while (i < raw.size() && depth > 0) {
                if (raw[i] == '{') depth++;
                else if (raw[i] == '}') {
                    depth--;
                    if (depth == 0) { i++; break; }
                }
                exprStr += raw[i];
                i++;
            }

            // Sub-lex and sub-parse the expression
            Lexer subLexer(exprStr);
            auto subTokens = subLexer.tokenize();
            Parser subParser(std::move(subTokens));
            auto expr = subParser.parseExpression();
            tpl->expressions.push_back(std::move(expr));
        } else {
            currentStr += raw[i];
            i++;
        }
    }
    tpl->strings.push_back(currentStr);
    return tpl;
}

} // namespace jsling
