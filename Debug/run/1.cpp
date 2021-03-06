#include <stdio.h>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

typedef vector <int> destination;
typedef map<int, destination> traffic;
typedef map<int, destination>::iterator traffic_iter;

void test(vector< vector< int >> &temp) {
    temp.push_back({1, 2});
    temp.push_back({1, 3});
    temp.push_back({2, 2});
    temp.push_back({2, 3});
}

int main(int argc, char **argv) {
    vector <vector <int>> temp;
    vector <int> temp1(3, 0);
    for ( int i = 0; i<3; ++i)  temp.push_back(temp1);
    vector <int> temp2{1,2,3,4,5};
    temp.resize(5);
    temp[4].insert(temp[4].end(), temp2.begin(), temp2.end());
    for (int number : temp[4])
        cout << number << "\t";
    cout <<endl; 
    return 0;
}

