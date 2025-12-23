#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstring>
#include <cstdio>
#include <vector>
#include <queue>
#include <map>
using namespace std;

// 定义秩类型
typedef int Rank;

// ====================== 1. 位图类 Bitmap 实现 ======================
class Bitmap {
private:
    unsigned char* M;   // 存储位图的字节数组
    Rank N;             // 字节数组的长度（总比特数为 N*8）
    Rank _sz;           // 有效比特的数量

    // 扩容操作：当访问的位超出当前范围时扩展空间
    void expand(Rank k) {
        if (k < 8 * N) return;  // 未越界，无需扩容
        Rank oldN = N;
        unsigned char* oldM = M;
        init(2 * k);            // 加倍扩容策略
        memcpy(M, oldM, oldN);  // 迁移旧数据
        delete[] oldM;
    }

    // 初始化位图空间
    void init(Rank n) {
        N = (n + 7) / 8;        // 按字节对齐，n比特需要N个字节
        M = new unsigned char[N];
        memset(M, 0, N);        // 初始化为全0
        _sz = 0;
    }

public:
    // 构造函数：指定初始容量（默认8比特）
    Bitmap(Rank n = 8) { init(n); }

    // 从文件读取位图
    Bitmap(char* file, Rank n = 8) {
        init(n);
        FILE* fp = fopen(file, "r");
        if (fp) {
            fread(M, sizeof(char), N, fp);
            fclose(fp);
        }
        // 统计有效比特数
        for (Rank k = 0, _sz = 0; k < n; k++) {
            _sz += test(k) ? 1 : 0;
        }
    }

    // 析构函数：释放内存
    ~Bitmap() {
        delete[] M;
        M = NULL;
        _sz = 0;
        N = 0;
    }

    // 获取有效比特数
    Rank size() { return _sz; }

    // 设置第k位为1
    void set(Rank k) {
        expand(k);
        if (!test(k)) {  // 避免重复计数
            _sz++;
            M[k >> 3] |= (0x80 >> (k & 0x07));  // k>>3 = k/8，k&0x07 = k%8
        }
    }

    // 清除第k位（置为0）
    void clear(Rank k) {
        expand(k);
        if (test(k)) {   // 避免重复计数
            _sz--;
            M[k >> 3] &= ~(0x80 >> (k & 0x07));
        }
    }

    // 测试第k位是否为1
    bool test(Rank k) {
        expand(k);
        return M[k >> 3] & (0x80 >> (k & 0x07));
    }

    // 导出到位图文件
    void dump(char* file) {
        FILE* fp = fopen(file, "w");
        if (fp) {
            fwrite(M, sizeof(char), N, fp);
            fclose(fp);
        }
    }

    // 将前n位转换为二进制字符串
    char* bits2string(Rank n) {
        expand(n - 1);  // 确保第n-1位有效
        char* s = new char[n + 1];
        s[n] = '\0';
        for (Rank i = 0; i < n; i++) {
            s[i] = test(i) ? '1' : '0';
        }
        return s;
    }
};

// ====================== 2. 二叉树节点结构 BinNode ======================
template <typename T>
struct BinNode {
    T data;               // 节点数据（哈夫曼树中为权重/字符）
    BinNode<T>* left;     // 左孩子
    BinNode<T>* right;    // 右孩子

    // 构造函数
    BinNode(T val = T()) : data(val), left(nullptr), right(nullptr) {}
};

// ====================== 3. 二叉树结构 BinTree 实现 ======================
template <typename T>
class BinTree {
protected:
    BinNode<T>* _root;  // 根节点
    int _size;          // 节点总数

public:
    BinTree() : _root(nullptr), _size(0) {}
    ~BinTree() { clear(_root); }

    // 清空子树
    void clear(BinNode<T>* node) {
        if (!node) return;
        clear(node->left);
        clear(node->right);
        delete node;
        _size--;
    }

    // 获取根节点
    BinNode<T>* root() { return _root; }

    // 设置根节点
    void setRoot(BinNode<T>* node) { _root = node; _size++; }

    // 获取节点数
    int size() { return _size; }
};

