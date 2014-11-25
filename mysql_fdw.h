/*-------------------------------------------------------------------------
 *
 * mysql_fdw.h
 * 		Foreign-data wrapper for remote MySQL servers
 *
 * Portions Copyright (c) 2012-2014, PostgreSQL Global Development Group
 *
 * Portions Copyright (c) 2004-2014, EnterpriseDB Corporation.
 *
 * IDENTIFICATION
 * 		mysql_fdw.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef MYSQL_FDW_H
#define MYSQL_FDW_H

#define list_length mysql_list_length
#define list_delete mysql_list_delete
#define list_free mysql_list_free

#include <mysql.h>
#undef list_length
#undef list_delete
#undef list_free

#include "foreign/foreign.h"
#include "lib/stringinfo.h"
#include "nodes/relation.h"
#include "utils/rel.h"

#define MYSQL_PREFETCH_ROWS	100
#define MYSQL_BLKSIZ		(1024 * 4)
#define MYSQL_PORT			3306
#define MAXDATALEN			1024 * 64
#define WAIT_TIMEOUT		180
#define INTERACTIVE_TIMEOUT 70

/*
 * Options structure to store the MySQL
 * server information
 */
typedef struct mysql_opt
{
	int     svr_port;			/* MySQL port number */
	char    *svr_address;		/* MySQL server ip address */
	char    *svr_username;		/* MySQL user name */
	char    *svr_password;		/* MySQL password */
	char    *svr_database;		/* MySQL database name */
	char    *svr_table;			/* MySQL table name */
} mysql_opt;

/*
 * FDW-specific information for ForeignScanState 
 * fdw_state.
 */
typedef struct MySQLFdwExecState
{
	MYSQL           *conn;              /* MySQL connection handle */
	MYSQL_STMT      *stmt;              /* MySQL prepared stament handle */
	char            *query;             /* Query string */
	Relation        rel;                /* relcache entry for the foreign table */
	List            *retrieved_attrs;   /* list of target attribute numbers */

	int             p_nums;             /* number of parameters to transmit */
	FmgrInfo        *p_flinfo;          /* output conversion functions for them */

	mysql_opt       *mysqlFdwOptions;   /* MySQL FDW options */

	Datum           *tts_values;        /* Values datum to bind for results */
	bool            *tts_isnull;        /* Nulls array bind for results */
	List            *attr_list;         /* query attribute list */
	List            *column_list;       /* Column list of MySQL Column structures */

	/* working memory context */
	MemoryContext   temp_cxt;           /* context for per-tuple temporary data */
} MySQLFdwExecState;


/* MySQL Column List */
typedef struct MySQLColumn
{
	int   attnum;          /* Attribute number */
	char  *attname;        /* Attribute name */
	int   atttype;         /* Attribute type */
} MySQLColumn;


int ((*_mysql_options)(MYSQL *mysql,enum mysql_option option, const void *arg));
int ((*_mysql_stmt_prepare)(MYSQL_STMT *stmt, const char *query, unsigned long length));
int ((*_mysql_stmt_execute)(MYSQL_STMT *stmt));
int ((*_mysql_stmt_fetch)(MYSQL_STMT *stmt));
int ((*_mysql_query)(MYSQL *mysql, const char *q));
bool ((*_mysql_stmt_attr_set)(MYSQL_STMT *stmt, enum enum_stmt_attr_type attr_type, const void *attr));
bool ((*_mysql_stmt_close)(MYSQL_STMT * stmt));
bool ((*_mysql_stmt_reset)(MYSQL_STMT * stmt));
bool ((*_mysql_free_result)(MYSQL_RES *result));
bool ((*_mysql_stmt_bind_param)(MYSQL_STMT *stmt, MYSQL_BIND * bnd));
bool ((*_mysql_stmt_bind_result)(MYSQL_STMT *stmt, MYSQL_BIND * bnd));

MYSQL_STMT	*((*_mysql_stmt_init)(MYSQL *mysql));
MYSQL_RES	*((*_mysql_stmt_result_metadata)(MYSQL_STMT *stmt));
int ((*_mysql_stmt_store_result)(MYSQL *mysql));
MYSQL_ROW	((*_mysql_fetch_row)(MYSQL_RES *result));
MYSQL_FIELD	*((*_mysql_fetch_field)(MYSQL_RES *result));
const char	*((*_mysql_error)(MYSQL *mysql));
void	((*_mysql_close)(MYSQL *sock));
MYSQL_RES* ((*_mysql_store_result)(MYSQL *mysql));

MYSQL	*((*_mysql_init)(MYSQL *mysql));
MYSQL	*((*_mysql_real_connect)(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db,
								unsigned int port,
								const char *unix_socket,
								unsigned long clientflag));

/* option.c headers */
extern bool mysql_is_valid_option(const char *option, Oid context);
extern mysql_opt *mysql_get_options(Oid foreigntableid);

/* depare.c headers */
extern void mysql_deparse_select(StringInfo buf, PlannerInfo *root, RelOptInfo *baserel,
							 Bitmapset *attrs_used, char *svr_table, List **retrieved_attrs);
extern void mysql_deparse_insert(StringInfo buf, PlannerInfo *root, Index rtindex, Relation rel, List *targetAttrs);
extern void mysql_deparse_update(StringInfo buf, PlannerInfo *root, Index rtindex, Relation rel, List *targetAttrs, char *attname);
extern void mysql_deparse_delete(StringInfo buf, PlannerInfo *root, Index rtindex, Relation rel, char *name);
extern void mysql_append_where_clause(StringInfo buf, PlannerInfo *root, RelOptInfo *baserel, List *exprs,
							 bool is_first,List **params);
extern void mysql_deparse_analyze(StringInfo buf, char *dbname, char *relname);


/* connection.c headers */
MYSQL *mysql_get_connection(ForeignServer *server, UserMapping *user, mysql_opt *opt);
MYSQL *mysql_connect(char *svr_address, char *svr_username, char *svr_password, char *svr_database, int svr_port);
void  mysql_cleanup_connection(void);
void mysql_rel_connection(MYSQL *conn);
#endif /* POSTGRES_FDW_H */