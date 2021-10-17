#include <stdlib.h>
#include <stdio.h>
#include "set.h"

typedef struct point{
    setElementT colorDist; // color difference
    int index; // palette entry
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
	while(set->start != NULL){
		myDataT* tmp;
		tmp = set->start;
		set->start = set->start->next;
		free(tmp);
	}
	free(set);
}


int setInsertElementSorted(setADT set, setElementT distance, int index)
{
    myDataT *insert, *prev, *curr;

    insert = (myDataT *) malloc(sizeof(myDataT));

    if(insert == NULL)
        return -1;

    insert->colorDist = distance;
    insert->next = NULL;

    prev = NULL;
    curr = set->start;
    
    while(curr)
    {
        if(curr->colorDist >= insert->colorDist) break;
        prev = curr;
        curr = curr->next;
    }
    if(prev == NULL)
    {
        insert->next = set->start;
        set->start = insert;
    }else{
        insert->next = prev->next;
        prev->next = insert;
    }

    if(insert->next == NULL)
        set->end = insert;
    
    insert->index = index;
    return insert->index;
}


void setPrint(setADT set)
{
    myDataT *node;
    //for loop will traverse and print each value in the linked list.
    for(node = set->start; node!=NULL; node = node->next)
        printf("distsance %f index %d\n", node->colorDist, node->index);

    printf("\n");
}

/*
 * setColorDistance will iterate through the whole linked list
 * and place the sorted index into pIndex passed from the 
 * fuction call in hideMessage()
 */
void setColorDistance(setADT set, int pIndex[]){
	myDataT *node;
	int i;
	i = 0;
	for(node = set->start; node != NULL; node = node->next){
		if(i == 256)
			return;
		pIndex[i] = node->index;
		i++;

	}
	return;
}
