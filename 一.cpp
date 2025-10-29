#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <iomanip>

using namespace std;

// 复数类定义
class Complex {
private:
    double real;  // 实部
    double imag;  // 虚部

public:
    // 构造函数
    Complex(double r = 0, double i = 0) : real(r), imag(i) {}

    // 获取实部和虚部
    double getReal() const { return real; }
    double getImag() const { return imag; }

    // 计算复数的模
    double mod() const { return sqrt(real * real + imag * imag); }

    // 重载相等运算符（考虑浮点数精度）
    bool operator==(const Complex& other) const {
        const double eps = 1e-6;
        return fabs(real - other.real) < eps && fabs(imag - other.imag) < eps;
    }

    // 友元函数：重载输出运算符
    friend ostream& operator<<(ostream& os, const Complex& c) {
        os << "(" << fixed << setprecision(1) << c.real << ", " << c.imag << ")";
        return os;
    }
};

// 复数比较函数（用于排序）
// 先按模比较，模相等则按实部比较
bool compareComplex(const Complex& a, const Complex& b) {
    double modA = a.mod();
    double modB = b.mod();
    const double eps = 1e-6;

    if (fabs(modA - modB) > eps) {
        return modA < modB;
    }
    else {
        return a.getReal() < b.getReal();
    }
}

// 生成随机复数向量
vector<Complex> generateRandomVector(int size, int range) {
    vector<Complex> vec;
    for (int i = 0; i < size; ++i) {
        double r = rand() % range;  // 实部：0~range-1的随机整数
        double i_val = rand() % range;  // 虚部：0~range-1的随机整数
        vec.emplace_back(r, i_val);
    }
    return vec;
}

// 查找复数在向量中的位置
int findComplex(const vector<Complex>& vec, const Complex& target) {
    for (int i = 0; i < vec.size(); ++i) {
        if (vec[i] == target) {
            return i;
        }
    }
    return -1;  // 未找到
}

