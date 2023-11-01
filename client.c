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

#define HOST "34.254.242.81"
#define PORT 8080 
#define CMD_LEN 30
#define BUFFERS_LEN 500




int main(int argc, char *argv[]) {
    char *message;
    char *response;
    int sockfd;
    
    char command[CMD_LEN];
    char cookies[5000];
    // user is not logged in yet
    int logged_in = 0;
    char token[5000] = "";
        
    printf("Welcome to the library!\n");

    while (1) {
        sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

        // getting the command
        fgets(command, CMD_LEN, stdin);
        command[strlen(command) - 1] = '\0';

        // checking what command it is
        if(strcmp(command, "exit") == 0) {
            close_connection(sockfd);
            break;
        }
        else if(strcmp(command, "register") == 0) {
            // username from stdin
            char username[BUFFERS_LEN];
            printf("username=");
            fgets(username, BUFFERS_LEN, stdin);
            username[strlen(username)] = '\0';

            // password from stdin
            char password[BUFFERS_LEN];
            printf("password=");
            fgets(password, BUFFERS_LEN, stdin);
            password[strlen(password)] = '\0';

            // if the username is empty or has spaces 
            if(strlen(username) != 0 && strchr(username, ' ') != NULL) {
                printf("The username you're trying to use is invalid\n");
                continue;
            }

            // if the password is empty or has spaces 
            if(strlen(password) != 0 && strchr(password, ' ') != NULL) {
                printf("The password you're trying to use is invalid\n");
                continue;
            }
            
            // computing the json
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            char *serialized_string = NULL;
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            serialized_string = json_serialize_to_string(value);
            
            // message for server
            message = compute_post_request("34.254.242.81:8080", "/api/v1/tema/auth/register", "application/json", serialized_string, NULL, NULL);
            
            send_to_server(sockfd, message);
            
            // response from server
            response = receive_from_server(sockfd);

            // extract the information from the server response to see if it is ok or if it is an error
            char* response_ok = basic_extract_json_response(response);

            // if there is no error
            if (response_ok == NULL) {
                printf("User registered successfully!\n");
            } else {
                printf("This username is already taken!\n");
            }

            // freeing the strings
            json_free_serialized_string(serialized_string);
            json_value_free(value);
            free(message);
            free(response);
        }
        else if (strcmp(command, "login") == 0) {
            // if the user is logged in
            if(logged_in == 1) {
                printf("You are already logged in");
                continue;
            }

            char username[BUFFERS_LEN];

            printf("username=");

            // username from stdin
            fgets(username, BUFFERS_LEN, stdin);
            username[strlen(username)] = '\0';

            char password[BUFFERS_LEN];

            printf("password=");

            // password from stdin
            fgets(password, BUFFERS_LEN, stdin);
            password[strlen(password)] = '\0';

            // if the username is empty or has spaces 
            if(strlen(username) != 0 && strchr(username, ' ') != NULL) {
                printf("The username you're trying to use is invalid\n");
                continue;
            }

            // if the password is empty or has spaces 
            if(strlen(password) != 0 && strchr(password, ' ') != NULL) {
                printf("The password you're trying to use is invalid\n");
                continue;
            }
                
            // computing the json
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            char *serialized_string = NULL;
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            serialized_string = json_serialize_to_string(value);
            
            // message for server
            message = compute_post_request("34.254.242.81:8080", "/api/v1/tema/auth/login", "application/json", serialized_string, NULL, NULL);
            
            send_to_server(sockfd, message);
            
            // response from server
            response = receive_from_server(sockfd);

            // extract the information from the server response to see if it is ok or if it is an error
            char* response_ok = basic_extract_json_response(response);
            
            // if there is no error
            if (response_ok == NULL) {
                // now the user is logged in
                logged_in = 1;

                // saving the cookie
                strcpy(cookies, get_cookie(response));
                printf("You have successfully logged in!\n");
            } else {
                // entering the wrong credentials
                if(strstr(response, "Credentials")) {
                    printf("Wrong credentials!\n");
                }
                else
                {
                    printf("There is no account with this username!\n");   
                }
            }

            // freeing the strings
            json_free_serialized_string(serialized_string);
            json_value_free(value);
            free(message);
            free(response);
        }
        else if (strcmp(command, "enter_library") == 0) {
            if(logged_in == 0){
                printf("To enter the library you must be logged in!\n");
                continue;
            }
            
            // message for server
            message = compute_get_request("34.254.242.81:8080", "/api/v1/tema/library/access", NULL, cookies, NULL);

            send_to_server(sockfd, message);
            
            // response from server
            response = receive_from_server(sockfd);
            
            // extract the token from the server response
            char* response_ok = basic_extract_json_response(response);
        
            // if there is no token given
            if (response_ok == NULL) {
                printf("No token given!\n");   
            } else {
                // saving the token
                strcpy(token, get_token(response_ok));

                printf("You have successfully entered the library!\n");
            }

            free(message);
            free(response);
        }
        else if (strcmp(command, "get_books") == 0) {
           // checking if the user is logged in
            if(logged_in == 0) {
                printf("You must be logged in to see the books!\n");
                continue;
            }
           
            // checking if the user has entered the library
            if(strcmp(token, "") == 0) {
                printf("You must enter the library to see the books!\n");
                continue;
            }

            // message for server
            message = compute_get_request("34.254.242.81:8080", "/api/v1/tema/library/books", "application/json", cookies, token);

            send_to_server(sockfd, message);
            
            // response from server
            response = receive_from_server(sockfd);
            
            // extract the token from the server response
            char* books = extract_books(response);
            books[strlen(books)] = '\0';

            // checking for empty list
            if(strcmp(books, "[]") == 0) {
                printf("There is no book in the list!\n");
            }
            else {
                printf("These are the books from the library:\n");
                printf("%s\n", books);
            }
            
            free(message);
            free(response);
        }
        else if (strcmp(command, "get_book") == 0) {
            // checking if the user is logged in
            if(logged_in == 0) {
                printf("You must be logged in to see the books!\n");
                continue;
            }
            
            // checking if the user has entered the library
            if(strcmp(token, "") == 0) {
                printf("You must enter the library to see the books!\n");
                continue;
            }

            // id from stdin
            char id[BUFFERS_LEN];
            printf("id=");
            fgets(id, BUFFERS_LEN, stdin);
            id[strlen(id) - 1] = '\0';

            // checking if the id is correctly introduced
            int ok = 1;

            for (int i = 0; i < strlen(id); i++) {
                if (id[i] < 48 || id[i] > 57) {
                    ok = 0;
                }
            }

            if(ok == 0) {
                printf("The id was introduced incorrectly\n");
                continue;
            }

            // create URL
            char url[50] = "/api/v1/tema/library/books";
            strcat(url, "/");
            strcat(url, id);

            // message for server
            message = compute_get_request("34.254.242.81:8080", url, "application/json", cookies, token);

            send_to_server(sockfd, message);
            
            // response from server
            response = receive_from_server(sockfd);

            // extract the information from the server response to see if it is ok or if it is an error
            char* response_ok = basic_extract_json_response(response);
        
            // if there is no book with the given id
            if (strstr(response_ok, "error") != NULL) {
                printf("There is no book with this specific id!\n");   
            } else {
                printf("Here is the book that you have requested:\n");
                printf("%s\n", response_ok);
            }

            free(message);
            free(response);
        }
        else if (strcmp(command, "add_book") == 0) {
            // checking if the user is logged in
            if(logged_in == 0) {
                printf("You must be logged in to add books!\n");
                continue;
            }
            
            // checking if the user has entered the library
            if(strcmp(token, "") == 0) {
                printf("You must enter the library to add books!\n");
                continue;
            }

            // title from stdin
            char title[BUFFERS_LEN];
            printf("title=");
            fgets(title, BUFFERS_LEN, stdin);
            title[strlen(title) - 1] = '\0';
            
            // author from stdin
            char author[BUFFERS_LEN];
            printf("author=");
            fgets(author, BUFFERS_LEN, stdin);
            author[strlen(author) - 1] = '\0';
            
            // genre from stdin
            char genre[BUFFERS_LEN];
            printf("genre=");
            fgets(genre, BUFFERS_LEN, stdin);
            genre[strlen(genre) - 1] = '\0';
            
            // publisher from stdin
            char publisher[BUFFERS_LEN];
            printf("publisher=");
            fgets(publisher, BUFFERS_LEN, stdin);
            publisher[strlen(publisher) - 1] = '\0';
            
            // page count from stdin
            char page_count[BUFFERS_LEN];
            printf("page_count=");
            fgets(page_count, BUFFERS_LEN, stdin);
            page_count[strlen(page_count) - 1] = '\0';
            
            // checking if the page count is correctly introduced
            int ok = 1;

            for (int i = 0; i < strlen(page_count); i++) {
                if (page_count[i] < 48 || page_count[i] > 57) {
                    ok = 0;
                }
            }

            if(ok == 0) {
                printf("The number of pages was introduced incorrectly\n");
                continue;
            }

            // computing the json
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            char *serialized_string = NULL;
            json_object_set_string(object, "title", title);
            json_object_set_string(object, "author", author);
            json_object_set_string(object, "publisher", publisher);
            json_object_set_string(object, "genre", genre);
            json_object_set_string(object, "page_count", page_count);
            serialized_string = json_serialize_to_string(value);
            
            // message for server
            message = compute_post_request("34.254.242.81:8080", "/api/v1/tema/library/books", "application/json", serialized_string, cookies, token);
        
            send_to_server(sockfd, message);
            
            // response from server
            response = receive_from_server(sockfd);

            // extract the information from the server response to see if it is ok or if it is an error
            char* response_ok = basic_extract_json_response(response);
            
            // if there is no error
            if (response_ok == NULL) {
                printf("The book was successfully added!\n");
            }
            else
            {
                printf("Unfortunately the book was not added. Please try again!\n");
            }
            
            // freeing the strings
            json_free_serialized_string(serialized_string);
            json_value_free(value);
            free(message);
            free(response);
        }
        else if (strcmp(command, "delete_book") == 0) {
            // checking if the user is logged in
            if(logged_in == 0) {
                printf("You must be logged in to see the books!\n");
                continue;
            }
            
            // checking if the user has entered the library
            if(strcmp(token, "") == 0) {
                printf("You must enter the library to see the books!\n");
                continue;
            }

            // id from stdin
            char id[BUFFERS_LEN];
            printf("id=");
            fgets(id, BUFFERS_LEN, stdin);
            id[strlen(id) - 1] = '\0';

            // checking if the id is correctly introduced
            int ok = 1;

            for (int i = 0; i < strlen(id); i++) {
                if (id[i] < 48 || id[i] > 57) {
                    ok = 0;
                }
            }

            if(ok == 0) {
                printf("The id was introduced incorrectly\n");
                continue;
            }

            // create URL
            char url[50] = "/api/v1/tema/library/books";
            strcat(url, "/");
            strcat(url, id);

            // message for server
            message = compute_delete_request("34.254.242.81:8080", url, "application/json", cookies, token);

            send_to_server(sockfd, message);
            
            // response from server
            response = receive_from_server(sockfd);

            // if there is no book with the given id
            if (strstr(response, "{\"error\":") != NULL) {
                printf("There is no book with this specific id!\n");   
            } else {
                printf("The book was successfully deleted!\n");
            }

            free(message);
            free(response);
        }
        else if (strcmp(command, "logout") == 0) {
            // checking if the user is logged in
            if(logged_in == 0) {
                printf("You cannot log out if you are not logged in!\n");
                continue;
            }

            // message for server
            message = compute_get_request("34.254.242.81:8080", "/api/v1/tema/auth/logout", "application/json", cookies, token);
        
            send_to_server(sockfd, message);
            
            // response from server
            response = receive_from_server(sockfd);

            // extract the information from the server response to see if it is ok or if it is an error
            char* response_ok = basic_extract_json_response(response);

            // if there is no error
            if(response_ok == NULL) {
                printf("You have successfully logged out!\n");
            }
            else {
                printf("An error occurred while logging out. Please try again!");
            }
            
            logged_in = 0;

            // remove token and cookies
            strcpy(token, "");
            strcpy(cookies, "");

            free(message);
            free(response);

        }
        else if (strcmp(command, "help") == 0) {
            printf("Here are the available commands:\n");
            printf("register\n");
            printf("login\n");
            printf("enter_library\n");
            printf("get_books\n");
            printf("get_book\n");
            printf("add_book\n");
            printf("delete_book\n");
            printf("logout\n");
            printf("exit\n");
        }
        else 
            printf("Invalid command! Enter 'help' to see the available commands!\n");

        close_connection(sockfd);
    }

    return 0;
}
