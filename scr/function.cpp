#include "structures.h"
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <iomanip>

// =====================================================================
//  Tokenizer
// =====================================================================
std::vector<Token> tokenize(const std::string& expr) {
    std::vector<Token> tokens;
    size_t i = 0;
    size_t n = expr.size();
    Token prevToken; // dung de phan biet dau +/- 1 ngoi hay 2 ngoi
    bool havePrev = false;

    while (i < n) {
        char c = expr[i];

        if (std::isspace((unsigned char)c)) { ++i; continue; }

        if (std::isdigit((unsigned char)c) || c == '.') {
            size_t start = i;
            bool dotSeen = (c == '.');
            ++i;
            while (i < n && (std::isdigit((unsigned char)expr[i]) || expr[i] == '.')) {
                if (expr[i] == '.') {
                    if (dotSeen) throw std::runtime_error("So khong hop le: nhieu dau '.'");
                    dotSeen = true;
                }
                ++i;
            }
            std::string numStr = expr.substr(start, i - start);
            double val = std::stod(numStr);
            Token t(val);
            tokens.push_back(t);
            prevToken = t; havePrev = true;
            continue;
        }

        if (std::isalpha((unsigned char)c)) {
            size_t start = i;
            ++i;
            while (i < n && (std::isalnum((unsigned char)expr[i]))) ++i;
            std::string name = expr.substr(start, i - start);
            Token t(TokenType::VARIABLE, name);
            tokens.push_back(t);
            prevToken = t; havePrev = true;
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
            // Xu ly dau +/- 1 ngoi: neu la dau tru dung dau bieu thuc hoac sau dau '(' hoac sau toan tu khac
            bool unary = (c == '-' || c == '+') &&
                         (!havePrev ||
                          prevToken.type == TokenType::OPERATOR ||
                          prevToken.type == TokenType::LPAREN);
            if (unary) {
                // bieu dien -x thanh (0 - x) bang cach chen so 0 truoc
                Token zero(0.0);
                tokens.push_back(zero);
            }
            Token t(TokenType::OPERATOR, std::string(1, c));
            tokens.push_back(t);
            prevToken = t; havePrev = true;
            ++i;
            continue;
        }

        if (c == '(') {
            Token t(TokenType::LPAREN, "(");
            tokens.push_back(t);
            prevToken = t; havePrev = true;
            ++i;
            continue;
        }

        if (c == ')') {
            Token t(TokenType::RPAREN, ")");
            tokens.push_back(t);
            prevToken = t; havePrev = true;
            ++i;
            continue;
        }

        throw std::runtime_error(std::string("Ky tu khong hop le trong bieu thuc: '") + c + "'");
    }

    if (tokens.empty()) throw std::runtime_error("Bieu thuc rong");

    return tokens;
}

// =====================================================================
//  Shunting-Yard: Infix -> Postfix
// =====================================================================
std::vector<Token> infixToPostfix(const std::vector<Token>& infixTokens) {
    Stack<Token> opStack;      // stack chua toan tu va dau ngoac
    Queue<Token> outputQueue;  // hang doi output theo thu tu postfix

    for (const Token& tok : infixTokens) {
        switch (tok.type) {
            case TokenType::NUMBER:
            case TokenType::VARIABLE:
                outputQueue.enqueue(tok);
                break;

            case TokenType::OPERATOR: {
                while (!opStack.empty() &&
                       opStack.top().type == TokenType::OPERATOR &&
                       (precedence(opStack.top().text) > precedence(tok.text) ||
                        (precedence(opStack.top().text) == precedence(tok.text) &&
                         !isRightAssociative(tok.text)))) {
                    outputQueue.enqueue(opStack.top());
                    opStack.pop();
                }
                opStack.push(tok);
                break;
            }

            case TokenType::LPAREN:
                opStack.push(tok);
                break;

            case TokenType::RPAREN: {
                bool foundLParen = false;
                while (!opStack.empty()) {
                    if (opStack.top().type == TokenType::LPAREN) {
                        opStack.pop();
                        foundLParen = true;
                        break;
                    }
                    outputQueue.enqueue(opStack.top());
                    opStack.pop();
                }
                if (!foundLParen) throw std::runtime_error("Bieu thuc sai: thieu dau '('");
                break;
            }
        }
    }

    while (!opStack.empty()) {
        if (opStack.top().type == TokenType::LPAREN)
            throw std::runtime_error("Bieu thuc sai: thieu dau ')'");
        outputQueue.enqueue(opStack.top());
        opStack.pop();
    }

    std::vector<Token> result;
    while (!outputQueue.empty()) {
        result.push_back(outputQueue.front());
        outputQueue.dequeue();
    }
    return result;
}

std::string tokensToString(const std::vector<Token>& tokens) {
    std::ostringstream oss;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i) oss << " ";
        oss << tokens[i].toString();
    }
    return oss.str();
}

// =====================================================================
//  ExpressionTree
// =====================================================================
ExpressionTree::ExpressionTree(const ExpressionTree& other) {
    root = copy(other.root);
}

ExpressionTree& ExpressionTree::operator=(const ExpressionTree& other) {
    if (this != &other) {
        destroy(root);
        root = copy(other.root);
    }
    return *this;
}

ExpressionTree::~ExpressionTree() {
    destroy(root);
}

void ExpressionTree::destroy(TreeNode* node) {
    if (!node) return;
    destroy(node->left);
    destroy(node->right);
    delete node;
}

TreeNode* ExpressionTree::copy(TreeNode* node) const {
    if (!node) return nullptr;
    TreeNode* n = new TreeNode(node->token);
    n->left = copy(node->left);
    n->right = copy(node->right);
    return n;
}

