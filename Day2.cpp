#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class Game {
private:
    vector<vector<int>> board; // 10x17 버섯 보드

public:
    Game() : board(10, vector<int>(17)) {}

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
                if (val == 0) continue;

                sum += val;
                if (r == r1) top = true;
                if (r == r2) bottom = true;
                if (c == c1) left = true;
                if (c == c2) right = true;
            }
        }
        return (sum == 10) && top && bottom && left && right;
    }

    // 유효한 사각형 탐색 및 좌표 반환
    vector<int> calculateMove() {
        for (int r1 = 0; r1 < 10; ++r1) {
            for (int c1 = 0; c1 < 17; ++c1) {
                for (int r2 = r1; r2 < 10; ++r2) {
                    for (int c2 = c1; c2 < 17; ++c2) {
                        if (isValid(r1, c1, r2, c2)) {
                            return { r1, c1, r2, c2 };
                        }
                    }
                }
            }
        }
        return { -1, -1, -1, -1 }; // 선택 가능한 사각형이 없을 경우
    }

    // 나중을 위한 확장: 상대 행동 반영 등
    void applyOpponentMove(int r1, int c1, int r2, int c2) {
        for (int r = r1; r <= r2; ++r)
            for (int c = c1; c <= c2; ++c)
                board[r][c] = 0; // 제거
    }

    // 내 턴 행동 적용(보드는 0 처리)
    void applyMyMove(int r1, int c1, int r2, int c2) {
        for (int r = r1; r <= r2; ++r)
            for (int c = c1; c <= c2; ++c)
                board[r][c] = 0; // 제거
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

            vector<int> move = game.calculateMove();
            game.applyMyMove(move[0], move[1], move[2], move[3]);

            cout << move[0] << " " << move[1] << " " << move[2] << " " << move[3] << endl;
            cout.flush();
        }

        else if (command == "OPP") {
            int r1, c1, r2, c2, t;
            ss >> r1 >> c1 >> r2 >> c2 >> t;
            game.applyOpponentMove(r1, c1, r2, c2);
        }

        else if (command == "FINISH") {
            break;
        }
    }

    return 0;
}
