#include "FM.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <omp.h>
#include <chrono>
#include <stack>
#include <queue>
#include <algorithm>
using namespace std;

FM::FM(char *file_name){
    this->file_name = file_name;
}

FM::~FM(){

}

void FM::sol(){
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

    int i=0; // current read net
    P_MAX = 0;
    while(getline(fin, line)){
        ss.str(line);
        int in;
        while(ss >> in){
            this->net_list[i].push_back(in);
            this->cell_list[in].push_back(i);
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
        for(int i=1;i<=cells;i++){
            cell_side[i] = 0;
            old_side[i] = 0;
        }
        old_size_A = cells;
        size_A = cells;
        balance = false;
    }
    else{
        unordered_set<int> tmp;
        queue<int> q;
        stack<int> s;
        s.push(random()%cells + 1);
        while(size_A < cells/2){
            int cell = s.top();
            s.pop();
            if(tmp.count(cell)){
                if(s.empty()){
                    s.push(random()%cells + 1);
                }
                continue;
            }
            tmp.insert(cell);
            cell_side[cell] = 0;
            size_A++;
            old_side[cell] = 0;
            for(auto n : cell_neighbor[cell]){
                if(!tmp.count(n))
                    s.push(n);
            }

            if(s.empty())
                s.push(random()%cells + 1);
        }
        old_size_A = size_A;
        for(int i=1;i<=cells;i++)
            if(!tmp.count(i))
                cell_side[i] = 1;
        cout<<"size A  "<<size_A<<endl;
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

void FM::make_bucket(){
    set_A = vector<unordered_set<int>>(2*P_MAX+1);
    set_B = vector<unordered_set<int>>(2*P_MAX+1);
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
        if(total_gain >= cur_gain){
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
    int iter_time = cells/2;
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

        if(set_A_ptr < 0 || set_B_ptr < 0){
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