#include "FM.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
using namespace std;
int main(int argc, char **argv){
    FM t = FM(argv[1]);
    // #pragma omp parallel for num_threads(4)
    t.sol();
    return 0;
}