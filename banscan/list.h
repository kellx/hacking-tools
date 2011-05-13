/*
 * simple linked list implementation
 * written by bind <bind@insidiae.org>
 */

typedef struct list_node_t {
  int size;
  void *data;
  struct list_node_t *next;
} list_node_t;

typedef struct list_t {
  list_node_t *head;
  list_node_t *tail;
  int list_size;
} list_t;

void list_init(list_t *);
void list_insert(void *, list_t *);
void list_remove(void **, list_t *);
void list_destroy(list_t *);

