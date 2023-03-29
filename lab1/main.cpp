#include "FM.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
using namespace std;
int main(int argc, char **argv){
    FM t = FM(argv[1]);
    FM a = FM("input2.txt.hgr");
    FM b = FM("input4.txt.hgr");
    ofstream fout("ftest.txt");
    FILE *output = fopen("test.txt", "w");
    srand(time(NULL));
    #pragma omp parallel for num_threads(4)
    for(int i=0;i<10000;i++){
        FM t = FM(argv[1]);
        int s = rand()%100000;
        t.s = s;
        t.sol();
        int cut = t.cut;
        fout<<"s = "<<s<<", 3:"<<t.cut<<endl;
        fprintf(output, "s = %d,  cut : %d\n",s,t.cut);
    }
    // t.sol();
    return 0;
}