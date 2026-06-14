#pragma once
#include "token.hpp"
#include "ast.hpp"
#include <vector>
#include <memory>

namespace jsling {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::unique_ptr<Program> parse();

private:
    std::vector<Token> tokens_;
    size_t pos_ = 0;

    const Token& current() const;
    const Token& peek() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& consume(TokenType type, const std::string& msg);
    const Token& advance();
    bool isAtEnd() const;

    // Statements
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<BlockStmt> parseBlock();
    std::unique_ptr<ASTNode> parseVarDecl();
    std::unique_ptr<ASTNode> parseFunctionDecl();
    std::unique_ptr<ASTNode> parseIf();
    std::unique_ptr<ASTNode> parseFor();
    std::unique_ptr<ASTNode> parseWhile();
    std::unique_ptr<ASTNode> parseDoWhile();
    std::unique_ptr<ASTNode> parseSwitch();
    std::unique_ptr<ASTNode> parseReturn();
    std::unique_ptr<ASTNode> parseBreak();
    std::unique_ptr<ASTNode> parseContinue();
    std::unique_ptr<ASTNode> parseExprStmt();

    // Expressions (precedence climbing)
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseAssignmentExpression();
    std::unique_ptr<ASTNode> parseConditionalExpression();
    std::unique_ptr<ASTNode> parseLogicalOrExpression();
    std::unique_ptr<ASTNode> parseLogicalAndExpression();
    std::unique_ptr<ASTNode> parseBitwiseOrExpression();
    std::unique_ptr<ASTNode> parseBitwiseXorExpression();
    std::unique_ptr<ASTNode> parseBitwiseAndExpression();
    std::unique_ptr<ASTNode> parseEqualityExpression();
    std::unique_ptr<ASTNode> parseRelationalExpression();
    std::unique_ptr<ASTNode> parseShiftExpression();
    std::unique_ptr<ASTNode> parseAdditiveExpression();
    std::unique_ptr<ASTNode> parseMultiplicativeExpression();
    std::unique_ptr<ASTNode> parseExponentiationExpression();
    std::unique_ptr<ASTNode> parseUnaryExpression();
    std::unique_ptr<ASTNode> parsePostfixExpression();
    std::unique_ptr<ASTNode> parseCallExpression();
    std::unique_ptr<ASTNode> parsePrimaryExpression();

    // Helpers
    std::vector<std::unique_ptr<ASTNode>> parseArguments();
    std::unique_ptr<ASTNode> parseArrayLiteral();
    std::unique_ptr<ASTNode> parseObjectLiteral();
    std::unique_ptr<ASTNode> parseTemplateLiteral();
    std::unique_ptr<ASTNode> parseArrowFunctionBody(std::vector<std::string> params, std::vector<std::unique_ptr<ASTNode>> defaults, bool hasRest, const std::string& restParam);
    void parseFunctionParams(std::vector<std::string>& params, std::vector<std::unique_ptr<ASTNode>>& defaults, bool& hasRest, std::string& restParam);
};

} // namespace jsling