void ExpressionTree::buildFromPostfix(const std::vector<Token>& postfixTokens) {
    destroy(root);
    root = nullptr;

    Stack<TreeNode*> nodeStack;

    for (const Token& tok : postfixTokens) {
        if (tok.type == TokenType::NUMBER || tok.type == TokenType::VARIABLE) {
            nodeStack.push(new TreeNode(tok));
        } else if (tok.type == TokenType::OPERATOR) {
            if (nodeStack.size() < 2)
                throw std::runtime_error("Bieu thuc postfix khong hop le (thieu toan hang)");
            TreeNode* right = nodeStack.top(); nodeStack.pop();
            TreeNode* left = nodeStack.top(); nodeStack.pop();
            TreeNode* opNode = new TreeNode(tok);
            opNode->left = left;
            opNode->right = right;
            nodeStack.push(opNode);
        } else {
            throw std::runtime_error("Token khong hop le khi xay cay bieu thuc");
        }
    }

    if (nodeStack.size() != 1)
        throw std::runtime_error("Bieu thuc postfix khong hop le (du toan hang/toan tu)");

    root = nodeStack.top();
    nodeStack.pop();
}

double ExpressionTree::evalNode(TreeNode* node, const std::map<std::string, double>& vars) const {
    if (!node) throw std::runtime_error("Cay bieu thuc rong");

    if (node->token.type == TokenType::NUMBER) return node->token.number;

    if (node->token.type == TokenType::VARIABLE) {
        auto it = vars.find(node->token.text);
        if (it == vars.end())
            throw std::runtime_error("Bien '" + node->token.text + "' chua duoc gan gia tri");
        return it->second;
    }

    double l = evalNode(node->left, vars);
    double r = evalNode(node->right, vars);
    const std::string& op = node->token.text;

    if (op == "+") return l + r;
    if (op == "-") return l - r;
    if (op == "*") return l * r;
    if (op == "/") {
        if (r == 0.0) throw std::runtime_error("Loi: chia cho 0");
        return l / r;
    }
    if (op == "^") return std::pow(l, r);

    throw std::runtime_error("Toan tu khong hop le: " + op);
}

double ExpressionTree::evaluate(const std::map<std::string, double>& vars) const {
    if (!root) throw std::runtime_error("Cay bieu thuc rong, hay nhap bieu thuc truoc");
    return evalNode(root, vars);
}

void ExpressionTree::toInfixHelper(TreeNode* node, std::string& out) const {
    if (!node) return;
    bool isLeaf = (!node->left && !node->right);
    if (!isLeaf) out += "(";
    toInfixHelper(node->left, out);
    out += node->token.toString();
    toInfixHelper(node->right, out);
    if (!isLeaf) out += ")";
}

void ExpressionTree::toPrefixHelper(TreeNode* node, std::string& out) const {
    if (!node) return;
    out += node->token.toString();
    if (node->left || node->right) out += " ";
    toPrefixHelper(node->left, out);
    if (node->left && node->right) out += " ";
    toPrefixHelper(node->right, out);
}

void ExpressionTree::toPostfixHelper(TreeNode* node, std::string& out) const {
    if (!node) return;
    toPostfixHelper(node->left, out);
    if (node->left) out += " ";
    toPostfixHelper(node->right, out);
    if (node->right) out += " ";
    out += node->token.toString();
}

std::string ExpressionTree::toInfix() const {
    if (!root) return "";
    std::string out;
    toInfixHelper(root, out);
    return out;
}

std::string ExpressionTree::toPrefix() const {
    if (!root) return "";
    std::string out;
    toPrefixHelper(root, out);
    return out;
}

std::string ExpressionTree::toPostfix() const {
    if (!root) return "";
    std::string out;
    toPostfixHelper(root, out);
    return out;
}

// In cay nghieng 90 do: goc ben trai, la ben phai
static void printAsciiRec(TreeNode* node, const std::string& prefix, bool isLeft, bool isRoot, std::ostringstream& oss) {
    if (!node) return;
    std::string childPrefix = prefix + (isRoot ? "   " : (isLeft ? "|  " : "   "));
    printAsciiRec(node->right, childPrefix, false, false, oss);

    oss << prefix;
    if (!isRoot) oss << (isLeft ? "\\--" : "/--");
    oss << node->token.toString() << "\n";

    std::string childPrefix2 = prefix + (isRoot ? "   " : (isLeft ? "   " : "|  "));
    printAsciiRec(node->left, childPrefix2, true, false, oss);
}

std::string ExpressionTree::toAsciiArt() const {
    if (!root) return "(cay rong)";
    std::ostringstream oss;
    printAsciiRec(root, "", false, true, oss);
    return oss.str();
}

int ExpressionTree::height(TreeNode* node) const {
    if (!node) return 0;
    int hl = height(node->left);
    int hr = height(node->right);
    return 1 + (hl > hr ? hl : hr);
}

// =====================================================================
//  History
// =====================================================================
void History::add(const std::string& infix, const std::string& postfix, double result) {
    std::ostringstream oss;
    oss << "Infix: " << infix
        << "  |  Postfix: " << postfix
        << "  |  Ket qua = " << std::fixed << std::setprecision(4) << result;
    records.push(oss.str());
}

std::vector<std::string> History::getAllNewestFirst() const {
    std::vector<std::string> out;
    Stack<std::string> copyStack(records); // dung constructor sao chep
    while (!copyStack.empty()) {
        out.push_back(copyStack.top());
        copyStack.pop();
    }
    return out;
}
