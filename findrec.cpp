#include "hashing.hpp"

int main(int argc, char **argv) {
    Hash r_hash('r');
    int count_blocks;
    int id;

    if(argc < 2) {
        cout << "./findrec <ID>\n";
        exit(1);
    }

    id = atoi(argv[1]);
    Artigo artigo = r_hash.retrieve_artigo(id, count_blocks);
    if(artigo.id != 0) {
        cout << "Id: " << artigo.id << endl;
        cout << "Título: " << artigo.titulo << endl;
        cout << "Autores: " << artigo.autores << endl;
        cout << "Ano de publicação: " << artigo.ano << endl;
        cout << "Número de citações: " << artigo.citacoes << endl;
        cout << "Data da última atualização: " << artigo.atualizacao << endl;
        cout << "Snippet: \"" << artigo.snippet << "\"."<< endl;
    } else {
        cout << "Artigo não encontrado.\n";
    }
}
