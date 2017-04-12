/*
 * tldlist.c
 *
 *  Created on: Apr 6, 2017
 *      Author: brian
 *		DuckID: bel
 *		HW: Project0
 *		This is my own work except:
 *		ideas and code from: https://gist.github.com/tonious/1377768
 */

#define DATE_LEN 20

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tldlist.h"
#include "date.h"

struct tldlist{
	Date *begin;
	Date *end;
	struct tldnode *root;
	int adds;
	int uniqueCount;
};

struct tldnode{
	struct tldnode *left;
	struct tldnode *right;
	char value[10];
	int count;
};

struct tlditerator{
	TLDNode **array;
	TLDNode *next;
	int size;
	int index;
};

//HELPER FUNCTIONS

static TLDNode *avl_create_node() {
	TLDNode *node = NULL;

	if( (node = malloc(sizeof(TLDNode))) == NULL) {
		return NULL;
	}

	node->left = NULL;
	node->right = NULL;
	node->count = 0;

	return node;
}

/* Left Left Rotate */
static TLDNode *avl_rotate_leftleft( TLDNode *node ) {
 	TLDNode *a = node;
	TLDNode *b = a->left;

	a->left = b->right;
	b->right = a;

	return( b );
}

/* Left Right Rotate */
static TLDNode *avl_rotate_leftright( TLDNode *node ) {
	TLDNode *a = node;
	TLDNode *b = a->left;
	TLDNode *c = b->right;

	a->left = c->right;
	b->right = c->left;
	c->left = b;
	c->right = a;

	return( c );
}

/* Right Left Rotate */
static TLDNode *avl_rotate_rightleft( TLDNode *node ) {
	TLDNode *a = node;
	TLDNode *b = a->right;
	TLDNode *c = b->left;

	a->right = c->left;
	b->left = c->right;
	c->right = b;
	c->left = a;

	return( c );
}

/* Right Right Rotate */
static TLDNode *avl_rotate_rightright( TLDNode *node ) {
	TLDNode *a = node;
	TLDNode *b = a->right;

	a->right = b->left;
	b->left = a;

	return( b );
}

/* Find the height of an AVL node recursively */
static int avl_node_height( TLDNode *node ) {
	int height_left = 0;
	int height_right = 0;

	if( node->left ) height_left = avl_node_height( node->left );
	if( node->right ) height_right = avl_node_height( node->right );

	return height_right > height_left ? ++height_right : ++height_left;
}

/* Find the balance of an AVL node */
static int avl_balance_factor( TLDNode *node ) {
	int bf = 0;

	if( node->left  ) bf += avl_node_height( node->left );
	if( node->right ) bf -= avl_node_height( node->right );

	return bf ;
}
/* Balance a given node */
static TLDNode *avl_balance_node( TLDNode *node ) {
	TLDNode *newroot = NULL;

	/* Balance our children, if they exist. */
	if( node->left )
		node->left  = avl_balance_node( node->left  );
	if( node->right )
		node->right = avl_balance_node( node->right );

	int bf = avl_balance_factor( node );

	if( bf >= 2 ) {
		/* Left Heavy */

		if( avl_balance_factor( node->left ) <= -1 )
			newroot = avl_rotate_leftright( node );
		else
			newroot = avl_rotate_leftleft( node );

	} else if( bf <= -2 ) {
		/* Right Heavy */

		if( avl_balance_factor( node->right ) >= 1 )
			newroot = avl_rotate_rightleft( node );
		else
			newroot = avl_rotate_rightright( node );

	} else {
		/* This node is balanced -- no change. */

		newroot = node;
	}

	return( newroot );
}

/* Balance a given tree */
static void avl_balance( TLDList *tree ) {

	TLDNode *newroot = NULL;

	newroot = avl_balance_node( tree->root );

	if( newroot != tree->root )  {
		tree->root = newroot;
	}

}

static int avl_insert( TLDList *tree, char *value ) {
	puts("inserting:");
	printf("%s\n", value);
	//success returns 1, failure returns 0
	TLDNode *node = NULL;
	TLDNode *next = NULL;
	TLDNode *last = NULL;

	/* Well, there must be a first case */
	if( tree->root == NULL ) {
		node = avl_create_node();
		if (node == NULL){
			return 0;
		}
		strcpy(node->value, value);
		tree->root = node;

	/* Okay.  We have a root already.  Where do we put this? */
	} else {
		next = tree->root;

		while(next != NULL) {
			last = next;

			if (value < next->value) {
				next = next->left;

			} else if( value > next->value ) {
				next = next->right;

			//We already inserted this node
			} else if( value == next->value ) {
				next->count++;
				return 1;
			}
		}

		node = avl_create_node();
		if (node == NULL){
			return 0;
		}
		strcpy(node->value, value);

		if( value < last->value ) last->left = node;
		if( value > last->value ) last->right = node;
	}
	tree->uniqueCount++;
	avl_balance( tree );
	return 1;
}


