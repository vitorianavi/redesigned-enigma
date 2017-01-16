#ifndef __LIB_HASHING__
#define __LIB_HASHING__

#include "general.hpp"
#include <vector>
#include <string.h>

struct OverflowAddr {
    long block_addr = -1;
    int offset = -1;
};

struct HeaderAddr {
    long block_addr;
    int bucket_index;
};

class Bucket {
public:
    vector<Artigo> artigos;
    OverflowAddr overflow_list;

    int retrieve(FILE *main_file, long offset);
    void store(FILE *main_file, long offset);
    void store_overflow(Artigo registro, long block_addr, FILE *main_file);
    OverflowAddr find_next_addr(OverflowAddr actual_addr, FILE *overflow_descriptor, char bloco[]);
    int find_artigo(int id);

private:
    void update_overflow_list(OverflowAddr reg_addr, FILE *overflow_file, FILE *main_file, long block_addr);
    void generate_new_overflow_block(Artigo registro, char bloco[]);
};

class Hash {
public:
    int size = 170231;
    int header_size = size*sizeof(HeaderAddr);
    HeaderAddr *addr_map;
    vector<Bucket> buckets;
    Bucket bucket_overflow;
    FILE *file_descriptor = NULL;
    char op_mode;

    Hash(char op_mode) {
        this->op_mode = op_mode;
        addr_map = (HeaderAddr*) malloc(sizeof(HeaderAddr)*size);

        switch(op_mode) {
            case 'w':
                create_file();
                break;
            case 'r':
                file_descriptor = fopen("hash_file.bin", "rb");
                gen_addresses();
                break;
            default:
                cout << "Modo de operação inválido!" << endl;
                exit(1);
                break;
        }
    }

    ~Hash() {
        free(addr_map);
        if(file_descriptor) fclose(file_descriptor);
    }

    inline int hash_function(int id) {
        int index = id % size;

        return index;
    }

    void create_file();
    void gen_addresses();
//    void store_header();
//    void load_header();
    void store(Artigo artigo);
    void retrieve();
    Artigo retrieve_artigo(int id, int& count_blocks);
    void print();

private:
    Artigo retrieve_artigo_overflow(int id, Bucket bucket, int& count_blocks);
};

#endif
