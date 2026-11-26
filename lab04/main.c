#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 1024

typedef struct Node {
    char *str;
    struct Node *next;
} Node;

int main(void) {
    char buf[MAX_LEN];
    Node *head = NULL;
    Node *tail = NULL;

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if (buf[0] == '.') {
            break;
        }

        Node *new_node = (Node *)malloc(sizeof(Node));
        if (new_node == NULL) {
            perror("malloc node");
            break;
        }

        size_t len = strlen(buf);
        new_node->str = (char *)malloc(len + 1);
        if (new_node->str == NULL) {
            perror("malloc str");
            free(new_node);
            break;
        }

        strcpy(new_node->str, buf);
        new_node->next = NULL;

        if (head == NULL) {
            head = tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    Node *curr = head;
    while (curr != NULL) {
        printf("%s", curr->str);
        Node *temp = curr;
        curr = curr->next;
        free(temp->str);
        free(temp);
    }

    return 0;
}
