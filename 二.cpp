#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stack>
#include <string>
#include <map>
#include <cctype>
#include <cmath>
#include <vector>
#include <stdexcept>

// 计算a op b的结果，返回pair(是否有效, 计算结果)
std::pair<bool, double> calculateOp(double a, double b, char op) {
    switch (op) {
    case '+': return std::make_pair(true, a + b);
    case '-': return std::make_pair(true, a - b);
    case '*': return std::make_pair(true, a * b);
    case '/':
        if (fabs(b) < 1e-9) return std::make_pair(false, 0);  // 处理除数为0（考虑浮点数精度）
        return std::make_pair(true, a / b);
    default: return std::make_pair(false, 0);
    }
}

// 字符串计算器核心函数
std::string calculate(const std::string& expr) {
    std::stack<double> numStack;       // 操作数栈
    std::stack<char> opStack;          // 运算符栈
    std::map<char, int> priority = {   // 优先级表
        {'(', 0}, {'+', 1}, {'-', 1}, {'*', 2}, {'/' , 2}
    };
    int n = expr.size();
    int i = 0;

    while (i < n) {
        char c = expr[i];
        if (isspace(c)) {  // 跳过空格
            i++;
            continue;
        }

        // 处理数字（整数/浮点数）
        if (isdigit(c) || c == '.') {
            int j = i;
            int dotCount = 0;
            while (j < n && (isdigit(expr[j]) || expr[j] == '.')) {
                if (expr[j] == '.') {
                    dotCount++;
                    if (dotCount > 1) {  // 多个小数点无效
                        return "式子无效";
                    }
                }
                j++;
            }
            // 转换数字（处理空字符串情况）
            if (j == i) {  // 没有有效数字
                return "式子无效";
            }
            std::string numStr = expr.substr(i, j - i);
            double num;
            try {
                num = std::stod(numStr);
            }
            catch (...) {
                return "式子无效";
            }
            numStack.push(num);
            i = j;
        }
        // 处理左括号
        else if (c == '(') {
            opStack.push(c);
            i++;
        }
        // 处理右括号
        else if (c == ')') {
            // 弹出运算符直到遇到左括号
            while (!opStack.empty() && opStack.top() != '(') {
                char op = opStack.top();
                opStack.pop();
                if (numStack.size() < 2) {
                    return "式子无效";
                }
                double b = numStack.top(); numStack.pop();
                double a = numStack.top(); numStack.pop();
                std::pair<bool, double> resPair = calculateOp(a, b, op);
                if (!resPair.first) {
                    return "式子无效";
                }
                numStack.push(resPair.second);
            }
            if (opStack.empty()) {  // 无匹配左括号
                return "式子无效";
            }
            opStack.pop();  // 弹出左括号
            i++;
        }
        // 处理运算符
        else if (c == '+' || c == '-' || c == '*' || c == '/') {
            // 处理负号（表达式开头或左括号/运算符后的'-'）
            if (c == '-' && (i == 0 || expr[i - 1] == '(' ||
                expr[i - 1] == '+' || expr[i - 1] == '-' ||
                expr[i - 1] == '*' || expr[i - 1] == '/')) {
                numStack.push(0.0);  // 负号等价于0 - 数
            }
            // 弹出高优先级运算符先计算
            while (!opStack.empty() && opStack.top() != '(' &&
                priority[c] <= priority[opStack.top()]) {
                char op = opStack.top();
                opStack.pop();
                if (numStack.size() < 2) {
                    return "式子无效";
                }
                double b = numStack.top(); numStack.pop();
                double a = numStack.top(); numStack.pop();
                std::pair<bool, double> resPair = calculateOp(a, b, op);
                if (!resPair.first) {
                    return "式子无效";
                }
                numStack.push(resPair.second);
            }
            opStack.push(c);
            i++;
        }
        // 无效字符
        else {
            return "式子无效";
        }
    }

    // 处理剩余运算符
    while (!opStack.empty()) {
        char op = opStack.top();
        opStack.pop();
        if (op == '(') {  // 未匹配左括号
            return "式子无效";
        }
        if (numStack.size() < 2) {
            return "式子无效";
        }
        double b = numStack.top(); numStack.pop();
        double a = numStack.top(); numStack.pop();
        std::pair<bool, double> resPair = calculateOp(a, b, op);
        if (!resPair.first) {
            return "式子无效";
        }
        numStack.push(resPair.second);
    }

    // 检查结果是否唯一
    if (numStack.size() != 1) {
        return "式子无效";
    }

    // 处理整数显示（去除多余的.0）
    double result = numStack.top();
    if (fabs(result - round(result)) < 1e-9) {  // 是整数
        return std::to_string(static_cast<long long>(round(result)));
    }
    else {
        // 优化浮点数显示（避免过多小数位）
        std::string resStr = std::to_string(result);
        resStr.erase(resStr.find_last_not_of('0') + 1, std::string::npos);
        if (resStr.back() == '.') {
            resStr.pop_back();
        }
        return resStr;
    }
}

// 测试
int main() {
    // 有效表达式测试
    std::vector<std::string> validTests = {
        "1+2",
        "1+2*3",
        "(1+2)*3",
        "6/2",
        "5/2",
        "-1+2",
        "3+(-2)",
        "( -1 - 2 )*3",
        "123+456",
        "1.5+2.5",
        "2.5*2",
        "12 + 34 * 2",
        "(10 + 20)*(30-25)/5",
        "5 - (-3)",
        "0.1+0.2"  // 新增浮点数精度测试
    };

    std::cout << "有效表达式测试：" << std::endl;
    for (const auto& expr : validTests) {
        std::cout << expr << " = " << calculate(expr) << std::endl;
    }

    // 无效表达式测试
    std::vector<std::string> invalidTests = {
        "(1+2",       // 括号不匹配
        "1++2",       // 连续运算符
        "5/0",        // 除数为0
        "12.3.4",     // 多个小数点
        "a+1",        // 无效字符
        ".5",         // 不完整数字（可选：可扩展支持，这里视为无效）
        "5."          // 不完整数字
    };

    std::cout << "\n无效表达式测试：" << std::endl;
    for (const auto& expr : invalidTests) {
        std::cout << expr << " = " << calculate(expr) << std::endl;
    }

    return 0;
}
