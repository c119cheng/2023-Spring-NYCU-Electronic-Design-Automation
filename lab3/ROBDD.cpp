#include "ROBDD.h"

ROBDD::ROBDD(char *filename){
    // read function
    string line;
    ifstream fin(filename);
    getline(fin, line);
    for(int i=0;i<line.length()-1;i++){
        vector<char> tmp;
        while(line[i] != '+'){
            if(line[i] == '.')
                break;
            tmp.push_back(line[i]);
            i++;
        }
        func.push_back(tmp);
    }

    // read orders
    while(getline(fin, line)){
        orders.push_back(line);
    }
}

ROBDD::~ROBDD(){

}

void write_dd(DdManager *gbm, DdNode *dd, string filename){
    FILE *outfile;
    outfile = fopen(filename.c_str(), "w");
    DdNode **ddnodearray = (DdNode**)malloc(sizeof(DdNode*));
    ddnodearray[0] = dd;
    Cudd_DumpDot(gbm, 1, ddnodearray, NULL, NULL, outfile);
    free(ddnodearray);
    fclose(outfile);
}

void ROBDD::show(){
    // show function
    for(auto literal : func){
        for(auto c : literal){
            cout<<c;
        }
        cout<<"+";
    }
    cout<<endl;

    // show order
    for(auto order : orders)
        cout<<order<<endl;
}



void ROBDD::sol(){
    best = INT_MAX;
    DdManager *gbm;
    gbm = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
    Cudd_AutodynDisable(gbm);
    DdNode *bdd, *var, *tmp;

    int pre_order[26];
    for(auto order : orders){
        gbm = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
        // Cudd_AutodynDisable(gbm);
        bdd = Cudd_ReadLogicZero(gbm);
        Cudd_Ref(bdd);

        int idx=0;
        for(auto c : order){
            pre_order[c - 'a'] = idx;
            idx++;
        }
        for(auto literal : func){
            
            DdNode *l = Cudd_ReadOne(gbm);
            Cudd_Ref(l);
            for(auto v : literal){
                if(v <= 'Z' && v >='A'){
                    // uppercase
                    var = Cudd_bddIthVar(gbm, pre_order[int(v)-int('A')]);
                    var = Cudd_Not(var);
                }
                else{
                    // lower case
                    var = Cudd_bddIthVar(gbm, pre_order[int(v)-int('a')]);    
                }
                tmp = Cudd_bddAnd(gbm, var, l);
                Cudd_Ref(tmp);
                Cudd_RecursiveDeref(gbm, l);
                l = tmp;
            }

            tmp = Cudd_bddOr(gbm, l, bdd);
            Cudd_Ref(tmp);
            Cudd_RecursiveDeref(gbm, bdd);
            bdd = tmp;
        }

        tmp = Cudd_BddToAdd(gbm, bdd);
        int size = Cudd_DagSize(tmp);
        if(size < best)
            best = size;
        Cudd_Quit(gbm);
    }
}

void ROBDD::output(char *filename){
    ofstream fout(filename);
    fout << best;
}