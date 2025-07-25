#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <tuple>
#include <random>

using namespace std;

class Game {
private:
    vector<vector<int>> board; // 10x17 버섯 보드
    vector<vector<bool>> opponentClaimed; // 상대방이 가져간 칸 표시
    vector<vector<bool>> myClaimed;  // 내가 점령한 칸 표시
    pair<int, int> lastMyMoveCenter = { -1, -1 };
    pair<int, int> lastOppMoveCenter = { -1, -1 };


    bool isOverlappingWithOpponent(const tuple<int,int,int,int,int,int>& myRect,
                                   const tuple<int,int,int,int,int,int>& oppRect) {
        auto [cnt1, area1, r1a, c1a, r2a, c2a] = myRect;
        auto [cnt2, area2, r1b, c1b, r2b, c2b] = oppRect;
        if (r2a < r1b || r2b < r1a) return false;
        if (c2a < c1b || c2b < c1a) return false;
        return true;
    }

public:
    Game() : board(10, vector<int>(17)), 
         opponentClaimed(10, vector<bool>(17, false)), 
         myClaimed(10, vector<bool>(17, false)) {}

    void setBoard(const vector<string>& rows) {
        for (int i = 0; i < 10; ++i)
            for (int j = 0; j < 17; ++j)
                board[i][j] = rows[i][j] - '0';
    }

    bool isValid(int r1, int c1, int r2, int c2) {
        if (r1 < 0 || c1 < 0 || r2 >= 10 || c2 >= 17) return false;
        int sum = 0;
        bool top = false, bottom = false, left = false, right = false;
        for (int r = r1; r <= r2; ++r) {
            for (int c = c1; c <= c2; ++c) {
                int val = board[r][c];
                if (val == 0 || opponentClaimed[r][c]) continue;
                sum += val;
                if (r == r1) top = true;
                if (r == r2) bottom = true;
                if (c == c1) left = true;
                if (c == c2) right = true;
            }
        }
        return (sum == 10) && top && bottom && left && right;
    }

    
vector<int> calculateMove(int myTime, int oppTime) {
    vector<tuple<int, int, int, int, int, int>> candidates;
    // (숫자 개수, 넓이, r1, c1, r2, c2)

    for (int r1 = 0; r1 < 10; ++r1) {
        for (int c1 = 0; c1 < 17; ++c1) {
            for (int r2 = r1; r2 < 10; ++r2) {
                for (int c2 = c1; c2 < 17; ++c2) {
                    if (isValid(r1, c1, r2, c2)) {
                        int count = 0;
                        for (int r = r1; r <= r2; ++r)
                            for (int c = c1; c <= c2; ++c)
                                if (board[r][c] != 0)
                                    count++;
                        int area = (r2 - r1 + 1) * (c2 - c1 + 1);
                        candidates.emplace_back(count, area, r1, c1, r2, c2);
                    }
                }
            }
        }
    }

    if (candidates.empty())
        return { -1, -1, -1, -1 };

    // 가장 넓은 후보 중 중앙 충돌 가능성 제거
    int maxArea = 0;
    for (auto& c : candidates)
        maxArea = max(maxArea, get<1>(c));

    vector<tuple<int, int, int, int, int, int>> safeCandidates;
    for (auto& c : candidates) {
        auto [cnt, area, r1, c1, r2, c2] = c;
        if (area != maxArea) continue;

        int center_r = (r1 + r2) / 2;
        int center_c = (c1 + c2) / 2;

        if (center_r >= 4 && center_r <= 6 && center_c >= 7 && center_c <= 9)
            continue;

        safeCandidates.push_back(c);
    }

    vector<tuple<int, int, int, int, int, int>> finalCandidates =
        safeCandidates.empty() ? candidates : safeCandidates;

    // 이전 나의 사각형 중심에서 너무 가까운 후보 제거
    vector<tuple<int, int, int, int, int, int>> filtered;
    for (auto& c : finalCandidates) {
        auto [cnt, area, r1, c1, r2, c2] = c;
        int center_r = (r1 + r2) / 2;
        int center_c = (c1 + c2) / 2;

        int dr = center_r - lastMyMoveCenter.first;
        int dc = center_c - lastMyMoveCenter.second;
        int dist2 = dr * dr + dc * dc;

        if (lastMyMoveCenter.first == -1 || dist2 > 4)
            filtered.push_back(c);
    }

    if (!filtered.empty())
        finalCandidates = filtered;

    // 상대방 중심과도 거리가 충분히 떨어진 후보 필터링
    vector<tuple<int, int, int, int, int, int>> filtered2;
    for (auto& c : finalCandidates) {
        auto [cnt, area, r1, c1, r2, c2] = c;
        int center_r = (r1 + r2) / 2;
        int center_c = (c1 + c2) / 2;

        int dr = center_r - lastOppMoveCenter.first;
        int dc = center_c - lastOppMoveCenter.second;
        int dist2 = dr * dr + dc * dc;

        if (lastOppMoveCenter.first == -1 || dist2 > 4)
            filtered2.push_back(c);
    }

    if (!filtered2.empty())
        finalCandidates = filtered2;

    // 가중치 기반 최고 점수 후보 선택 (무작위 없음)
    int bestScore = -1e9;
    tuple<int, int, int, int, int, int> bestCandidate;

    for (auto& c : finalCandidates) {
        auto [cnt, area, r1, c1, r2, c2] = c;

        int score = 2 * cnt - area;

        int center_r = (r1 + r2) / 2;
        int center_c = (c1 + c2) / 2;
        if (center_r >= 4 && center_r <= 6 && center_c >= 7 && center_c <= 9)
            score -= 100;

        if (lastOppMoveCenter.first != -1) {
            int dr = center_r - lastOppMoveCenter.first;
            int dc = center_c - lastOppMoveCenter.second;
            int dist2 = dr * dr + dc * dc;
            score -= dist2;
        }

        if (score > bestScore) {
            bestScore = score;
            bestCandidate = c;
        }
    }

    auto [cnt, area, r1, c1, r2, c2] = bestCandidate;

    cout << "[DEBUG] 점수 기반 전략 선택: (" << r1 << "," << c1 << ") ~ (" << r2 << "," << c2 << "), 점수: " << bestScore << endl;
    cout.flush();

    return { r1, c1, r2, c2 };
}

