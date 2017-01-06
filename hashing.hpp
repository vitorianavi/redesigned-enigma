#ifndef __LIB_HASHING__
#define __LIB_HASHING__

#include "general.hpp"
#include <vector>
#include <string.h>

class Bucket {
public:
    vector<Artigo> artigos;

    int retrieve(FILE *main_file, long offset=0L);
    void store(FILE *main_file);
    int find_artigo(int id);

private:
    void store_overflow();
};

class Hash {
public:
    const int SIZE = 145903;
    vector<Bucket> buckets;

    Hash() {
        buckets.reserve(SIZE);
    }

    inline int hash_function(int id) {
        int index = id % SIZE;

        return index;
    }

    void insert(Artigo artigo);
    void store(const char *filename);
    void retrieve(const char *filename);
    Artigo retrieve_artigo(int id, const char *filename);
    void print();

private:
    Artigo retrieve_artigo_overflow(int id);
};

#endif
