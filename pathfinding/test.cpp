#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "search.hpp"
#include "test.hpp"

using namespace std;

void _read_settings(const string& line, pathfinding::Settings& settings) {
    settings.max_jump_height = 0;
    settings.air_stride = 2;

    istringstream in(line);

    in >> settings.max_jump_height;

    if (in.eof()) return;

    in >> settings.air_stride;

    if (in.eof()) return;
}

bool _read_tests(ifstream& file, vector<pathfinding::Test>& tests) {
    string row;

    while (true) {
        file >> row;

        if (file.eof()) break;

        int index = tests.size();
        tests.resize(tests.size() + 1);

        pathfinding::Test& test = tests[index];
        test.tag = row;

        file >> test.graph.width;

        if (file.eof()) return false;

        file >> test.graph.height;

        if (file.eof()) return false;

        getline(file, row); // read to end of line

        getline(file, row);

        if (file.eof()) return false;

        _read_settings(row, test.settings);

        for (int r = 0; r < test.graph.height; r++) {
            file >> row;

            if (file.eof()) return false;

            for (int c = 0; c < min((int)row.length(), test.graph.width); c++) {
                char kind = row[c];
                switch (kind) {
                    case '#':
                    {
                        test.graph.set_at(c, r, pathfinding::FLOOR_TILEKIND);
                        break;
                    }
                    case 'S':
                    {
                        test.start.x = c;
                        test.start.y = r;
                        break;
                    }
                    case 'G':
                    {
                        test.goal.x = c;
                        test.goal.y = r;
                        break;
                    }
                    default: break;
                }
            }
        }

        for (int r = 0; r < test.graph.height; r++) {
            file >> row;

            if (file.eof()) break;

            for (int c = 0; c < min((int)row.length(), test.graph.width); c++) {
                char item = row[c];
                if (item != '*') continue;

                pathfinding::State tile;
                tile.x = c;
                tile.y = r;
                test.expected_path.insert(tile);
            }
        }
    }

    return true;
}

bool _run_tests(vector<pathfinding::Test>& tests, pathfinding::Test& failed_test, vector<pathfinding::State>& actual_path, int& test_num) {
    for (int i = 0; i < tests.size(); i++) {
        test_num = i + 1;
        failed_test = tests[i];

        pathfinding::search(
            failed_test.graph, failed_test.settings,
            failed_test.start, failed_test.goal.x, failed_test.goal.y,
            actual_path);

        if (actual_path.size() != failed_test.expected_path.size()) return false;

        for (auto& _path_step : actual_path) {
            auto path_step = _path_step.only_position();
            if (failed_test.expected_path.find(path_step) != failed_test.expected_path.end()) continue;

            return false;
        }
    }

    return true;
}

int main(int cargs, char** args) {
    if (cargs < 2) {
        cout << "Needs file argument." << endl;
        return 0;
    }

    string filename(args[1]);
    ifstream file_in(filename);

    if (!file_in.good()) {
        cout << "Invalid file: " << filename << endl;
        return 0;
    }


    std::vector<pathfinding::Test> tests;
    if (!_read_tests(file_in, tests)) {
        file_in.close();
        cout << "Error reading tests." << endl;
        return 0;
    }

    file_in.close();

    if (tests.empty()) {
        cout << "No tests!" << endl;
        return 0;
    }

    int failed_test_num;
    pathfinding::Test failed_test;
    vector<pathfinding::State> actual_path;
    if (_run_tests(tests, failed_test, actual_path, failed_test_num)) {
        if (tests.size() == 1) {
            cout << "The test passed." << endl;
        }
        else {
            cout << "All " << tests.size() << " tests passed." << endl;
        }
        return 0;
    }

    cout << endl;

    cout << "Test[" << failed_test_num << "]: " << failed_test.tag << " failed." << endl;
    cout << "Settings: " << endl;
    cout << "Max Height Jump: " << failed_test.settings.max_jump_height << endl;
    cout << "Air Stride: " << failed_test.settings.air_stride << endl;

    cout << "EXPECTED: " << endl;
    for (auto& part : failed_test.expected_path) {
        cout << "[(" << part.x << ", " << part.y << ")" << " - " << part.jump << "] ";
    } 
    cout << endl;

    cout << "ACTUAL: " << endl;
    for (auto& part : actual_path) {
        cout << "[(" << part.x << ", " << part.y << ")" << " - " << part.jump << "] ";
    }
    cout << endl;

    cout << endl;

    return 0;
}
