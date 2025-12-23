#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <cmath>

using namespace std;
using namespace chrono;

// 边界框数据结构
struct BoundingBox {
    float x1, y1, x2, y2;  // 左上角和右下角坐标
    float confidence;      // 置信度

    // 计算两个框的IoU
    float calculateIoU(const BoundingBox& other) const {
        float interX1 = max(x1, other.x1);
        float interY1 = max(y1, other.y1);
        float interX2 = min(x2, other.x2);
        float interY2 = min(y2, other.y2);

        if (interX1 >= interX2 || interY1 >= interY2) return 0.0f;

        float interArea = (interX2 - interX1) * (interY2 - interY1);
        float area1 = (x2 - x1) * (y2 - y1);
        float area2 = (other.x2 - other.x1) * (other.y2 - other.y1);
        float unionArea = area1 + area2 - interArea;

        return interArea / unionArea;
    }
};

// ====================== 排序算法实现 ======================
// 1. 冒泡排序（按置信度降序）
template <typename T>
void bubbleSort(vector<T>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; ++i) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1; ++j) {
            if (arr[j].confidence < arr[j + 1].confidence) {
                swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

// 2. 插入排序（按置信度降序）
template <typename T>
void insertionSort(vector<T>& arr) {
    int n = arr.size();
    for (int i = 1; i < n; ++i) {
        T key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j].confidence < key.confidence) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// 归并排序辅助函数
template <typename T>
void merge(vector<T>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    vector<T> L(n1), R(n2);

    for (int i = 0; i < n1; ++i) L[i] = arr[left + i];
    for (int j = 0; j < n2; ++j) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i].confidence >= R[j].confidence) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

// 3. 归并排序（按置信度降序）
template <typename T>
void mergeSort(vector<T>& arr, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSort(arr, left, mid);
    mergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

// 快速排序分区函数
template <typename T>
int partition(vector<T>& arr, int low, int high) {
    float pivot = arr[high].confidence;
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (arr[j].confidence >= pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return i + 1;
}

// 4. 快速排序（按置信度降序）
template <typename T>
void quickSort(vector<T>& arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// ====================== 数据生成函数 ======================
// 随机数生成器初始化
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<float> coordDist(0.0f, 1000.0f);  // 坐标范围0~1000
uniform_real_distribution<float> sizeDist(10.0f, 50.0f);    // 框大小10~50
uniform_real_distribution<float> confDist(0.0f, 1.0f);      // 置信度0~1

// 1. 生成随机分布的边界框
vector<BoundingBox> generateRandomBoxes(int num) {
    vector<BoundingBox> boxes;
    for (int i = 0; i < num; ++i) {
        float x1 = coordDist(gen);
        float y1 = coordDist(gen);
        float x2 = x1 + sizeDist(gen);
        float y2 = y1 + sizeDist(gen);
        boxes.push_back({ x1, y1, x2, y2, confDist(gen) });
    }
    return boxes;
}

// 2. 生成聚集分布的边界框（80%集中在中心区域，20%随机分布）
vector<BoundingBox> generateClusteredBoxes(int num) {
    vector<BoundingBox> boxes;
    int clusterNum = num * 0.8;
    int randomNum = num - clusterNum;

    // 中心聚集区域：x∈[400,600], y∈[400,600]
    uniform_real_distribution<float> clusterCoordDist(400.0f, 600.0f);
    for (int i = 0; i < clusterNum; ++i) {
        float x1 = clusterCoordDist(gen);
        float y1 = clusterCoordDist(gen);
        float x2 = x1 + sizeDist(gen);
        float y2 = y1 + sizeDist(gen);
        boxes.push_back({ x1, y1, x2, y2, confDist(gen) });
    }

    // 补充随机分布的框
    for (int i = 0; i < randomNum; ++i) {
        float x1 = coordDist(gen);
        float y1 = coordDist(gen);
        float x2 = x1 + sizeDist(gen);
        float y2 = y1 + sizeDist(gen);
        boxes.push_back({ x1, y1, x2, y2, confDist(gen) });
    }
    return boxes;
}

// ====================== NMS算法实现 ======================
// 基础NMS（传入排序后的框，IoU阈值可配置）
vector<BoundingBox> basicNMS(const vector<BoundingBox>& sortedBoxes, float iouThreshold = 0.5f) {
    vector<BoundingBox> result;
    vector<bool> suppressed(sortedBoxes.size(), false);

    for (int i = 0; i < sortedBoxes.size(); ++i) {
        if (suppressed[i]) continue;
        result.push_back(sortedBoxes[i]);
        for (int j = i + 1; j < sortedBoxes.size(); ++j) {
            if (suppressed[j]) continue;
            float iou = sortedBoxes[i].calculateIoU(sortedBoxes[j]);
            if (iou > iouThreshold) {
                suppressed[j] = true;
            }
        }
    }
    return result;
}

// ====================== 性能测试函数 ======================
// 测试单个排序算法的耗时（包含NMS整体耗时）
void testSortPerformance(const string& sortName,
    void (*sortFunc)(vector<BoundingBox>&),
    vector<BoundingBox> boxes,
    const string& distType) {
    // 计时排序阶段
    auto startSort = high_resolution_clock::now();
    if (sortName == "快速排序") {
        quickSort(boxes, 0, boxes.size() - 1);
    }
    else if (sortName == "归并排序") {
        mergeSort(boxes, 0, boxes.size() - 1);
    }
    else {
        sortFunc(boxes);
    }
    auto endSort = high_resolution_clock::now();
    double sortTime = duration_cast<microseconds>(endSort - startSort).count() / 1000.0;  // 转毫秒

    // 计时NMS阶段
    auto startNMS = high_resolution_clock::now();
    vector<BoundingBox> nmsResult = basicNMS(boxes);
    auto endNMS = high_resolution_clock::now();
    double nmsTime = duration_cast<microseconds>(endNMS - startNMS).count() / 1000.0;

    // 输出结果
    cout << "数据分布: " << distType
        << " | 数据规模: " << boxes.size()
        << " | 排序算法: " << sortName
        << " | 排序耗时: " << sortTime << "ms"
        << " | NMS耗时: " << nmsTime << "ms"
        << " | 总耗时: " << (sortTime + nmsTime) << "ms"
        << " | NMS保留框数: " << nmsResult.size() << endl;
}

int main() {
    // 测试的数据集规模列表
    vector<int> testSizes = { 100, 1000, 5000, 10000 };
    // 排序算法列表（函数指针+名称）
    vector<pair<void (*)(vector<BoundingBox>&), string>> sortAlgos = {
        {bubbleSort<BoundingBox>, "冒泡排序"},
        {insertionSort<BoundingBox>, "插入排序"},
        {[](vector<BoundingBox>& arr) { mergeSort(arr, 0, arr.size() - 1); }, "归并排序"},
        {[](vector<BoundingBox>& arr) { quickSort(arr, 0, arr.size() - 1); }, "快速排序"}
    };

    cout << "========== 随机分布数据集测试 ==========" << endl;
    for (int size : testSizes) {
        vector<BoundingBox> boxes = generateRandomBoxes(size);
        for (auto& algo : sortAlgos) {
            testSortPerformance(algo.second, algo.first, boxes, "随机分布");
        }
        cout << "----------------------------------------" << endl;
    }

    cout << "\n========== 聚集分布数据集测试 ==========" << endl;
    for (int size : testSizes) {
        vector<BoundingBox> boxes = generateClusteredBoxes(size);
        for (auto& algo : sortAlgos) {
            testSortPerformance(algo.second, algo.first, boxes, "聚集分布");
        }
        cout << "----------------------------------------" << endl;
    }

    return 0;
}