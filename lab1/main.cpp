#include "FM.h"
#include <iostream>
using namespace std;
int main(int argc, char **argv){
    FM t = FM(argv[1]);
    t.sol();
    return 0;
}