#ifndef __UTIL__
#include "util.h"
#endif

#ifndef __EXPRESS__
#define __EXPRESS__
#pragma comment(lib, "ws2_32.lib")

#define SERVER_BUFFER_SIZE 1024
#define SERVER_MAX_CONNECTION 256

WSADATA wsa;
SOCKET server_fd, client_socket;
struct sockaddr_in server, client;
int client_size;

typedef struct {
    char* key;
    char* value;
} request_header;

typedef struct {
    request_header* headers;
    int length;
} request_headers;

typedef struct {
    char* key;
    char* value;
} request_cookie;

typedef struct {
    request_cookie* cookies;
    int length;
} request_cookies;

typedef struct {
    char* key;
    char* value;
} form_data;

typedef struct {
    form_data* data;
    int length;
} form_datas;

typedef struct {
    string key;
    string value;
} query_param;

typedef struct {
    query_param* values;
    int length;
} query_params;

typedef struct {
    split_result params;
    query_params query;
    request_headers header;
    request_cookies cookies;
    form_datas body;
} http_request;

typedef struct {
    int status;
    strcallback send;
    constcharcallback send_file; 
    twostrcallback cookie;
    constcharcallback redirect;
} http_response;

typedef http_request* request;
typedef http_response* response;

typedef void(*listencallback)(u_short);
typedef void(*httpcallback)(request, response);
typedef void(*getcallback)(char*, httpcallback);
typedef void(*postcallback)(char*, httpcallback);

typedef struct {
    listencallback listen;
    voidfunccallback close;
    getcallback get;
    postcallback post;
} SERVER;

typedef struct {
    string path;
    httpcallback callback;
} getMethod;

typedef struct {
    string path;
    httpcallback callback;
} postMethod;

getMethod get_method[8];
int get_length = 0;

postMethod post_method[8];
int post_length = 0;

long long counter = 0;

char method[16];
char* path_get;
SOCKET send_client;
char* send_start_line;
char send_headers[BUF_MAX];
string send_body;
char* send_data;
http_request send_request;
http_response send_response;
char* send_cookie;
char* send_redirect_url;

voidcallback __onclose = NULL;

void handle_client(SOCKET);
char* generate_cookie(const char*, const char*);
void __server_send(void);

void __response_send(char* body) {
    send_body = str_create(body);
    __server_send();
}
void __response_send_file(const char* file_name) {
    char* html = read_file(file_name);
    if (html == NULL) { __response_send("load failed"); }
    __response_send(html);
    free(html);
}
void __response_cookie(const char* key, const char* value) {
    send_cookie = generate_cookie(key, value);
}
void __response_redirect(const char* url) {
    if (url == NULL) { return; }
    send_redirect_url = url;
}

