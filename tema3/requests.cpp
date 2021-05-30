#include <stdlib.h> /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                          char **cookies, int cookies_count, char* authorization) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add headers and/or cookies, according to the protocol format

    // add authorization tocken
    if (authorization != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Authorization: Bearer ");
        strcat(line, authorization);
        compute_message(message, line);
    }

    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");

        for (int i = 0; i < cookies_count - 1; i++) {
            strcat(line, cookies[i]);
            strcat(line, ";");
        }

        strcat(line, cookies[cookies_count - 1]);
        compute_message(message, line);
    }

    // add final new line
    compute_message(message, "");

    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char *content_type, char *body_data,
                           char **cookies, int cookies_count,
                           char *authorization) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add necessary headers 
    // in order to write Content-Length you must first compute the message size
   	 
    memset(line, 0, LINELEN);
	sprintf(line, "Content-Type: %s", content_type);
	compute_message(message, line);

   	int body_data_size = strlen(body_data);

    memset(line, 0, LINELEN);
	sprintf(line, "Content-Length: %d", body_data_size);
	compute_message(message, line);

    // add authorization tocken
    if (authorization != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Authorization: Bearer ");
        strcat(line, authorization);
        compute_message(message, line);
    }

    // add cookies
    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");

        for (int i = 0; i < cookies_count - 1; i++) {
            strcat(line, cookies[i]);
            strcat(line, ";");
        }

        strcat(line, cookies[cookies_count - 1]);
        compute_message(message, line);
    }

    // add new line at end of header
	compute_message(message, "");

    // add the actual payload data
	strcat(message, body_data);

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *query_params,
                          char **cookies, int cookies_count, char* authorization) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // add the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // add headers and/or cookies, according to the protocol format

    // add authorization tocken
    if (authorization != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Authorization: Bearer ");
        strcat(line, authorization);
        compute_message(message, line);
    }

    if (cookies != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");

        for (int i = 0; i < cookies_count - 1; i++) {
            strcat(line, cookies[i]);
            strcat(line, ";");
        }

        strcat(line, cookies[cookies_count - 1]);
        compute_message(message, line);
    }

    // add final new line
    compute_message(message, "");

    free(line);
    return message;
}
