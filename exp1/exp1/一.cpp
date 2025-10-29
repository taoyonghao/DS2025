#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <iomanip>

using namespace std;

// �����ඨ��
class Complex {
private:
    double real;  // ʵ��
    double imag;  // �鲿

public:
    // ���캯��
    Complex(double r = 0, double i = 0) : real(r), imag(i) {}

    // ��ȡʵ�����鲿
    double getReal() const { return real; }
    double getImag() const { return imag; }

    // ���㸴����ģ
    double mod() const { return sqrt(real * real + imag * imag); }

    // �����������������Ǹ��������ȣ�
    bool operator==(const Complex& other) const {
        const double eps = 1e-6;
        return fabs(real - other.real) < eps && fabs(imag - other.imag) < eps;
    }

    // ��Ԫ������������������
    friend ostream& operator<<(ostream& os, const Complex& c) {
        os << "(" << fixed << setprecision(1) << c.real << ", " << c.imag << ")";
        return os;
    }
};

// �����ȽϺ�������������
// �Ȱ�ģ�Ƚϣ�ģ�����ʵ���Ƚ�
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

// ���������������
vector<Complex> generateRandomVector(int size, int range) {
    vector<Complex> vec;
    for (int i = 0; i < size; ++i) {
        double r = rand() % range;  // ʵ����0~range-1���������
        double i_val = rand() % range;  // �鲿��0~range-1���������
        vec.emplace_back(r, i_val);
    }
    return vec;
}

// ���Ҹ����������е�λ��
int findComplex(const vector<Complex>& vec, const Complex& target) {
    for (int i = 0; i < vec.size(); ++i) {
        if (vec[i] == target) {
            return i;
        }
    }
    return -1;  // δ�ҵ�
}

// ��������ʵ��
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
        if (!swapped) break;  // ��������ǰ�˳�
    }
}

// �鲢�������������ϲ���������������
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

// �鲢����ݹ麯��
void mergeSortHelper(vector<Complex>& vec, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortHelper(vec, left, mid);
        mergeSortHelper(vec, mid + 1, right);
        merge(vec, left, mid, right);
    }
}

// �鲢�������
void mergeSort(vector<Complex>& vec) {
    if (vec.empty()) return;
    mergeSortHelper(vec, 0, vec.size() - 1);
}

// ���������㷨ʱ�䣨���غ��룩
double testSortTime(void (*sortFunc)(vector<Complex>&), vector<Complex> vec) {
    clock_t start = clock();
    sortFunc(vec);
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC * 1000;
}

// ������������
vector<Complex> generateOrderedVector(int size, int range) {
    vector<Complex> vec = generateRandomVector(size, range);
    sort(vec.begin(), vec.end(), compareComplex);
    return vec;
}

// ������������
vector<Complex> generateReverseVector(int size, int range) {
    vector<Complex> vec = generateOrderedVector(size, range);
    reverse(vec.begin(), vec.end());
    return vec;
}

// ���ֲ����½磨��һ��ģ >= m��Ԫ�أ�
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

// ������ң�ģ����[m1, m2)��Ԫ��
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
    srand(time(0));  // ��ʼ���������

    // ��һ���֣�����������������
    cout << "===== ��һ���֣����������������� =====" << endl;
    vector<Complex> vec = generateRandomVector(8, 5);  // ����8��Ԫ�ص�����
    cout << "ԭʼ����: ";
    for (const auto& c : vec) cout << c << " ";
    cout << endl;

    // ���Ҳ���
    random_shuffle(vec.begin(), vec.end());
    cout << "���Һ�:   ";
    for (const auto& c : vec) cout << c << " ";
    cout << endl;

    // ���Ҳ���
    if (!vec.empty()) {
        Complex target = vec[rand() % vec.size()];
        int pos = findComplex(vec, target);
        cout << "���� " << target << " ��λ��: " << pos << endl;
    }

    // �������
    Complex insertC(6, 6);
    int insertPos = rand() % (vec.size() + 1);
    vec.insert(vec.begin() + insertPos, insertC);
    cout << "���� " << insertC << " ��λ��" << insertPos << "��: ";
    for (const auto& c : vec) cout << c << " ";
    cout << endl;

    // ɾ������
    if (!vec.empty()) {
        Complex delC = vec[rand() % vec.size()];
        auto it = find(vec.begin(), vec.end(), delC);
        if (it != vec.end()) vec.erase(it);
        cout << "ɾ�� " << delC << " ��: ";
        for (const auto& c : vec) cout << c << " ";
        cout << endl;
    }

    // Ψһ������
    sort(vec.begin(), vec.end(), compareComplex);  // ������ʹ�ظ�������
    auto last = unique(vec.begin(), vec.end());
    vec.erase(last, vec.end());
    cout << "Ψһ����: ";
    for (const auto& c : vec) cout << c << " ";
    cout << "\n" << endl;

    // �ڶ����֣�����Ч�ʱȽ�
    cout << "===== �ڶ����֣�����Ч�ʱȽ� =====" << endl;
    const int testSize = 1000;  // �������ݹ�ģ
    const int range = 100;

    // �������ֲ�������
    vector<Complex> randomVec = generateRandomVector(testSize, range);
    vector<Complex> orderedVec = generateOrderedVector(testSize, range);
    vector<Complex> reverseVec = generateReverseVector(testSize, range);

    // ������������
    double bubbleRandom = testSortTime(bubbleSort, randomVec);
    double bubbleOrdered = testSortTime(bubbleSort, orderedVec);
    double bubbleReverse = testSortTime(bubbleSort, reverseVec);

    // ���Թ鲢����
    double mergeRandom = testSortTime(mergeSort, randomVec);
    double mergeOrdered = testSortTime(mergeSort, orderedVec);
    double mergeReverse = testSortTime(mergeSort, reverseVec);

    // ������
    cout << fixed << setprecision(2);
    cout << "�����㷨 | �������(ms) | ˳������(ms) | ��������(ms)\n";
    cout << "----------------------------------------------------\n";
    cout << "�������� | " << setw(12) << bubbleRandom
        << " | " << setw(12) << bubbleOrdered
        << " | " << setw(12) << bubbleReverse << "\n";
    cout << "�鲢���� | " << setw(12) << mergeRandom
        << " | " << setw(12) << mergeOrdered
        << " | " << setw(12) << mergeReverse << "\n\n";

    // �������֣��������
    cout << "===== �������֣�������� =====" << endl;
    vector<Complex> searchVec = generateOrderedVector(15, 10);  // ������������
    cout << "������������ģֵ��: " << endl;
    for (const auto& c : searchVec) {
        cout << c << " (ģ: " << setprecision(2) << c.mod() << ")  ";
    }
    cout << endl;

    // ����ģ����[2.0, 6.0)֮���Ԫ��
    double m1 = 2.0, m2 = 6.0;
    vector<Complex> result = rangeSearch(searchVec, m1, m2);

    cout << "ģ����[" << m1 << ", " << m2 << ")��Ԫ��: " << endl;
    for (const auto& c : result) {
        cout << c << " (ģ: " << setprecision(2) << c.mod() << ")  ";
    }
    cout << endl;

    return 0;
}

