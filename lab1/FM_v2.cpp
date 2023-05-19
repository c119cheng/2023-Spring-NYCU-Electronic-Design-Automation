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
    make_gain_list();
    printCuts();
    cur_gain = 0;
    total_gain = 0;
    // old_size_A = size_A;
    while(!stop_FM){
        cout<<"start FM"<<endl;
        FM_move();
        printCuts();
        auto now = chrono::system_clock::now();
        if(now - start_time > chrono::seconds(25))
           break;
        cout<<"make bucket"<<endl;
        // break;
        make_bucket();
        // make_gain_list();
        cout<<"size_A"<<size_A<<endl;
    }

    cout<<size_A << "   size   "<< cells - size_A<<endl;
        printCuts();
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
    unordered_set<int> *net_set = new unordered_set<int>[cells+1];
    int i=0; // current read net
    P_MAX = 0;
    while(getline(fin, line)){
        ss.str(line);
        int in;
        while(ss >> in){
            this->net_list[i].push_back(in);
            this->cell_list[in].push_back(i);
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
    // bucket list
    /*
    for(int i=1;i<=cells;i++){
        this->locked.insert(i);
    }
    */
    this->locked = vector<int>(cells+1);

    // making neighbor set
    stack<int> s;
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
                if(cell_neighbor[cur_cell].size() < 5)
                    s.push(cur_cell);
            }
        }
    }
    


    // initial random result
    this->cell_gain = new int[this->cells+1];
    this->cell_side = new bool[this->cells+1];
    this->old_side = new bool[this->cells+1];
    this->size_A = 0;
    this->size_B = 0;
    /*
    srand(time(NULL) + random());
    
    for(int i=1;i<=cells;i++){
        bool a_or_b = random()%2;
        this->cell_side[i] = (a_or_b);
        if(a_or_b)
            size_B++;
        else
            size_A++;
    }
    */
    /*
    for(int i=1;i<=cells;i++){
        if(i<cells/2){
            this->cell_side[i] = 0;
            size_A++;
        }
        else{
            this->cell_side[i] = 1;

        }
    }
    */
    // dfs for initial partition
    
    std::unordered_set<int> tmp;
    int n = nets;
    /*
    int cur_cell = s.top();
    s.pop();
    while(size_A < cells/2){
        if(tmp.count(cur_cell) || cur_cell == 0){
            cur_cell = s.top();
            s.pop();
            continue;
        }
        tmp.insert(cur_cell);
        size_A++;
        cell_side[cur_cell] = 0;
        int max_conn = 0;
        int idx = 0;
        for(auto neighbor : cell_neighbor[cur_cell]){
            if(tmp.count(neighbor)){
                continue;
            }
            //unordered_set<int> inter_nets;
            
            int conn = 0;
            // cout<<"there"<<endl;
            for(auto a:cell_list[cur_cell]){
                if(net_list[a].size() > 5)
                    continue;
                // cout<<"here"<<endl;
                if(net_set[neighbor].count(a))
                    // inter_nets.insert(a);
                    conn++;
            }
            if(conn > max_conn){
                cur_cell = neighbor;
                max_conn = conn;
            }
        }
    }

    */
    
    /*
    for(int i=0;i<nets;i++){
        if(net_list[i].size()>4)
            continue;
        for(auto c:net_list[i]){
            if(!tmp.count(c)){
                cell_side[c] = 0;
                size_A++;
                tmp.insert(c);
            }
        }
        if(size_A > cells/2)
            break;
    }
    */
    /*
    queue<int> q;
    q.push(s.top());
    s.pop();
    while(!q.empty() && size_A < cells/2){
        int cur_cell = q.front();
        q.pop();
        if(tmp.count(cur_cell)){
            if(q.empty()){
                q.push(s.top());
                s.pop();
            }
            continue;
        }
        tmp.insert(cur_cell);
        size_A++;
        cell_side[cur_cell] = 0;
        for(auto n : cell_neighbor[cur_cell]){
            if(!tmp.count(n) && cell_neighbor[n].size() < 5){
                q.push(n);
            }
        }
        if(q.empty()){
            q.push(s.top());
            s.pop();
        }
    }
    while(!q.empty())
        q.pop();
    q.push(s.top());
    s.pop();
    while(!q.empty() && size_B < cells/2){
        int cur_cell = q.front();
        q.pop();
        if(tmp.count(cur_cell)){
            if(q.empty()){
                q.push(s.top());
                s.pop();
            }
            continue;
        }
        tmp.insert(cur_cell);
        size_B++;
        cell_side[cur_cell] = 1;
        for(auto n : cell_neighbor[cur_cell]){
            if(!tmp.count(n) && cell_neighbor[n].size() < 5){
                q.push(n);
            }
        }
        if(q.empty()){
            q.push(s.top());
            s.pop();
        }
    }
    */
    cout<<size_A << "   "<<max <<"   "<<size_B<<endl;

    for(int i=1;i<=cells;i++){
        if(!tmp.count(i)){
            //if(i%2 == 0){
            if(size_A < cells/2){
                cell_side[i] = 0;
                size_A++;
            }
            else
                cell_side[i] = 1;
        }
    }

    for(int i=1;i<=cells;i++)
        cell_side[i] = 0;
    size_A = cells;
    // for FM move
    this->step_gain = new int[this->cells];
    this->step_move = new int[this->cells];
    // for debug
    /*
    for(int i=0;i<this->cells;i++){
        node* cur = cell_list[i].getHead();
        while(cur){
            cout<<cur->val<<" ";
            cur = cur->next;
        }
        cout<<endl;
    }
    */

    // neighbor debug
    /*
    for(int i=1;i<=this->cells;i++){
        cout<<"cell " << i << "neighbor"<<endl;
        for(auto cur :cell_neighbor[i]){
            cout<<cur<<" ";
        }
        cout<<endl;
    }
    */
}


