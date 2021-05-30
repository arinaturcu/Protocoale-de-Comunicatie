#include <nlohmann/json.hpp>
#include <bits/stdc++.h>
#include <iostream>
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

using json = nlohmann::ordered_json;
using namespace std;

#define PORT 8080

class Handler {
 private:
    char host[14] = "34.118.48.238";

    char content_type[17] = "application/json";
    char register_link[27] = "/api/v1/tema/auth/register";
    char login_link[24] = "/api/v1/tema/auth/login";
    char enter_library_link[28] = "/api/v1/tema/library/access";
    char books_link[27] = "/api/v1/tema/library/books";
    char logout_link[25] = "/api/v1/tema/auth/logout";

    char **cookies;
    char *token_access;

 public:
    Handler() {
        this->cookies = NULL;
        this->token_access = NULL;
    }

    char *take_cookie(char *response) {
        char *start;
        char *end;

        start = response;

        while (1) {
            if (strncmp(start, "Set-Cookie: ", 12) == 0) {
                start = start + 12;
                end = start;

                while (*end != ';') {
                    end++;
                }

                break;
            }

            start++;
        }
        
        *end = '\0';
        return start;
    }

    void handle_auth(char *auth_type) {
        char *message;
        char *response;
        char buffer[BUFLEN];
        json credentials;

        int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

        printf("username=");
        memset(buffer, 0, BUFLEN);
        fgets(buffer, BUFLEN, stdin);

        buffer[strlen(buffer) - 1] = '\0';
        credentials["username"] = buffer;

        printf("password=");
        memset(buffer, 0, BUFLEN);
        fgets(buffer, BUFLEN, stdin);

        buffer[strlen(buffer) - 1] = '\0';
        credentials["password"] = buffer;

        char *data = strdup(credentials.dump().c_str());
        if (strcmp(auth_type, "register") == 0) {
            message = compute_post_request(host, register_link, content_type, data, NULL, 0, NULL);
        }

        if (strcmp(auth_type, "login") == 0) {
            message = compute_post_request(host, login_link, content_type, data, NULL, 0, NULL);
        }

        printf("REQUEST AUTH:\n%s\n", message);

        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        printf("RESPONSE AUTH:\n%s\n", response);

        if (strcmp(auth_type, "login") == 0 && strlen(response) != 0) {
            cookies = (char **)malloc(sizeof(char*));
            cookies[0] = (char *)malloc(BUFLEN * sizeof(char));
            strcpy(cookies[0], take_cookie(response));
        }

        free(message);
        free(response);
        free(data);
    }

    void handle_enter_library() {
        char *message;
        char *response;

        int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

        message = compute_get_request(host, enter_library_link, NULL, cookies, 1, NULL);
        printf("REQUEST:\n%s\n", message);

        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        printf("RESPONSE:\n%s\n", response);

        json data;
        char *body_data = strstr(response, "{");
        data = json::parse(string(body_data));

        if (data.find("token") != data.end()) {
            token_access = strdup(data["token"].dump().c_str() + 1);
            token_access[strlen(token_access) - 1] = '\0';
        }

        free(message);
        free(response);
    }

    void handle_get_books() {
        char *message;
        char *response;

        int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

        message = compute_get_request(host, books_link, NULL, NULL, 0, token_access);
        printf("REQUEST:\n%s\n", message);

        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        printf("RESPONSE:\n%s\n", response);

        free(message);
        free(response);
    }

    void handle_get_book() {
        char *message;
        char *response;

        cout << "id=";
        char *id = (char *)malloc(BUFLEN * sizeof(char));
        fgets(id, BUFLEN, stdin);
        id[strlen(id) - 1] = '\0';

        int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

        char *get_book_link = (char *) malloc(BUFLEN * sizeof(char));
        strcpy(get_book_link, books_link);
        strcat(get_book_link, "/");
        strcat(get_book_link, id);

        message = compute_get_request(host, get_book_link, NULL, NULL, 0, token_access);
        printf("REQUEST:\n%s\n", message);

        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        printf("RESPONSE:\n%s\n", response);

        free(id);
        free(get_book_link);
        free(message);
        free(response);
    }

