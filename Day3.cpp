#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <tuple>

using namespace std;

class Game {
private:
    vector<vector<int>> board; // 10x17 버섯 보드
    vector<vector<bool>> opponentClaimed; // 상대방이 가져간 칸 표시
public:
    Game() : board(10, vector<int>(17)), opponentClaimed(10, vector<bool>(17, false)) {}

    // INIT 명령으로 초기 보드 설정
    void setBoard(const vector<string>& rows) {
        for (int i = 0; i < 10; ++i)
            for (int j = 0; j < 17; ++j)
                board[i][j] = rows[i][j] - '0';
    }

    // 유효한 사각형인지 확인
    bool isValid(int r1, int c1, int r2, int c2) {
        if (r1 < 0 || c1 < 0 || r2 >= 10 || c2 >= 17) return false;

        int sum = 0;
        bool top = false, bottom = false, left = false, right = false;

        for (int r = r1; r <= r2; ++r) {
            for (int c = c1; c <= c2; ++c) {
                int val = board[r][c];
                if (val == 0 || opponentClaimed[r][c]) 
                    continue;

                if (val == 0) 
                    continue;

                sum += val;

                if (r == r1) 
                    top = true;
                if (r == r2) 
                    bottom = true;
                if (c == c1) 
                    left = true;
                if (c == c2) 
                    right = true;
            }
        }
        return (sum == 10) && top && bottom && left && right;
    }

    vector<int> calculateMove(int myTime, int oppTime) {
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

        cout << "[DEBUG] 후보 사각형 수: " << candidates.size() << endl;
        for (const auto& cand : candidates) {
            cout << " - (" << get<2>(cand) << "," << get<3>(cand)
                << ") ~ (" << get<4>(cand) << "," << get<5>(cand) << ")" << endl;
        }
        cout.flush();

        if (candidates.empty())
            return { -1, -1, -1, -1 };

        // 정렬 기준: 숫자 개수 > 넓이 > 위치
        sort(candidates.begin(), candidates.end(),
            [](const tuple<int, int, int, int, int, int>& a,
                const tuple<int, int, int, int, int, int>& b) {
                    int cntA = get<0>(a), areaA = get<1>(a);
                    int r1a = get<2>(a), c1a = get<3>(a);
                    int cntB = get<0>(b), areaB = get<1>(b);
                    int r1b = get<2>(b), c1b = get<3>(b);
                    if (cntA != cntB) return cntA > cntB;
                    if (areaA != areaB) return areaA > areaB;
                    if (r1a != r1b) return r1a < r1b;
                    return c1a < c1b;
            });

        const auto& [cnt, area, r1, c1, r2, c2] = candidates[0];
        cout << "[DEBUG] 선택된 사각형: (" << r1 << "," << c1 << ") ~ (" << r2 << "," << c2 << ")" << endl;
        cout.flush();

        return { r1, c1, r2, c2 };
    }


    // 나중을 위한 확장: 상대 행동 반영 등
    void applyOpponentMove(int r1, int c1, int r2, int c2) {
        for (int r = r1; r <= r2; ++r)
            for (int c = c1; c <= c2; ++c) {
                board[r][c] = 0;  // 제거
                opponentClaimed[r][c] = true;  // 소유권 표시
            }
    }


    // 내 턴 행동 적용(보드는 0 처리)
    void applyMyMove(int r1, int c1, int r2, int c2) {
        for (int r = r1; r <= r2; ++r)
            for (int c = c1; c <= c2; ++c)
                board[r][c] = 0; // 제거
    }

    int getMaxValueInRectangle(int r1, int c1, int r2, int c2) {
        int maxVal = 0;
        for (int r = r1; r <= r2; ++r)
            for (int c = c1; c <= c2; ++c)
                maxVal = max(maxVal, board[r][c]);
        return maxVal;
    }

    // debugging method
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

// 명령어 파싱 및 흐름 처리
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

            // -1 -1 -1 -1 은 상대가 패스한 경우 → 무시
            if (r1 != -1)
                game.applyOpponentMove(r1, c1, r2, c2);
            // game.printBoard();  // ← 디버깅용 출력
        }

        else if (command == "FINISH") {
            break;
        }
    }

    return 0;
}
