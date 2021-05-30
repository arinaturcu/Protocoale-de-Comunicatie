#include <nlohmann/json.hpp>
#include <bits/stdc++.h>
#include <iostream>
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
    char register_path[27] = "/api/v1/tema/auth/register";
    char login_path[24] = "/api/v1/tema/auth/login";
    char enter_library_path[28] = "/api/v1/tema/library/access";
    char books_path[27] = "/api/v1/tema/library/books";
    char logout_path[25] = "/api/v1/tema/auth/logout";

    char **cookies;
    char *token_access;

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

 public:
    Handler() {
        this->cookies = NULL;
        this->token_access = NULL;
    }

    void handle_auth(string auth_type) {
        char *message;
        char *response;
        string buffer;
        json credentials;
        json response_json;

        int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

        cout << "username=";
        getline(cin, buffer);
        credentials["username"] = buffer;

        cout << "password=";
        getline(cin, buffer);
        credentials["password"] = buffer;

        char *data = strdup(credentials.dump().c_str());
        if (auth_type == "register") {
            message = compute_post_request(host, register_path, content_type, data, NULL, 0, NULL);
        }

        if (auth_type == "login") {
            if (cookies != NULL) {
                free(cookies[0]);
                free(cookies);
                cookies = NULL;
            }

            if (token_access != NULL) {
                free(token_access);
                token_access = NULL;
            }

            message = compute_post_request(host, login_path, content_type, data, NULL, 0, NULL);
        }

        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);

        char *response_data = strstr(response, "{");
        if (response_data != NULL) response_json = json::parse(string(response_data));

        if (response_json.find("error") != response_json.end()) {
            cout << "\nError: " << response_json["error"] << endl << endl;

            free(message);
            free(response);
            free(data);
            return;
        }

        response_data = strstr(response, " ");
        char *fine = strstr(response_data, "\n");
        *fine = '\0';

        cout << "\nRESPONSE:" << response_data << endl << endl;

        if (auth_type == "login" && strlen(response) != 0) {
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

        message = compute_get_request(host, enter_library_path, NULL, cookies, 1, NULL);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);

        json data;
        char *body_data = strstr(response, "{");
        data = json::parse(string(body_data));

        char *response_data = strstr(response, " ");
        char *fine = strstr(response_data, "\n");
        *fine = '\0';

        cout << "\nRESPONSE:" << response_data << endl << endl;

        if (data.find("token") != data.end()) {
            token_access = strdup(data["token"].dump().c_str() + 1);
            token_access[strlen(token_access) - 1] = '\0';
        } else if (data.find("error") != data.end()) {
            cout << "\nError: " << data["error"] << endl << endl;
        }

        free(message);
        free(response);
    }

    void handle_get_books() {
        char *message;
        char *response;

        int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

        message = compute_get_request(host, books_path, NULL, NULL, 0, token_access);

        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);

        json data;
        char *body_data = strstr(response, "[");
        if (body_data == NULL) body_data = strstr(response, "{");

        data = json::parse(string(body_data));

        char *response_data = strstr(response, " ");
        char *fine = strstr(response_data, "\n");
        *fine = '\0';

        cout << "\nRESPONSE:" << response_data << endl;

        if (data.find("error") != data.end()) {
            cout << "Error: " << data["error"] << endl << endl;
        } else {
            cout << "Array of books:\n[";

            for (auto book : data) {
                cout << "\n\tid: " << book["id"] << ", ";
                cout << "title: " << book["title"];
            }

            if (data.size() != 0) cout << endl; else cout << "Empty";
            cout << "]" << endl << endl;
        }

        free(message);
        free(response);
    }

    void handle_get_book() {
        char *message;
        char *response;

        cout << "id=";
        string id;
        getline(cin, id);

        int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

        char *get_book_path = (char *) malloc(BUFLEN * sizeof(char));
        strcpy(get_book_path, books_path);
        strcat(get_book_path, "/");
        strcat(get_book_path, id.c_str());

        message = compute_get_request(host, get_book_path, NULL, NULL, 0, token_access);
        send_to_server(sockfd, message);

        response = receive_from_server(sockfd);

        json data;
        char *body_data = strstr(response, "[");
        if (body_data == NULL) body_data = strstr(response, "{");

        data = json::parse(string(body_data));

        char *response_data = strstr(response, " ");
        char *fine = strstr(response_data, "\n");
        *fine = '\0';

        cout << "\nRESPONSE:" << response_data << endl;

        if (data.find("error") != data.end()) {
            cout << "Error: " << data["error"] << endl << endl;
        } else {
            auto book = data[0];
            cout << "Book with id "  << id << ":\n";
            cout << "\ttitle: "      << book["title"]      << endl;
            cout << "\tauthor: "     << book["author"]     << endl;
            cout << "\tpublisher: "  << book["publisher"]  << endl;
            cout << "\tgenre: "      << book["genre"]      << endl;
            cout << "\tpage_count: " << book["page_count"] << endl << endl;
        }

        free(get_book_path);
        free(message);
        free(response);
    }

    void handle_add_book() {
        char *message;
        char *response;

        json book;
        json response_json;
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
        message = compute_post_request(host, books_path, content_type, data, NULL, 0, token_access);

        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);

        char *response_data = strstr(response, "{");
        if (response_data != NULL) response_json = json::parse(string(response_data));

        if (response_json.find("error") != response_json.end()) {
            cout << "\nError: " << response_json["error"] << endl << endl;
        } else {
            response_data = strstr(response, " ");
            char *fine = strstr(response_data, "\n");
            *fine = '\0';

            cout << "\nRESPONSE:" << response_data << endl << endl;
        }

        free(message);
        free(response);
        free(data);
    }

    void handle_delete_book() {
        char *message;
        char *response;
        json response_json;

        cout << "id=";
        string id;
        getline(cin, id);

        int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

        char *get_book_path = (char *) malloc(BUFLEN * sizeof(char));
        strcpy(get_book_path, books_path);
        strcat(get_book_path, "/");
        strcat(get_book_path, id.c_str());

        message = compute_delete_request(host, get_book_path, NULL, NULL, 0, token_access);
        send_to_server(sockfd, message);

        response = receive_from_server(sockfd);

        char *response_data = strstr(response, "{");
        if (response_data != NULL) response_json = json::parse(string(response_data));

        if (response_json.find("error") != response_json.end()) {
            cout << "\nError: " << response_json["error"] << endl << endl;
        } else {
            response_data = strstr(response, " ");
            char *fine = strstr(response_data, "\n");
            *fine = '\0';

            cout << "\nRESPONSE:" << response_data << endl << endl;
        }

        free(get_book_path);
        free(message);
        free(response);
    }

    void handle_logout() {
        char *message;
        char *response;
        json response_json;

        int sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);

        message = compute_get_request(host, logout_path, NULL, cookies, 1, NULL);
        send_to_server(sockfd, message);

        response = receive_from_server(sockfd);

        char *response_data = strstr(response, "{");
        if (response_data != NULL) response_json = json::parse(string(response_data));

        if (response_json.find("error") != response_json.end()) {
            cout << "\nError: " << response_json["error"] << endl << endl;
        } else {
            response_data = strstr(response, " ");
            char *fine = strstr(response_data, "\n");
            *fine = '\0';

            cout << "\nRESPONSE:" << response_data << endl << endl;
        }

        free(message);
        free(response);

        if (cookies != NULL) {
            free(cookies[0]);
            free(cookies);
        }
        if (token_access != NULL) {
            free(token_access);
        }

        cookies = NULL;
        token_access = NULL;
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
    int sockfd;

    sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);
    Handler h = Handler();

    string buffer;

    while (getline(cin, buffer)) {
        buffer.erase(0, buffer.find_first_not_of(" \t\n\r\f\v"));
	    buffer.erase(buffer.find_last_not_of(" \t\n\r\f\v") + 1);

        if (buffer == "register" || buffer == "login") {
            h.handle_auth(buffer);
            continue;
        }

        if (buffer == "logout") {
            h.handle_logout();
            continue;
        }

        if (buffer == "enter_library") {
            h.handle_enter_library();
            continue;
        }

        if (buffer == "get_books") {
            h.handle_get_books();
            continue;
        }

        if (buffer == "get_book") {
            h.handle_get_book();
            continue;
        }

        if (buffer == "add_book") {
            h.handle_add_book();
            continue;
        }

        if (buffer == "delete_book") {
            h.handle_delete_book();
            continue;
        }

        if (buffer == "exit") {
            h.handle_exit(sockfd);
            continue;
        }

        cout << "Invalid command!\n";
    }

    close_connection(sockfd);

    return 0;
}
