#ifndef _MORE_CONTROL_H_
#define _MORE_CONTROL_H_

#ifdef __cplusplus
extern "C"
{
#endif

    int add_mod_more_control(const int devid, const char *serviceid, const int serviceid_len, const char *request, char **response, int *response_len);
    int query_more_control(const int devid, const char *serviceid, const int serviceid_len, const char *request, char **response, int *response_len);
    int delete_more_control(const int devid, const char *serviceid, const int serviceid_len, const char *request, char **response, int *response_len);

#ifdef __cplusplus
}
#endif
#endif