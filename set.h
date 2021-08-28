#ifndef _set_h_
#define _set_h_

typedef double setElementT;
typedef struct setCDT *setADT;

setADT setNew(); //create a new empty set
void clearSet(setADT s); //free the space allocated for the set s

int setInsertElementSorted(setADT s, setElementT e, int index);
    /*if not successful, return 0; otherwise, return the number of elements
    in the set (including the element just inserted). Also note that the elements
    might be given in different orders, but your function should always keep
    the set in a sorted manner after each insertion*/

void setPrint(setADT s);
    //print elements of s, a = {2,5,7}
void setColorDistance(setADT s, int j[]);

#endif
