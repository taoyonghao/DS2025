#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stack>
#include <string>
#include <map>
#include <cctype>
#include <cmath>
#include <vector>
#include <stdexcept>

// ����a op b�Ľ��������pair(�Ƿ���Ч, ������)
std::pair<bool, double> calculateOp(double a, double b, char op) {
    switch (op) {
    case '+': return std::make_pair(true, a + b);
    case '-': return std::make_pair(true, a - b);
    case '*': return std::make_pair(true, a * b);
    case '/':
        if (fabs(b) < 1e-9) return std::make_pair(false, 0);  // �������Ϊ0�����Ǹ��������ȣ�
        return std::make_pair(true, a / b);
    default: return std::make_pair(false, 0);
    }
}

// �ַ������������ĺ���
std::string calculate(const std::string& expr) {
    std::stack<double> numStack;       // ������ջ
    std::stack<char> opStack;          // �����ջ
    std::map<char, int> priority = {   // ���ȼ���
        {'(', 0}, {'+', 1}, {'-', 1}, {'*', 2}, {'/' , 2}
    };
    int n = expr.size();
    int i = 0;

    while (i < n) {
        char c = expr[i];
        if (isspace(c)) {  // �����ո�
            i++;
            continue;
        }

        // �������֣�����/��������
        if (isdigit(c) || c == '.') {
            int j = i;
            int dotCount = 0;
            while (j < n && (isdigit(expr[j]) || expr[j] == '.')) {
                if (expr[j] == '.') {
                    dotCount++;
                    if (dotCount > 1) {  // ���С������Ч
                        return "ʽ����Ч";
                    }
                }
                j++;
            }
            // ת�����֣�������ַ��������
            if (j == i) {  // û����Ч����
                return "ʽ����Ч";
            }
            std::string numStr = expr.substr(i, j - i);
            double num;
            try {
                num = std::stod(numStr);
            }
            catch (...) {
                return "ʽ����Ч";
            }
            numStack.push(num);
            i = j;
        }
        // ����������
        else if (c == '(') {
            opStack.push(c);
            i++;
        }
        // ����������
        else if (c == ')') {
            // ���������ֱ������������
            while (!opStack.empty() && opStack.top() != '(') {
                char op = opStack.top();
                opStack.pop();
                if (numStack.size() < 2) {
                    return "ʽ����Ч";
                }
                double b = numStack.top(); numStack.pop();
                double a = numStack.top(); numStack.pop();
                std::pair<bool, double> resPair = calculateOp(a, b, op);
                if (!resPair.first) {
                    return "ʽ����Ч";
                }
                numStack.push(resPair.second);
            }
            if (opStack.empty()) {  // ��ƥ��������
                return "ʽ����Ч";
            }
            opStack.pop();  // ����������
            i++;
        }
        // ���������
        else if (c == '+' || c == '-' || c == '*' || c == '/') {
            // �����ţ����ʽ��ͷ��������/��������'-'��
            if (c == '-' && (i == 0 || expr[i - 1] == '(' ||
                expr[i - 1] == '+' || expr[i - 1] == '-' ||
                expr[i - 1] == '*' || expr[i - 1] == '/')) {
                numStack.push(0.0);  // ���ŵȼ���0 - ��
            }
            // ���������ȼ�������ȼ���
            while (!opStack.empty() && opStack.top() != '(' &&
                priority[c] <= priority[opStack.top()]) {
                char op = opStack.top();
                opStack.pop();
                if (numStack.size() < 2) {
                    return "ʽ����Ч";
                }
                double b = numStack.top(); numStack.pop();
                double a = numStack.top(); numStack.pop();
                std::pair<bool, double> resPair = calculateOp(a, b, op);
                if (!resPair.first) {
                    return "ʽ����Ч";
                }
                numStack.push(resPair.second);
            }
            opStack.push(c);
            i++;
        }
        // ��Ч�ַ�
        else {
            return "ʽ����Ч";
        }
    }

    // ����ʣ�������
    while (!opStack.empty()) {
        char op = opStack.top();
        opStack.pop();
        if (op == '(') {  // δƥ��������
            return "ʽ����Ч";
        }
        if (numStack.size() < 2) {
            return "ʽ����Ч";
        }
        double b = numStack.top(); numStack.pop();
        double a = numStack.top(); numStack.pop();
        std::pair<bool, double> resPair = calculateOp(a, b, op);
        if (!resPair.first) {
            return "ʽ����Ч";
        }
        numStack.push(resPair.second);
    }

    // ������Ƿ�Ψһ
    if (numStack.size() != 1) {
        return "ʽ����Ч";
    }

    // ����������ʾ��ȥ�������.0��
    double result = numStack.top();
    if (fabs(result - round(result)) < 1e-9) {  // ������
        return std::to_string(static_cast<long long>(round(result)));
    }
    else {
        // �Ż���������ʾ���������С��λ��
        std::string resStr = std::to_string(result);
        resStr.erase(resStr.find_last_not_of('0') + 1, std::string::npos);
        if (resStr.back() == '.') {
            resStr.pop_back();
        }
        return resStr;
    }
}

// ����
int main() {
    // ��Ч���ʽ����
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
        "0.1+0.2"  // �������������Ȳ���
    };

    std::cout << "��Ч���ʽ���ԣ�" << std::endl;
    for (const auto& expr : validTests) {
        std::cout << expr << " = " << calculate(expr) << std::endl;
    }

    // ��Ч���ʽ����
    std::vector<std::string> invalidTests = {
        "(1+2",       // ���Ų�ƥ��
        "1++2",       // ���������
        "5/0",        // ����Ϊ0
        "12.3.4",     // ���С����
        "a+1",        // ��Ч�ַ�
        ".5",         // ���������֣���ѡ������չ֧�֣�������Ϊ��Ч��
        "5."          // ����������
    };

    std::cout << "\n��Ч���ʽ���ԣ�" << std::endl;
    for (const auto& expr : invalidTests) {
        std::cout << expr << " = " << calculate(expr) << std::endl;
    }

    return 0;
}
