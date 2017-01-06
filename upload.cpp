#include "general.hpp"
#include "hashing.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

Hash hash_table;

void normalize(string& str) {
    str.erase(remove_if(str.begin(), str.end(), [](char c) {
        return !isspace(c) && !isalpha(c) && c != '|' && c != '-'; }), str.end());
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

            hash_table.insert(artigo);
            count += 1;
        }
    }

    file.close();

    return count;
}

int main() {
    int records = parser("artigo.csv");
    hash_table.store("hash_file.bin");

    cout << "Registros: " << records << endl;
//    hash_table.retrieve("hash_file.bin");
    //Artigo artigo = hash_table.retrieve_artigo(4, "hash_file.bin");
    //cout << artigo.id << endl;
    //cout << artigo.titulo << endl;
}
