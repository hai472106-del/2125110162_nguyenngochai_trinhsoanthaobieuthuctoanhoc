#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "structures.h"

static void printLine() {
    std::cout << "----------------------------------------------------\n";
}

static void printMenu() {
    printLine();
    std::cout << "  TRINH SOAN THAO BIEU THUC TOAN HOC\n";
    printLine();
    std::cout << "1. Nhap bieu thuc moi (Infix)\n";
    std::cout << "2. Xem dang Postfix (RPN)\n";
    std::cout << "3. Tinh gia tri bieu thuc\n";
    std::cout << "4. Ve Expression Tree (ASCII)\n";
    std::cout << "5. In 3 dang: Infix / Prefix / Postfix (tu cay)\n";
    std::cout << "6. Gan bien (vd: x=5) va tinh bieu thuc theo bien\n";
    std::cout << "7. Xem lich su tinh toan\n";
    std::cout << "0. Thoat\n";
    std::cout << "(Cac test case tu dong: chay file rieng 'run_tests', xem tests/test_cases.cpp)\n";
    printLine();
    std::cout << "Chon chuc nang: ";
}

// Doc mot dong an toan
static std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

// Trang thai chuong trinh
struct AppState {
    std::string currentInfix;
    std::vector<Token> infixTokens;
    std::vector<Token> postfixTokens;
    ExpressionTree tree;
    std::map<std::string, double> variables;
    History history;
    bool hasExpression = false;
};

static void doEnterExpression(AppState& st) {
    std::string expr = readLine("Nhap bieu thuc infix (vd: (3 + 4) * 2 - 1): ");
    try {
        st.infixTokens = tokenize(expr);
        st.postfixTokens = infixToPostfix(st.infixTokens);
        st.tree.buildFromPostfix(st.postfixTokens);
        st.currentInfix = expr;
        st.hasExpression = true;
        std::cout << ">> Da nhap va parse thanh cong.\n";
        std::cout << "   Postfix (RPN): " << tokensToString(st.postfixTokens) << "\n";
    } catch (const std::exception& e) {
        std::cout << ">> Loi: " << e.what() << "\n";
        st.hasExpression = false;
    }
}

static bool checkHasExpr(const AppState& st) {
    if (!st.hasExpression) {
        std::cout << ">> Ban chua nhap bieu thuc hop le. Hay chon muc 1 truoc.\n";
        return false;
    }
    return true;
}

static void doShowPostfix(AppState& st) {
    if (!checkHasExpr(st)) return;
    std::cout << "Postfix (RPN): " << tokensToString(st.postfixTokens) << "\n";
}

// Thu thap danh sach bien co trong bieu thuc hien tai
static std::vector<std::string> collectVariableNames(const std::vector<Token>& tokens) {
    std::vector<std::string> names;
    for (const Token& t : tokens) {
        if (t.type == TokenType::VARIABLE) {
            bool found = false;
            for (auto& n : names) if (n == t.text) { found = true; break; }
            if (!found) names.push_back(t.text);
        }
    }
    return names;
}

static void doEvaluate(AppState& st) {
    if (!checkHasExpr(st)) return;
    auto varNames = collectVariableNames(st.infixTokens);
    for (auto& name : varNames) {
        if (st.variables.find(name) == st.variables.end()) {
            std::string s = readLine("Nhap gia tri cho bien '" + name + "': ");
            try {
                st.variables[name] = std::stod(s);
            } catch (...) {
                std::cout << ">> Gia tri khong hop le, dat mac dinh = 0\n";
                st.variables[name] = 0;
            }
        } else {
            std::cout << "(dung gia tri da luu: " << name << " = " << st.variables[name] << ")\n";
        }
    }
    try {
        double result = st.tree.evaluate(st.variables);
        std::cout << ">> Ket qua = " << result << "\n";
        st.history.add(st.currentInfix, tokensToString(st.postfixTokens), result);
    } catch (const std::exception& e) {
        std::cout << ">> Loi khi tinh gia tri: " << e.what() << "\n";
    }
}

static void doDrawTree(AppState& st) {
    if (!checkHasExpr(st)) return;
    std::cout << "Expression Tree (nghieng 90 do, goc ben trai):\n\n";
    std::cout << st.tree.toAsciiArt() << "\n";
}

static void doShowThreeForms(AppState& st) {
    if (!checkHasExpr(st)) return;
    std::cout << "Infix   : " << st.tree.toInfix() << "\n";
    std::cout << "Prefix  : " << st.tree.toPrefix() << "\n";
    std::cout << "Postfix : " << st.tree.toPostfix() << "\n";
}

static void doAssignVariable(AppState& st) {
    std::string s = readLine("Nhap gan bien (vd: x=5): ");
    size_t pos = s.find('=');
    if (pos == std::string::npos) {
        std::cout << ">> Cu phap sai. Vi du dung: x=5\n";
        return;
    }
    std::string name = s.substr(0, pos);
    std::string valStr = s.substr(pos + 1);
    // trim khoang trang
    auto trim = [](std::string x) {
        size_t a = x.find_first_not_of(" \t");
        size_t b = x.find_last_not_of(" \t");
        if (a == std::string::npos) return std::string("");
        return x.substr(a, b - a + 1);
    };
    name = trim(name);
    valStr = trim(valStr);
    try {
        double val = std::stod(valStr);
        st.variables[name] = val;
        std::cout << ">> Da gan " << name << " = " << val << "\n";
    } catch (...) {
        std::cout << ">> Gia tri khong hop le\n";
        return;
    }

    if (st.hasExpression) {
        std::string ans = readLine("Ban co muon tinh lai bieu thuc hien tai voi bien vua gan khong? (y/n): ");
        if (!ans.empty() && (ans[0] == 'y' || ans[0] == 'Y')) {
            doEvaluate(st);
        }
    }
}

static void doShowHistory(const AppState& st) {
    if (st.history.empty()) {
        std::cout << ">> Chua co lich su tinh toan nao.\n";
        return;
    }
    auto recs = st.history.getAllNewestFirst();
    std::cout << "Lich su tinh toan (moi nhat truoc):\n";
    int idx = 1;
    for (auto& r : recs) {
        std::cout << "  [" << idx++ << "] " << r << "\n";
    }
}

int main() {
    AppState st;
    std::cout << "Chao mung den voi Trinh Soan Thao Bieu Thuc Toan Hoc!\n";

    while (true) {
        printMenu();
        std::string choice;
        std::getline(std::cin, choice);
        if (choice.empty()) continue;

        char c = choice[0];
        switch (c) {
            case '1': doEnterExpression(st); break;
            case '2': doShowPostfix(st); break;
            case '3': doEvaluate(st); break;
            case '4': doDrawTree(st); break;
            case '5': doShowThreeForms(st); break;
            case '6': doAssignVariable(st); break;
            case '7': doShowHistory(st); break;
            case '0':
                std::cout << "Tam biet!\n";
                return 0;
            default:
                std::cout << ">> Lua chon khong hop le, vui long thu lai.\n";
        }
        std::cout << "\n";
    }
    return 0;
}
