#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sqlite3.h"

#define DB_NAME "local_scene.db"
static sqlite3 *local_scene_db;
static const char *tables_name[] = {"scene_local", "scene_control", "scene_control_group_name"};
static const char *tables_sql_create[] =
    {"CREATE TABLE %s(\
    ID INTEGER PRIMARY KEY AUTOINCREMENT,\
    RULEID TEXT UNIQUE NOT NULL,\
    SCENEID TEXT NOT NULL);",
     "CREATE TABLE %s(\
    ID INTEGER PRIMARY KEY AUTOINCREMENT,\
    GROUPID TEXT NOT NULL,\
    PRODUCTKEY VARCHAR(33),\
    DEVICEID VARCHAR(33) NOT NULL,\
    SWITCHLOOP TEXT NOT NULL);",
     "CREATE TABLE %s(\
    ID INTEGER PRIMARY KEY AUTOINCREMENT,\
    GROUPID TEXT UNIQUE NOT NULL,\
    GROUPNAME TEXT NOT NULL);"};
static const char *tables_sql_reset[] =
    {"DELETE FROM %s;",
     "DELETE FROM sqlite_sequence WHERE name = %s;"};

static int table_delete_by_field(const char *table_name, const char *field_name, const char *field_value)
{
#define TABLE_DELETE_BY_FIELD "DELETE FROM %s WHERE %s = ?;"
    char sql_buf[255];
    snprintf(sql_buf, sizeof(sql_buf), TABLE_DELETE_BY_FIELD, table_name, field_name);

    sqlite3_stmt *pstmt;
    const char *pzTail;

    int rc = sqlite3_prepare_v2(local_scene_db, sql_buf, -1, &pstmt, &pzTail);
    if (SQLITE_OK != rc)
    {
        printf("sqlite3_prepare_v2:%s\n", sqlite3_errmsg(local_scene_db));
        sqlite3_finalize(pstmt);
        return rc;
    }
    sqlite3_bind_text(pstmt, 1, field_value, strlen(field_value), NULL);

    sqlite3_step(pstmt);
    sqlite3_reset(pstmt);
    sqlite3_finalize(pstmt);
    return 0;
}

int table0_delete(const char *field_value)
{
    return table_delete_by_field(tables_name[0], "SCENEID", field_value);
}

int table1_delete(const char *field_value)
{
    return table_delete_by_field(tables_name[1], "GROUPID", field_value);
}

int table2_delete(const char *field_value)
{
    return table_delete_by_field(tables_name[2], "GROUPID", field_value);
}

static int table_insert_by_field(const char *table_name, const char (*field_value)[48], const unsigned char field_num)
{
#define TABLE_INSERT_BY_FIELD "INSERT OR REPLACE INTO %s VALUES (NULL%s);"
    if (field_value == NULL || field_num == 0)
        return -1;

    char field_adapt_buf[16] = {0};
    for (int i = 0; i < field_num; ++i)
    {
        strcat(field_adapt_buf, ",?");
    }
    char sql_buf[255];
    snprintf(sql_buf, sizeof(sql_buf), TABLE_INSERT_BY_FIELD, table_name, field_adapt_buf);
    printf("sql insert:%s\n", sql_buf);
    sqlite3_stmt *pstmt;
    const char *pzTail;

    int rc = sqlite3_prepare_v2(local_scene_db, sql_buf, -1, &pstmt, &pzTail);
    if (SQLITE_OK != rc)
    {
        printf("sqlite3_prepare_v2:%s\n", sqlite3_errmsg(local_scene_db));
        sqlite3_finalize(pstmt);
        return rc;
    }

    for (int i = 0; i < field_num; ++i)
    {
        printf("sql bind text:%s\n", field_value[i]);
        sqlite3_bind_text(pstmt, i + 1, field_value[i], strlen(field_value[i]), NULL);
    }
    sqlite3_step(pstmt);
    sqlite3_reset(pstmt);
    sqlite3_finalize(pstmt);
    return 0;
}

int table0_insert(const char *devId, const char *modelId)
{
    char field_value[2][48];
    strcpy(field_value[0], devId);
    strcpy(field_value[1], modelId);
    return table_insert_by_field(tables_name[0], field_value, sizeof(field_value) / sizeof(field_value[0]));
}

int table1_insert(const char *group_id, const char *product_key, const char *device_id, const char *switch_loop)
{
    char field_value[4][48];
    strcpy(field_value[0], group_id);
    strcpy(field_value[1], product_key);
    strcpy(field_value[2], device_id);
    strcpy(field_value[3], switch_loop);
    return table_insert_by_field(tables_name[1], field_value, sizeof(field_value) / sizeof(field_value[0]));
}

int table2_insert(const char *group_id, const char *group_name)
{
    char field_value[2][48];
    strcpy(field_value[0], group_id);
    strcpy(field_value[1], group_name);

    return table_insert_by_field(tables_name[2], field_value, sizeof(field_value) / sizeof(field_value[0]));
}

int table0_select(const char *src_scene_id, const char *sceneId, int (*select_func)(const char *, const char *, const char *))
{
#define TABLE0_SELECT "SELECT * FROM %s WHERE SCENEID = ?;"
    char sql_buf[255];
    snprintf(sql_buf, sizeof(sql_buf), TABLE0_SELECT, tables_name[0]);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(local_scene_db, sql_buf, -1, &stmt, NULL);
    if (SQLITE_OK != rc)
    {
        printf("sqlite3_prepare_v2\n");
        sqlite3_finalize(stmt);
    }
    sqlite3_bind_text(stmt, 1, sceneId, strlen(sceneId), NULL);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        //一列一列地去读取每一条记录 1表示列
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *rule_id = sqlite3_column_text(stmt, 1);
        const unsigned char *scene_id = sqlite3_column_text(stmt, 2);
        printf("ID:%d rule_id:%s scene_id:%s\n", id, rule_id, scene_id);
        if (select_func != NULL)
            select_func(src_scene_id, (const char *)rule_id, (const char *)scene_id);
    }
    return sqlite3_finalize(stmt);
}

