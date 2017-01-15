#include "hashing.hpp"

void Hash::create_file() {
    char *zeros;
    long size = this->SIZE * BLOCK_SIZE;

    zeros = (char*) malloc(size);
    memset(zeros, 0, size);

    file_descriptor = fopen("hash_file.bin", "wb");

    if(!file_descriptor) {
        cout << "Erro ao abrir arquivo." << endl;
        return;
    }

    fwrite(zeros, 1, size, file_descriptor);
    cabecalho(file_descriptor);
    fclose(file_descriptor);
    file_descriptor = NULL;
}

void Hash::gera_enderecos(Cabecalho *registros) {
    Cabecalho aux;
    long addr_offset = TAM_CABECALHO;
    for (int i = 0; i < this->SIZE; i++) {
        aux.bucket_index = i;
        aux.block_addr = addr_offset;
        registros[i] = aux;
        addr_offset += BLOCK_SIZE;
    }
}

void Hash::cabecalho(FILE *file_descriptor) {
    Cabecalho *buckets;
    char bloco[SIZE];
    int fanout = SIZE/BLOCK_SIZE, i, j = 0;

    buckets = (Cabecalho*) malloc(sizeof(Cabecalho)*this->SIZE);
    gera_enderecos(buckets);


    for (i = 0; i < (this->SIZE); i += 341) {
        cout << "Inserindo bloco:" << j << endl;
        memset(bloco, 0, sizeof(char)*SIZE);
        memcpy(&bloco[0], &buckets[i], sizeof(Cabecalho)*341);
        fwrite(bloco, 1, BLOCK_SIZE, file_descriptor);
        cout << "bloco " << j << " inserido" << endl;
        j++;
    }

}

void Hash::find_bloco(int index) {
    Cabecalho *buckets, registro;
    Artigo artigo;
    char bloco[SIZE];
    int fanout = SIZE/BLOCK_SIZE, i,status;

    buckets = (Cabecalho*) malloc(sizeof(Cabecalho)*341);

    FILE *main_file = fopen("hash_file.bin", "rb");

    status = fread(bloco, 1, SIZE, main_file);

    if(status) {
        for (i = 0; i < fanout; i++) {
            memcpy(&registro, &bloco[sizeof(Cabecalho)*i], sizeof(Cabecalho));
            if (registro.bucket_index == index) {
                cout << "Endereço: "<< registro.block_addr << endl;
                fseek(main_file, registro.block_addr, SEEK_SET);
                status = fread(bloco, 1, BLOCK_SIZE, main_file);
                if(status) {
                    for (int j = 0; j < REG_PER_BLOCK; j++) {
                        memcpy(&artigo, &bloco[sizeof(Artigo)*j], sizeof(Artigo));
                        cout << artigo.id << endl;
                        return;
                    }
                }
            }
        }
    }
}


void Hash::store(Artigo registro) {
    int index = hash_function(registro.id);
    long addr = (index * BLOCK_SIZE) + TAM_CABECALHO;
    bool found = false;
    Bucket bucket;

    if(!file_descriptor) {
        file_descriptor = fopen("hash_file.bin", "rb+");

        if(!file_descriptor) {
            cout << "Erro ao abrir arquivo." << endl;
            return;
        }
    }

    bucket.retrieve(file_descriptor, addr);
    for (int i = 0; i < bucket.artigos.size(); i++) {
        if(bucket.artigos[i].id == 0) {
            bucket.artigos[i] = registro;
            found = true;
            break;
        }
    }

    if(!found) {
        bucket.store_overflow(registro, addr, file_descriptor);
    } else {
        bucket.overflow_list.block_addr = -1;
        bucket.overflow_list.offset = -1;
        bucket.store(file_descriptor, addr);
    }
}

