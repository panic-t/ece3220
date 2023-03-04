#include <stdio.h>


typedef struct bst_node_tag {
    int key, count;
    struct bst_node_tag *left, *right;
} bst_node;

int main(int argc, char *argv[]);
bst_node *search_insert_bst(bst_node **head, int searchkey);
void rec_file_print(FILE *output, bst_node *head);