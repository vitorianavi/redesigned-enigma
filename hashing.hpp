#ifndef __LIB_HASHING__
#define __LIB_HASHING__

#include "general.hpp"
#include <vector>
#include <string.h>

struct OverflowAddr {
    long block_addr = -1;
    int offset = -1;
};

struct Cabecalho{
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
    //const int SIZE = 145903; 1/7
    //const int SIZE = 127679; 1/8
    const int SIZE = 85121;
    const int TAM_CABECALHO = SIZE*12;
    vector<Bucket> buckets;
    Bucket bucket_overflow;
    FILE *file_descriptor = NULL;

    Hash() {

    }

    ~Hash() {
        if(file_descriptor) fclose(file_descriptor);
    }

    inline int hash_function(int id) {
        int index = id % SIZE;

        return index;
    }

    void create_file();
    void gera_enderecos(Cabecalho *registros);
    void cabecalho(FILE *file_descriptor);
    void find_bloco(int index);
    void store(Artigo artigo);
    void retrieve();
    Artigo retrieve_artigo(int id);
    void print();

private:
    Artigo retrieve_artigo_overflow(int id, Bucket bucket);
};

#endif