// 起泡排序实现
void bubbleSort(vector<Complex>& vec) {
    int n = vec.size();
    bool swapped;
    for (int i = 0; i < n - 1; ++i) {
        swapped = false;
        for (int j = 0; j < n - i - 1; ++j) {
            if (!compareComplex(vec[j], vec[j + 1])) {
                swap(vec[j], vec[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;  // 已有序，提前退出
    }
}

// 归并排序辅助函数：合并两个有序子数组
void merge(vector<Complex>& vec, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    vector<Complex> L(n1), R(n2);

    for (int i = 0; i < n1; ++i) L[i] = vec[left + i];
    for (int j = 0; j < n2; ++j) R[j] = vec[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (compareComplex(L[i], R[j])) {
            vec[k++] = L[i++];
        }
        else {
            vec[k++] = R[j++];
        }
    }

    while (i < n1) vec[k++] = L[i++];
    while (j < n2) vec[k++] = R[j++];
}

// 归并排序递归函数
void mergeSortHelper(vector<Complex>& vec, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortHelper(vec, left, mid);
        mergeSortHelper(vec, mid + 1, right);
        merge(vec, left, mid, right);
    }
}

// 归并排序入口
void mergeSort(vector<Complex>& vec) {
    if (vec.empty()) return;
    mergeSortHelper(vec, 0, vec.size() - 1);
}

// 测试排序算法时间（返回毫秒）
double testSortTime(void (*sortFunc)(vector<Complex>&), vector<Complex> vec) {
    clock_t start = clock();
    sortFunc(vec);
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC * 1000;
}

// 生成有序向量
vector<Complex> generateOrderedVector(int size, int range) {
    vector<Complex> vec = generateRandomVector(size, range);
    sort(vec.begin(), vec.end(), compareComplex);
    return vec;
}

// 生成逆序向量
vector<Complex> generateReverseVector(int size, int range) {
    vector<Complex> vec = generateOrderedVector(size, range);
    reverse(vec.begin(), vec.end());
    return vec;
}

// 二分查找下界（第一个模 >= m的元素）
int lowerBound(const vector<Complex>& vec, double m) {
    int left = 0, right = vec.size();
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (vec[mid].mod() < m) {
            left = mid + 1;
        }
        else {
            right = mid;
        }
    }
    return left;
}

// 区间查找：模介于[m1, m2)的元素
vector<Complex> rangeSearch(const vector<Complex>& orderedVec, double m1, double m2) {
    vector<Complex> result;
    int start = lowerBound(orderedVec, m1);
    int end = lowerBound(orderedVec, m2);
    for (int i = start; i < end; ++i) {
        result.push_back(orderedVec[i]);
    }
    return result;
}

int main() {
    srand(time(0));  // 初始化随机种子

    // 第一部分：测试向量基本操作
    cout << "===== 第一部分：复数向量基本操作 =====" << endl;
    vector<Complex> vec = generateRandomVector(8, 5);  // 生成8个元素的向量
    cout << "原始向量: ";
    for (const auto& c : vec) cout << c << " ";
    cout << endl;

    // 置乱操作
    random_shuffle(vec.begin(), vec.end());
    cout << "置乱后:   ";
    for (const auto& c : vec) cout << c << " ";
    cout << endl;

    // 查找操作
    if (!vec.empty()) {
        Complex target = vec[rand() % vec.size()];
        int pos = findComplex(vec, target);
        cout << "查找 " << target << " 的位置: " << pos << endl;
    }

    // 插入操作
    Complex insertC(6, 6);
    int insertPos = rand() % (vec.size() + 1);
    vec.insert(vec.begin() + insertPos, insertC);
    cout << "插入 " << insertC << " 到位置" << insertPos << "后: ";
    for (const auto& c : vec) cout << c << " ";
    cout << endl;

    // 删除操作
    if (!vec.empty()) {
        Complex delC = vec[rand() % vec.size()];
        auto it = find(vec.begin(), vec.end(), delC);
        if (it != vec.end()) vec.erase(it);
        cout << "删除 " << delC << " 后: ";
        for (const auto& c : vec) cout << c << " ";
        cout << endl;
    }

    // 唯一化操作
    sort(vec.begin(), vec.end(), compareComplex);  // 先排序使重复项相邻
    auto last = unique(vec.begin(), vec.end());
    vec.erase(last, vec.end());
    cout << "唯一化后: ";
    for (const auto& c : vec) cout << c << " ";
    cout << "\n" << endl;

    // 第二部分：排序效率比较
    cout << "===== 第二部分：排序效率比较 =====" << endl;
    const int testSize = 1000;  // 测试数据规模
    const int range = 100;

    // 生成三种测试向量
    vector<Complex> randomVec = generateRandomVector(testSize, range);
    vector<Complex> orderedVec = generateOrderedVector(testSize, range);
    vector<Complex> reverseVec = generateReverseVector(testSize, range);

    // 测试起泡排序
    double bubbleRandom = testSortTime(bubbleSort, randomVec);
    double bubbleOrdered = testSortTime(bubbleSort, orderedVec);
    double bubbleReverse = testSortTime(bubbleSort, reverseVec);

    // 测试归并排序
    double mergeRandom = testSortTime(mergeSort, randomVec);
    double mergeOrdered = testSortTime(mergeSort, orderedVec);
    double mergeReverse = testSortTime(mergeSort, reverseVec);

    // 输出结果
    cout << fixed << setprecision(2);
    cout << "排序算法 | 随机序列(ms) | 顺序序列(ms) | 逆序序列(ms)\n";
    cout << "----------------------------------------------------\n";
    cout << "起泡排序 | " << setw(12) << bubbleRandom
        << " | " << setw(12) << bubbleOrdered
        << " | " << setw(12) << bubbleReverse << "\n";
    cout << "归并排序 | " << setw(12) << mergeRandom
        << " | " << setw(12) << mergeOrdered
        << " | " << setw(12) << mergeReverse << "\n\n";

    // 第三部分：区间查找
    cout << "===== 第三部分：区间查找 =====" << endl;
    vector<Complex> searchVec = generateOrderedVector(15, 10);  // 生成有序向量
    cout << "有序向量（含模值）: " << endl;
    for (const auto& c : searchVec) {
        cout << c << " (模: " << setprecision(2) << c.mod() << ")  ";
    }
    cout << endl;

    // 查找模介于[2.0, 6.0)之间的元素
    double m1 = 2.0, m2 = 6.0;
    vector<Complex> result = rangeSearch(searchVec, m1, m2);

    cout << "模介于[" << m1 << ", " << m2 << ")的元素: " << endl;
    for (const auto& c : result) {
        cout << c << " (模: " << setprecision(2) << c.mod() << ")  ";
    }
    cout << endl;

    return 0;
}