void __server_on_close(voidcallback func) {
    __onclose = func;
}
void __server_close(void) {
    if (__onclose != NULL) { __onclose(); }
    closesocket(server_fd);
    WSACleanup();
    exit(1);
}
void __server_listen(u_short port) {
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed.\n");
        __server_close();
    }
    if (listen(server_fd, SERVER_MAX_CONNECTION) == SOCKET_ERROR) {
        printf("Listen failed.\n");
        __server_close();
    }
    printf("listening on %d\n", port);
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr*)&client, &client_size);
        if (client_socket == INVALID_SOCKET) {
            printf("Accept failed.\n");
            __server_close();
        }
        handle_client(client_socket);
    }
}
void __server_get(char* path, httpcallback callback) {
    if (get_length == 8) {
        printf("Default method count limit has been reached.");
        return;
    }
    if (path == NULL) { return; }
    getMethod temp;
    string str;
    str = str_create(path);
    temp.path = str;
    temp.callback = callback;
    get_method[get_length] = temp;
    get_length++;
}
void __server_post(char* path, httpcallback callback) {
    if (post_length == 8) {
        printf("Default method count limit has been reached.");
        return;
    }
    if (path == NULL) { return; }
    postMethod temp;
    string str;
    str = str_create(path);
    temp.path = str;
    temp.callback = callback;
    post_method[post_length] = temp;
    post_length++;
}
void __server_send(void) {
    if (!equal(send_cookie, "")) { send_cookie = add(add("Set-Cookie: ", send_cookie), "\r\n"); }
    if (!equal(send_redirect_url, "")) { send_redirect_url = add(add("Location: ", send_redirect_url), "\r\n"); }
    if (str_empty(send_body)) { send_body = str_create("404 not found"); }
    if (equal(path_get, "/")) { path_get = "root"; }
    switch (send_response.status) {
    case 200:
        send_start_line = "HTTP/1.1 200 OK\r\n\0";
        break;
    case 302:
        send_start_line = "HTTP/1.1 302 Found\r\n\0";
        break;
    case 404:
        send_start_line = "HTTP/1.1 404 NOT FOUND\r\n\0";
        break;
    case 500:
        send_start_line = "HTTP/1.1 500 REDIRECT\r\n\0";
        break;
    default:
        send_start_line = "HTTP/1.1 200 OK\r\n\0";
    }
    sprintf(send_headers, "Content-Type: text/html\r\nConnection: close\r\nContent-Length: %d\r\n%s%s\r\n", send_body.length, send_cookie, send_redirect_url);
    send_data = add(add(send_start_line, send_headers), send_body.value);
    if (!equal(send_cookie, "")) { free(send_cookie); }
    send(send_client, send_data, strlen(send_data), 0);
    int point = str_find_char(send_body, '\n');
    bool large = (send_body.length < 20 || point != -1);
    if (point != -1) { str_set(&send_body, str_slice(send_body, 0, point)); }
    char* now = get_current_time();
    char* log_temp = send_body.length > 19 ? slice(send_body.value, 0, 19) : send_body.value;
    printf("\n[ %lld | %s ]\n%s : %s\nRES : %s\n", counter++, now, method, path_get, send_body.value);

    free(now);
}
void __server_send_setup(void) {
    send_body.value = NULL;
    send_body.length = 0;
    send_response.status = 200;
    send_response.send = __response_send;
    send_response.send_file = __response_send_file;
    send_response.cookie = __response_cookie;
    send_response.redirect = __response_redirect;
    send_cookie = "";
    send_redirect_url = "";
    path_get = "404";
}

request_headers parse_request_headers(char* request) {
    request_headers headers;
    headers.headers = (request_header*)malloc(sizeof(request_header) * 30);
    headers.length = 0;
    char* line = strtok(request, "\r\n");
    while (line != NULL) {
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            headers.headers[headers.length].key = strdup(line);
            headers.headers[headers.length].value = strdup(colon + 2);
            headers.length++;
        }
        line = strtok(NULL, "\r\n");
    }
    return headers;
}
form_datas parse_form_data(char* body) {
    form_datas data;
    data.data = (form_data*)malloc(sizeof(form_data) * 30);
    data.length = 0;
    char* pair = strtok(body, "&");
    while (pair != NULL) {
        char* equals = strchr(pair, '=');
        if (equals) {
            *equals = '\0';
            data.data[data.length].key = strdup(pair);
            data.data[data.length].value = strdup(equals + 1);
            data.length++;
        }
        pair = strtok(NULL, "&");
    }
    return data;
}
char* generate_cookie(const char* key, const char* value) {
    size_t len = strlen(key) + strlen(value) + 1;
    char* cookie = (char*)malloc(len + 1);
    if (cookie == NULL) { return NULL; }
    sprintf(cookie, "%s=%s", key, value);
    return cookie;
}
query_params parse_query(string query) {
    query_params result;
    result.length = 0;
    result.values = NULL;
    string query_copy = query;
    if (query.value[0] == '?') {
        query_copy = str_create(str_slice(query, 1, query.length));
        if (query_copy.length == ERR_MALLOC_FAIL) { return result; }
    }
    else { return result; }
    split_result pairs = str_split(query_copy, '&');
    if (pairs.length == ERR_MALLOC_FAIL) {
        if (query_copy.value != query.value) { free(query_copy.value); }
        return result;
    }
    if (pairs.length == 0 && !str_empty(query_copy)) {
        pairs.values[0] = query_copy;
        pairs.length = 1;
    }
    query_param* params = (query_param*)malloc(pairs.length * sizeof(query_param));
    if (params == NULL) {
        if (query_copy.value != query.value) {
            free(query_copy.value);
        }
        split_result_free(pairs);
        result.length = ERR_MALLOC_FAIL;
        return result;
    }
    for (int i = 0; i < pairs.length; i++) {
        split_result query = str_split(pairs.values[i], '=');
        if (query.values == NULL) {
            continue;
        }
        if (query.length == 2) {
            params[result.length].key = query.values[0];
            params[result.length].value = query.values[1];
            result.length++;
        }
        else {
            for (int j = 0; j < query.length; j++) {
                free(query.values[j].value);
            }
        }
        free(query.values);
    }
    result.values = params;
    if (query_copy.value != query.value) {
        free(query_copy.value);
    }
    split_result_free(pairs);
    return result;
}
request_cookies parse_cookies(request_headers headers) {
    request_cookies cookies;
    cookies.cookies = (request_cookie*)malloc(sizeof(request_cookie) * 30);
    cookies.length = 0;

    for (int i = 0; i < headers.length; i++) {
        if (strcmp(headers.headers[i].key, "Cookie") == 0) {
            char* cookie_str = headers.headers[i].value;
            char* pair = strtok(cookie_str, "; ");
            while (pair != NULL) {
                char* equals = strchr(pair, '=');
                if (equals) {
                    *equals = '\0';
                    cookies.cookies[cookies.length].key = strdup(pair);
                    cookies.cookies[cookies.length].value = strdup(equals + 1);
                    cookies.length++;
                }
                pair = strtok(NULL, "; ");
            }
            break;
        }
    }
    return cookies;
}

