#ifndef FM_H
#define FM_H
#include <unordered_set>
#include <map>
#include <vector>
#include <chrono>
#include <ctime>
#include <stack>
#include <list>
using namespace std;
class FM{
    private:
        char *file_name;

        int nets; // net's idx are 0 ~ nets-1
        int cells; //cell's idx are 1 ~ cells-1
        
        vector<int> *cell_list; //cell[i] conneted nets
        vector<int> *net_list; //net[i] connected cells
        vector<int> net_A_size; // num of cell on net[i] is on left partition
        vector<int> net_B_size; 
        std::unordered_set<int> *cell_neighbor;
        unordered_set<int> *net_set;
        // acceptable partition size
        int min; //minimun acceptable partition size
        int max;
        
        int *cell_gain;
        bool *cell_side;
        bool *old_side;
        int size_A;
        int old_size_A;

        int size_B;

        // bucket list for gain
        // bool *is_locked;
        int P_MAX;
        std::vector<int> locked;
        std::vector<std::unordered_set<int>> set_A;
        std::vector<std::unordered_set<int>> set_B;
        int set_A_ptr;
        int set_B_ptr;
        // for FM mode
        int *step_gain;
        int *step_move;
        bool stop_FM;
        bool balance;
        int cut;

    public:
        FM(char *);
        ~FM();
        void sol();
        void loadfile();
        void make_gain_list();
        void cal_gain(const int&, int&); //give a cell will return its gain wiht int&
        void cal_coGain(const int&, const int&, int&);
        void FM_move();
        void update_neighbor(const int&);
        void output();
        void printCuts();
        void make_bucket();
        void printGainList();
        void initial_partition(int);
        void real_FM();
        void getCuts();
        void test();
        void cal_all_gain();
        void update_gain(const int&);
};


#endif
