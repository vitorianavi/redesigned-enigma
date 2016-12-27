#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define HASH_SIZE 131101

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

void print_hash() {
    int index;

    for (index = 0; index < HASH_SIZE; index++) {
        for (auto artigo:hash_table[index].artigos) {
            cout << artigo.id << " ";
        }
        cout << endl;
    }
}

void store_hash(const char *filename) {
    FILE *hash_file, *overflow_file;
    int index, i, cont;
    Artigo bloco[4];

    hash_file = fopen(filename, "wb");
    overflow_file = fopen("hash_overflow.bin", "wb");

    if(!hash_file || !overflow_file) {
        cout << "Erro ao abrir arquivo\n";
    }

    for (index = 0; index < HASH_SIZE; index++) {
        memset(bloco, 0, 4*sizeof(Artigo));

        // armazena os registros de um bloco em um vetor e escreve no arquivo principal
        for (i = 0; i < hash_table[index].artigos.size(); i++) {
            if(i > 2) break;
            bloco[i] = hash_table[index].artigos[i];
        //    cout << bloco[i].id << endl;
        }

        int status = fwrite(bloco, sizeof(Artigo), 4, hash_file);
    //    cout << status << endl;

        // armazena os registros restantes em blocos e escreve no arquivo de overflow
        memset(bloco, 0, 4*sizeof(Artigo));
        cont = 0;
        for (; i < hash_table[index].artigos.size(); i++) {
            if(cont > 2) {
                cont = 0;
                fwrite(bloco, sizeof(Artigo), 4, overflow_file);
                memset(bloco, 0, 4*sizeof(Artigo));
            }
            bloco[cont] = hash_table[index].artigos[i];
            cont += 1;
        }

    }

    fclose(hash_file);
    fclose(overflow_file);
}

void retrieve_hash(const char *filename) {
    FILE *hash_file, *overflow_file;
    int index, i, cont, status;
    Artigo bloco[4];

    hash_file = fopen(filename, "wb");
    overflow_file = fopen("hash_overflow.bin", "wb");

    status = fread(bloco, 4096, 1, hash_file);
    cout << bloco[0].id << endl;
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
    //print_hash();
    store_hash("hash_file.bin");
    retrieve_hash("hash_file.bin");

    cout << sizeof(Artigo);
}
