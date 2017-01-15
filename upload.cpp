#include "hashing.hpp"
#include "intbtree.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

Hash hash_table('w');

void normalize(string& str) {
    str.erase(remove_if(str.begin(), str.end(), [](char c) {
        return !isprint(c); }), str.end());
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
    int count = 0;

    ifstream file(filename);

    if(!file.is_open()) {
        cout << "Falha ao abrir o arquivo " << filename << ".\n";
        return 0;
    }

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
                memcpy(artigo.autores, tokens[next_pos].c_str(), 99);
                artigo.autores[99] = '\0';
                next_pos += 2;
            } else {
                // não há autores
                artigo.citacoes = stoi(tokens[next_pos]);
                artigo.autores[0] = '\0';
                next_pos += 1;
            }

            strcpy(artigo.atualizacao, tokens[next_pos].c_str());
            normalize(tokens[next_pos+1]);
            memcpy(artigo.snippet, tokens[next_pos+1].c_str(), 99);
            artigo.snippet[99] = '\0';

            hash_table.store(artigo);
            count += 1;
        }
    }

    file.close();

    return count;
}

void gen_primary_index(vector<HeaderAddr> addrs) {
    IntBTree btree('w');
    for (auto addr:addrs) {
        btree.insert(addr.bucket_index, addr.block_addr);
    }
}

int main(int argc, char **argv) {
    if(argc < 2) {
        cout << "./upload <arquivo de entrada>" << endl;
        exit(1);
    }

    cout << "Creating data file...\n";
    int records = parser(argv[1]);
    cout << "Number of records: " << records << endl;

    cout << "Building primary index...\n";
    gen_primary_index(hash_table.addr_map);
}
