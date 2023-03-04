

typedef struct stack_node_tag {
    void *key;
    int size;
    struct stack_node_tag *link;
} stack_node;

void wrapup();
int main(int argc, char const *argv[]);
