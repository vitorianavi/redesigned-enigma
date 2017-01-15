#ifndef __LIB_BTREE__
#define __LIB_BTREE__

#include "general.hpp"

// #define ORDER 204
#define ORDER 204

struct Node {
    int count_keys; // quantidade de chaves atualmente armazenadas no nó
    int keys[ORDER]; // chaves
    long data_pointers[ORDER]; // offsets dos registros ancora no arquivo de dados
    long node_pointers[ORDER+1]; // offsets dos nós (blocos) filhos no arquivo de índice
};

class BTree {
public:
    const int MAX_KEYS = ORDER;
    long root_addr;
    char op_mode;
    FILE *index_file = NULL;
    FILE *data_file = NULL;

    BTree(const char *data_file_name, char op_mode) {
        this->op_mode = op_mode;
        data_file = fopen(data_file_name, "rb");
        root_addr = -1;
    }

    ~BTree() {
        fclose(data_file);
        if(index_file) fclose(index_file);
    }

    void create_index();
    void print();
    void insert(int key, long data_addr);
    long get_key_data(int key);

private:
    long recursive_search(int key, long curr_node_addr);
    bool recursive_insert(long curr_node_addr, int key, long data_offset, int& up_key, long& up_data_offset, long& right_child_addr);
    void split(Node node, int &new_key, long &new_data_addr, long &new_key_right, long curr_node_addr);
    void print_recursive(long curr_node_addr);
    void add_key(Node& node, int key, long data_offset, long right_child_addr, int position);
    bool find_key(Node node, int key, int &index);
    Node get_node(long node_addr);
    long write_node(Node node, long addr=-1);
    Node create_empty_node();
};

#endif
