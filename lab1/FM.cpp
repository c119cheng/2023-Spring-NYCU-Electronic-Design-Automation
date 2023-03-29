#include "FM.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <omp.h>
#include <chrono>
#include <stack>
#include <queue>
#include <algorithm>
#include <cmath>
using namespace std;

FM::FM(char *file_name){
    this->file_name = file_name;
}

FM::~FM(){

}

void FM::sol(){
    srand(s);
    this->stop_FM = false;
    auto start_time = chrono::system_clock::now();
    this->loadfile();
    this->initial_partition();
    printCuts();
    cur_gain = 0;
    total_gain = 0;
    // old_size_A = size_A;
    make_gain_list();
    int old_cur = nets;
    while(!stop_FM && cells > 50000){
        cout<<"start FM"<<endl;
        make_bucket();
        cout<<"FM MOVE"<<endl;
        FM_move();
        printCuts();
        if(balance)
            break;
        auto now = chrono::system_clock::now();
        if(now - start_time > chrono::seconds(25))
           break;
    }
    
    while(!stop_FM){
        make_gain_list();
        real_FM();
        auto now = chrono::system_clock::now();
        if(now - start_time > chrono::seconds(25))
           break;
    }
    
    /*
    getCuts();
    // Simulated Annealing
    float T = 10;
    float T_stop = pow(10,-12);
    float r = 0.99;
    int N = cells;
    int prev_cost = cut;
    int cur_cost = cut; 
    int count = 0;
    int best = cut;
    cout<<"SA"<<endl;
    while(chrono::system_clock::now() - start_time < chrono::seconds(25) && T >= T_stop){
        for(int i=0;i<N;i++){
            if(i%10 == 0){
                //move from side set to another
                bool side = random()%2;
                if((side && size_A >= max) || (!side && size_A <= min))
                    continue;
                int cell = random()%cells + 1;
                while(cell_side[cell] != side)
                    cell = random()%cells + 1;
                int gain = 0;
                cal_gain(cell, gain);
                bool do_work = false;
                if(gain > 0){
                    do_work = true;
                }
                else{
                    float ex = gain/T;
                    float p = exp(ex);
                    float th = float(rand())/float(RAND_MAX);
                    if(p >= th)
                        do_work = true;
                }
                if(do_work){
                    cout<<"do something  "<<cell<<endl;
                    cell_side[cell] = !side;
                    cur_cost -= gain;
                    if(side)
                        size_A++;
                    else
                        size_A--;
                }
            }
            else if(i%10 < 5){
                //swap two cell
                int cell_A = random()%cells + 1;
                int cell_B = random()%cells + 1;
                while(cell_side[cell_A] != 0){
                    cell_A = random()%cells + 1;
                }
                while(cell_side[cell_B] != 1){
                    cell_B = random()%cells + 1;
                }

                int gain = 0;
                cal_coGain(cell_A, cell_B, gain);
                bool do_work = false;
                if(gain > 0){
                    do_work = true;
                }
                else{
                    float ex = gain/T;
                    float p = exp(ex);
                    float th = float(rand())/float(RAND_MAX);
                    if(p >= th)
                        do_work = true;
                }
                if(do_work){
                    cout<<"do something swap "<<endl;
                    cell_side[cell_A] = 1;
                    cell_side[cell_B] = 0;
                    cur_cost -= gain;
                }
            }
            else{
                // move a net
                int n = random()%nets;
                int size = net_list[n].size();
                if(size_A - size <= min || size_A + size >= max)
                    continue;
                int target = cell_side[*net_list[n].begin()];
                int gain = 0;
                for(auto c : net_list[n]){
                    if(cell_side[c] != target)
                        cal_gain(c, gain);
                }
                bool do_work = false;
                if(gain > 0){
                    do_work = true;
                }
                else{
                    float ex = gain/T;
                    float p = exp(ex);
                    float th = float(rand())/float(RAND_MAX);
                    if(p >= th)
                        do_work = true;
                }
                if(do_work){
                    cout<<"do something swap line "<<endl;
                    for(auto c : net_list[n]){
                        if(cell_side[c] != target)
                            cell_side[c] = target;
                        cur_cost -= gain;
                    }
                    printCuts();
                    cur_cost = cut;
                }
            }

            //update
            T *= r;
            if(prev_cost == cur_cost)
                count++;
            else
                count = 0;
            if(count >= 100)
                break;
            prev_cost = cur_cost;
            if(cur_cost < best){
                best = cur_cost;
                for(int i=1;i<=cells;i++)
                    old_side[i] = cell_side[i];
            }
            cout<<T<<"   "<<cur_cost<<endl;
        }
    }
    
    for(int i=1;i<=cells;i++)
        cell_side[i] = old_side[i];
    cout<<"best cut "<<best<<endl;
    */
    getCuts();
    cout<<"start output"<<endl;
    output();
    cout<<"finish output"<<endl;
}