void Hash::retrieve() {
    int status;
    int i = 0;
    long offset = TAM_CABECALHO;

    if(!file_descriptor) file_descriptor = fopen("hash_file.bin", "rb+");

    do {
        Bucket bucket;
        status = bucket.retrieve(file_descriptor, offset);
        offset += 4096;
        i += 1;
    } while(status);

    fclose(file_descriptor);
    file_descriptor = NULL;
}

// função para pesquisar no arquivo
Artigo Hash::retrieve_artigo(int id) {
    Bucket bucket;
    long int addr;
    int art_pos;
    int index = hash_function(id);

    addr = (index * BLOCK_SIZE) + TAM_CABECALHO;

    if(!file_descriptor) file_descriptor = fopen("hash_file.bin", "rb");

    bucket.retrieve(file_descriptor, addr);
    art_pos = bucket.find_artigo(id);

    if(art_pos == -1) {
        return retrieve_artigo_overflow(id, bucket);
    }

    return bucket.artigos[art_pos];
}

Artigo Hash::retrieve_artigo_overflow(int id, Bucket bucket) {

    FILE *overflow_file;
    char bloco[BLOCK_SIZE];
    Artigo registro;

    memset(&registro, 0, sizeof(Artigo));
    overflow_file = fopen("hash_overflow.bin", "rb");

    Artigo artigo;
    OverflowAddr of_addr;
    OverflowAddr next_addr = bucket.overflow_list;

    while(next_addr.block_addr != -1) {
        of_addr = next_addr;
        next_addr = bucket.find_next_addr(of_addr, overflow_file, bloco);
        memcpy(&artigo, &bloco[of_addr.offset], sizeof(Artigo));

        if(artigo.id == id) {
            registro = artigo;
            break;
        }
    }

    fclose(overflow_file);

    return registro;
}

void Hash::print() {
    int index;

    for (index = 0; index < this->SIZE; index++) {
        for (auto artigo:buckets[index].artigos) {
            cout << "id_artigo: " << artigo.id << " ";
        }
        cout << endl;
    }
}

int Bucket::retrieve(FILE *main_file, long offset) {
    Artigo registro;
    char bloco[BLOCK_SIZE];
    int i;

    fseek(main_file, offset, SEEK_SET);
    int status = fread(bloco, 1, BLOCK_SIZE, main_file);
    if(status) {
        for (i = 0; i < REG_PER_BLOCK; i++) {
            memcpy(&registro, &bloco[sizeof(Artigo)*i], sizeof(Artigo));

            artigos.push_back(registro);
        }
        memcpy(&overflow_list, &bloco[sizeof(Artigo)*i], sizeof(OverflowAddr));
    }

    return status;
}

void Bucket::store(FILE *main_file, long offset) {
    char bloco[BLOCK_SIZE];
    int count = 0;

    memset(bloco, 0, BLOCK_SIZE);
    // armazena os registros de um bloco em um vetor e escreve no arquivo principal
    for (auto artigo:artigos) {
        memcpy(&bloco[sizeof(Artigo)*count], &artigo, sizeof(Artigo));
        count += 1;
    }
    memcpy(&bloco[sizeof(Artigo)*count], &overflow_list, sizeof(OverflowAddr));

    fseek(main_file, offset, SEEK_SET);
    fwrite(bloco, BLOCK_SIZE, 1, main_file);
}

