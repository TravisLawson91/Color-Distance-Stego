#include <stdlib.h>
#include <stdio.h>
#include "set.h"

typedef struct point{
    setElementT x; // color difference
    int count; // palette entry
    struct point *next;
}myDataT;

struct setCDT{
    myDataT *start;
    myDataT *end;
};


setADT setNew()
{
    setADT tmp;
    tmp = malloc(sizeof(struct setCDT));
    if(tmp == NULL)
        return NULL;
    tmp->start = tmp->end = NULL;
    return tmp;
}

void clearSet(setADT set)
{
	/*
	myDataT *cp, *next;
	cp = set->start;
	while ( cp != NULL){
		next = cp->next;
		free(cp);
		cp = next;
	}
	free(set);
    	set->start = set->end = NULL;
	*/
	while(set->start != NULL){
		myDataT* tmp;
		tmp = set->start;
		set->start = set->start->next;
		free(tmp);
	}
	free(set);
}


int setInsertElementSorted(setADT s, setElementT e, int index)
{
    myDataT *b, *prev, *curr;

    b = (myDataT *) malloc(sizeof(myDataT));

    if(b == NULL)
        return -1;

    b->x = e;
    b->next = NULL;

    prev = NULL;
    curr = s->start;
    
    while(curr)
    {
        if(curr->x >= b->x) break;
        prev = curr;
        curr = curr->next;
    }
    if(prev == NULL)
    {
        b->next = s->start;
        s->start = b;
    }else{
        b->next = prev->next;
        prev->next = b;
    }

    if(b->next == NULL)
        s->end = b;
    
    b->count = index;
    return b->count;
}


void setPrint(setADT s)
{
    myDataT *b;
    //for loop will traverse and print each value in the linked list.
    for(b = s->start; b!=NULL; b = b->next)
        printf("distsance %f index %d\n", b->x, b->count);

    printf("\n");
}

// function to return the smallest color distance and index
void setColorDistance(setADT s, int j[]){
	myDataT *b;
	int i;
	i = 0;
	for(b = s->start; b != NULL; b = b->next){
		if(i == 256)
			return;
		j[i] = b->count;
		i++;

	}
	return;
}
