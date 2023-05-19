#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <climits>
#include "cudd.h"
using namespace std;
class ROBDD{
    private:
        int best;
        vector<vector<char>> func;
        vector<string> orders;
    public:
        ROBDD(char *filename);
        ~ROBDD();
        
        void show();
        void sol();
        void output(char *filename);
};