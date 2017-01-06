#ifndef __LIB_GENERAL__
#define __LIB_GENERAL__

#include <iostream>
#include <fstream>

#define BLOCK_SIZE 4096
#define REG_PER_BLOCK 7

using namespace std;

class Artigo {
public:
    int id;
    int ano;
    int citacoes;
    char titulo[300];
    char autores[100];
    char atualizacao[20];
    char snippet[100];
};

#endif
