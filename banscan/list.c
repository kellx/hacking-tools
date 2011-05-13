#include <stdio.h>
#include <stdlib.h>
#include "list.h"

void list_init(list_t *list)
{
  list->head = NULL;
  list->tail = NULL;
  list->list_size = 0;
}

void list_insert(void *ptr, list_t *list)
{
  list_node_t *node;

  node = (list_node_t *) malloc(sizeof(list_node_t));

  node->data = ptr;

  if(!list->list_size) 
    list->tail = node;

  node->next = list->head;
  list->head = node;

  list->list_size++;
}

void list_remove(void **ptr, list_t *list)
{
  list_node_t *node;

  if(!list->list_size) {
    *ptr = NULL;
    return;
  }
  
  *ptr = list->head->data; 

  node = list->head;
  list->head = list->head->next;

  if(list->list_size == 1)
    list->tail = NULL;

  free(node);
  list->list_size--;
}

void list_destroy(list_t *list)
{
  void *data;

  while(list->list_size > 0) 
    list_remove((void **)&data, list);
}
