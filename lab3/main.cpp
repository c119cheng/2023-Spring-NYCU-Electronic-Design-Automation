#include "ROBDD.h"

int main(int argc, char **argv){
    ROBDD a(argv[1]);
    a.sol();
    a.output(argv[2]);
    return 0;
}