#include "hashing.hpp"

void Hash::insert(Artigo artigo) {
    int index = hash_function(artigo.id);

    buckets[index].artigos.push_back(artigo);
}

void Hash::store(const char *filename) {
    FILE *hash_file;

    hash_file = fopen(filename, "wb");

    if(!hash_file) {
        cout << "Erro ao abrir arquivo " << filename << endl;
        return;
    }

    for (int i = 0; i < this->SIZE; i++) {
        buckets[i].store(hash_file);
        buckets[i].artigos.clear();
    }

    fclose(hash_file);
}

void Hash::retrieve(const char *filename) {
    FILE *hash_file;
    int status;
    int i = 0;
    long offset = 0L;

    hash_file = fopen(filename, "rb");

    do {
        Bucket bucket;
        status = buckets[i].retrieve(hash_file, offset);
        offset += 4096;
        i += 1;
    } while(status);

    fclose(hash_file);
}

// função para pesquisar no arquivo
Artigo Hash::retrieve_artigo(int id, const char *filename) {
    FILE *hash_file;
    Bucket bucket;
    long int addr;
    int art_pos;
    int index = hash_function(id);

    addr = index * BLOCK_SIZE;

    hash_file = fopen(filename, "rb");

    bucket.retrieve(hash_file, addr);
    art_pos = bucket.find_artigo(id);

    if(art_pos == -1) {
        // procurar no arquivo de overflow
        return retrieve_artigo_overflow(id);
    }

    return bucket.artigos[art_pos];
}

Artigo Hash::retrieve_artigo_overflow(int id) {
    FILE *overflow_file;
    char bloco[BLOCK_SIZE];
    Artigo registro;

    overflow_file = fopen("hash_overflow.bin", "rb");

    int status = fread(bloco, 1, BLOCK_SIZE, overflow_file);
    if(status) {
        for (int i = 0; i < REG_PER_BLOCK; i++) {
            memcpy(&registro, &bloco[sizeof(Artigo)*i], sizeof(Artigo));

            if(registro.id == id) return registro;
        }

        status = fread(bloco, 1, BLOCK_SIZE, overflow_file);
    }

    fclose(overflow_file);

    //não encontrou artigo
    memset(&registro, 0, sizeof(Artigo));
    return registro;
}

void Hash::print() {
    int index;

    for (index = 0; index < this->SIZE; index++) {
        for (auto artigo:buckets[index].artigos) {
            cout << artigo.id << " ";
        }
        cout << endl;
    }
}

int Bucket::retrieve(FILE *main_file, long offset) {
    Artigo registro;
    char bloco[BLOCK_SIZE];

    fseek(main_file, offset, SEEK_SET);
    int status = fread(bloco, 1, BLOCK_SIZE, main_file);
    if(status) {
        for (int i = 0; i < REG_PER_BLOCK; i++) {
            memcpy(&registro, &bloco[sizeof(Artigo)*i], sizeof(Artigo));
            artigos.push_back(registro);
        }
    }

    return status;
}

void Bucket::store(FILE *main_file) {
    char bloco[BLOCK_SIZE];
    int count = 0;

    memset(bloco, 0, BLOCK_SIZE);
    // armazena os registros de um bloco em um vetor e escreve no arquivo principal
    for (auto artigo:artigos) {
        if(count == REG_PER_BLOCK)  {
            store_overflow();
            break;
        }
        memcpy(&bloco[sizeof(Artigo)*count], &artigo, sizeof(Artigo));
        count += 1;
    }

    fwrite(bloco, BLOCK_SIZE, 1, main_file);
}

void Bucket::store_overflow() {
    FILE *overflow_file;
    char bloco[BLOCK_SIZE];
    int count = 0;

    overflow_file = fopen("hash_overflow.bin", "ab");

    // armazena os registros restantes em blocos e escreve no arquivo de overflow
    memset(bloco, 0, BLOCK_SIZE);
    for (int i = REG_PER_BLOCK; i < artigos.size(); i++) {
        if(count >= REG_PER_BLOCK) {
            count = 0;
            fwrite(bloco, 1, BLOCK_SIZE, overflow_file);
            memset(bloco, 0, BLOCK_SIZE);
        }
        memcpy(&bloco[sizeof(Artigo)*count], &artigos[i], sizeof(Artigo));
        count += 1;
    }

    fclose(overflow_file);
}

int Bucket::find_artigo(int id) {
    for (int i = 0; i < artigos.size(); i++) {
        if(artigos[i].id == id) return i;
    }
    return -1;
}
