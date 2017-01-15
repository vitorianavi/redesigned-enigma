#include "intbtree.hpp"

void IntBTree::print() {
    if(!index_file) {
        index_file = fopen("primary_index_file.bin", "rb");
        if(!index_file) {
            printf("Arquivo de índice não existente.\n");
            return;
        }
    }

    print_recursive(root_addr);
}

// DFS para imprimir nós da árvore
void IntBTree::print_recursive(long curr_node_addr) {
    if(curr_node_addr == -1) return;

    int i;
    Node node = get_node(curr_node_addr);

    cout << "qtd chaves: " << node.count_keys << endl;
    cout << "keys: " << endl;
    for (i = 0; i < node.count_keys; i++) {
        cout << node.keys[i] << "|" << node.data_pointers[i] << endl;
    }

    cout << "pointer: ";
    for (i = 0; i <= node.count_keys; i++) {
        cout << node.node_pointers[i] << endl;
        print_recursive(node.node_pointers[i]);
    }

    cout << endl;
}

void IntBTree::create_index() {
    if(op_mode == 'w') index_file = fopen("primary_index_file.bin", "wb+");
    else if(op_mode == 'r') index_file = fopen("primary_index_file.bin", "rb");
    else exit(1);

    Node node;
    node = create_empty_node();
    this->root_addr = write_node(node);
}

void IntBTree::insert(int key, long data_addr) {
    bool has_new_root;
    int new_key;
    long key_data_offset, right_child_addr;
    Node node;

    has_new_root = recursive_insert(this->root_addr, key, data_addr, new_key, key_data_offset, right_child_addr);

    if(has_new_root) {
        node.count_keys = 1;
        node.keys[0] = new_key;
        node.data_pointers[0] = key_data_offset;
        node.node_pointers[0] = root_addr;
        node.node_pointers[1] = right_child_addr;
        this->root_addr = write_node(node);
    }
}

bool IntBTree::recursive_insert(long curr_node_addr, int key, long data_offset, int& up_key, long& up_data_offset, long& right_child_addr) {
    if(curr_node_addr == -1) {
        up_key = key;
        up_data_offset = data_offset;
        right_child_addr = -1;

        return true;
    }

    int position;
    bool has_new_key;
    Node node = get_node(curr_node_addr);

    if(find_key(node, key, position)) {
        cout << "chave duplicada \n";
        exit(1);
    }

    has_new_key = recursive_insert(node.node_pointers[position], key, data_offset, up_key, up_data_offset, right_child_addr);

    if(has_new_key) {
        if(node.count_keys < MAX_KEYS) {

            add_key(node, up_key, up_data_offset, right_child_addr, position);
            write_node(node, curr_node_addr);

            has_new_key = false;

        } else {
            split(node, up_key, up_data_offset, right_child_addr, curr_node_addr);
            cout << up_key << " " << right_child_addr << endl;
            has_new_key = true;
        }
    }

    return has_new_key;
}

void IntBTree::split(Node node, int &new_key, long &new_data_addr, long &new_key_right, long curr_node_addr) {
    int middle, start;
    int new_key_location;
    Node new_node; // bloco da direita

    new_node = create_empty_node();

    middle = MAX_KEYS/2;

    // Transfere 'metade' das chaves para o novo bloco
    if(new_key > node.keys[middle]) start = middle+1;
    else start = middle;
    int count = 0;
    for (int i = start; i < MAX_KEYS; i++) {
        add_key(new_node, node.keys[i], node.data_pointers[i], node.node_pointers[i+1], count);

        // não pode APAGAR
        node.count_keys -= 1;
        count += 1;
    }
    // Atualizando primeiro ponteiro do novo nó, que será o último do nó atual
    new_node.node_pointers[0] = node.node_pointers[node.count_keys+1];

    if(start == middle+1) {
        // Caso a nova chave esteja na 2a metade do split
        // localiza a posição da nova chave dentro do novo bloco
        find_key(new_node, new_key, new_key_location);
        add_key(new_node, new_key, new_data_addr, new_key_right, new_key_location);

    } else {
        // Caso a nova chave seja a que tem que subir ou esteja na 1a metade do split
        find_key(node, new_key, new_key_location);
        add_key(node, new_key, new_data_addr, new_key_right, new_key_location);
    }

    // Armazenando a chave que deve subir
    new_key = node.keys[middle];
    new_data_addr = node.data_pointers[middle];

    // Esvaziando a posição do nó onde estava a chave central (pode APAGAR)
    node.keys[middle] = -1;
    node.data_pointers[middle] = -1;
    node.node_pointers[middle+1] = -1;

    // não pode APAGAR
    node.count_keys -= 1;

    write_node(node, curr_node_addr); // Reescrevendo nó que foi dividido
    // Armazenando o endereço do novo bloco criado (que ficará à direita da chave que vai subir)
    new_key_right = write_node(new_node);
}

long IntBTree::get_key_data(int key, int& count_blocks) {
    count_blocks = 0;
    return recursive_search(key, root_addr, count_blocks);
}

long IntBTree::recursive_search(int key, long curr_node_addr, int& count_blocks) {
    if(curr_node_addr == -1) return -1;

    int position;
    count_blocks += 1;
    Node node = get_node(curr_node_addr);
    if(find_key(node, key, position)) {
        return node.data_pointers[position];
    }

    return recursive_search(key, node.node_pointers[position], count_blocks);
}

Node IntBTree::create_empty_node() {
    Node node;
    node.count_keys = 0;
    memset(node.keys, -1, MAX_KEYS*4);
    memset(node.data_pointers, -1, MAX_KEYS*8);
    memset(node.node_pointers, -1, (MAX_KEYS+1)*8);

    return node;
}

// Insere uma nova chave em um nó. Deve haver espaço livre.
void IntBTree::add_key(Node& node, int key, long data_offset, long right_child_addr, int position) {
    int i;

    node.count_keys += 1;
    // Chaves, seus respectivos ponteiros para dados e ponteiros para nós são movidos.
    for (i = node.count_keys; i > position; i--) {
        node.keys[i] = node.keys[i-1];
        node.data_pointers[i] = node.data_pointers[i-1];
        node.node_pointers[i+1] = node.node_pointers[i];
    }

    node.keys[i] = key;
    node.data_pointers[i] = data_offset;
    node.node_pointers[i+1] = right_child_addr;
}

// Procura uma chave dentro de um nó. Se a chave já existe found = true, caso contrario o
// index recebe a posição ao qual ele deve ser inserido.
bool IntBTree::find_key(Node node, int key, int &index) {
    bool found = false;
    index = node.count_keys;

    for (index = 0; index < node.count_keys; index++) {
        if(key < node.keys[index]) {
            break;
        } else {
            if(key == node.keys[index]) {
                found = true;
                break;
            }
        }
    }
    return found;
}

Node IntBTree::get_node(long node_addr) {
    char bloco[BLOCK_SIZE];
    Node node;

    fseek(index_file, node_addr, SEEK_SET);
    fread(bloco, 1, BLOCK_SIZE, index_file);

    memcpy(&node, bloco, sizeof(Node));

    return node;
}

long IntBTree::write_node(Node node, long addr) {
    char bloco[BLOCK_SIZE];

    memcpy(bloco, &node, sizeof(Node));
    if(addr != -1) {
        fseek(index_file, addr, SEEK_SET);
    }
    else {
        fseek(index_file, 0, SEEK_END);
        addr = ftell(index_file);
    }

    fwrite(bloco, 1, BLOCK_SIZE, index_file);

    return addr;
}
