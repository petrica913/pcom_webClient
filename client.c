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
#include "parson.h"

char *serialize_string(char *username, char *password)
{
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    char *serialized_string = json_serialize_to_string(root_value);
    return serialized_string;
}

void add_book_error(char title[100], char author[100], char genre[100], char publisher[100], int page_count)
{
    if (strcmp(title, "") == 0) {
        printf("ERROR: Invalid title.\n\n");
    }
    if (strcmp(author, "") == 0) {
        printf("ERROR: Invalid author.\n\n");
    }
    if (strcmp(genre, "") == 0) {
        printf("ERROR: Invalid genre.\n\n");
    }
    if (strcmp(publisher, "") == 0) {
        printf("ERROR: Invalid publisher.\n\n");
    }
    if (page_count < 0) {
        printf("ERROR: Invalid page count.\n\n");
    }
}

void read_add_book(char title[100], char author[100], char genre[100], char publisher[100], int *page_count)
{
    printf("title=");
    fgets(title, 100, stdin);
    title[strlen(title) - 1] = '\0';

    printf("author=");
    fgets(author, 100, stdin);
    author[strlen(author) - 1] = '\0';
    
    printf("genre=");
    fgets(genre, 100, stdin);
    genre[strlen(genre) - 1] = '\0';

    printf("publisher=");
    fgets(publisher, 100, stdin);
    publisher[strlen(publisher) - 1] = '\0';

    printf("page_count=");
    scanf("%d", page_count);
    getchar();
}

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    char *cookies;
    char *token;
    int sockfd;
    int logged_in = 0;
    int has_token = 0;
    char *input[COMMAND_LEN];
    int ok_command = 0;
    char username[USR_PASSWD_LEN];
    char password[USR_PASSWD_LEN];


    while (1) {
        ok_command = 0;
        sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
        DIE(sockfd < 0, "open_connection");

        // reads command from user
        memset(input, 0, COMMAND_LEN * sizeof(char));
        char input[COMMAND_LEN];
        fgets(input, COMMAND_LEN, stdin);
        input[strlen(input) - 1] = '\0';

        if (strcmp(input, "register") == 0) {
            if (logged_in == 1) {
                printf("ERROR: You are already logged in.\n\n");
                close(sockfd);
                continue;
            }

            printf("username=");
            fgets(username, USR_PASSWD_LEN, stdin);
            username[strlen(username) - 1] = '\0';

            printf("password=");
            fgets(password, USR_PASSWD_LEN, stdin);
            password[strlen(password) - 1] = '\0';
            
            char *serialized_string = serialize_string(username, password);

            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            message = compute_post_request(IP_SERVER, "/api/v1/tema/auth/register", "application/json", &serialized_string, 1, NULL, 0, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if (strstr(response, "201 Created") != NULL) {
                printf("SUCCESS: User registered.\n\n");
            } else {
                printf("ERROR: User already exists.\n\n");
            }
            ok_command = 1;
            close(sockfd);
            free(message);
            free(response);
            continue;
        }

        if (strcmp(input, "login") == 0) {
            if (logged_in == 1) {
                printf("ERROR: You are already logged in.\n\n");
                close(sockfd);
                continue;
            }

            printf("username=");
            fgets(username, USR_PASSWD_LEN, stdin);
            username[strlen(username) - 1] = '\0';

            printf("password=");
            fgets(password, USR_PASSWD_LEN, stdin);
            password[strlen(password) - 1] = '\0';
            
            char *serialized_string = serialize_string(username, password);

            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            message = compute_post_request(IP_SERVER, "/api/v1/tema/auth/login", "application/json", &serialized_string, 1, NULL, 0, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if (strstr(response, "200 OK") != NULL) {
                printf("SUCCESS: User logged in succesfuly.\n\n");
                logged_in = 1;
                // get cookies from response
                cookies = get_cookies(response);
            } else {
                logged_in = 0;
                printf("ERROR: Bad credentials.\n\n");
            }
            ok_command = 1;
            close(sockfd);
            free(message);
            free(response);
            continue;
        }
        
        if (strcmp(input, "enter_library") == 0) {
            if (logged_in == 0) {
                printf("ERROR: You are not logged in.\n\n");
                close(sockfd);
                ok_command = 1;
                continue;
            }
            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request(IP_SERVER, "/api/v1/tema/library/access", NULL, &cookies, 1, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if (strstr(response, "200 OK") != NULL) {
                printf("SUCCESS: You have access to the library.\n\n");
                // get token from response
                token = get_token(response);
                has_token = 1;
            } else {
                printf("ERROR: You do not have access to the library.\n\n");
            }
            close(sockfd);
            free(message);
            free(response);
            ok_command = 1;
            continue;
        }

        if (strcmp(input, "get_books") == 0) {
            // printf("Get books\n");
            if (logged_in == 0) {
                printf("ERROR: You are not logged in.\n\n");
                close(sockfd);
                ok_command = 1;
                continue;
            }
            if (has_token == 0) {
                printf("ERROR: You do not have access to the library.\n\n");
                close(sockfd);
                ok_command = 1;
                continue;
            }
            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request(IP_SERVER, "/api/v1/tema/library/books", NULL, NULL, 0, token);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            // printf("%s\n", response);

            if (strstr(response, "200 OK") != NULL) {
                char *json_start = strstr(response, "[");
                // printf("%s\n", json_start);
                JSON_Value *root_value = json_parse_string(json_start); 
                JSON_Array *books = json_value_get_array(root_value);
                printf("%ld\n", json_array_get_count(books));
                printf("[\n");
                for (int i = 0; i < json_array_get_count(books); i++) {
                    JSON_Object *book = json_array_get_object(books, i);
                    if (book == NULL) {
                        continue;
                    }
                    int id = (int)json_object_get_number(book, "id");
                    const char *title = json_object_get_string(book, "title");
                    printf("    {\n");
                    printf("    id: %d,\n", id);
                    printf("    title: \"%s\"\n", title);
                    if (i < json_array_get_count(books) - 1) {
                        printf("    },\n");
                    } else {
                        printf("    }\n");
                    }
                }
                printf("]\n");
            } else {
                printf("ERROR: Books not received.\n\n");
            }
            close(sockfd);
            free(message);
            free(response);
            ok_command = 1;
            continue;
        }

        if (strcmp(input, "get_book") == 0) {
            // printf("Get book\n");
            if (logged_in == 0) {
                printf("ERROR: You are not logged in.\n\n");
                close(sockfd);
                ok_command = 1;
                continue;
            }
            if (has_token == 0) {
                printf("ERROR: You do not have access to the library.\n\n");
                close(sockfd);
                ok_command = 1;
                continue;
            }
            printf("id=");
            char id[10];
            fgets(id, 10, stdin);
            id[strlen(id) - 1] = '\0';

            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            if (id == NULL) {
                printf("ERROR: Invalid id.\n\n");
                close(sockfd);
                ok_command = 1;
                continue;
            }

            char endpoint[100];
            sprintf(endpoint, "/api/v1/tema/library/books/%s", id);

            message = compute_get_request(IP_SERVER, endpoint, id, NULL, 0, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            // printf("%s\n", response);
            char *json_start = basic_extract_json_response(response);
            // printf ("%s\n", json_start);
            char *code = strchr(response, ' ') + 1;
            int status_code = atoi(code);
            if (status_code < 200 || status_code >= 300) {
                printf("ERROR: Bad status code.\n\n");
                close(sockfd);
                free(message);
                free(response);
                ok_command = 1;
                continue;
            }
            
            if (json_start != NULL) {
                JSON_Value *root_value = json_parse_string(json_start);
                JSON_Object *book = json_value_get_object(root_value);
                const char *title = json_object_get_string(book, "title");
                const char *author = json_object_get_string(book, "author");
                const char *publisher = json_object_get_string(book, "publisher");
                const char *genre = json_object_get_string(book, "genre");
                const int page_count = (int)json_object_get_number(book, "page_count");
                const int id = (int)json_object_get_number(book, "id");
                printf("id: %d\n", id);
                printf("title: %s\n", json_object_get_string(book, "title"));
                printf("author: %s\n", json_object_get_string(book, "author"));
                printf("publisher: %s\n", json_object_get_string(book, "publisher"));
                printf("genre: %s\n", json_object_get_string(book, "genre"));
                printf("page_count: %d\n", page_count);

            } else {
                printf("ERROR: Book with id: %s doesn't exist.\n\n", id);
            }

            close(sockfd);
            free(message);
            free(response);
            ok_command = 1;
            continue;
        }

        if (strcmp(input, "add_book") == 0) {
            if (logged_in == 0) {
                printf("ERROR: You are not logged in.\n\n");
                close(sockfd);
                ok_command = 1;
                continue;
            }
            if (has_token == 0) {
                printf("ERROR: You do not have access to the library.\n\n");
                close(sockfd);
                ok_command = 1;
                continue;
            }
            char title[100];
            char author[100];
            char genre[100];
            char publisher[100];
            int page_count;

            read_add_book(title, author, genre, publisher, &page_count);

            sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
            if (strcmp(title, "") == 0 || strcmp(author, "") == 0 || strcmp(genre, "") == 0 || strcmp(publisher, "") == 0 || page_count < 0) {
                add_book_error(title, author, genre, publisher, page_count);
                close(sockfd);
                ok_command = 1;
                continue;
            }
            
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);

            json_object_set_string(root_object, "title", title);
            json_object_set_string(root_object, "author", author);
            json_object_set_string(root_object, "genre", genre);
            json_object_set_string(root_object, "publisher", publisher);
            json_object_set_number(root_object, "page_count", page_count);

            char *serialized_string = json_serialize_to_string(root_value);

            message = compute_post_request(IP_SERVER, "/api/v1/tema/library/books", "application/json", &serialized_string, 1, NULL, 0, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            printf("%s\n", response);

            if (strstr(response, "error") == NULL) {
                printf("SUCCESS: Book added.\n\n");
            } else {
                printf("ERROR: Book not added.\n\n");
            }
            close(sockfd);
            free(message);
            free(response);
            json_free_serialized_string(serialized_string);
            json_value_free(root_value);
            ok_command = 1;
            continue;
        }

        if (strcmp(input, "exit") == 0) {
            printf("Exit\n");
            close(sockfd);
            exit(0);
            break;
        }
        // if the command is wrong, print an error message
        if (ok_command == 0) {
            printf("ERROR: That command does not exit.\n");
            printf("Available commands: register, login, enter_library, get_books, ");
            printf("get_book, add_book, delete_book, logout, exit.\n\n");
            close(sockfd);
            continue;
        }
    }

    return 0;
}
