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

#define MSG_LEN 8192

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

    sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
        
    // Ex 1.1: GET dummy from main server
    message = compute_get_request("34.118.48.238", "/api/v1/dummy", NULL, NULL, 0);
    printf("REQUEST1:\n%s\n", message);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("RESPONSE1:\n%s\n", response);

    free(message);
    free(response);

    // Ex 1.2: POST dummy and print response from main server
    char **data = (char**)calloc(3, sizeof(char *));

    for (int i = 0; i < 3; ++i) {
        data[i] = calloc(50, sizeof(char));
        strcpy(data[i], "chestii d-alea cringe ca la restul laboratoarelor");
    }

    message = compute_post_request("34.118.48.238", "/api/v1/dummy",
        "application/x-www-form-urlencoded", data, 3, NULL, 0);
    printf("REQUEST2:\n%s\n", message);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("RESPONSE2:\n%s\n", response);

    for (int i = 0; i < 3; i++) {
        free(data[i]);
    }
    
    free(data);

    // Ex 2: Login into main server
    data = (char**)calloc(2, sizeof(char*));
    data[0] = calloc(50, sizeof(char));
    data[1] = calloc(50, sizeof(char));
    strcpy(data[0], "username=student");
    strcpy(data[1], "password=student");

    message = compute_post_request("34.118.48.238", "/api/v1/auth/login",
            "application/x-www-form-urlencoded", data, 2, NULL, 0);
    printf("REQUEST3:\n%s\n", message);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("RESPONSE3:\n%s\n", response);

    free(data[0]);
    free(data[1]);
    free(data);

    // Ex 3: GET weather key from main server
    char **cookies = calloc(1, sizeof(char*));
    cookies[0] = calloc(300, sizeof(char));
    strcpy(cookies[0], "connect.sid=s%3AQhIcHWww-M-c7XHOqgd0-3mBtNmR34YE.dYODhoPYF5uEHhQRfoKl9HnWMpUS5OfUOO0zohXKUgI");

    message = compute_get_request("34.118.48.238", "/api/v1/weather/key", NULL,
            cookies, 1);
    printf("REQUEST4:\n%s\n", message);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("RESPONSE4:\n%s\n", response);

    free(message);
    free(response);
    // Ex 4: GET weather data from OpenWeather API
    message = compute_get_request("34.118.48.238", "/api/v1/auth/logout", NULL,
            NULL, 0);
    printf("REQUEST5:\n%s\n", message);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    printf("RESPONSE5:\n%s\n", response);

    free(message);
    free(response);

    // Ex 5: POST weather data for verification to main server
    // Ex 6: Logout from main server

    // BONUS: make the main server return "Already logged in!"

    // free the allocated data at the end!

    close_connection(sockfd);

    return 0;
}
