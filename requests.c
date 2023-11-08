#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params, 
                            char *cookie, char *token, bool delete)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    /* Scriem numele metodei, URL, request params (daca exista) si 
     tipul protocolului. Tratam in functie de situatie, deoarece la
     delete_book vom avea nevoie sa trimitem DELETE. */
    if(delete == false){
        if (query_params != NULL)
            sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
        else
            sprintf(line, "GET %s HTTP/1.1", url);
    }
    else {
        if (query_params != NULL)
            sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
        else
            sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    
    compute_message(message, line);

    // Adaugam host-ul
    if (host != NULL) {
        sprintf(line, "Host: %s", host);
    }
    compute_message(message, line);

    /* Adaugam cookie-ul si token-ul ca si headere necesare,
    respectand tipul protocolului. */
    if (cookie != NULL) {
            sprintf(line, "Cookie: %s", cookie);
            compute_message(message, line);
    }

    if(token != NULL){
            sprintf(line, "Authorization: Bearer %s", token);
            compute_message(message, line);
    }
    // Adaugam newline-ul final
    compute_message(message, "");
    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char *cookie, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // Numele metodei, URL si protocol
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Adaugam host-ul
    if (host != NULL) {
        sprintf(line, "Host: %s", host);
    }
    compute_message(message, line);

    /* Adaugam Content-Type si Content-Length (obligatorii), alaturi de headerele necesare
    (cookie si token)*/
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    int content_len = 0;
    sprintf(line, "%s", body_data[0]);
    content_len += strlen(body_data[0]);
    strcat(body_data_buffer, line);

    sprintf(line, "Content-Length: %d", content_len);
    compute_message(message, line);

    // Adaugam cookie-ul
     if (cookie != NULL) {
            sprintf(line, "Cookie: %s", cookie);
            compute_message(message, line);
    }

    if(token != NULL){
            sprintf(line, "Authorization: Bearer %s", token);
            compute_message(message, line);
    }
    // Adaugam newline la finalul headerului
    compute_message(message, "");
    // Adaugam payload data
    compute_message(message, body_data_buffer);
    free(line);
    return message;
}