void FM::make_gain_list(){
    set_A.clear();
    set_B.clear();
    set_A_ptr = 0;
    set_B_ptr = 0;
    for(int c=1;c<=cells;c++){
    // for(auto c : locked){
        // node* tmp = cur->next;
        int gain = 0;
        cal_gain(c, gain);
        // move cur from to bucket list
        cell_gain[c] = gain;
        if(cell_side[c]){
            set_B[gain+P_MAX].insert(c);
            if(set_B_ptr < gain + P_MAX)
                set_B_ptr = gain + P_MAX;
        }
        else{
            set_A[gain+P_MAX].insert(c);
            if(set_A_ptr < gain + P_MAX)
                set_A_ptr = gain + P_MAX;
        }
        locked[c] = 0;
    }
    // locked.clear();
    // make_bucket();
}

void FM::cal_gain(const int& in, int& gain){
    bool cur_side = cell_side[in];
    // for(node* net_node=cell_list[in].getHead();net_node!=nullptr;net_node = net_node->next){
    for(const auto& net_node : cell_list[in]){
        bool lonely = true; // cur cell is the only cell in cur_side on this net
        bool together = true; // cur cell is in the same cur_side with all other cell on this net

        // for(node* cell=net_list[net_node->val].getHead();cell!=nullptr;cell=cell->next){
        for(const auto& cell : net_list[net_node]){
            if(cell != in){
                // bool tmp = cell_side[cell];
                lonely &= (cell_side[cell] != cur_side);
                // together &= (tmp == cur_side);
                // if(!lonely && !together)
                //    break;
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
    // cout<<cur->val<<" gain "<<gain<<endl;
}

void FM::FM_move(){
    //stop_FM = true;
    int ti = cells/2;
    for(int lock_num=0;lock_num<ti;lock_num++){
        // cout<<"start"<<endl;
        /*
        auto it_A = set_A.rbegin();
        auto it_B = set_B.rbegin();
        auto ta = set_A.rbegin();
        auto tb = set_B.rbegin();

        while(it_A!=set_A.rend() && it_A->second.empty()){
            ta = it_A++;
            set_A.erase(ta->first);
        }
        while(it_B!=set_B.rend() && it_B->second.empty()){
            tb = it_B++;
            set_B.erase(tb->first);
        }
        
        if(set_A.empty() || set_B.empty()){
            cout<<"one empty"<<endl;
            ti = lock_num;
            break;
        }
        
        int c;
        if(((it_A->first >= it_B->first && size_A > min )|| size_A == max)){
            c = *(it_A->second.begin());
            locked.insert(c);
            it_A->second.erase(c);
            size_A--;
        }
        else{
            c = *(it_B->second.begin());
            locked.insert(c);
            it_B->second.erase(c);
            size_A++;
        }
        */
        // cout<<"hera"<<endl;
        // cout<<"there"<<set_A_ptr<<"   "<<set_B_ptr<<endl;
        // set_A_ptr = 2*P_MAX;
        // set_B_ptr = 2*P_MAX;
        while(set_A[set_A_ptr].empty()){
            set_A_ptr--;
        }
        while(set_B[set_B_ptr].empty()){
            set_B_ptr--;
        }
        if(( set_A_ptr<0) && (  set_B_ptr<0)){
            cout<<"two empty"<<endl;
            ti = lock_num;
            break;
        }
        // cout<<"there"<<set_A_ptr<<"   "<<set_B_ptr<<endl;
        int c;
        if(set_B_ptr >=0 && size_A < min){
            // cout<<"A < min"<<endl;
            c = *set_B[set_B_ptr].begin();
            //remove from bucket list
            set_B[set_B_ptr].erase(c);
            size_A++;
        }
        else if(set_A_ptr >= 0 && size_A >= max){
            // cout<<"A >= max"<<endl;
            c = *set_A[set_A_ptr].begin();
            //remove from bucket list
            set_A[set_A_ptr].erase(c);
            size_A--;
        }
        else if( set_A_ptr < 0){
                        // cout<<"move from B to A"<<endl;
            c = *set_B[set_B_ptr].begin();
            //remove from bucket list
            set_B[set_B_ptr].erase(c);
            size_A++;
        }
        else if(set_B_ptr < 0){
            c = *set_A[set_A_ptr].begin();
            //remove from bucket list
            set_A[set_A_ptr].erase(c);
            size_A--;
        }
        else if((set_A_ptr > set_B_ptr && size_A > min) || size_A >= max){
            // cout<<"move from A to B"<<endl;
            c = *set_A[set_A_ptr].begin();
            //remove from bucket list
            set_A[set_A_ptr].erase(c);
            size_A--;
        }
        else{
            // cout<<"move from B to A"<<endl;
            c = *set_B[set_B_ptr].begin();
            //remove from bucket list
            set_B[set_B_ptr].erase(c);
            size_A++;
        }
        // cout<<"finish erase"<<endl;
        // cout<<"far"<<endl;
        locked[c] = 1;
        cell_side[c] = !cell_side[c];
        //this->step_gain[lock_num] = cell_gain[c];
        //this->step_move[lock_num] = c;
        
        total_gain += cell_gain[c];
        // cout<<"update"<<endl;
        update_neighbor(c);
        cell_gain[c] = -1 * cell_gain[lock_num];
        //cout<<step_balance[lock_num]<<endl;
        // early stop condition
        // if(cell_gain[c] < -5){
        //     break;
        // }
        if(total_gain > cur_gain){
            // stop_FM = false;
            cur_gain = total_gain;
            old_size_A = size_A;
            for(int i=1;i<=cells;i++)
                old_side[i] = cell_side[i];
           // cout<<"total gain"<<total_gain<<endl;
        }
        // printCuts();
    }
    /*
    // get partial sum
    int max_step_gain = step_gain[0];
    int max_idx = -1;
    for(int i=1;i<ti;i++){
        step_gain[i] = step_gain[i] + step_gain[i-1];
        if(step_gain[i] > max_step_gain){
            max_step_gain = step_gain[i];
            max_idx = i;
        }
    }

    cout<<"max_step_gain : "<<max_step_gain<<endl;
    if(max_step_gain <= 0 || max_idx == -1){
        this->stop_FM = true;
    }

    // update new partition
    for(int i=0;i<=max_idx;i++){
        old_side[step_move[i]] = !old_side[step_move[i]];
    }
    for(int i=max_idx+1;i<ti;i++){
        if(old_side[step_move[i]]){
            size_A--;
        }
        else{
            size_A++;
        }
    }
    for(int i=1;i<=this->cells;i++)
        cell_side[i] = old_side[i];

    */
    total_gain = cur_gain;
    size_A = old_size_A;
    for(int i=1;i<=cells;i++){
        cell_side[i] = old_side[i];

    }
    for(int i=1;i<=cells;i++)
        locked[i] = 0;
}


void FM::update_neighbor(const int& in){
    for(const auto& c : cell_neighbor[in]){
        if(locked[c] == 0){
            int gain = 0;
            cal_gain(c, gain);
            if(old_side[c]){ //cell is in B
                set_B[cell_gain[c]+P_MAX].erase(c);
                set_B[gain+P_MAX].insert(c);
                if(set_B_ptr < gain + P_MAX)
                    set_B_ptr = gain + P_MAX;
            }
            else{
                set_A[cell_gain[c]+P_MAX].erase(c);
                set_A[gain+P_MAX].insert(c);
                if(set_A_ptr < gain + P_MAX)
                    set_A_ptr = gain + P_MAX;           
            }
            
            cell_gain[c] = gain;
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
    for(int i=1;i<=cells;i++){
        if(cell_side[i]){
            set_B[cell_gain[i] + P_MAX].insert(i);
        }
        else{
            set_A[cell_gain[i] + P_MAX].insert(i);
        }
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