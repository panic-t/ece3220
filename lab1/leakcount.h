

typedef struct avl_node_tag {
    void *key;
    size_t size;
    struct avl_node_tag *left, *right;
    int lheight, rheight;
} avl_node;

