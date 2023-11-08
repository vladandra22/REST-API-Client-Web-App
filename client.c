#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "parson.h"
#include "requests.h"
#include <stdbool.h>

#define HOST "34.254.242.81"
#define PORT 8080

int has_space(const char *str){
    for(int i = 0; i < strlen(str); i++){
        if(str[i] == ' ') {
            return 1; 
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    /* Pointeri folositi pentru compute_post_reqeust si
    compute_get_request. */
    char *message;
    char *response;
    // Buffer in care imi salvez cookie-ul
    char cookie[8000];
    // Pointer pentru token-ul primit
    char *token;
    /* Initializez socket-ul cu -1 si voi reinnoi conexiunea la fiecare
    caz in parte pentru a nu ajunge in situatia de timeout. */
    int sockfd = -1;
    // log = daca userul este online sau nu
    bool log = false;
    // delete = daca dorim sa facem un request pentru a sterge o carte
    bool delete = false;
    // Bufferul pentru comanda primita ca input de la tastatura
    char cmd[21];
    while(1){
        memset(cmd, 0, 21);
        fgets(cmd, 20, stdin);
        cmd[strlen(cmd) - 1] = 0;
        if(strcmp(cmd, "register") == 0){
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                printf("Conexiune esuata. \n");
                continue;
            }
            if(log){
                printf("Un user este deja logat. \n");
                close(sockfd);
                continue;
            }
            // Citim de la tastatura username-ul si parola
            char username[51], password[51];
            printf("%s", "username=");
            memset(username, 0, 51);
            fgets(username, sizeof(username), stdin);
            // strcspn returneazza pozitia primului \n.
            username[strcspn(username, "\n")] = 0;
            if(has_space(username)) {
                printf("Username-ul nu are voie sa contina spatii.\n");
                continue;
            }

            printf("%s", "password=");
            memset(password, 0, 51);
            fgets(password, sizeof(password), stdin);
            password[strcspn(password, "\n")] = 0;

            if(has_space(password)) {
                printf("Parola nu are voie sa contina spatii.\n");
                continue;
            }

            if (strlen(username) == 0 || strlen(password) == 0) {
                printf("Username si parola nu pot fi goale.\n");
                continue;
            }
            // Colectam datele user-ului cu ajutorul parson
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            char *payload = json_serialize_to_string_pretty(value);

            // Trimitem initial cookie = NULL si token = NULL catre request
            message = compute_post_request(HOST,
                "/api/v1/tema/auth/register", 
               "application/json", &payload, 1, NULL, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            if(strstr(response, " taken")){
                printf("Eroare! Username folosit deja de altcineva. \n");
            }
            else {
                puts(response);
            }
            json_free_serialized_string(payload);
            json_value_free(value);
            close(sockfd);
        }
        // Aceeasi implementare ca si la register
        else if(strcmp(cmd, "login") == 0){
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                printf("Conexiune esuata. \n");
                continue;
            }
            if(log){
                printf("Un user este deja logat. \n");
                close(sockfd);
                continue;
            }
            char username[51], password[51];
            printf("%s", "username=");
            memset(username, 0, 51);
            fgets(username, sizeof(username), stdin);
            // strcspn returneazza pozitia primului \n.
            username[strcspn(username, "\n")] = 0;
            if(has_space(username)) {
                printf("Username-ul nu are voie sa contina spatii.\n");
                continue;
            }

            printf("%s", "password=");
            memset(password, 0, 51);
            fgets(password, sizeof(password), stdin);
            password[strcspn(password, "\n")] = 0;

            if(has_space(password)) {
                printf("Parola nu are voie sa contina spatii.\n");
                continue;
            }

            if (strlen(username) == 0 || strlen(password) == 0) {
                printf("Username si parola nu pot fi goale.\n");
                continue;
            }

            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            char *payload = json_serialize_to_string_pretty(value);

            message = compute_post_request(HOST,
            "/api/v1/tema/auth/login",
            "application/json", &payload, 1, NULL, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            if(strstr(response, "Credentials are not good")){
                printf("Eroare! Credentialele nu se potrivesc.\n");
            }
            else if(strstr(response, "No account")){
                printf("Eroare! User-ul nu exista. \n");
            }
            else puts(response);
            json_free_serialized_string(payload);
            json_value_free(value);

            // Extrag cookie din raspuns
            char *aux_cookie = strstr(response, "connect.sid");
            if(aux_cookie == NULL)
                continue;
            char *fin = strstr(aux_cookie, ";");
            size_t cookie_len = fin - aux_cookie;
            strncpy(cookie, aux_cookie, cookie_len);
            // Adaugam terminatorul de sir
            cookie[cookie_len] = '\0'; 
            // Marcam user-ul ca fiind online
            log = true;
            printf("\n\nCookie-ul primit: %s\n\n", cookie);
            close(sockfd);
        }
        else if(strcmp(cmd, "enter_library") == 0){
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                printf("Conexiune esuata. \n");
                continue;
            }
            if(token != NULL && strlen(token) != 0){
                printf("Esti deja in biblioteca. \n");
                continue;
            }
            if(log){
                // Facem request cu un cookie si obtinem un token
                message = compute_get_request(HOST, "/api/v1/tema/library/access", "", cookie, NULL, delete);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                puts(response);
                
                // Extragem token-ul din raspuns
                token = strstr(response, "token");
                token += 8;
                token[strlen(token) - 2] = '\0';
                printf("\n\nToken-ul primit: %s\n\n", token);
                close(sockfd);
            }
            else {
                printf("Nu esti logat! \n");
                continue;
            }
        }
        // Afisam cartile din biblioteca
       else if(strcmp(cmd, "get_books") == 0){
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                printf("Conexiune esuata. \n");
                continue;
            }
            if(log){
                message = compute_get_request(HOST, "/api/v1/tema/library/books", "", cookie, token, delete);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                if(strstr(response, "error") || strlen(response) == 0){
                    printf("Eroare accesare librarie. \n");
                }
                else {
                    puts(response);
                }
                close(sockfd);
            }
            else {
                if(!log)
                    printf("Nu esti logat! \n");
                if(token == NULL || strlen(token) == 0){
                    printf("Nu ai acces la biblioteca! \n");
                }
                continue;
            }
        }
        else if(strcmp(cmd, "get_book") == 0){
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                printf("Conexiune esuata. \n");
                continue;
            }
            if(log){
                /* Primim ca input id-ul unei carti si verificam
                ca acesta sa fie valid. */
                printf("%s", "id=");
                int id;
                int res = scanf("%d", &id);
                if(res != 1 || getchar() != '\n') {
                    printf("ID-ul trebuie sa fie numar valid.\n");
                    continue;
                }
                char addr[60];
                sprintf(addr, "/api/v1/tema/library/books/%d", id);
                message = compute_get_request(HOST, addr, "", cookie, token, delete);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                if(strstr(response, "No book was found")){
                    printf("ID invalid. Cartea nu a fost gasita. \n");
                }
                else if(strstr(response, "error") || strlen(response) == 0){
                    printf("Eroare accesare librarie. \n");
                }
                else puts(response);
                close(sockfd);
            }
            else {
                if(!log)
                    printf("Nu esti logat! \n");
                if(token == NULL || strlen(token) == 0){
                    printf("Nu ai acces la biblioteca! \n");
                }
                close(sockfd);
                continue;
            }
        }
        else if(strcmp(cmd, "add_book") == 0){
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                printf("Conexiune esuata. \n");
                continue;
            }
            if(log && token != NULL){
                char title[100];
                char author[100];
                char genre[100];
                char publisher[100];
                int page_count = 0;
                printf("%s", "title=");
                fgets(title, sizeof(title), stdin);
                // Eliminam whitespace-ul ramas
                title[strcspn(title, "\n")] = '\0'; 
                printf("%s", "author=");
                fgets(author, sizeof(author), stdin);
                author[strcspn(author, "\n")] = '\0';
                printf("%s", "genre=");
                fgets(genre, sizeof(genre), stdin);
                genre[strcspn(genre, "\n")] = '\0';
                printf("%s", "publisher=");
                fgets(publisher, sizeof(publisher), stdin);
                publisher[strcspn(publisher, "\n")] = '\0';
                printf("%s", "page_count=");
                int res = scanf("%d", &page_count);
                /* getchar() elimina newline-ul ramas in buffer
                dupa citirea lui page_count */
                if(strlen(title) == 0 || strlen(author) == 0 || strlen(genre) == 0 ||
                strlen(publisher) == 0 || (res != 1 || getchar() != '\n')){
                    printf("Format gresit!\n");
                    continue;
                }

                char page_count_str[10];
                sprintf(page_count_str, "%d", page_count);

                // Extragem datele necesare si le adaugam in payload
                JSON_Value *value = json_value_init_object();
				JSON_Object *object = json_value_get_object(value);
				json_object_set_string(object, "title", title);
				json_object_set_string(object, "author", author);
				json_object_set_string(object, "genre", genre);
                json_object_set_string(object, "page_count", page_count_str);
                json_object_set_string(object, "publisher", publisher);
                char *payload = json_serialize_to_string_pretty(value);
                
                // Generam request-ul
                message = compute_post_request(HOST,
                "/api/v1/tema/library/books", "application/json", &payload, 1, cookie, token);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                if(strstr(response, "error")){
                    printf("Informatii incorecte.\n");
                }
                else puts(response);
                json_free_serialized_string(payload);
                json_value_free(value);
                close(sockfd);
            }
            else {
                if(!log)
                    printf("Nu esti logat! \n");
                if(token == NULL || strlen(token) == 0){
                    printf("Nu ai acces la biblioteca! \n");
                }
                continue;
            }
        }
        else if(strcmp(cmd, "delete_book") == 0){
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                printf("Conexiune esuata. \n");
                continue;
            }
            if(log && token != NULL){
                printf("%s", "id=");
                int id;
                int res = scanf("%d", &id);
                if(res != 1 || getchar() != '\n') {
                    printf("ID-ul trebuie sa fie numar valid.\n");
                    continue;
                }
                char addr[60];
                sprintf(addr, "/api/v1/tema/library/books/%d", id);
                /* De data asta, vom face un DELETE request, pentru care nu am
                mai facut o functie auxiliara, ci doar am marcat un camp auxiliar
                pentru a schimba metoda, restul abordarii fiind identica. */
                delete = true;
                message = compute_get_request(HOST, addr, "", cookie, token, delete);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                if(strstr(response, "No book was found")){
                    printf("ID invalid. Cartea nu a fost gasita, nu poate fi stearsa. \n");
                }
                else puts(response);
                printf("\nCartea a fost stearsa cu succes. \n\n");
                // Resetam campul
                delete = false;
                close(sockfd);
            }
            else {
                if(!log)
                    printf("Nu esti logat! \n");
                if(token == NULL || strlen(token) == 0)
                    printf("Nu ai acces la biblioteca! \n");
                continue;
            }
        }
        else if(strcmp(cmd, "logout") == 0){
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                printf("Conexiune esuata. \n");
                continue;
            }
            if(log){
                message = compute_get_request(HOST, "/api/v1/tema/auth/logout", "", cookie, token, delete);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                puts(response);
                // Eliberam campurile folosite
                log = false;
                memset(cookie, 0, 8000);
                token = "";
                close(sockfd);
            }
            else {
                printf("Nu esti conectat. \n");
                continue;
            }
        }
        else if(strcmp(cmd, "exit") == 0){
            break;
        }
        
    }
    return 0;
}
