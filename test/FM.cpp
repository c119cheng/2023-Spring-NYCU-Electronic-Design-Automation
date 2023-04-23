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
#include <list>
#include <utility>
using namespace std;

FM::FM(char *file_name){
    this->file_name = file_name;
}

FM::~FM(){

}


void FM::test(){
    auto start_time = chrono::system_clock::now();
    this->loadfile();
    int best = nets;
    vector<int> best_side(cells+1);
    int prev = nets;
    int count = 0;
    while(1){
        size_A = 0;
        size_B = 0;
        int initial_net = random()%(nets);
        this->initial_partition(initial_net);
        for(int i=1;i<=cells;i++)
            old_side[i] = cell_side[i];

        old_size_A = size_A;

        stop_FM = false;
        // printCuts();
        while(!stop_FM){
            for(int i=0;i<nets;i++){
                net_A_size[i] = 0;
                net_B_size[i] = 0;
                for(const auto& cell:net_list[i]){
                    if(cell_side[cell]){
                        net_B_size[i]++;
                    }
                    else{
                        net_A_size[i]++;
                    }
                }
            }


            cal_all_gain();
            make_bucket();
            real_FM();
            auto now = chrono::system_clock::now();
            if(now - start_time > chrono::seconds(25))
                break;
        }
        getCuts();
        // cout<<"Cuts "<<this->cut<<endl;

        bool balance = (size_A >= min && size_A <= max);
        if(cut < best && balance){
            best = cut;
            for(int i=1;i<=cells;i++){
                best_side[i] = cell_side[i];
            }
        }
        if(prev == best)
            count++;
        if(count > 100)
            break;
        prev = best;
    
        auto now = chrono::system_clock::now();
        if(now - start_time > chrono::seconds(25))
            break;
    }
    for(int i=1;i<=cells;i++)
        cell_side[i] = best_side[i];

    output();
}