void FM::loadfile(){
    ifstream fin(this->file_name);
    string line;
    getline(fin, line);
    stringstream ss(line);
    ss >> this->nets >> this->cells;
    ss.clear();
    
    this->cell_list = new std::vector<int>[this->cells+1];
    this->net_list = new std::vector<int>[this->nets+1];
    this->cell_neighbor = new std::vector<int>[this->cells+1];
    if(cells < 50000)
        this->net_set = new unordered_set<int>[cells+1];
    int i=0; // current read net
    P_MAX = 0;
    while(getline(fin, line)){
        ss.str(line);
        int in;
        while(ss >> in){
            this->net_list[i].push_back(in);
            this->cell_list[in].push_back(i);
            if(cells < 50000)
                net_set[in].insert(i);
            if(cell_list[in].size() > P_MAX)
                P_MAX = cell_list[in].size();
        }
        ss.clear();
        i++;
    }

    this->set_A = vector<unordered_set<int>>(2*P_MAX+1);
    this->set_B = vector<unordered_set<int>>(2*P_MAX+1);

    // 0.45 balance factor given by TA
    this->min = (this->cells ) * 0.451;
    this->max = (this->cells ) * 0.549 + 1;
    this->locked = vector<int>(cells+1);

    this->cell_gain = new int[this->cells+1];
    this->cell_side = new bool[this->cells+1];
    this->old_side = new bool[this->cells+1];
    this->size_A = 0;
    this->size_B = 0;

    // for FM move
    this->step_gain = new int[this->cells];
    this->step_move = new int[this->cells];

    // making neighbor set
    unordered_set<int> *cn = new unordered_set<int>[cells+1]; // cell neighbor set
    // for(auto c : locked){
    for(int c=1;c<=cells;c++){
        for(auto connect_net : cell_list[c]){
            for(auto cur_cell : net_list[connect_net]){
                if(cur_cell != c){
                    if(!cn[cur_cell].count(c)){
                        cn[cur_cell].insert(c);
                        cell_neighbor[cur_cell].push_back(c);
                    }
                }
            }
        }
    }
}

void FM::initial_partition(){
    if(cells > 50000){
        size_A = 0;
        for(int i=1;i<=cells;i++){
            if(random()%20000 ==0){
                cell_side[i] = 1;
                old_side[i] = 1;
            }
            else{
                cell_side[i] = 0;
                old_side[i] = 0;
                size_A++;
            }
        }
        old_size_A = size_A;
        cout<<size_A<<endl;
        balance = false;
    }
    else{
        /*
        unordered_set<int> tmp;
        queue<int> q;
        q.push(random()%cells + 1);
        while(size_A < min){
            int cell = q.front();
            q.pop();
            if(tmp.count(cell)){
                if(q.empty()){
                    q.push(random()%cells + 1);
                }
                continue;
            }

            tmp.insert(cell);
            cell_side[cell] = 0;
            size_A++;
            old_side[cell] = 0;
            
            int max_conn = -1;
            int max_idx = -1;
            for(auto n : cell_neighbor[cell]){
                if(tmp.count(n))
                    continue;

                float conn = 0; 
                for(auto cn : cell_list[cell])
                    if(net_set[n].count(cn))
                        conn += 1.0/net_list[cn].size();
                if(conn > max_conn){
                    max_conn = conn;
                    max_idx = n;
                }
            }
            if(max_idx != -1)
                q.push(max_idx);
            if(q.empty())
                q.push(random()%cells + 1);
        }
        */
        // current best
        unordered_set<int> tmp;
        for(int i=1;i<=cells;i++){
            if(i<=min){
                tmp.insert(i);
                cell_side[i] = 0;
                size_A++;
                old_side[i] = 0;
            }
        }
        

        old_size_A = size_A;
        for(int i=1;i<=cells;i++){
            if(!tmp.count(i)){
                cell_side[i] = 1;
                old_side[i] = 1;
            }
        }

        
    }
}

