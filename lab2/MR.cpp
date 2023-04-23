#include "MR.h"

sol::sol(char *in, char *out){
    input_file = in;
    output_file = out;
}

sol::~sol(){
}

void sol::readFile(){
    ifstream fin(input_file);
    string line;

    // row and col
    getline(fin, line);
    stringstream ss(line);
    ss >> line >> row;
    ss.clear();
    getline(fin, line);
    ss.str(line);
    ss >> line >> col;
    ss.clear();
    
    // block
    int numOfBlock;
    getline(fin, line);
    getline(fin, line);
    ss.str(line);
    ss >> line >> numOfBlock;
    ss.clear();
    blockList = vector<vector<int>>(numOfBlock, vector<int>(4));
    for(int i=0;i<numOfBlock;i++){
        getline(fin, line);
        ss.str(line);
        ss >> blockList[i][0] >> blockList[i][1] >> blockList[i][2] >> blockList[i][3];
        ss.clear();
    }
    
    // net
    int numOfNet;
    getline(fin, line);
    getline(fin, line);
    ss.str(line);
    ss >> line >> numOfNet;
    ss.clear();
    netList = vector<vector<int>>(numOfNet + 1, vector<int>(4));
    for(int i=1;i<=numOfNet;i++){
        getline(fin, line);
        ss.str(line);
        ss >> line >> netList[i][0] >> netList[i][1] >> netList[i][2] >> netList[i][3];
        ss.clear();
    }

}

void sol::showInfo(){
    cout <<"row : "<< row <<"  col : "<< col <<endl;
    cout <<endl<<"Block list"<<endl;
    for(const auto& b : blockList){
        cout << b[0] << " " << b[1] << " " << b[2] << " " << b[3] <<endl;
    }
    cout<<endl<<"Net list"<<endl;
    for(const auto& n : netList){
        cout << n[0] << " " << n[1] << " " << n[2] << " " << n[3] << endl;
    }
}

void sol::solve(){
    readFile();
    setArr();

    MR();

    output();
}

void sol::setArr(){
    // arr == 0 : free
    // arr == -1 : has blcok
    // arr == n : net number
    arr = vector<vector<int>>(row ,vector<int>(col, 0));

    // set block
    for(const auto& b : blockList){
        for(int i=b[0];i<=b[1];i++){
            for(int j=b[2];j<=b[3];j++){
                arr[j][i] = -1;
            }
        }
    }

    // set net source and target
    for(int i=1;i<netList.size();i++){
        arr[netList[i][1]][netList[i][0]] = i;
        arr[netList[i][3]][netList[i][2]] = i; 
    }

    gridNum = vector<int>(netList.size());
    ans = vector<vector<vector<int>>>(netList.size());
}

void sol::MR(){
    // min head
    // heap for netlist respect to dist
    // g = grid for routing
    
    // pqii netQ;
    // vector<int> netDist(netList.size());
    netDist = vector<int>(netList.size());
    for(int i=1;i<netList.size();i++){
        netDist[i] = abs(netList[i][0] - netList[i][1]) + abs(netList[i][2] - netList[i][3]);
        netQ.push({netDist[i], i});
        waitForR.insert(i);
    }

    while(!netQ.empty() || !pri_netQ.empty()){
        pair<int, int> cur;
        if(netQ.empty() || !pri_netQ.empty()){
            cur = pri_netQ.front();
            pri_netQ.pop();
        }
        else{
            cur = netQ.top();
            netQ.pop();
        }
        g = vector<vector<int>>(row, vector<int>(col, -1));
        /*
        showG(cur);
        cout<<endl;
        */
        waitForR.erase(cur.second);
        Hadlock(cur, netQ, netDist);
        /*
        showG(cur);
        cout<<endl;
        showA();
        cout<<endl;
        */
    }

}

