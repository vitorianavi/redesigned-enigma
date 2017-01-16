#include "intbtree.hpp"

int main() {
    IntBTree btree('w');

    for (int i = 0; i < 14; i++) {
        btree.insert(i, i+1);
    }

    btree.print();
}