void FM::make_gain_list(){
    for(int i=0;i<=2*P_MAX;i++){
        set_A[i].clear();
        set_B[i].clear();
    }
    set_A_ptr = 0;
    set_B_ptr = 0;
    for(int c=1;c<=cells;c++){
        int gain = P_MAX;
        cal_gain(c, gain);
        // move cur from to bucket list
        cell_gain[c] = gain - P_MAX;
        if(cell_side[c]){
            set_B[gain].insert(c);
            if(set_B_ptr < gain)
                set_B_ptr = gain;
        }
        else{
            set_A[gain].insert(c);
            if(set_A_ptr < gain)
                set_A_ptr = gain;
        }
        locked[c] = 0;
    }
}

void FM::cal_gain(const int& in, int& gain){
    bool cur_side = cell_side[in];
    for(const auto& net_node : cell_list[in]){
        bool lonely = true; // cur cell is the only cell in cur_side on this net
        bool together = true; // cur cell is in the same cur_side with all other cell on this net

        for(const auto& cell : net_list[net_node]){
            if(cell != in){
                lonely &= (cell_side[cell] != cur_side);
                if(!lonely)
                    break;
            }
        }

        for(const auto& cell : net_list[net_node]){
            if(cell != in){
                together &= (cell_side[cell] == cur_side);
                if(!together)
                    break;
            }
        }
        
        if(lonely)
            gain++;
        if(together)
            gain--;
        
    }
}

void FM::update_neighbor(const int& in){
    for(const auto& c : cell_neighbor[in]){
        if(cell_neighbor[c].size() > 1000 && cells > 10000)
            continue;
        if(locked[c] == 0){
            int gain = P_MAX;
            cal_gain(c, gain);
            if(old_side[c]){ //cell is in B
                set_B[cell_gain[c]+P_MAX].erase(c);
                set_B[gain].insert(c);
                if(set_B_ptr < gain)
                    set_B_ptr = gain;
            }
            else{
                set_A[cell_gain[c]+P_MAX].erase(c);
                set_A[gain].insert(c);
                if(set_A_ptr < gain)
                    set_A_ptr = gain;           
            }
            
            cell_gain[c] = gain - P_MAX;
        }
    }
}

void FM::output(){
    FILE *output = fopen("output.txt", "w");
    for(int i=1;i<=cells;i++){
        fprintf(output, "%d\n", (int)cell_side[i]);
    }
    fclose(output);
}

void FM::printCuts(){
    int cuts = 0;
    for(int i=0;i<nets;i++){
        for(int j=1;j<net_list[i].size();j++){
            if(cell_side[net_list[i][j-1]] != cell_side[net_list[i][j]]){
                cuts++;
                break;
            }
        }
    }
    cout<<"Cuts : "<<cuts<<endl;
    cut = cuts;
}

void FM::getCuts(){
    cut = 0;
    for(int i=0;i<nets;i++){
        for(int j=1;j<net_list[i].size();j++){
            if(cell_side[net_list[i][j-1]] != cell_side[net_list[i][j]]){
                cut++;
                break;
            }
        }
    }
}

void FM::make_bucket(){
    // set_A = vector<unordered_set<int>>(2*P_MAX+1);
    // set_B = vector<unordered_set<int>>(2*P_MAX+1);
    set_B_ptr = 0;
    set_A_ptr = 0;
    for(int i=1;i<=cells;i++){
        if(cell_side[i]){
            set_B[cell_gain[i] + P_MAX].insert(i);
            if(set_B_ptr < cell_gain[i] + P_MAX)
                set_B_ptr = cell_gain[i] + P_MAX;
        }
        else{
            set_A[cell_gain[i] + P_MAX].insert(i);
            if(set_A_ptr < cell_gain[i] + P_MAX)
                set_A_ptr = cell_gain[i] + P_MAX;
        }
        locked[i] = 0;
    }
}

void FM::printGainList(){
    cout<<"Set A"<<endl;
    for(int i=2*P_MAX;i>=0;i--){
        for(auto c : set_A[i])
            cout<<c<<" ";
        cout<<endl;
    }
    cout<<"Set B"<<endl;
    for(int i=2*P_MAX;i>=0;i--){
        for(auto c : set_B[i])
            cout<<c<<" ";
        cout<<endl;
    }
    
}

