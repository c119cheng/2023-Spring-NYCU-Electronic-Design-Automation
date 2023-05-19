#ifndef FHB_H
#define FHB_H
#include <unordered_set>
using namespace std;

typedef struct cluster{
    int idx;
    int size;
    unordered_set<int> neighbor;
    vector<int> nets;
}vertex;

class FHB{
    vector<unordered_set<vertex>> V; //V[i] i is for differnt hypergraph
    vector<vector<unordered_set<vertex>> edges;
    public:
        FHB();
        ~FHB();
        void loadfile(char *);
}

#endif