// ====================== 4. 哈夫曼树 HuffTree 实现 ======================
// 哈夫曼节点数据类型：存储字符和权重
struct HuffData {
    char ch;    // 字符（仅叶子节点有效）
    int weight; // 权重（字符出现次数）

    HuffData(char c = '\0', int w = 0) : ch(c), weight(w) {}

    // 重载比较运算符（用于优先队列）
    bool operator<(const HuffData& other) const {
        return weight > other.weight;  // 小顶堆：权重小的优先
    }
};

// 哈夫曼树：继承自二叉树
class HuffTree : public BinTree<HuffData> {
private:
    // 递归生成哈夫曼编码
    void generateCode(BinNode<HuffData>* node, Bitmap& bit, Rank depth, map<char, string>& codeMap) {
        if (!node) return;
        // 叶子节点：记录编码
        if (!node->left && !node->right) {
            char* str = bit.bits2string(depth);
            codeMap[node->data.ch] = string(str);
            delete[] str;
            return;
        }
        // 左子树：编码加0
        bit.clear(depth);
        generateCode(node->left, bit, depth + 1, codeMap);
        // 右子树：编码加1
        bit.set(depth);
        generateCode(node->right, bit, depth + 1, codeMap);
    }

public:
    // 构建哈夫曼树（输入字符频率表）
    void buildTree(map<char, int>& freqMap) {
        // 小顶堆（优先队列）
        priority_queue<HuffData> pq;
        for (auto& pair : freqMap) {
            pq.push(HuffData(pair.first, pair.second));
        }

        // 构建哈夫曼树
        while (pq.size() > 1) {
            // 取出两个权重最小的节点
            HuffData leftData = pq.top(); pq.pop();
            HuffData rightData = pq.top(); pq.pop();
            // 创建新父节点
            HuffData parentData('\0', leftData.weight + rightData.weight);
            BinNode<HuffData>* parent = new BinNode<HuffData>(parentData);
            parent->left = new BinNode<HuffData>(leftData);
            parent->right = new BinNode<HuffData>(rightData);
            // 父节点入堆
            pq.push(parentData);
            // 更新树的根和大小
            this->setRoot(parent);
        }
    }

    // 生成哈夫曼编码表
    map<char, string> getHuffCode() {
        map<char, string> codeMap;
        Bitmap bit;
        generateCode(this->_root, bit, 0, codeMap);
        return codeMap;
    }
};

// ====================== 5. 辅助函数：统计《I have a dream》字符频率 ======================
map<char, int> countCharFreq(const string& text) {
    map<char, int> freqMap;
    // 初始化26个字母的频率为0
    for (char c = 'a'; c <= 'z'; c++) {
        freqMap[c] = 0;
    }
    // 统计文本中字母频率（忽略大小写）
    for (char ch : text) {
        if (ch >= 'A' && ch <= 'Z') {
            ch = tolower(ch);
        }
        if (ch >= 'a' && ch <= 'z') {
            freqMap[ch]++;
        }
    }
    return freqMap;
}

// ====================== 6. 字符串编码/解码函数 ======================
// 编码字符串
string encodeString(const string& str, map<char, string>& codeMap) {
    string res;
    for (char ch : str) {
        if (ch >= 'A' && ch <= 'Z') ch = tolower(ch);
        res += codeMap[ch];
    }
    return res;
}

// 解码字符串
string decodeString(const string& code, HuffTree& tree) {
    string res;
    BinNode<HuffData>* cur = tree.root();
    for (char bit : code) {
        if (bit == '0') {
            cur = cur->left;
        }
        else {
            cur = cur->right;
        }
        // 到达叶子节点，记录字符
        if (!cur->left && !cur->right) {
            res += cur->data.ch;
            cur = tree.root();  // 重置到根节点
        }
    }
    return res;
}