void Bucket::store_overflow(Artigo registro, long main_block_addr, FILE *main_file) {
    FILE *overflow_file = NULL;
    char bloco[BLOCK_SIZE];
    long file_size;
    int block_addr;
    bool found_free = false;
    OverflowAddr of_addr;

    overflow_file = fopen("hash_overflow.bin", "rb+");

    if(!overflow_file) {
        overflow_file = fopen("hash_overflow.bin", "wb+");
    }

    fseek(overflow_file, 0, SEEK_END);
    file_size = ftell(overflow_file);

    if(file_size > 0) block_addr = file_size - 4096;
    else block_addr = 0;

    if(file_size > 0) {
        fseek(overflow_file, block_addr, SEEK_SET);
        fread(bloco, 1, BLOCK_SIZE, overflow_file);

        int passo = sizeof(Artigo)+sizeof(OverflowAddr);
        //verifica se tem registro vazio
        for(int i = 0; i < BLOCK_SIZE-passo; i += passo) {
            Artigo artigo;
            OverflowAddr null_off_addr;
            memcpy(&artigo, &bloco[i], sizeof(Artigo));

            if(artigo.id == 0) {
                // reposiciona o ponteiro do arquivo para que o bloco seja reescrito
                fseek(overflow_file, -BLOCK_SIZE, SEEK_CUR);
                // escreve o novo registro na posição disponível
                memcpy(&bloco[i], &registro, sizeof(Artigo));
                // escreve o campo de endereco null
                memcpy(&bloco[i+sizeof(Artigo)], &null_off_addr, sizeof(OverflowAddr));
                //pega o endereco do registro adicionado
                of_addr.block_addr = block_addr;
                of_addr.offset = i;

                found_free = true;

                break;
            }
        }

        if(!found_free) {
            generate_new_overflow_block(registro, bloco);

            //pega o endereco do registro adicionado
            of_addr.block_addr = block_addr + BLOCK_SIZE;
            of_addr.offset = 0;
        }
    } else {

        generate_new_overflow_block(registro, bloco);
        of_addr.block_addr = 0;
        of_addr.offset = 0;
    }

    fwrite(bloco, 1, BLOCK_SIZE, overflow_file);

    update_overflow_list(of_addr, overflow_file, main_file, main_block_addr);

    fclose(overflow_file);
}

void Bucket::generate_new_overflow_block(Artigo registro, char bloco[]) {
    OverflowAddr null_off_addr;

    memset(bloco, 0, BLOCK_SIZE);
    // escreve o novo registro no início do bloco
    memcpy(&bloco[0], &registro, sizeof(Artigo));
    // escreve o campo de endereco null
    memcpy(&bloco[sizeof(Artigo)], &null_off_addr, sizeof(OverflowAddr));
}

void Bucket::update_overflow_list(OverflowAddr new_reg_addr, FILE *overflow_file, FILE *main_file, long main_block_addr) {
    char bloco[BLOCK_SIZE];

    // faz o update da lista de overflow
    if(overflow_list.block_addr == -1) {
        overflow_list = new_reg_addr;
        store(main_file, main_block_addr);
    } else {
        OverflowAddr of_addr;
        OverflowAddr next_addr = overflow_list;
        while(next_addr.block_addr != -1) {
            of_addr = next_addr;
            next_addr = find_next_addr(of_addr, overflow_file, bloco);
        }

        // reposiciona o fseek para reescrever o bloco
        fseek(overflow_file, of_addr.block_addr, SEEK_SET);

        memcpy(&bloco[of_addr.offset+sizeof(Artigo)], &new_reg_addr, sizeof(OverflowAddr));

        fwrite(bloco, 1, BLOCK_SIZE, overflow_file);
    }
}

OverflowAddr Bucket::find_next_addr(OverflowAddr actual_addr, FILE *overflow_descriptor, char bloco[]) {
    OverflowAddr next_addr;

    // posiciona o ponteiro do arquivo no endereco do bloco
    fseek(overflow_descriptor, actual_addr.block_addr, SEEK_SET);
    // lê o bloco
    fread(bloco, 1, BLOCK_SIZE, overflow_descriptor);

    // usa o offset para recuperar o endereço do próximo registro
    memcpy(&next_addr, &bloco[actual_addr.offset+sizeof(Artigo)], sizeof(OverflowAddr));

    return next_addr;
}

int Bucket::find_artigo(int id) {
    for (int i = 0; i < artigos.size(); i++) {
        if(artigos[i].id == id) return i;
    }
    return -1;
}