bool query_contain(query_params params, char* key) {
    for (int i = 0; i < params.length; i++) {
        if (equal(params.values[i].key.value, key)) { return true; }
    }
    return false;
}
string query_get(query_params params, char* key) {
    for (int i = 0; i < params.length; i++) {
        if (equal(params.values[i].key.value, key)) { return params.values[i].value; }
    }
    string temp = { NULL, 0 };
    return temp;
}

void handle_client(SOCKET client_socket) {
    send_client = client_socket;

    char buffer[SERVER_BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, SERVER_BUFFER_SIZE - 1, 0);
    if (bytes_received >= SERVER_BUFFER_SIZE - 1) { return; }
    if (bytes_received == SOCKET_ERROR) { return; }
    buffer[bytes_received] = '\0';
    char path[256];
    _ = sscanf(buffer, "%s %s", method, path);
    if (equal(path, "/favicon.ico")) { return; }
    if (equal(path, "/close")) { __server_close(); }
    char temp[256];
    if (equal(method, "GET")) {
        for (int i = 0; i < get_length; i++) {
            strcpy(temp, get_method[i].path.value);
            if ((end_with(temp, ':') && start_with(path, slice(temp, 0, sizeof_str(temp) - 1))) || equal(path, temp)) {
                __server_send_setup();
                path_get = temp;
                split_result params = split_result_shift(split(path, '/'), 2);
                send_request.params = params;
                send_request.header = parse_request_headers(path);
                send_request.cookies = parse_cookies(send_request.header);
                if (params.length > 0) { send_request.query = parse_query(params.values[0]); }
                get_method[i].callback(&send_request, &send_response);
                split_result_free(params);
                free(send_body.value);
                return closesocket(client_socket);
            }
        }
    }
    if (equal(method, "POST")) {
        for (int i = 0; i < post_length; i++) {
            strcpy(temp, post_method[i].path.value);
            if (equal(path, temp)) {
                __server_send_setup();
                path_get = temp;
                send_request.header = parse_request_headers(path);
                send_request.cookies = parse_cookies(send_request.header);
                send_request.body = parse_form_data(buffer);
                post_method[i].callback(&send_request, &send_response);
                free(send_body.value);
                return closesocket(client_socket);
            }
        }
    }
    closesocket(client_socket);
}

SERVER express(void) {
    client_size = sizeof(client);
    srand(time(NULL));

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed.\n");
        exit(1);
    }
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        exit(1);
    }

    SERVER server;
    server.close = __server_on_close;
    server.listen = __server_listen;
    server.get = __server_get;
    server.post = __server_post;

    return server;
}

#endif