void FM::FM_move(){
    int iter_time = cells/2;
    int max_gain = total_gain;
    // int cur_gain = total_gain;
    // total_gain = 0;
    // cur_gain = 0;
    int max_idx = 0;
    for(int lock_num=1;lock_num<=iter_time;lock_num++){
        while(set_A[set_A_ptr].empty()){
            set_A_ptr--;
        }
        while(set_B[set_B_ptr].empty()){
            set_B_ptr--;
        }

        int cell;
        if(size_A < min || set_A_ptr < 0){
            cell = *set_B[set_B_ptr].begin();
            set_B[set_B_ptr].erase(cell);
            size_A++;
        }
        else if(size_A >= max || set_B_ptr < 0){
            cell = *set_A[set_A_ptr].begin();
            set_A[set_A_ptr].erase(cell); 
            size_A--;
        }
        else if((set_A_ptr > set_B_ptr && size_A > min)){
            cell = *set_A[set_A_ptr].begin();
            set_A[set_A_ptr].erase(cell); 
            size_A--;
        }
        else{
            cell = *set_A[set_A_ptr].begin();
            set_A[set_A_ptr].erase(cell); 
            size_A--;
        }

        locked[cell] = 1;
        cell_side[cell] = !cell_side[cell];
        step_move[lock_num] = cell;
        update_neighbor(cell);
        total_gain += cell_gain[cell];
        // cell_gain[cell] = -1 * cell_gain[cell];
        if(total_gain > cur_gain){
            cur_gain = total_gain;
            max_idx = lock_num;
        }
    }
    
    total_gain = cur_gain;
    
    for(int i=1;i<=max_idx;i++)
        old_side[step_move[i]] = !old_side[step_move[i]];
    size_A = 0;
    for(int i=1;i<=cells;i++){
        cell_side[i] = old_side[i];
        if(!cell_side[i])
            size_A++;
    }
    if(size_A >= min && size_A <= max)
        balance = true;
}

void FM::real_FM(){
    int iter_time = cells;
    int total_gain = 0;
    int max_gain = 0;
    int max_idx = 0;
    for(int lock_num=1;lock_num<=iter_time;lock_num++){
        while(set_A[set_A_ptr].empty()){
            set_A_ptr--;
        }
        while(set_B[set_B_ptr].empty()){
            set_B_ptr--;
        }

        if(set_A_ptr < 0 && set_B_ptr < 0){
            break;
        }
        int cell;
        if(size_A < min || set_A_ptr < 0){
            cell = *set_B[set_B_ptr].begin();
            set_B[set_B_ptr].erase(cell);
            size_A++;
        }
        else if(size_A >= max || set_B_ptr < 0){
            cell = *set_A[set_A_ptr].begin();
            set_A[set_A_ptr].erase(cell); 
            size_A--;
        }
        else if((set_A_ptr > set_B_ptr && size_A > min)){
            cell = *set_A[set_A_ptr].begin();
            set_A[set_A_ptr].erase(cell); 
            size_A--;
        }
        else{
            cell = *set_A[set_A_ptr].begin();
            set_A[set_A_ptr].erase(cell); 
            size_A--;
        }

        locked[cell] = 1;
        cell_side[cell] = !cell_side[cell];
        step_move[lock_num] = cell;
        update_neighbor(cell);
        total_gain += cell_gain[cell];
        if(total_gain >= max_gain){
            max_gain = total_gain;
            max_idx = lock_num;
        }
    }

    if(max_gain <= 0){
        size_A = 0;
        stop_FM = true;
        for(int i=1;i<=cells;i++){
            cell_side[i] = old_side[i];
            if(!cell_side[i])
                size_A++;
        }
        return ;
    }
    
    for(int i=1;i<=max_idx;i++)
        old_side[step_move[i]] = !old_side[step_move[i]];
    size_A = 0;
    for(int i=1;i<=cells;i++){
        cell_side[i] = old_side[i];
        if(!cell_side[i])
            size_A++;
    }
    cout<<"step gain: "<<max_gain<<endl;
}

void FM::cal_coGain(const int& in_A, const int& in_B, int& gain){
    for(const auto& net_node : cell_list[in_A]){
        bool lonely = true;
        bool together = true;

        for(const auto& cell : net_list[net_node]){
            if(cell == in_B){
                lonely = false;
                together = false;
                break;
            }
            if(cell != in_A){
                bool tmp = cell_side[cell];
                lonely &= (tmp != false);
                together &= (tmp == false);
                if(!lonely && !together)
                    break;
            }
        }

        if(lonely)
            gain++;
        if(together)
            gain--;
    }
    for(const auto& net_node : cell_list[in_B]){
        bool lonely = true;
        bool together = true;

        for(const auto& cell : net_list[net_node]){
            if(cell == in_A){
                lonely = false;
                together = false;
                break;
            }
            if(cell != in_B){
                bool tmp = cell_side[cell];
                lonely &= (tmp != true);
                together &= (tmp == true);
                if(!lonely && !together)
                    break;
            }
        }

        if(lonely)
            gain++;
        if(together)
            gain--;
    }
}