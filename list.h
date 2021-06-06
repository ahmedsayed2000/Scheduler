

struct process {
    int id;
    int arrival;
    int runtime;
    int priority;
    //int TA;
    //int waiting;
};

struct node {
    struct process* data;
    struct node* next;
};

struct list {
    int count;
    struct node* head;
};

void initList(struct list* ls){
    ls->head = NULL;
    ls->count=0;
}

void add(struct list* ls , struct process* item){
    if(ls->head == NULL){
        ls->head = (struct node*) malloc(sizeof(struct node));
        ls->head->data = item;
        ls->head->next = NULL;
    }
    else{
        struct node* ptr = ls->head;
        while(ptr->next){
            ptr = ptr->next;
        }
        struct node* new_data = (struct node*) malloc(sizeof(struct node));
        new_data->data = item;
        new_data->next = NULL;
        ptr->next = new_data;
    }
}

struct process* search(struct list* ls , int arrival){
     struct node* ptr = ls->head;
     while(ptr){
        if(ptr->data->arrival == arrival)
            return ptr->data;
        ptr = ptr->next;
     }
     return NULL;
}

void clear(struct list* ls , struct process* item){
    if(item == ls->head->data){
        struct node* ptr = ls->head;
        ls->head = ptr->next;
        free(ptr);
    }
    else{
        struct node* ptr = ls->head;
        while(ptr->next->data != item)
            ptr = ptr->next;
        
        struct node* rm = ptr->next;
        ptr->next = rm->next;
        free(rm);
    }
}

void display(struct list* ls){
    struct node* ptr = ls->head;
    while(ptr){
        printf("%d\t" , ptr->data->id);
        ptr = ptr->next;
    }
    printf("\n");
}

struct process* get_first(struct list* ls){
    if(ls->head != NULL)
    {
        struct process* ptr = ls->head->data;
        clear(ls , ptr);
        return ptr;
    }
    else
        return NULL;
}

int isEmpty(struct list* ls){
    if(ls->head == NULL)
        return 1;
    else
        return 0;
}

void add_sjf(struct list* ls , struct process* item){
    if(ls->head == NULL){
        ls->head =(struct node*) malloc(sizeof(struct node));
        ls->head->data = item;
        ls->head->next = NULL;
        return;
    }
    else{
        struct node* ptr = ls->head;
        struct node* back = ls->head;
        while(ptr){
            if(ptr->data->runtime <= item->runtime){
                back = ptr;
                ptr=ptr->next;
            }
            else{
                struct node* new_node = (struct node*) malloc(sizeof(struct node));
                if(ptr==ls->head){
                    struct node* temp = (struct node*) malloc(sizeof(struct node));
                    temp = ls->head;
                    new_node->data = item;
                    new_node->next = back;
                    ls->head = new_node;
                    return;
                }
                new_node->data = item;
                new_node->next = ptr;
                back->next = new_node;
                return;
            }
        }
        struct node* new_node = (struct node*) malloc(sizeof(struct node));
        new_node->data = item;
        new_node->next = back->next;
        back->next = new_node;
    }
}