int table1_select(const char *group_id, void *arg, int (*select_func)(const char *, const char *, const char *, const char *, void *))
{
#define TABLE1_SELECT "SELECT * FROM %s %s;"
    char sql_buf[255];

    sqlite3_stmt *stmt;
    int rc;

    if (group_id == NULL)
    {
        snprintf(sql_buf, sizeof(sql_buf), TABLE1_SELECT, tables_name[1], "");
        rc = sqlite3_prepare_v2(local_scene_db, sql_buf, -1, &stmt, NULL);
        if (SQLITE_OK != rc)
        {
            printf("sqlite3_prepare_v2 error\n");
            sqlite3_finalize(stmt);
            return rc;
        }
    }
    else
    {
        snprintf(sql_buf, sizeof(sql_buf), TABLE1_SELECT, tables_name[1], "WHERE GROUPID = ?");
        printf("table1_select %s:%s\n", sql_buf, group_id);
        rc = sqlite3_prepare_v2(local_scene_db, sql_buf, -1, &stmt, NULL);
        if (SQLITE_OK != rc)
        {
            printf("sqlite3_prepare_v2 error\n");
            sqlite3_finalize(stmt);
            return rc;
        }
        sqlite3_bind_text(stmt, 1, group_id, strlen(group_id), NULL);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        //一列一列地去读取每一条记录 1表示列
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *group_id = sqlite3_column_text(stmt, 1);
        const unsigned char *product_key = sqlite3_column_text(stmt, 2);
        const unsigned char *device_id = sqlite3_column_text(stmt, 3);
        const unsigned char *switch_loop = sqlite3_column_text(stmt, 4);

        printf("ID:%d group_id:%s product_key:%s device_id:%s switch_loop:%s\n", id, group_id, product_key, device_id, switch_loop);
        if (select_func != NULL)
            select_func((const char *)group_id, (const char *)product_key, (const char *)device_id, (const char *)switch_loop, arg);
    }
    printf("table1_select end\n");
    return sqlite3_finalize(stmt);
}

int table2_select(const char *group_id, void *arg, int (*select_func)(const char *, const char *, void *))
{
#define TABLE2_SELECT "SELECT * FROM %s %s;"
    char sql_buf[255];

    sqlite3_stmt *stmt;
    int rc;
    if (group_id == NULL)
    {
        snprintf(sql_buf, sizeof(sql_buf), TABLE2_SELECT, tables_name[2], "");
        rc = sqlite3_prepare_v2(local_scene_db, sql_buf, -1, &stmt, NULL);
        if (SQLITE_OK != rc)
        {
            printf("sqlite3_prepare_v2\n");
            sqlite3_finalize(stmt);
        }
    }
    else
    {
        snprintf(sql_buf, sizeof(sql_buf), TABLE2_SELECT, tables_name[2], "WHERE GROUPID = ?");
        rc = sqlite3_prepare_v2(local_scene_db, sql_buf, -1, &stmt, NULL);
        if (SQLITE_OK != rc)
        {
            printf("sqlite3_prepare_v2\n");
            sqlite3_finalize(stmt);
        }
        sqlite3_bind_text(stmt, 1, group_id, strlen(group_id), NULL);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        //一列一列地去读取每一条记录 1表示列
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *group_id = sqlite3_column_text(stmt, 1);
        const unsigned char *group_name = sqlite3_column_text(stmt, 2);
        printf("ID:%d group_id:%s group_name:%s\n", id, group_id, group_name);
        if (select_func != NULL)
            select_func((const char *)group_id, (const char *)group_name, arg);
    }
    return sqlite3_finalize(stmt);
}
//------------------------------
void databseReset(void)
{
    char *errMsg = NULL;
    int rc;
    char sql_buf[255];
    for (int i = 0; i < sizeof(tables_name) / sizeof(tables_name[0]); ++i)
    {
        for (int j = 0; j < sizeof(tables_sql_reset) / sizeof(tables_sql_reset[0]); ++j)
        {
            snprintf(sql_buf, sizeof(sql_buf), tables_sql_reset[j], tables_name[i]);
            rc = sqlite3_exec(local_scene_db, sql_buf, NULL, NULL, &errMsg);
            if (SQLITE_OK != rc)
            {
                printf("sqlite3_exec errmsg:%s\n", errMsg);
                sqlite3_free(errMsg);
            }
        }
    }
}

static void databse_table_init(void)
{
    char *errMsg = NULL;
    int rc;
    char sql_buf[255];
    for (int i = 0; i < sizeof(tables_name) / sizeof(tables_name[0]); ++i)
    {
        snprintf(sql_buf, sizeof(sql_buf), tables_sql_create[i], tables_name[i]);
        rc = sqlite3_exec(local_scene_db, sql_buf, NULL, NULL, &errMsg);
        if (SQLITE_OK != rc)
        {
            printf("sqlite3_exec errmsg:%s\n", errMsg);
            sqlite3_free(errMsg);
        }
    }
}
//---------------------------------
void databaseClose(void)
{
    sqlite3_close_v2(local_scene_db);
}

void databaseInit(void)
{
    int rc = sqlite3_open_v2(DB_NAME, &local_scene_db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    if (SQLITE_OK != rc)
    {
        printf("sqlite3_open_v2 errmsg:%s\n", sqlite3_errmsg(local_scene_db));
        sqlite3_close_v2(local_scene_db);
    }
    databse_table_init();
}
