#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 8192
#define WWW_ROOT "www"

void handle_client(int client_socket, const char *client_ip);
void send_text_response(int client_socket, const char *status, const char *content_type, const char *body);
void send_file_response(int client_socket, const char *status, const char *file_path, const char *content_type);
const char *get_content_type(const char *path);
int contains_path_traversal(const char *path);
void format_http_date(char *buffer, size_t size, time_t raw_time);
void log_request(const char *client_ip, const char *method, const char *path, const char *status);

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("\nServer running on http://localhost:%d\n", PORT);

printf("\nAvailable routes:\n");
printf("http://localhost:%d/\n", PORT);
printf("http://localhost:%d/about\n", PORT);
printf("http://localhost:%d/contact\n", PORT);
printf("http://localhost:%d/projects\n", PORT);
printf("http://localhost:%d/style.css\n", PORT);
printf("http://localhost:%d/network.jpg\n", PORT);

printf("\nOpen these in your browser (Ctrl+Click in VS Code terminal may work)\n\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        handle_client(client_fd, inet_ntoa(client_addr.sin_addr));
        close(client_fd);
    }

    close(server_fd);
    return 0;
}

void handle_client(int client_socket, const char *client_ip) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) {
        return;
    }

    buffer[bytes_received] = '\0';

    printf("\n===== REQUEST FROM %s =====\n%s\n", client_ip, buffer);

    char method[16];
    char path[256];
    char version[32];

    if (sscanf(buffer, "%15s %255s %31s", method, path, version) != 3) {
        log_request(client_ip, "UNKNOWN", "UNKNOWN", "400 Bad Request");
        send_text_response(
            client_socket,
            "400 Bad Request",
            "text/html; charset=UTF-8",
            "<html><body><h1>400 Bad Request</h1></body></html>"
        );
        return;
    }

    printf("Method: %s\n", method);
    printf("Path: %s\n", path);
    printf("Version: %s\n", version);

    if (strcmp(method, "GET") != 0) {
        log_request(client_ip, method, path, "405 Method Not Allowed");
        send_text_response(
            client_socket,
            "405 Method Not Allowed",
            "text/html; charset=UTF-8",
            "<html><body><h1>405 Method Not Allowed</h1><p>Only GET is supported.</p></body></html>"
        );
        return;
    }

    if (contains_path_traversal(path)) {
        log_request(client_ip, method, path, "400 Bad Request");
        send_text_response(
            client_socket,
            "400 Bad Request",
            "text/html; charset=UTF-8",
            "<html><body><h1>400 Bad Request</h1><p>Invalid path.</p></body></html>"
        );
        return;
    }

    char full_path[512];

    if (strcmp(path, "/") == 0) {
        snprintf(full_path, sizeof(full_path), "%s/index.html", WWW_ROOT);
    } else if (strcmp(path, "/about") == 0) {
        snprintf(full_path, sizeof(full_path), "%s/about.html", WWW_ROOT);
    } else if (strcmp(path, "/contact") == 0) {
        snprintf(full_path, sizeof(full_path), "%s/contact.html", WWW_ROOT);
    } else if (strcmp(path, "/projects") == 0) {
        snprintf(full_path, sizeof(full_path), "%s/projects.html", WWW_ROOT);
    } else {
        snprintf(full_path, sizeof(full_path), "%s%s", WWW_ROOT, path);
    }

    struct stat st;
    if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) {
        log_request(client_ip, method, path, "200 OK");
        send_file_response(client_socket, "200 OK", full_path, get_content_type(full_path));
    } else {
        char not_found_path[512];
        snprintf(not_found_path, sizeof(not_found_path), "%s/404.html", WWW_ROOT);

        log_request(client_ip, method, path, "404 Not Found");

        if (stat(not_found_path, &st) == 0 && S_ISREG(st.st_mode)) {
            send_file_response(client_socket, "404 Not Found", not_found_path, "text/html; charset=UTF-8");
        } else {
            send_text_response(
                client_socket,
                "404 Not Found",
                "text/html; charset=UTF-8",
                "<html><body><h1>404 Not Found</h1></body></html>"
            );
        }
    }
}

void send_text_response(int client_socket, const char *status, const char *content_type, const char *body) {
    char date_header[128];
    format_http_date(date_header, sizeof(date_header), time(NULL));

    int body_length = (int)strlen(body);

    char response[BUFFER_SIZE];
    int response_length = snprintf(
        response,
        sizeof(response),
        "HTTP/1.1 %s\r\n"
        "Date: %s\r\n"
        "Server: MicroCServer/1.0\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status,
        date_header,
        content_type,
        body_length,
        body
    );

    send(client_socket, response, response_length, 0);
}

void send_file_response(int client_socket, const char *status, const char *file_path, const char *content_type) {
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        send_text_response(
            client_socket,
            "500 Internal Server Error",
            "text/html; charset=UTF-8",
            "<html><body><h1>500 Internal Server Error</h1></body></html>"
        );
        return;
    }

    struct stat st;
    if (stat(file_path, &st) != 0) {
        fclose(file);
        send_text_response(
            client_socket,
            "500 Internal Server Error",
            "text/html; charset=UTF-8",
            "<html><body><h1>500 Internal Server Error</h1></body></html>"
        );
        return;
    }

    char date_header[128];
    char modified_header[128];
    format_http_date(date_header, sizeof(date_header), time(NULL));
    format_http_date(modified_header, sizeof(modified_header), st.st_mtime);

    char headers[1024];
    int header_length = snprintf(
        headers,
        sizeof(headers),
        "HTTP/1.1 %s\r\n"
        "Date: %s\r\n"
        "Server: MicroCServer/1.0\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Last-Modified: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        status,
        date_header,
        content_type,
        (long)st.st_size,
        modified_header
    );

    send(client_socket, headers, header_length, 0);

    char file_buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
        send(client_socket, file_buffer, bytes_read, 0);
    }

    fclose(file);
}

const char *get_content_type(const char *path) {
    const char *ext = strrchr(path, '.');

    if (ext == NULL) return "application/octet-stream";
    if (strcmp(ext, ".html") == 0) return "text/html; charset=UTF-8";
    if (strcmp(ext, ".css") == 0) return "text/css; charset=UTF-8";
    if (strcmp(ext, ".js") == 0) return "application/javascript; charset=UTF-8";
    if (strcmp(ext, ".txt") == 0) return "text/plain; charset=UTF-8";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";

    return "application/octet-stream";
}

int contains_path_traversal(const char *path) {
    return strstr(path, "..") != NULL;
}

void format_http_date(char *buffer, size_t size, time_t raw_time) {
    struct tm *time_info = gmtime(&raw_time);
    strftime(buffer, size, "%a, %d %b %Y %H:%M:%S GMT", time_info);
}

void log_request(const char *client_ip, const char *method, const char *path, const char *status) {
    FILE *log_file = fopen("server.log", "a");
    if (log_file == NULL) {
        return;
    }

    char timestamp[128];
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", time_info);

    fprintf(log_file, "[%s] IP=%s METHOD=%s PATH=%s STATUS=%s\n",
            timestamp, client_ip, method, path, status);

    fclose(log_file);
}