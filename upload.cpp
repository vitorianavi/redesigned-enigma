#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
//#include <boost/algorithm/string.hpp>

#define HASH_SIZE 131101
#define BLOCK_SIZE 4096
#define BLOCKS_PER_BUCKET 1

using namespace std;

class Artigo {
public:
    int id;
    int ano;
    int citacoes;
    char titulo[296];
    char autores[992];
    char atualizacao[20];
    char snippet[534];
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
    int index, i;
    char bloco[BLOCK_SIZE];

    hash_file = fopen(filename, "wb");
    overflow_file = fopen("hash_overflow.bin", "wb");

    if(!hash_file || !overflow_file) {
        cout << "Erro ao abrir arquivo\n";
        return;
    }

    for (index = 0; index < HASH_SIZE; index++) {
        memset(bloco, 0, BLOCK_SIZE);

        // armazena os registros de um bloco em um vetor e escreve no arquivo principal
        for (i = 0; i < hash_table[index].artigos.size(); i++) {
            if(i > 2) break;
            memcpy(&bloco[sizeof(Artigo)*i], &hash_table[index].artigos[i], sizeof(Artigo));
            cout << hash_table[index].artigos[i].id << endl;
        }

        cout << index << " in " << ftell(hash_file) << endl;
        fwrite(bloco, BLOCK_SIZE, 1, hash_file);

        // armazena os registros restantes em blocos e escreve no arquivo de overflow
        memset(bloco, 0, BLOCK_SIZE);
        int cont = 0;
        for (i = 3; i < hash_table[index].artigos.size(); i++) {
            if(cont > 2) {
                cont = 0;
                fwrite(bloco, 1, BLOCK_SIZE, overflow_file);
                memset(bloco, 0, BLOCK_SIZE);
            }
            memcpy(&bloco[sizeof(Artigo)*cont], &hash_table[index].artigos[i], sizeof(Artigo));
            cont += 1;
        }

    }

    fclose(overflow_file);
    fclose(hash_file);
}

void retrieve_hash(const char *filename) {
    FILE *hash_file, *overflow_file;
    int i;
    char bloco[BLOCK_SIZE];
    Artigo registros[3];

    hash_file = fopen(filename, "rb");
    overflow_file = fopen("hash_overflow.bin", "rb");

    int status = fread(bloco, 1, BLOCK_SIZE, hash_file);
    while(status) {
        for (i = 0; i < 3; i++) {
            memcpy(&registros[i], &bloco[sizeof(Artigo)*i], sizeof(Artigo));
            cout << registros[i].id << "\n";
        }

        cout << ftell(hash_file) << endl;
        status = fread(bloco, 1, BLOCK_SIZE, hash_file);
    }

    fclose(overflow_file);
    fclose(hash_file);
}

// função para pesquisar no arquivo (2A PARTE DO TRAB SEPARAR ISSO DEPOIS)
void find_hash_file(int id, const char *filename) {
    FILE *hash_file;
    long int addr;
    int i, found=0;
    char bloco[BLOCK_SIZE];
    vector<Artigo> registros(3);
    int index = hash_function(id);

    addr = index*BLOCK_SIZE*BLOCKS_PER_BUCKET;
    cout << addr << endl;

    hash_file = fopen(filename, "rb");
    fseek(hash_file, addr, SEEK_CUR);
    cout << ftell(hash_file) << endl;
    fread(bloco, 1, BLOCK_SIZE, hash_file);
    for (i = 0; i < 3; i++) {
        memcpy(&registros[i], &bloco[sizeof(Artigo)*i], sizeof(Artigo));
        cout << registros[i].id << endl;
    }

    for (auto registro:registros) {
        if(registro.id == id) {
            found = 1;
            cout << "Registro encontrado em " << index << ".";
            break;
        }
    }

    if(!found) {
        // procurar no arquivo de overflow
    }
}

void normalize(string& str) {
    str.erase(remove_if(str.begin(), str.end(), [](char c) {
        return !isspace(c) && !isalpha(c) && c != '|'; }), str.end());
}

bool is_number(const char str[]) {
    if(str[0] != '-' && !isdigit(str[0])) {
        return false;
    }

    int len = strlen(str);
    for (int i = 1; i < len; i++) {
        if(!isdigit(str[i])) {
            return false;
        }
    }

    return true;
}

int parser(const char filename[]) {
    Artigo artigo;
    int n_tokens;
    string buffer, aux_buffer;
    vector<string> tokens;

    ifstream file(filename);

    if(!file.is_open()) {
        cout << "Falha ao abrir o arquivo " << filename << ".\n";
        return 0;
    }

    int biggest = 0;
    string bigst_str, bigst_id;
    buffer = "";
    while(getline(file, aux_buffer)) {
        // "buffer" não estará vazio se a linha lida anteriormente estiver "quebrada"
        buffer = buffer + aux_buffer;
        boost::algorithm::split_regex(tokens, buffer, boost::regex("\";\"|\";;\"|\";"));

        n_tokens = tokens.size();
        if(n_tokens > 5) {
            // linha completa
            // campos que podem não existir: "título"[1] ou "autores"[3]
            int next_pos;
            buffer = "";

            // apaga as aspas do início do campo ID
            tokens[0].erase(0, 1);
            // apaga as aspas do último campo (se houver)
            size_t pos = tokens[n_tokens-1].find("\"");
            if(pos != std::string::npos)
                tokens[n_tokens-1].erase(pos);

            artigo.id = stoi(tokens[0]);
            // verifica se o token do campo "ano" é um número, caso contrário, o campos "título" está vazio (não há token para ele)
            if(is_number(tokens[2].c_str())) {
                artigo.ano = stoi(tokens[2]);
                normalize(tokens[1]);
                strcpy(artigo.titulo, tokens[1].c_str());
                next_pos = 3;
            } else {
                // não há título
                artigo.ano = stoi(tokens[1]);
                artigo.titulo[0] = '\0';
                next_pos = 2;
            }

            // verifica se o token do campo "citações" é um número, caso contrário, o campos "autores" está vazio (não há token para ele)
            if(is_number(tokens[next_pos+1].c_str())) {
                artigo.citacoes = stoi(tokens[next_pos+1]);
                normalize(tokens[next_pos]);
                strcpy(artigo.autores, tokens[next_pos].c_str());
                next_pos += 2;
            } else {
                // não há autores
                artigo.citacoes = stoi(tokens[next_pos]);
                artigo.autores[0] = '\0';
                next_pos += 1;
            }

            cout << artigo.id << " " << artigo.autores << endl;

            strcpy(artigo.atualizacao, tokens[next_pos].c_str());
            normalize(tokens[next_pos+1]);
            strcpy(artigo.snippet, tokens[next_pos+1].c_str());

            if (strlen(artigo.autores) > biggest) {
                biggest = strlen(artigo.autores);
                bigst_str = artigo.autores;
                bigst_id = tokens[0];
            }

        } else {
            // linha quebrada
            cout << tokens.size() << " ";
            for (auto token:tokens) {
                cout << "[" << token << "] ";
            }
            cout << endl;
        }
    }

    cout << biggest << " " << bigst_id << endl;
    cout << bigst_str << endl;

    file.close();

    return 1;
}

int main() {
    int status;

    hash_table = (Bucket*) malloc(sizeof(Bucket)*HASH_SIZE);

    status = parser("artigo.csv");
    //print_hash();
//    store_hash("hash_file.bin");
//    retrieve_hash("hash_file.bin");
//    find_hash_file(4, "artigo.csv");

    cout << sizeof(Artigo);
}
