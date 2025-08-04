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
    vector<pair<int, int>> opponentHistory; // Day 13: 상대 이동 이력 저장

    bool isOverlappingWithOpponent(const tuple<int,int,int,int,int,int>& myRect,
                                   const tuple<int,int,int,int,int,int>& oppRect) {
        auto [cnt1, area1, r1a, c1a, r2a, c2a] = myRect;
        auto [cnt2, area2, r1b, c1b, r2b, c2b] = oppRect;
        if (r2a < r1b || r2b < r1a) return false;
        if (c2a < c1b || c2b < c1a) return false;
        return true;
    }

    pair<int, int> predictOpponentDirection() {
        if (opponentHistory.size() < 2) return {0, 0};
        int dx = 0, dy = 0;
        for (size_t i = 1; i < opponentHistory.size(); ++i) {
            dx += opponentHistory[i].first - opponentHistory[i - 1].first;
            dy += opponentHistory[i].second - opponentHistory[i - 1].second;
        }
        return {dx, dy};
    }

    bool isInPredictedPath(int cx, int cy, pair<int,int> predDir) {
        if (lastOppMoveCenter.first == -1) return false;
        int px = lastOppMoveCenter.first + predDir.first;
        int py = lastOppMoveCenter.second + predDir.second;
        int dist = abs(cx - px) + abs(cy - py);
        return dist <= 2;
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
        double timeRatio = (double)myTime / (oppTime + 1);

        bool avoidCenter = true;
        bool avoidOverlap = false;
        bool preferShape = true;

        if (timeRatio > 1.5) {
            avoidCenter = true;
            avoidOverlap = true;
            preferShape = true;
        } else if (timeRatio < 0.7) {
            avoidCenter = false;
            avoidOverlap = false;
            preferShape = false;
        }

        vector<tuple<int, int, int, int, int, int>> candidates;
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

        int maxArea = 0;
        for (auto& c : candidates)
            maxArea = max(maxArea, get<1>(c));

        vector<tuple<int, int, int, int, int, int>> filtered;
        for (auto& c : candidates) {
            auto [cnt, area, r1, c1, r2, c2] = c;
            if (area != maxArea) continue;

            int center_r = (r1 + r2) / 2;
            int center_c = (c1 + c2) / 2;

            if (avoidCenter && center_r >= 4 && center_r <= 6 && center_c >= 7 && center_c <= 9)
                continue;

            if (avoidOverlap && lastOppMoveCenter.first != -1) {
                tuple<int, int, int, int, int, int> myRect = c;
                tuple<int, int, int, int, int, int> oppRect = {
                    0, 0,
                    lastOppMoveCenter.first - 1, lastOppMoveCenter.second - 1,
                    lastOppMoveCenter.first + 1, lastOppMoveCenter.second + 1
                };
                if (isOverlappingWithOpponent(myRect, oppRect)) continue;
            }

            int dr = center_r - lastMyMoveCenter.first;
            int dc = center_c - lastMyMoveCenter.second;
            int dist2 = dr * dr + dc * dc;
            if (lastMyMoveCenter.first != -1 && dist2 <= 4) continue;

            filtered.push_back(c);
        }

        if (filtered.empty()) filtered = candidates;

        int bestScore = -1e9;
        tuple<int, int, int, int, int, int> bestCandidate;
        pair<int, int> predDir = predictOpponentDirection();

        for (auto& c : filtered) {
            auto [cnt, area, r1, c1, r2, c2] = c;
            int center_r = (r1 + r2) / 2;
            int center_c = (c1 + c2) / 2;

            int score = 2 * cnt - area;

            if (avoidCenter && center_r >= 4 && center_r <= 6 && center_c >= 7 && center_c <= 9)
                score -= 100;

            if (lastOppMoveCenter.first != -1) {
                int dr = center_r - lastOppMoveCenter.first;
                int dc = center_c - lastOppMoveCenter.second;
                int dist2 = dr * dr + dc * dc;
                score -= dist2;
            }

            if (preferShape) {
                int width = c2 - c1 + 1;
                int height = r2 - r1 + 1;
                int aspectPenalty = abs(width - height);
                score -= aspectPenalty * 3;
            }

            for (int r = max(0, r1 - 1); r <= min(9, r2 + 1); ++r)
                for (int c = max(0, c1 - 1); c <= min(16, c2 + 1); ++c)
                    if (opponentClaimed[r][c])
                        score -= 10;

            if (isInPredictedPath(center_r, center_c, predDir))
                score -= 7;

            if (score > bestScore) {
                bestScore = score;
                bestCandidate = c;
            }
        }

        auto [cnt, area, r1, c1, r2, c2] = bestCandidate;
        cout << "[DEBUG] 13일차 전략 선택: (" << r1 << "," << c1 << ") ~ (" << r2 << "," << c2 << "), 점수: " << bestScore << endl;
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
        opponentHistory.push_back(lastOppMoveCenter);
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
