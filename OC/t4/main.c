#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_CAPACITY 1024

typedef struct ListNode {
    char *text_data;            
    struct ListNode *next_node; 
} ListNode;

int main(void)
{
    
    char input_buf[LINE_CAPACITY];

   
    ListNode *first = NULL;
    ListNode *last = NULL;

    while (fgets(input_buf, sizeof(input_buf), stdin) != NULL) {

        
        if (input_buf[0] == '.') {
            break;
        }

       
        size_t str_len = strlen(input_buf);

       
        ListNode *new_elem = malloc(sizeof(ListNode));
        if (!new_elem) {
            perror("Ошибка");
            return 1;
        }

        
        new_elem->text_data = malloc(str_len + 1);
        if (!new_elem->text_data) {
            perror("Ошибка");
            free(new_elem); 
            return 1;
        }

        
        strcpy(new_elem->text_data, input_buf);

        
        new_elem->next_node = NULL;

       
        if (first == NULL) {
           
            first = new_elem;
            last = new_elem;
        } else {
            
            last->next_node = new_elem;
            
            last = new_elem;
        }
    }

    printf("\nСодержимое сохраненного списка:\n");

    
    ListNode *cursor = first;

    
    while (cursor != NULL) {
        
        printf("%s", cursor->text_data);

        
        cursor = cursor->next_node;
    }

  
    cursor = first;

    while (cursor != NULL) {
        
        ListNode *step_ahead = cursor->next_node;

        
        free(cursor->text_data); 
        free(cursor);            

        
        cursor = step_ahead;
    }

    return 0;
}