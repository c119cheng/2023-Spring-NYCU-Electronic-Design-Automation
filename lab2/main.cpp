#include "MR.h"
int main(int argc, char **argv){
    sol A(argv[1], argv[2]);
    A.solve();
    return 0;
}