    void applyOpponentMove(int r1, int c1, int r2, int c2) {
        for (int r = r1; r <= r2; ++r)
            for (int c = c1; c <= c2; ++c) {
                board[r][c] = 0;
                opponentClaimed[r][c] = true;
            }
        lastOppMoveCenter = { (r1 + r2) / 2, (c1 + c2) / 2 };
    }

    void applyMyMove(int r1, int c1, int r2, int c2) {
        for (int r = r1; r <= r2; ++r)
            for (int c = c1; c <= c2; ++c) {
                board[r][c] = 0;
                myClaimed[r][c] = true;
            }
        lastMyMoveCenter = { (r1 + r2) / 2, (c1 + c2) / 2 };
    }

    int getMaxValueInRectangle(int r1, int c1, int r2, int c2) {
        int maxVal = 0;
        for (int r = r1; r <= r2; ++r)
            for (int c = c1; c <= c2; ++c)
                maxVal = max(maxVal, board[r][c]);
        return maxVal;
    }

    void printBoard() const {
        for (int r = 0; r < 10; ++r) {
            for (int c = 0; c < 17; ++c) {
                cout << board[r][c];
            }
            cout << endl;
        }
        cout << "------------------------" << endl;
    }
};

int main() {
    string line;
    Game game;
    while (getline(cin, line)) {
        stringstream ss(line);
        string command;
        ss >> command;

        if (command == "READY") {
            string order;
            ss >> order;
            cout << "OK" << endl;
            cout.flush();
        }
        else if (command == "INIT") {
            vector<string> rows(10);
            for (int i = 0; i < 10; ++i) {
                getline(cin, rows[i]);
            }
            game.setBoard(rows);
        }
        else if (command == "TIME") {
            int myTime, oppTime;
            ss >> myTime >> oppTime;
            vector<int> move = game.calculateMove(myTime, oppTime);
            game.applyMyMove(move[0], move[1], move[2], move[3]);
            cout << move[0] << " " << move[1] << " " << move[2] << " " << move[3] << endl;
            cout.flush();
        }
        else if (command == "OPP") {
            int r1, c1, r2, c2, t;
            ss >> r1 >> c1 >> r2 >> c2 >> t;
            if (r1 != -1)
                game.applyOpponentMove(r1, c1, r2, c2);
        }
        else if (command == "FINISH") {
            break;
        }
    }
    return 0;
}