// ====================== 主函数：测试实验功能 ======================
int main() {
    // 1. 完整《I have a dream》原文
    string speech =
        "I am happy to join with you today in what will go down in history as the greatest demonstration for freedom in the history of our nation."
        "Five score years ago, a great American, in whose symbolic shadow we stand today, signed the Emancipation Proclamation. This momentous decree came as a great beacon light of hope to millions of Negro slaves who had been seared in the flames of withering injustice. It came as a joyous daybreak to end the long night of their captivity."
        "But one hundred years later, the Negro still is not free. One hundred years later, the life of the Negro is still sadly crippled by the manacles of segregation and the chains of discrimination. One hundred years later, the Negro lives on a lonely island of poverty in the midst of a vast ocean of material prosperity. One hundred years later, the Negro is still languished in the corners of American society and finds himself an exile in his own land. And so we've come here today to dramatize a shameful condition."
        "In a sense we've come to our nation's capital to cash a check. When the architects of our republic wrote the magnificent words of the Constitution and the Declaration of Independence, they were signing a promissory note to which every American was to fall heir. This note was a promise that all men, yes, black men as well as white men, would be guaranteed the \"unalienable Rights\" of \"Life, Liberty and the pursuit of Happiness.\" It is obvious today that America has defaulted on this promissory note, insofar as her citizens of color are concerned. Instead of honoring this sacred obligation, America has given the Negro people a bad check, a check which has come back marked \"insufficient funds.\""
        "But we refuse to believe that the bank of justice is bankrupt. We refuse to believe that there are insufficient funds in the great vaults of opportunity of this nation. And so, we've come to cash this check, a check that will give us upon demand the riches of freedom and the security of justice."
        "We have also come to this hallowed spot to remind America of the fierce urgency of Now. This is no time to engage in the luxury of cooling off or to take the tranquilizing drug of gradualism. Now is the time to make real the promises of democracy. Now is the time to rise from the dark and desolate valley of segregation to the sunlit path of racial justice. Now is the time to lift our nation from the quicksands of racial injustice to the solid rock of brotherhood. Now is the time to make justice a reality for all of God's children."
        "It would be fatal for the nation to overlook the urgency of the moment. This sweltering summer of the Negro's legitimate discontent will not pass until there is an invigorating autumn of freedom and equality. Nineteen sixty-three is not an end, but a beginning. And those who hope that the Negro needed to blow off steam and will now be content will have a rude awakening if the nation returns to business as usual. And there will be neither rest nor tranquility in America until the Negro is granted his citizenship rights. The whirlwinds of revolt will continue to shake the foundations of our nation until the bright day of justice emerges."
        "But there is something that I must say to my people, who stand on the warm threshold which leads into the palace of justice: In the process of gaining our rightful place, we must not be guilty of wrongful deeds. Let us not seek to satisfy our thirst for freedom by drinking from the cup of bitterness and hatred. We must forever conduct our struggle on the high plane of dignity and discipline. We must not allow our creative protest to degenerate into physical violence. Again and again, we must rise to the majestic heights of meeting physical force with soul force."
        "The marvelous new militancy which has engulfed the Negro community must not lead us to a distrust of all white people, for many of our white brothers, as evidenced by their presence here today, have come to realize that their destiny is tied up with our destiny. And they have come to realize that their freedom is inextricably bound to our freedom."
        "We cannot walk alone."
        "And as we walk, we must make the pledge that we shall always march ahead."
        "We cannot turn back."
        "There are those who are asking the devotees of civil rights, \"When will you be satisfied?\" We can never be satisfied as long as the Negro is the victim of the unspeakable horrors of police brutality. We can never be satisfied as long as our bodies, heavy with the fatigue of travel, cannot gain lodging in the motels of the highways and the hotels of the cities. We cannot be satisfied as long as the negro's basic mobility is from a smaller ghetto to a larger one. We can never be satisfied as long as our children are stripped of their self-hood and robbed of their dignity by signs stating: \"For Whites Only.\" We cannot be satisfied as long as a Negro in Mississippi cannot vote and a Negro in New York believes he has nothing for which to vote. No, no, we are not satisfied, and we will not be satisfied until \"justice rolls down like waters, and righteousness like a mighty stream.\""
        "I am not unmindful that some of you have come here out of great trials and tribulations. Some of you have come fresh from narrow jail cells. And some of you have come from areas where your quest -- quest for freedom left you battered by the storms of persecution and staggered by the winds of police brutality. You have been the veterans of creative suffering. Continue to work with the faith that unearned suffering is redemptive. Go back to Mississippi, go back to Alabama, go back to South Carolina, go back to Georgia, go back to Louisiana, go back to the slums and ghettos of our northern cities, knowing that somehow this situation can and will be changed."
        "Let us not wallow in the valley of despair, I say to you today, my friends."
        "And so even though we face the difficulties of today and tomorrow, I still have a dream. It is a dream deeply rooted in the American dream."
        "I have a dream that one day this nation will rise up and live out the true meaning of its creed: \"We hold these truths to be self-evident, that all men are created equal.\""
        "I have a dream that one day on the red hills of Georgia, the sons of former slaves and the sons of former slave owners will be able to sit down together at the table of brotherhood."
        "I have a dream that one day even the state of Mississippi, a state sweltering with the heat of injustice, sweltering with the heat of oppression, will be transformed into an oasis of freedom and justice."
        "I have a dream that my four little children will one day live in a nation where they will not be judged by the color of their skin but by the content of their character."
        "I have a dream today!"
        "I have a dream that one day, down in Alabama, with its vicious racists, with its governor having his lips dripping with the words of \"interposition\" and \"nullification\" -- one day right there in Alabama little black boys and black girls will be able to join hands with little white boys and white girls as sisters and brothers."
        "I have a dream today!"
        "I have a dream that one day every valley shall be exalted, and every hill and mountain shall be made low, the rough places will be made plain, and the crooked places will be made straight; \"and the glory of the Lord shall be revealed and all flesh shall see it together.\""
        "This is our hope, and this is the faith that I go back to the South with."
        "With this faith, we will be able to hew out of the mountain of despair a stone of hope. With this faith, we will be able to transform the jangling discords of our nation into a beautiful symphony of brotherhood. With this faith, we will be able to work together, to pray together, to struggle together, to go to jail together, to stand up for freedom together, knowing that we will be free one day."
        "And this will be the day -- this will be the day when all of God's children will be able to sing with new meaning:"
        "My country 'tis of thee, sweet land of liberty, of thee I sing. Land where my fathers died, land of the Pilgrim's pride, From every mountainside, let freedom ring!"
        "And if America is to be a great nation, this must become true."
        "And so let freedom ring from the prodigious hilltops of New Hampshire."
        "Let freedom ring from the mighty mountains of New York."
        "Let freedom ring from the heightening Alleghenies of Pennsylvania."
        "Let freedom ring from the snow-capped Rockies of Colorado."
        "Let freedom ring from the curvaceous slopes of California."
        "But not only that:"
        "Let freedom ring from Stone Mountain of Georgia."
        "Let freedom ring from Lookout Mountain of Tennessee."
        "Let freedom ring from every hill and molehill of Mississippi."
        "From every mountainside, let freedom ring."
        "And when this happens, and when we allow freedom ring, when we let it ring from every village and every hamlet, from every state and every city, we will be able to speed up that day when all of God's children, black men and white men, Jews and Gentiles, Protestants and Catholics, will be able to join hands and sing in the words of the old Negro spiritual:"
        "Free at last! Free at last!"
        "Thank God Almighty, we are free at last!";

    // 2. 统计字符频率
    map<char, int> freqMap = countCharFreq(speech);
    cout << "=== 完整原文字符频率表 ===" << endl;
    for (auto& pair : freqMap) {
        cout << pair.first << ": " << pair.second << endl;
    }

    // 3. 构建哈夫曼树
    HuffTree huffTree;
    huffTree.buildTree(freqMap);

    // 4. 生成哈夫曼编码表
    map<char, string> codeMap = huffTree.getHuffCode();
    cout << "\n=== 基于完整原文的哈夫曼编码表 ===" << endl;
    for (auto& pair : codeMap) {
        cout << pair.first << ": " << pair.second << endl;
    }

    // 5. 测试编码：对dream和自定义单词编码
    string words[] = { "dream", "nation", "equal", "children", "freedom", "justice" };
    for (string word : words) {
        string code = encodeString(word, codeMap);
        string decode = decodeString(code, huffTree);
        cout << "\n单词 " << word << ":" << endl;
        cout << "  编码: " << code << endl;
        cout << "  解码: " << decode << endl;
    }

    return 0;
}

