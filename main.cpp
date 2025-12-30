#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;

int P; // 进程数
int R; // 资源种类数

vector<vector<int>> Max;
vector<vector<int>> Allocation;
vector<vector<int>> Need;
vector<int> Available;
vector<int> Request;
vector<bool> Finished; // 进程是否已结束（资源已回收）

// 输入系统状态
void inputData() {
    Max.resize(P, vector<int>(R));
    Allocation.resize(P, vector<int>(R));
    Need.resize(P, vector<int>(R));
    Available.resize(R);
    Finished.assign(P, false);

    cout << "请输入最大需求矩阵 Max:\n";
    for (int i = 0; i < P; i++)
        for (int j = 0; j < R; j++)
            cin >> Max[i][j];

    cout << "请输入分配矩阵 Allocation:\n";
    for (int i = 0; i < P; i++)
        for (int j = 0; j < R; j++)
            cin >> Allocation[i][j];

    // 自动计算 Need = Max - Allocation
    for (int i = 0; i < P; i++)
        for (int j = 0; j < R; j++)
            Need[i][j] = Max[i][j] - Allocation[i][j];

    cout << "请输入可用资源向量 Available:\n";
    for (int j = 0; j < R; j++)
        cin >> Available[j];
}

// 判断进程是否已满足最大需求（Need 全为 0）
bool isProcessFull(int pid) {
    for (int j = 0; j < R; j++) {
        if (Need[pid][j] != 0) return false;
    }
    return true;
}

// 资源回收：释放某进程已分配资源
void recycleProcess(int pid) {
    if (pid < 0 || pid >= P) return;
    if (Finished[pid]) return;

    for (int j = 0; j < R; j++) {
        Available[j] += Allocation[pid][j];
        Allocation[pid][j] = 0;
        // 回收后进程结束：Need 设为 0（不再请求）
        Need[pid][j] = 0;
    }
    Finished[pid] = true;
}

// 检查并询问是否回收已“满配”的进程
bool promptRecycleIfAny() {
    bool recycledAny = false;
    for (int i = 0; i < P; i++) {
        if (Finished[i]) continue;
        if (!isProcessFull(i)) continue;

        cout << "\n提示：P" << i + 1 << " 已满足最大需求（Need 全为 0）。是否回收其资源并结束进程？(y/n)：";
        char ch;
        cin >> ch;
        if (ch == 'y' || ch == 'Y') {
            recycleProcess(i);
            recycledAny = true;
            cout << "已回收 P" << i + 1 << " 的资源。\n";
        } else {
            cout << "未回收 P" << i + 1 << " 的资源（仍占用 Allocation）。\n";
        }
    }

    return recycledAny;
}

// 比较函数 a >= b
bool isEnough(const vector<int>& a, const vector<int>& b) {
    for (int i = 0; i < R; i++)
        if (a[i] < b[i])
            return false;
    return true;
}

// 安全性检测
bool checkSafety(vector<int>& safeSeq) {
    vector<int> Work = Available;
    vector<bool> Finish(P, false);
    safeSeq.clear();

    for (int count = 0; count < P; count++) {
        bool found = false;
        for (int i = 0; i < P; i++) {
            if (!Finish[i] && isEnough(Work, Need[i])) {
                for (int j = 0; j < R; j++)
                    Work[j] += Allocation[i][j];
                Finish[i] = true;
                safeSeq.push_back(i + 1);
                found = true;
                break;
            }
        }
        if (!found) break;
    }

    for (bool f : Finish)
        if (!f) return false;
    return true;
}

