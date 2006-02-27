#ifndef GMCORE_DB_H
#define GMCORE_DB_H

/*This header file contains all the exported definitions and declarations of the DB class*/


#define VISITED_MASK     	4          /* ...00000000000000100 */
#define VALUE_MASK       	3          /* ...00000000000000011 */
#define MEX_MAX	  		31	   /* 2^5-1 */
#define MEX_SHIFT		3	   /* ...00000000011111000 */
#define MEX_MASK		(MEX_MAX << MEX_SHIFT)
#define REMOTENESS_SHIFT 	8          /* ...01111111100000000 */
#define REMOTENESS_MAX     	255      /* We encode draws as this */
#define REMOTENESS_TWOBITS 	254      /* we encode TwoBits as this */
#define REMOTENESS_MASK  	(REMOTENESS_MAX << REMOTENESS_SHIFT)

#define kMaxMexValue		32
#define kBadMexValue  		-1 /* -1 Can never be a valid mex value */
/* this is saying that the bad value of remoteness is the one that cause
   a draw. Will this be ok... hmm */
#define kBadRemoteness		REMOTENESS_MAX

typedef enum known_dbs_enum{
    nulldb, memdb, twobitdb, colldb, univdb
} known_db_types;

typedef struct DB {

    /* for Database authors: */

    /* These 7 functions need to be reduced to the 2 that are specified
       in the DB Class. Like this for ease of implementation/switchover
       a single get and put is really all we need.
       - last words of Scott that are not necessarily true anymore*/

    /* here are the things that ARE true:*/

    /* all these 12 functions may assume the completion of any error-checking
       for their parameters */

    /* for functions that return a BOOLEAN, make it TRUE if the operation succeeds,
       FALSE otherwise
       for functions that return a VALUE, undecided means failure
       for functions that return a REMOTENESS, kBadRemoteness means failure
       for functions that return a MEX, kBadMexValue means failure
       for the write operations to be successful, these functions are required to
       return whatever is IN ITS DB DATA STRUCTURE (ARRAY, ETC.) for comparison */

    /* for a DB to be useful you have to implement at least the value-related functions*/
    /* if gamesman does not see those two available it will exit right away.
       For the other function it will simply return a bad value or do nothing */
    /* if gamesman sees an error during range-checking, it will exit. This happens
       even when, for example, the cannonical sibling of a position is within range,
       but the position itself is not. */

    void        (*free_db)       	();

    VALUE	(*get_value)		(POSITION pos);
    VALUE	(*put_value)		(POSITION pos, VALUE val);
    VALUE   (*original_put_value) (POSITION pos, VALUE val);

    REMOTENESS  (*get_remoteness)	(POSITION pos);
    void	(*put_remoteness)	(POSITION pos, REMOTENESS val);

    BOOLEAN     (*check_visited)	(POSITION pos);
    void	(*mark_visited)		(POSITION pos);
    void     	(*unmark_visited)	(POSITION pos);

    MEX		(*get_mex)		(POSITION pos);
    void	(*put_mex)		(POSITION pos, MEX mex);

    BOOLEAN	(*save_database)	();
    BOOLEAN	(*load_database)	();

} DB_Table;

typedef struct db_list_struct{
    DB_Table *db;
    struct db_list_struct *next_db;
} db_list;

VALUE       db_original_put_value(POSITION pos, VALUE data);

/* external interface functions. Limited to one db at a time, therefore needs to be
   changed */

/* General */
void		CreateDatabases		();
void		InitializeDatabases	();
void		DestroyDatabases       	();

/* Since the solvers will These will be deprecated soon */
/* Value */
VALUE		GetValueOfPosition	(POSITION pos);
VALUE		StoreValueOfPosition	(POSITION pos, VALUE val);

/* Remoteness */
REMOTENESS	Remoteness		(POSITION pos);
void		SetRemoteness		(POSITION pos, REMOTENESS val);

/* Visited */
BOOLEAN		Visited			(POSITION pos);
void		MarkAsVisited		(POSITION pos);
void		UnMarkAsVisited		(POSITION pos);
void	        UnMarkAllAsVisited  	();

/* Mex */
void		MexStore		(POSITION pos, MEX mex);
MEX		MexLoad			(POSITION pos);

/* Persistence */
BOOLEAN		SaveDatabase		();
BOOLEAN		LoadDatabase		();

#endif /* GMCORE_DB_H */