void sol::Hadlock(const pair<int,int>& in, pqii& netQ, const vector<int>& netDist){

    bool reached = false;
    int i=0;

    int sourceX = netList[in.second][0];
    int sourceY = netList[in.second][1];
    int targetX = netList[in.second][2];
    int targetY = netList[in.second][3];

    unordered_set<int> roadBlock;
    vector<vector<pair<int,int>>> prev = vector<vector<pair<int,int>>>(row, vector<pair<int,int>>(col, {-1, -1}));


    queue<pair<int,int>> start_point;
    start_point.push({sourceY, sourceX});
    while(!reached){
        int size = start_point.size();

        if(size == 0){
            roadBlock.erase(in.second);
            erase_net(roadBlock);
            roadBlock.clear();
            start_point.push({sourceY, sourceX});
            g = vector<vector<int>>(row, vector<int>(col, -1));
            prev = vector<vector<pair<int,int>>>(row, vector<pair<int,int>>(col, {-1, -1}));
            continue;
        }

        while(size--){
            pair<int, int> cur = start_point.front();
            start_point.pop();

            // BFS
            queue<pair<int, int>> q;
            q.push(cur);
            while(!q.empty()){
                cur = q.front();
                q.pop();
                if(g[cur.first][cur.second] == i)
                    continue;
                g[cur.first][cur.second] = i;
                if(cur.first == targetY && cur.second == targetX){
                    reached = true;
                    break;
                }


                int toX = (cur.second < targetX) ? 1 : -1;
                int toY = (cur.first < targetY) ? 1 : -1;
       

                // to target
                if(cur.first + toY < row && cur.first + toY >= 0 && (g[cur.first + toY][cur.second] == -1) && (arr[cur.first + toY][cur.second] == 0 || arr[cur.first + toY][cur.second] == in.second)){
                    q.push({cur.first + toY, cur.second});
                    if(prev[cur.first + toY][cur.second].first == -1)
                        prev[cur.first + toY][cur.second] = cur;
                }
                if(cur.second + toX < col && cur.second + toX >= 0 && (g[cur.first][cur.second + toX] == -1) && (arr[cur.first][cur.second + toX] == 0 || arr[cur.first][cur.second + toX] == in.second)){
                    q.push({cur.first, cur.second + toX});
                    if(prev[cur.first][cur.second + toX].first == -1)
                        prev[cur.first][cur.second + toX] = cur;
                }
                // away target
                if(cur.first - toY < row && cur.first - toY >= 0 && (g[cur.first - toY][cur.second] == -1) && (arr[cur.first - toY][cur.second] == 0 || arr[cur.first - toY][cur.second] == in.second)){
                    start_point.push({cur.first - toY, cur.second});
                    if(prev[cur.first - toY][cur.second].first == -1)
                        prev[cur.first - toY][cur.second] = cur;
                }
                if(cur.second - toX < col && cur.second - toX >= 0 && (g[cur.first][cur.second - toX] == -1) && (arr[cur.first][cur.second - toX] == 0 || arr[cur.first][cur.second - toX] == in.second)){
                    start_point.push({cur.first, cur.second - toX});
                    if(prev[cur.first][cur.second - toX].first == -1)
                        prev[cur.first][cur.second - toX] = cur;
                }
            
                // save raodBlock
                if(cur.first + toY < row && cur.first + toY >= 0 && arr[cur.first + toY][cur.second] > 0)
                    roadBlock.insert(arr[cur.first + toY][cur.second]);
                if(cur.second + toX < col && cur.second + toX >= 0 && arr[cur.first][cur.second + toX] > 0)
                    roadBlock.insert(arr[cur.first][cur.second + toX]);
                if(cur.first - toY < row && cur.first - toY >= 0 && arr[cur.first - toY][cur.second] > 0)
                    roadBlock.insert(arr[cur.first - toY][cur.second]);
                if(cur.second - toX < col && cur.second - toX >= 0 && arr[cur.first][cur.second - toX] > 0)
                    roadBlock.insert(arr[cur.first][cur.second - toX]);

            }

        }
        i++;
    }


    // trace back
    gridNum[in.second] = 0;
    int prev_dir = -1;
    int curX = targetX;
    int curY = targetY;
    int startX = targetX;
    int startY = targetY;
    //cout<<"target prev" <<prev[]" " 
    while(1){
        // cout<<"cur X and Y : "<<curX<<" "<<curY<<endl;
        pair<int, int> next = prev[curY][curX];
        if(next.first == -1 || next.second == -1){
            //found source
            ans[in.second].push_back({startX, startY, curX, curY});
            return ;
        }
        else if((prev_dir == 0 && next.first != curY) || (prev_dir == 1 && next.second != curX)){
            ans[in.second].push_back({startX, startY, curX, curY});
            startX = curX;
            startY = curY;
        }

        if(curX == next.second)
            prev_dir = 1;
        else
            prev_dir = 0;

        arr[curY][curX] = in.second;
        curX = next.second;
        curY = next.first;
        gridNum[in.second]++;
    }
}

void sol::showG(const pair<int, int>& in){
    int sourceX = netList[in.second][0];
    int sourceY = netList[in.second][1];
    int targetX = netList[in.second][2];
    int targetY = netList[in.second][3];
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            if(i == sourceY && j == sourceX)
                cout<<" S ";
            else if(i == targetY && j == targetX)
                cout<<" T ";
            else if(arr[i][j] == -1)
                cout<<" b ";
            else if(g[i][j] == -1)
                cout<<g[i][j]<<" ";
            else
                cout<<" "<<g[i][j]<<" ";
        }
        cout<<endl;
    }

}

void sol::showA(){
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            if(arr[i][j] == -1)
                cout<<arr[i][j]<<" ";
            else
                cout<<" "<<arr[i][j]<<" ";
        }
        cout<<endl;
    }
}

void sol::output(){
    ofstream fout(output_file);
    for(int i=1;i<netList.size();i++){
        fout<<"net"<<i<<" "<<gridNum[i] -1<<endl;
        fout<<"begin"<<endl;
        for(int j=ans[i].size()-1;j>=0;j--){
            fout<<ans[i][j][2]<<" "<<ans[i][j][3]<<" "<<ans[i][j][0]<<" "<<ans[i][j][1]<<endl;
        }
        fout<<"end"<<endl;
    }
}

void sol::erase_net(const unordered_set<int>& in){
    // remove in arr;
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++)
            if(in.count(arr[i][j]))
                arr[i][j] = 0;
    }

    for(int i=1;i<netList.size();i++){
        arr[netList[i][1]][netList[i][0]] = i;
        arr[netList[i][3]][netList[i][2]] = i; 
    }

    // erase ans
    for(const auto& i : in){
        gridNum[i] = 0;
        ans[i].clear();
        if(!waitForR.count(i)){
            pri_netQ.push({netDist[i], i});
            waitForR.insert(i);
        }
    }
}
