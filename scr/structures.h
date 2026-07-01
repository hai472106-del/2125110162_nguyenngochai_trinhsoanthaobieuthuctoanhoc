#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdexcept>
#include <cstddef>
#include <vector>
#include <string>
#include <map>

// =====================================================================
//  1) Stack<T> - cai dat bang danh sach lien ket don (LIFO)
//     Dung trong: Shunting-Yard (toan tu), xay Expression Tree tu
//     postfix, va luu Lich su tinh toan (History).
// =====================================================================
template <typename T>
class Stack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& d, Node* n = nullptr) : data(d), next(n) {}
    };
    Node* topNode;
    size_t count;

public:
    Stack() : topNode(nullptr), count(0) {}

    Stack(const Stack& other) : topNode(nullptr), count(0) {
        // sao chep giu nguyen thu tu (dinh -> day)
        std::vector<Node*> nodes;
        for (Node* p = other.topNode; p; p = p->next) nodes.push_back(p);
        for (size_t i = nodes.size(); i-- > 0;) push(nodes[i]->data);
    }
    Stack& operator=(const Stack&) = delete;

    ~Stack() { clear(); }

    void push(const T& value) {
        topNode = new Node(value, topNode);
        ++count;
    }

    void pop() {
        if (empty()) throw std::runtime_error("Stack rong: khong the pop");
        Node* tmp = topNode;
        topNode = topNode->next;
        delete tmp;
        --count;
    }

    T& top() {
        if (empty()) throw std::runtime_error("Stack rong: khong co phan tu top");
        return topNode->data;
    }

    const T& top() const {
        if (empty()) throw std::runtime_error("Stack rong: khong co phan tu top");
        return topNode->data;
    }

    bool empty() const { return topNode == nullptr; }
    size_t size() const { return count; }

    void clear() {
        while (!empty()) pop();
    }
};

// =====================================================================
//  2) Queue<T> - cai dat bang danh sach lien ket don (FIFO)
//     Dung trong: hang doi token dau ra cua thuat toan Shunting-Yard.
// =====================================================================
template <typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& d, Node* n = nullptr) : data(d), next(n) {}
    };
    Node* frontNode;
    Node* backNode;
    size_t count;

public:
    Queue() : frontNode(nullptr), backNode(nullptr), count(0) {}

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    ~Queue() { clear(); }

    void enqueue(const T& value) {
        Node* node = new Node(value);
        if (backNode) backNode->next = node;
        else frontNode = node;
        backNode = node;
        ++count;
    }

    void dequeue() {
        if (empty()) throw std::runtime_error("Queue rong: khong the dequeue");
        Node* tmp = frontNode;
        frontNode = frontNode->next;
        if (!frontNode) backNode = nullptr;
        delete tmp;
        --count;
    }

    T& front() {
        if (empty()) throw std::runtime_error("Queue rong: khong co phan tu front");
        return frontNode->data;
    }

    const T& front() const {
        if (empty()) throw std::runtime_error("Queue rong: khong co phan tu front");
        return frontNode->data;
    }

    bool empty() const { return frontNode == nullptr; }
    size_t size() const { return count; }

    void clear() {
        while (!empty()) dequeue();
    }
};

// =====================================================================
//  3) Token - don vi nho nhat cua bieu thuc (so, bien, toan tu, ngoac)
// =====================================================================
enum class TokenType {
    NUMBER,     // vi du: 3, 4.5
    VARIABLE,   // vi du: x, y
    OPERATOR,   // + - * / ^
    LPAREN,     // (
    RPAREN      // )
};

struct Token {
    TokenType type;
    double number = 0.0;   // dung khi type == NUMBER
    std::string text;      // dung khi type == VARIABLE hoac OPERATOR ( "+","-","*","/","^" )

    Token() : type(TokenType::NUMBER), number(0.0) {}
    Token(TokenType t, const std::string& s) : type(t), text(s) {}
    Token(double v) : type(TokenType::NUMBER), number(v) {}

    std::string toString() const {
        if (type == TokenType::NUMBER) {
            if (number == (long long)number)
                return std::to_string((long long)number);
            return std::to_string(number);
        }
        return text;
    }
};

// Do uu tien toan tu
inline int precedence(const std::string& op) {
    if (op == "+" || op == "-") return 1;
    if (op == "*" || op == "/") return 2;
    if (op == "^") return 3;
    return 0;
}

// Tinh ket hop ben phai (^ la right-associative)
inline bool isRightAssociative(const std::string& op) {
    return op == "^";
}

// =====================================================================
//  4) Tokenizer - tach chuoi bieu thuc thanh danh sach Token
// =====================================================================
// Nem std::runtime_error neu bieu thuc chua ky tu khong hop le
std::vector<Token> tokenize(const std::string& expr);

// =====================================================================
//  5) Shunting-Yard - chuyen Infix -> Postfix (dung Stack + Queue)
// =====================================================================
std::vector<Token> infixToPostfix(const std::vector<Token>& infixTokens);

// Ghep danh sach token thanh chuoi de hien thi, cach nhau boi khoang trang
std::string tokensToString(const std::vector<Token>& tokens);

// =====================================================================
//  6) TreeNode / ExpressionTree - Binary Tree luu cau truc bieu thuc
// =====================================================================
struct TreeNode {
    Token token;
    TreeNode* left;
    TreeNode* right;
    TreeNode(const Token& t) : token(t), left(nullptr), right(nullptr) {}
};

class ExpressionTree {
private:
    TreeNode* root;

    void destroy(TreeNode* node);
    TreeNode* copy(TreeNode* node) const;
    double evalNode(TreeNode* node, const std::map<std::string, double>& vars) const;
    void toInfixHelper(TreeNode* node, std::string& out) const;
    void toPrefixHelper(TreeNode* node, std::string& out) const;
    void toPostfixHelper(TreeNode* node, std::string& out) const;
    int height(TreeNode* node) const;

public:
    ExpressionTree() : root(nullptr) {}
    ExpressionTree(const ExpressionTree& other);
    ExpressionTree& operator=(const ExpressionTree& other);
    ~ExpressionTree();

    bool empty() const { return root == nullptr; }

    // Xay cay tu danh sach token postfix (dung Stack<TreeNode*>)
    void buildFromPostfix(const std::vector<Token>& postfixTokens);

    double evaluate(const std::map<std::string, double>& vars) const;

    std::string toInfix() const;
    std::string toPrefix() const;
    std::string toPostfix() const;

    // Ve cay dang ASCII ra std::string (nhieu dong)
    std::string toAsciiArt() const;
};

// =====================================================================
//  7) History - luu lich su tinh toan bang Stack<string>
// =====================================================================
class History {
private:
    Stack<std::string> records;
public:
    void add(const std::string& infix, const std::string& postfix, double result);

    // Tra ve danh sach ban ghi theo thu tu tu moi nhat -> cu nhat (dac tinh LIFO cua Stack)
    std::vector<std::string> getAllNewestFirst() const;

    bool empty() const { return records.empty(); }
};

#endif // STRUCTURES_H
