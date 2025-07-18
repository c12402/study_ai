#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int main() {
    string line;
    vector<vector<int>> board(10, vector<int>(17)); // 10x17 보드 선언

    while (getline(cin, line)) {
        stringstream ss(line);
        string command;
        ss >> command;

        if (command == "READY") {
            string order;
            ss >> order;  // FIRST 또는 SECOND
            cout << "OK" << endl;
            cout.flush();
        }

        else if (command == "INIT") {
            // 보드 초기화
            for (int i = 0; i < 10; ++i) {
                string row;
                getline(cin, row);  // 다음 줄 10개 읽어야 함

                for (int j = 0; j < 17; ++j) {
                    board[i][j] = row[j] - '0'; // 문자 → 숫자 변환 후 저장
                }
            }
        }

        else if (command == "TIME") {
            int myTime, oppTime;
            ss >> myTime >> oppTime;

            // 아직 전략 없음 → 무조건 패스
            cout << "-1 -1 -1 -1" << endl;
            cout.flush();  // 꼭 flush!
        }

        else if (command == "OPP") {
            int r1, c1, r2, c2, t;
            ss >> r1 >> c1 >> r2 >> c2 >> t;
            // 상대 행동은 나중에 반영
        }

        else if (command == "FINISH") {
            break;  // 종료
        }
    }

    return 0;
}