void FM::sol(){
    // srand(s);
    this->stop_FM = false;
    auto start_time = chrono::system_clock::now();
    this->loadfile();
    this->initial_partition(2);

    for(int i=1;i<=cells;i++)
        old_side[i] = cell_side[i];
    old_size_A = size_A;
    cout<<"size A "<<size_A<<"  size B  "<<size_B<<endl;
    cout<<"min  "<<min<<"   max   "<<max<<endl;
    printCuts();
    make_gain_list();
    int old_cur = nets;
    while(!stop_FM && cells > 50000 && 0){
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
    
    getCuts();
    // cout<<"start output"<<endl;
    output();
    // cout<<"finish output"<<endl;
}

void FM::loadfile(){
    ifstream fin(this->file_name);
    string line;
    getline(fin, line);
    stringstream ss(line);
    ss >> this->nets >> this->cells;
    ss.clear();
    
    this->cell_list = new std::vector<int>[this->cells+1];
    this->net_list = new std::vector<int>[this->nets];
    this->net_A_size = vector<int>(this->nets, 0);
    this->net_B_size = vector<int>(this->nets, 0);
    this->cell_neighbor = new std::unordered_set<int>[this->cells+1];
    
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

    this->set_A = vector< unordered_set<int> >(2*P_MAX+1);
    this->set_B = vector< unordered_set<int> >(2*P_MAX+1);

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
}

void FM::initial_partition(int in=1){
    vector<bool> tmp(cells+1, false);
    for(int i=in;i<nets;i++){
        bool side_A = false;
        bool side_B = false;
        for(const auto& cell : net_list[i])
            if(tmp[cell]){
                if(cell_side[cell])
                    side_B = true;
                else
                    side_A = true;
            }

        bool side;
        if(side_A && side_B){
            continue;
        }
        else if(side_A){
            side = 0;
        }
        else if(side_B){
            side = 1;
        }
        else{
            if(size_A > size_B)
                side = 1;
            else
                side = 0;
        }

        for(const auto& cell : net_list[i]){
            if(!tmp[cell]){
                if(side && size_B <= max){
                    cell_side[cell] = 1;
                    size_B++;
                }
                else{
                    cell_side[cell] = 0;
                    size_A++;
                }
                tmp[cell] = 1;
            }
        }
    }
    // vector<bool> tmp(cells+1, false);
    // int ini = in;
    for(int i=0;i<in;i++){
        bool side_A = false;
        bool side_B = false;
        for(const auto& cell : net_list[i])
            if(tmp[cell]){
                if(cell_side[cell])
                    side_B = true;
                else
                    side_A = true;
            }

        bool side;
        if(side_A && side_B){
            continue;
        }
        else if(side_A){
            side = 0;
        }
        else if(side_B){
            side = 1;
        }
        else{
            if(size_A > size_B)
                side = 1;
            else
                side = 0;
        }

        for(const auto& cell : net_list[i]){
            if(!tmp[cell]){
                if(side && size_B <= max){
                    cell_side[cell] = 1;
                    size_B++; 
                }
                else{
                    cell_side[cell] = 0;
                    size_A++;
                }
                tmp[cell] = 1;
            }
        }
    }

    for(int i=1;i<=cells;i++){
        if(!tmp[i]){
            if(size_A <= cells/2){
                cell_side[i] = 0;
                size_A++;
                for(const auto& n:cell_list[i])
                    net_A_size[n]++;
            }
            else{
                cell_side[i] = 1;
                size_B++;
                for(const auto& n:cell_list[i])
                    net_B_size[n]++;
            }
        }
    }

    for(int i=0;i<nets;i++){
        net_A_size[i] = 0;
        net_B_size[i] = 0;
        for(const auto& cell:net_list[i]){
            if(cell_side[cell]){
                net_B_size[i]++;
            }
            else{
                net_A_size[i]++;
            }
        }
    }

    /*
    // for debug
    for(int i=1;i<=cells;i++){
        cout<<"cell "<<i<<" is in side "<<cell_side[i]<<endl;
    }
    for(int i=0;i<nets;i++){
        cout<<"net "<<i<<" side A and B are "<<net_A_size[i]<<"  "<<net_B_size[i]<<endl;
    }
    */
    /*
    balance = (size_A >= min && size_A <= max);   
    cout<<"initial balance "<<balance<<endl;
    getCuts();
    cout<<"initial cuts "<<cut<<endl;
    cout<< size_A <<"   "<<size_B<<endl;
    */
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

void FM::cal_all_gain(){
    for(int i=1;i<=cells;i++)
        cell_gain[i] = 0;

    for(int i=0;i<nets;i++){
        if(net_A_size[i] > 1 && net_B_size[i] > 1)
            continue;
        if(net_A_size[i] == 1){
            for(const auto&cell : net_list[i]){
                if(cell_side[cell] == 0){
                    cell_gain[cell]++;
                    break;
                }
            }
        }
        else if(net_A_size[i] == 0){
            for(auto const& cell : net_list[i])
                cell_gain[cell]--;
        }
        if(net_B_size[i] == 1){
            for(const auto& cell : net_list[i]){
                if(cell_side[cell] == 1){
                    cell_gain[cell]++;
                    break;
                }
            }
        }
        else if(net_B_size[i] == 0){
            for(auto const& cell : net_list[i])
                cell_gain[cell]--;
        }
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

void FM::update_gain(const int& in){
    // reserve cell gain
    unordered_set<int> visited;
    vector< std::pair<int, int> > pre_gain;

    for(const auto& i : cell_list[in]){
        if(cell_side[in]){
            // From A to B
            if(net_B_size[i] == 0){
                for(const auto& cell : net_list[i])
                    if(!locked[cell]){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]++;
                    }
            }
            else if(net_B_size[i] == 1){
                for(const auto& cell : net_list[i])
                    if(!locked[cell] && cell_side[cell] == 1){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]--;
                        break;
                    }
            }
            net_B_size[i]++;
            net_A_size[i]--;

            if(net_A_size[i] == 0){
                for(const auto& cell : net_list[i])
                    if(!locked[cell]){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]--;
                    }
            }
            else if(net_A_size[i] == 1){
                for(const auto& cell : net_list[i])
                    if(!locked[cell] && cell_side[cell] == 0){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]++;
                        break;
                    }
            }
        }       
        else{
            // From B to A
            if(net_A_size[i] == 0){
                for(const auto& cell : net_list[i])
                    if(!locked[cell]){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]++;
                    }
            }
            else if(net_A_size[i] == 1){
                for(const auto& cell : net_list[i])
                    if(!locked[cell] && cell_side[cell] == 0){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]--;
                        break;
                    }
            }
            net_A_size[i]++;
            net_B_size[i]--;

            if(net_B_size[i] == 0){
                for(const auto& cell : net_list[i])
                    if(!locked[cell]){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]--;
                    }
            }
            else if(net_B_size[i] == 1){
                for(const auto& cell : net_list[i])
                    if(!locked[cell] && cell_side[cell] == 1){
                        if(!visited.count(cell)){
                            pre_gain.push_back({cell, cell_gain[cell]});
                            visited.insert(cell);
                        }
                        cell_gain[cell]++;
                        break;
                    }
            }
        }
    }

    // update bucket list
    for(const auto p : pre_gain){
        if(cell_gain[p.first] != p.second){
            if(cell_side[p.first]){
                set_B[p.second + P_MAX].erase(p.first);
                set_B[cell_gain[p.first] + P_MAX].insert(p.first);
                if(set_B_ptr < cell_gain[p.first] + P_MAX)
                    set_B_ptr = cell_gain[p.first] + P_MAX;
            }
            else{
                set_A[p.second + P_MAX].erase(p.first);
                set_A[cell_gain[p.first] + P_MAX].insert(p.first);
                if(set_A_ptr < cell_gain[p.first] + P_MAX)
                    set_A_ptr = cell_gain[p.first] + P_MAX;
            }
        }
    }
}

