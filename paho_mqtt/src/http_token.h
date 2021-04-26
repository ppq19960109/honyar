#ifndef __HTTP_TOKEN_H_
#define __HTTP_TOKEN_H_

int curl_http_get_token(const char *clientId, const char *mac, char *passwd);
#endif