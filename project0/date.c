/*
 * date.c
 *
 *  Created on: Apr 6, 2017
 *	  Author: brian
 */

#include "date.h"
#include <string.h>
#include <stdlib.h>


struct date {
	char yyyymmdd[11];
};


/*
 * date_create creates a Date structure from `datestr`
 * `datestr' is expected to be of the form "dd/mm/yyyy"
 * returns pointer to Date structure if successful,
 *		 NULL if not (syntax error)
 */
Date *date_create(char *datestr){
	//malloc enough space for the date instance
	Date *dateStruct = (Date *)malloc(sizeof(Date));

	//NULL check. d is pointer to null if malloc fails
	if (dateStruct != NULL) {
		//copy the string in reverse order
		int i, len = strlen(datestr);
		char *end = datestr + len - 1;
		for(i = 0; i<len ; end--, i++){
			dateStruct->yyyymmdd[i] = *end;
		}
		dateStruct->yyyymmdd[i] = "\0";
	}
	return dateStruct;
}

/*
 * date_duplicate creates a duplicate of `d'
 * returns pointer to new Date structure if successful,
 *	NULL if not (memory allocation failure)
 */
Date *date_duplicate(Date *d){
	//malloc enough space for the dup date instance
	Date *dupStruct = (Date *)malloc(sizeof(Date));
	//NULL check. dupStruct is pointer to null if malloc fails
	if (dupStruct != NULL) {
		strcpy(dupStruct->yyyymmdd, d->yyyymmdd);
	}
	return dupStruct;
}

/*
 * date_compare compares two dates, returning <0, 0, >0 if
 * date1<date2, date1==date2, date1>date2, respectively
 */
int date_compare(Date *date1, Date *date2){
	return strcmp(date1->yyyymmdd, date2->yyyymmdd);
}

/*
 * date_destroy returns any storage associated with `d' to the system
 */
void date_destroy(Date *d){
	free(d->yyyymmdd);
	free(d);
}

