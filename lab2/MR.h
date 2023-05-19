#ifndef MR_H
#define MR_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <unordered_set>
#include <climits>
#include <algorithm>
using namespace std;
struct cmp{
    bool operator()(const pair<int, int>& a, const pair<int, int>& b){
        return a.first > b.first;
    }
};
typedef priority_queue<pair<int, int>, vector<pair<int,int>>, cmp> pqii;

class sol{
    private:
        char *input_file;
        char *output_file;
        int row, col;
        vector<vector<int>> blockList;
        vector<vector<int>> netList;

        vector<vector<int>> arr;
        vector<vector<int>> g;
        
        pqii netQ;
        queue<pair<int,int>> pri_netQ;
        vector<int> netDist;
        unordered_set<int> waitForR;

        vector<int> gridNum;
        vector<vector<vector<int>>> ans;
    public:
        sol(char *, char *);
        ~sol();
        void solve();

        void readFile();
        void showInfo();

        void setArr();

        void MR();
        void Hadlock(const pair<int,int>&, pqii&, const vector<int>&);
        void showG(const pair<int, int>&);
        void showA();
        void output();
        void erase_net(const unordered_set<int>&);
};


#endif