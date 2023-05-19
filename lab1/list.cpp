#include "list.h"
#include <iostream>
using namespace std;
list::list(){
    this->head = nullptr;
    this->tail = nullptr;
}

list::~list(){
    node* cur = head;
    while(cur){
        cur = cur->next;
        delete head;
        head = cur;
    }
}

void list::append(int in){
    if(head){
        tail->next = new node;
        tail = tail->next;
        tail->val = in;
    }
    else{
        head = new node;
        head->val = in;
        tail = head;
    }
}

node* list::getHead(){
    return head;
}

void list::append(node* in){
    if(head){
        tail->next = in;
        tail = tail->next;
    }
    else{
        head = in;
        tail = in;
    }
}

node* list::remove(int in){
    if(head->val == in){
        node *cur = head;
        this->head = head->next;
        cur->next = nullptr;
        return cur;
    }
    node* prev = head;
    node* cur = head->next;
    while(cur->val != in){
        prev = cur;
        cur = cur->next;
        if(!cur){
            cout<<"something wrong"<<endl;
        }
    }
    prev->next = cur->next;
    cur->next = nullptr;
    return cur;
}

void list::append_no_duplicate(int in){
    if(!head){
        head = new node;
        head->val = in;
        tail = head;
    }
    node *cur = head;
    while(cur){
        if(cur->val == in)
            return ;
        cur = cur->next;
    }
    tail->next = new node;
    tail = tail->next;
    tail->val = in;
}