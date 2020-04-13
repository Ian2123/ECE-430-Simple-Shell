struct node {				
       int pid, prints;
       char *name;
       struct node *next;
	
};


struct queue {				
       struct node *head;
       struct node *tail ;
		 struct queue *next;
};

     
void enqueue(int item, char *s, struct queue *q) {		
     struct node *p;						

     p= (struct node *)malloc(sizeof(struct node));
     p->pid=item;
     p->name=s;	
     p->next=NULL;
     if (q->head==NULL) q->head=q->tail=p;
     else 
	  {
          q->tail->next=p;
          q->tail=p;
     }

}


int dequeue(struct queue *q) {		
    int item;				
    struct node *p;
    
    item=q->head->pid;
    p=q->head;
    q->head = q->head->next;
    free(p);
	 return item;
}


void delete(struct queue *q, int key)
{
	if(q->head == NULL) return;
   if (q->head->pid == key)
   {
        struct node *p = q->head;
        q->head = q->head->next;
        free(p);
        return;
    }
    struct node *current = q->head->next;
    struct node *previous = q->head;
    while (current != NULL && previous != NULL)
    {
      if (current->pid == key)
      {
        struct node *tmp = current;
        if (current == q->tail)
          q->tail = previous;
        previous->next = current->next;
        free(tmp);
        return;
      }
      previous = current;
      current = current->next;
    }
    return;  
}


void sjf_enqueue(int in_id, int in_prints, char * s, struct node **np)
{
   struct node * n = (struct node *) malloc(sizeof(struct node));

   if(n == NULL)
   {
      printf("OUT OF MEMORY!!");
      exit(1);
   }

   n->pid = in_id;
	n->prints = in_prints;
   n->name = s;

   while(*np != NULL && (*np)->prints < in_prints)
      np = &((*np)->next);
   n->next = *np;
   *np = n;
}

