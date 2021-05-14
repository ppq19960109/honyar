#ifndef _DATABASE_H_
#define _DATABASE_H_

#ifdef __cplusplus
extern "C"
{
#endif
    void databaseInit(void);
    int databaseClose(void);
    int databseReset(void);

    int table0_insert(const char *devId, const char *modelId);
    int table0_delete(const char *devId);
    int table0_select(const char *src_scene_id, const char *sceneId, int (*select_func)(const char *, const char *, const char *));

    int table1_delete(const char *field_value);
    int table2_delete(const char *field_value);
    int table1_insert(const char *group_id, const char *product_key, const char *device_id, const char *switch_loop);
    int table2_insert(const char *group_id, const char *group_name);
    int table1_select(const char *group_id, void *arg, int (*select_func)(const char *, const char *, const char *, const char *, void *));
    int table2_select(const char *group_id, void *arg, int (*select_func)(const char *, const char *, void *));
#ifdef __cplusplus
}
#endif
#endif