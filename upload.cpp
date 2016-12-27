#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

#define HASH_SIZE 262151

using namespace std;

class Artigo {
public:
    int id;
    char titulo[250];
    int ano;
    char autores[50];
    int citacoes;
    char atualizacao[20];
    char snippet[1024];
};

class Bucket {
public:
    vector<Artigo> artigos;
};

Bucket *hash_table;

int hash_function(int id) {
    int index = id % HASH_SIZE;

    return index;
}

void insert_hash(Bucket *hash_table, Artigo artigo) {
    int index = hash_function(artigo.id);


    cout << "Inserting " << artigo.id << " in " << index << ".\n";
    hash_table[index].artigos.push_back(artigo);
}

int parser(const char filename[]) {
    FILE *arquivo;
    Artigo artigo;

    arquivo = fopen(filename, "r");
    if(!arquivo) {
        cout << "Falha ao abrir o arquivo " << filename << ".\n";
        return 0;
    }

    int read = fscanf(arquivo, "\"%d\";", &artigo.id);
    while(read > 0) {
        fscanf(arquivo, "\"%[^\"]\";\"%d\";\"%[^\"]\";", artigo.titulo, &artigo.ano, artigo.autores);
        fscanf(arquivo, "\"%d\";\"%[^\"]\";", &artigo.citacoes, artigo.atualizacao);
        read = fscanf(arquivo, "\"%[^\"]\"\n", artigo.snippet);
        if(read < 1) {
            fscanf(arquivo, "%[^\n]\n", artigo.snippet);
        }

        insert_hash(hash_table, artigo); //populando hash
        //artigos.push_back(artigo);

        printf("id: %d\n", artigo.id);
        //printf("ID: %d TITULO: %s ANO: %d AUTORES: %s\n", artigo.id, artigo.titulo, artigo.ano, artigo.autores);
        //printf("CITACOES: %d ATUALIZACAO: %s SNIPPET: %s\n", artigo.citacoes, artigo.atualizacao, artigo.snippet);

        read = fscanf(arquivo, "\"%d\";", &artigo.id);
    }

    return 1;
}

int main() {
    vector<Artigo> artigos;
    int status;

    hash_table = (Bucket*) malloc(sizeof(Bucket)*HASH_SIZE);

    status = parser("artigo.csv");
    cout << sizeof(Artigo);
}