void FM::output(){
    FILE *output = fopen("output.txt", "w");
    for(int i=1;i<=cells;i++){
        fprintf(output, "%d\n", (int)cell_side[i]);
    }
    fclose(output);
    // cout<<"finish output"<<endl;
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
    for(int i=0;i<=2*P_MAX;i++){
        set_A[i].clear();
        set_B[i].clear();
    }
    // set_A = vector<unordered_set<int>>(2*P_MAX+1);
    // set_B = vector<unordered_set<int>>(2*P_MAX+1);
    set_B_ptr = 0;
    set_A_ptr = 0;
    for(int i=1;i<=cells;i++){
        if(cell_side[i]){
            int idx = cell_gain[i] + P_MAX;
            set_B[idx].insert(i);
            if(set_B_ptr < idx)
                set_B_ptr = idx;
        }
        else{
            int idx = cell_gain[i] + P_MAX;
            set_A[idx].insert(i);
            if(set_A_ptr < idx)
                set_A_ptr = idx;
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
    int iter_time = cells;
    int max_gain = 0;
    int cur_gain = 0;
    int total_gain = 0;
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
        while(set_A_ptr >= 0 &&set_A[set_A_ptr].empty()){
            set_A_ptr--;
        }
        while(set_B_ptr >= 0 &&set_B[set_B_ptr].empty()){
            set_B_ptr--;
        }
        int cell;

        if(set_A_ptr < 0 && set_B_ptr < 0){
            break;
        }
        else if(set_A_ptr < 0){
            cell = *set_B[set_B_ptr].begin();
            set_B[set_B_ptr].erase(cell);
            size_A++;
        }
        else if(set_B_ptr < 0){
            cell = *set_A[set_A_ptr].begin();
            set_A[set_A_ptr].erase(cell); 
            size_A--;     
        }
        else if((set_A_ptr > set_B_ptr && size_A > min) || size_A >= max){
            cell = *set_A[set_A_ptr].begin();
            set_A[set_A_ptr].erase(cell); 
            size_A--;
        }
        else{
            cell = *set_B[set_B_ptr].begin();
            set_B[set_B_ptr].erase(cell);
            size_A++;
        }

        locked[cell] = 1;
        cell_side[cell] = !cell_side[cell];
        step_move[lock_num] = cell;
        // update_neighbor(cell);
        update_gain(cell);

        total_gain += cell_gain[cell];
        if(total_gain >= max_gain){
            max_gain = total_gain;
            max_idx = lock_num;
        }
    }
    // cout<<"balance  "<<balance<<endl;
    if(max_gain <= 0){
        stop_FM = true;
    }
    
    for(int i=1;i<=max_idx;i++)
        old_side[step_move[i]] = !old_side[step_move[i]];
    size_A = 0;
    for(int i=1;i<=cells;i++){
        cell_side[i] = old_side[i];
        if(!cell_side[i])
            size_A++;
    }
    // cout<<"step gain: "<<max_gain<<endl;
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