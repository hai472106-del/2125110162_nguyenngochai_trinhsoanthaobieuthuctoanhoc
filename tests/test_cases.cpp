// tests/test_cases.cpp
// File test doc lap, co main() rieng, dung chung structures.h/functions.cpp
// voi chuong trinh chinh nhung KHONG dung chung main() cua src/main.cpp.
//
// Bien dich:
//   g++ -std=c++17 -Wall -I../src test_cases.cpp ../src/functions.cpp -o run_tests
// Chay:
//   ./run_tests

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include "../src/structures.h"

struct TestCase {
    std::string name;
    std::string expr;          // bieu thuc infix (bo qua neu dung postfixDirect)
    std::map<std::string, double> vars;
    double expected;
    bool postfixDirect = false; // true neu test nhap postfix truc tiep (khong qua Shunting-Yard)
};

static bool approxEqual(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) < eps;
}

// Test rieng: xay Expression Tree truc tiep tu danh sach token postfix cho san
static double evalDirectPostfix() {
    std::vector<Token> postfix;
    postfix.push_back(Token(3.0));
    postfix.push_back(Token(4.0));
    postfix.push_back(Token(TokenType::OPERATOR, "+"));
    postfix.push_back(Token(2.0));
    postfix.push_back(Token(TokenType::OPERATOR, "*"));
    postfix.push_back(Token(1.0));
    postfix.push_back(Token(TokenType::OPERATOR, "-"));
    ExpressionTree tree;
    tree.buildFromPostfix(postfix);
    return tree.evaluate({});
}

int main() {
    std::vector<TestCase> tests = {
        { "Phep toan co ban + uu tien + ngoac", "(3 + 4) * 2 - 1", {}, 13.0, false },
        { "Xay cay truc tiep tu postfix (3 4 + 2 * 1 -)", "", {}, 13.0, true },
        { "Bieu thuc co bien: x^2 + 2*x + 1, x=5", "x^2 + 2*x + 1", {{"x", 5.0}}, 36.0, false },
        { "Phep chia + ngoac: 10 / (2 + 3)", "10 / (2 + 3)", {}, 2.0, false },
        { "Luy thua ket hop phai: 2 ^ 3 ^ 2 = 2^(3^2)", "2 ^ 3 ^ 2", {}, 512.0, false },
        { "Dau tru mot ngoi: -5 + 3", "-5 + 3", {}, -2.0, false },
        { "Nhieu bien: a*b + c, a=2,b=3,c=4", "a*b + c", {{"a",2.0},{"b",3.0},{"c",4.0}}, 10.0, false },
    };

    int passCount = 0;
    int total = (int)tests.size();

    std::cout << "=========================================\n";
    std::cout << " CHAY TEST CASES - Trinh Soan Thao Bieu Thuc Toan Hoc\n";
    std::cout << "=========================================\n\n";

    for (size_t i = 0; i < tests.size(); ++i) {
        const TestCase& tc = tests[i];
        std::cout << "Test " << (i + 1) << ": " << tc.name << "\n";
        try {
            double result;
            if (tc.postfixDirect) {
                result = evalDirectPostfix();
            } else {
                auto infixToks = tokenize(tc.expr);
                auto postfixToks = infixToPostfix(infixToks);
                ExpressionTree tree;
                tree.buildFromPostfix(postfixToks);
                result = tree.evaluate(tc.vars);
            }
            bool ok = approxEqual(result, tc.expected);
            std::cout << "  Ket qua = " << result << " | Ky vong = " << tc.expected
                      << " | " << (ok ? "PASS" : "FAIL") << "\n\n";
            if (ok) ++passCount;
        } catch (const std::exception& e) {
            std::cout << "  FAIL (loi ngoai le: " << e.what() << ")\n\n";
        }
    }

    std::cout << "=========================================\n";
    std::cout << " KET QUA: " << passCount << "/" << total << " test PASS\n";
    std::cout << "=========================================\n";

    return (passCount == total) ? 0 : 1;
}
