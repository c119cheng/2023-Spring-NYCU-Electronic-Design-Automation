#ifndef LIST_H
#define LIST_H

struct node{
    int val;
    node *next;  
};

class list{
    private:
        node* head;
        node* tail;

    public:
        list();
        ~list();
        void append(int);
        node* getHead();
        node* remove(int);
        void append(node*);
        void append_no_duplicate(int);
};
/*
struct node_d{
    int val;
    node_d *prev;
    node_d *next;
}

class double_list{
    private:
        node* head;
        node* tail;

    public:
        list();
        ~list();
        void append(int);
        node* getHead();
        node* remove(int);
        void append(node*);
}
*/

#endif