// 资源请求处理
void requestTest(int pid, const vector<int>& Req) {
    pid--; // 下标转换
    if (pid < 0 || pid >= P) {
        cout << "错误：进程编号非法。\n";
        return;
    }
    if (Finished[pid]) {
        cout << "错误：P" << pid + 1 << " 已结束（资源已回收），不能再请求资源。\n";
        return;
    }
    // 若进程已满足最大需求（Need 全为 0），视为 Full：不再接受资源请求
    if (isProcessFull(pid)) {
        cout << "错误：P" << pid + 1 << " 已满足最大需求（Full），不能再请求资源。\n";
        return;
    }

    bool exceedNeed = false, exceedAvail = false;
    for (int i = 0; i < R; i++) {
        if (Req[i] > Need[pid][i]) exceedNeed = true;
        if (Req[i] > Available[i]) exceedAvail = true;
    }

    if (exceedNeed) {
        cout << "错误：请求量超过进程最大需求 Need。\n";
        return;
    }
    if (exceedAvail) {
        cout << "错误：请求量超过系统可用资源 Available。\n";
        return;
    }

    // 试分配
    for (int i = 0; i < R; i++) {
        Available[i] -= Req[i];
        Allocation[pid][i] += Req[i];
        Need[pid][i] -= Req[i];
    }

    vector<int> safeSeq;
    if (checkSafety(safeSeq)) {
        cout << "允许 P" << pid + 1 << " 请求，安全序列为：";
        for (int p : safeSeq) cout << "P" << p << " ";
        cout << endl;
    } else {
        cout << "不允许 P" << pid + 1 << " 请求（不安全），回滚。\n";
        // 回滚
        for (int i = 0; i < R; i++) {
            Available[i] += Req[i];
            Allocation[pid][i] -= Req[i];
            Need[pid][i] += Req[i];
        }
    }
}

// 显示系统状态
void printState() {
    cout << "\n系统状态：\n";
    cout << "进程  Allocation        Need         状态\n";
    for (int i = 0; i < P; i++) {
        cout << "P" << i + 1 << "   ";
        for (int j = 0; j < R; j++) cout << setw(3) << Allocation[i][j];
        cout << "      ";
        for (int j = 0; j < R; j++) cout << setw(3) << Need[i][j];
        cout << "      " << (Finished[i] ? "Finished" : (isProcessFull(i) ? "Full" : "Running")) << endl;
    }

    cout << "Available: ";
    for (int j = 0; j < R; j++) cout << Available[j] << " ";
    cout << endl;
}

// 主函数
int main() {
    cout << "请输入进程数 P：";
    cin >> P;
    cout << "请输入资源种类数 R：";
    cin >> R;

    inputData();

    vector<int> safeSeq;
    if (checkSafety(safeSeq)) {
        cout << "\n初始状态安全，安全序列为：";
        for (int p : safeSeq) cout << "P" << p << " ";
        cout << endl;
    } else
        cout << "\n初始状态不安全！\n";

    printState();
    if (promptRecycleIfAny()) {
        printState();
    }

    while (true) {
        int n;
        cout << "\n请输入请求资源的进程编号（0退出）：";
        cin >> n;
        if (n == 0) break;

        // 若进程已结束或已满配，则不再输入请求向量
        if (n < 1 || n > P) {
            cout << "错误：进程编号非法。\n";
            continue;
        }
        if (Finished[n - 1]) {
            cout << "错误：P" << n << " 已结束（资源已回收），不能再请求资源。\n";
            continue;
        }
        if (isProcessFull(n - 1)) {
            cout << "错误：P" << n << " 已满足最大需求（Full），不能再请求资源。\n";
            continue;
        }

        Request.resize(R);
        cout << "请输入请求向量 Request：\n";
        for (int i = 0; i < R; i++) cin >> Request[i];

        requestTest(n, Request);
        printState();
        if (promptRecycleIfAny()) {
            printState();
        }
    }

    return 0;
}

/*
4
3
3 2 2
6 1 3
3 1 4
4 2 2
1 0 0
5 1 1
2 1 1
0 0 2

1 1 2
*/


/*
7 5 3
3 2 2
9 0 2
2 2 2
4 3 3

0 1 0
2 0 0
3 0 2
2 1 1
0 0 2

3 3 2
*/