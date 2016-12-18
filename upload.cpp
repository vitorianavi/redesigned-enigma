#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

class Artigo {
public:
    int id;
    char titulo[300];
    int ano;
    char autores[1024];
    int citacoes;
    char atualizacao[20];
    char snippet[1024];
};

int parser(const char filename[], vector<Artigo>& artigos) {
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

        artigos.push_back(artigo);
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

    status = parser("artigo.csv", artigos);
}
