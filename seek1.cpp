#include "hashing.hpp"
#include "intbtree.hpp"

int main(int argc, char **argv) {
    Bucket block;
    Hash hash_table('r');
    IntBTree btree('r');
    Artigo artigo;
    int id, index, count_blocks;
    long block_addr;
    bool found = false;

    if(argc < 2) {
        cout << "./seek1 <ID>\n";
        exit(1);
    }

    id = atoi(argv[1]);
    index = hash_table.hash_function(id);
    block_addr = btree.get_key_data(index, count_blocks);

    block.retrieve(hash_table.file_descriptor, block_addr);
    for (auto registro:block.artigos) {
        if(registro.id == 0) break;

        if(registro.id == id) {
            artigo = registro;
            found = true;
            break;
        }
    }

    if(found) {
        cout << "Id: " << artigo.id << endl;
        cout << "Título: " << artigo.titulo << endl;
        cout << "Autores: " << artigo.autores << endl;
        cout << "Ano de publicação: " << artigo.ano << endl;
        cout << "Número de citações: " << artigo.citacoes << endl;
        cout << "Data da última atualização: " << artigo.atualizacao << endl;
        cout << "Snippet: \"" << artigo.snippet << "\"."<< endl << endl;
    } else {
        cout << "Artigo não encontrado.\n\n";
    }

    cout << "Número de blocos lidos: " << count_blocks + 1 << endl;
    cout << "Número de blocos totais do arquivo: " << btree.get_file_size()/BLOCK_SIZE;
    cout << " (1 bloco de cabeçalho + " << btree.get_file_size()/BLOCK_SIZE -1 << " nós)." << endl;
}