    void handle_add_book() {
        char *message;
        char *response;

        json book;
        string buffer;

        cout << "title=";
        getline(cin, buffer);
        book["title"] = buffer;

        cout << "author=";
        getline(cin, buffer);
        book["author"] = buffer;

        cout << "genre=";
        getline(cin, buffer);
        book["genre"] = buffer;

        cout << "page_count=";
        getline(cin, buffer);
        book["page_count"] = buffer;

        cout << "publisher=";
        getline(cin, buffer);
        book["publisher"] = buffer;

        int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

        char *data = strdup(book.dump().c_str());
        message = compute_post_request(host, books_link, content_type, data, NULL, 0, token_access);
        printf("REQUEST:\n%s\n", message);

        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        printf("RESPONSE:\n%s\n", response);

        free(message);
        free(response);
        free(data);
    }

    void handle_logout() {
        char *message;
        char *response;

        int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

        if (cookies == NULL) return;

        message = compute_get_request(host, logout_link, NULL, cookies, 1, NULL);
        printf("REQUEST:\n%s\n", message);

        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        printf("RESPONSE:\n%s\n", response);

        free(message);
        free(response);

        free(cookies[0]);
        free(cookies);
        free(token_access);

        cookies = NULL;
        token_access = NULL;
    }

    void handle_delete_book() {
        char *message;
        char *response;

        cout << "id=";
        char *id = (char *)malloc(BUFLEN * sizeof(char));
        fgets(id, BUFLEN, stdin);
        id[strlen(id) - 1] = '\0';

        int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

        char *get_book_link = (char *) malloc(BUFLEN * sizeof(char));
        strcpy(get_book_link, books_link);
        strcat(get_book_link, "/");
        strcat(get_book_link, id);

        message = compute_delete_request(host, get_book_link, NULL, NULL, 0, token_access);
        printf("REQUEST:\n%s\n", message);

        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        printf("RESPONSE:\n%s\n", response);

        free(id);
        free(get_book_link);
        free(message);
        free(response);
    }

    void handle_exit(int sockfd) {
        if (cookies != NULL) {
            free(cookies[0]);
            free(cookies);
        }

        if (token_access != NULL) {
            free(token_access);
        }

        close_connection(sockfd);
        exit(0);
    }
};

int main(int argc, char *argv[]) {
    char host[] = "34.118.48.238";
    char *buffer;
    int sockfd;

    sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);
    Handler h = Handler();

    buffer = (char *) calloc(BUFLEN, sizeof(char));

    while (fgets(buffer, BUFLEN, stdin) != NULL) {
        buffer[strlen(buffer) - 1] = '\0';

        if (strncmp(buffer, "register", 9) == 0 || strncmp(buffer, "login", 6) == 0) {
            h.handle_auth(buffer);
            continue;
        }

        if (strncmp(buffer, "logout", 7) == 0) {
            h.handle_logout();
            continue;
        }

        if (strncmp(buffer, "enter_library", 14) == 0) {
            h.handle_enter_library();
            continue;
        }

        if (strncmp(buffer, "get_books", 10) == 0) {
            h.handle_get_books();
            continue;
        }

        if (strncmp(buffer, "get_book", 9) == 0) {
            h.handle_get_book();
            continue;
        }

        if (strncmp(buffer, "add_book", 9) == 0) {
            h.handle_add_book();
            continue;
        }

        if (strncmp(buffer, "delete_book", 12) == 0) {
            h.handle_delete_book();
            continue;
        }

        if (strncmp(buffer, "exit", 5) == 0) {
            free(buffer);
            h.handle_exit(sockfd);
            continue;
        }

        cout << "Invalid command!\n";
    }

    close_connection(sockfd);

    return 0;
}