/* Do a depth first traverse of a node. */
static void avl_traverse_node_dfs( TLDNode *node, TLDIterator *iter ) {
	puts("traversing");
	printf("%s\n", node->value);
	if(node->left){
		avl_traverse_node_dfs(node->left, iter);
	}
	iter->next = node;
	iter->next++;


	if( node->right){
		avl_traverse_node_dfs( node->right, iter);
	}



}

/* Do a depth first traverse of a tree. and populate the iter with all nodes */
static void avl_traverse_dfs( TLDList *tree, TLDIterator *iter) {

	avl_traverse_node_dfs( tree->root, iter );
}

/*
 * tldlist_create generates a list structure for storing counts against
 * top level domains (TLDs)
 *
 * creates a TLDList that is constrained to the `begin' and `end' Date's
 * returns a pointer to the list if successful, NULL if not
 */
TLDList *tldlist_create(Date *begin, Date *end){
	//malloc enough space for the TLDList instance
	TLDList *tldstruct = (TLDList *)malloc(sizeof(TLDList));

	if (tldstruct == NULL){
		return NULL;
	}
	tldstruct->root = NULL;
	tldstruct->begin = begin;
	tldstruct->end = end;
	tldstruct->adds = 0;
	tldstruct->uniqueCount = 0;

	return tldstruct;
}

/*
 * tldlist_destroy destroys the list structure in `tld'
 *
 * all heap allocated storage associated with the list is returned to the heap
 */
void tldlist_destroy(TLDList *tld){
	free(tld);
}

/*
 * tldlist_add adds the TLD contained in `hostname' to the tldlist if
 * `d' falls in between the begin and end dates associated with the list;
 * returns 1 if the entry was counted, 0 if not
 */
int tldlist_add(TLDList *tld, char *hostname, Date *d){
	//if begin<d && d<end
	int afterStart = date_compare(tld->begin, d) < 0;
	int beforEnd = date_compare(d, tld->end) > 0;

	if (!(afterStart && beforEnd)){ //falls out of date range
		return 0;
	}

	if (!(avl_insert(tld, hostname))){ //failed to insert
		return 0;
	}
	tld->adds++;
	return 1;
}

/*
 * tldlist_count returns the number of successful tldlist_add() calls since
 * the creation of the TLDList
 */
long tldlist_count(TLDList *tld){
	return tld->adds;
}

/*
 * tldlist_iter_create creates an iterator over the TLDList; returns a pointer
 * to the iterator if successful, NULL if not
 */
TLDIterator *tldlist_iter_create(TLDList *tld){
	TLDIterator *iter = (TLDIterator *) malloc(sizeof(TLDIterator));
	if (iter != NULL){
		TLDNode **array = (TLDNode **) malloc((tld->uniqueCount) * sizeof(TLDNode *));
		if(array != NULL){
			//initialize
			iter->array = array;
			iter->next = array[0];
			iter->size = tld->uniqueCount;
			iter->index = 0;

			//traverse and populate
			avl_traverse_dfs(tld, iter);
			iter->next = array[0];
		}
	}
	return iter;
}

/*
 * tldlist_iter_next returns the next element in the list; returns a pointer
 * to the TLDNode if successful, NULL if no more elements to return
 */
TLDNode *tldlist_iter_next(TLDIterator *iter){
	if(iter->index >= iter->size){
		return NULL;
	}

	TLDNode *temp = iter->next;
	iter->next++;
	iter->index++;
	return temp;
}

/*
 * tldlist_iter_destroy destroys the iterator specified by `iter'
 */
void tldlist_iter_destroy(TLDIterator *iter){
	free(iter->array);
	free(iter);
}

/*
 * tldnode_tldname returns the tld associated with the TLDNode
 */
char *tldnode_tldname(TLDNode *node){
	return node->value;
}

/*
 * tldnode_count returns the number of times that a log entry for the
 * corresponding tld was added to the list
 */
long tldnode_count(TLDNode *node){
	return node->count;
}





