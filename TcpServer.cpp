#include "TcpServer.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

static const int BUFFER_SIZE = 1024;
static const int MAX_NUM = 1000000;

static bool wantsClose(const char* req) {
    const char* p = strstr(req, "Connection:");
    if (!p) return false;
    p += 11;
    while (*p == ' ') p++;
    return strncasecmp(p, "close", 5) == 0;
}

static bool parseSumQuery(const char* path, int& a, int& b) {
    if (strncmp(path, "/sum", 4) != 0) return false;
    if (path[4] != '\0' && path[4] != '?') return false;

    a = 0;
    b = 0;
    const char* q = strchr(path, '?');
    if (!q) return false;
    q++;

    while (*q) {
        if (strncmp(q, "a=", 2) == 0) {
            a = atoi(q + 2);
            q = strchr(q, '&');
            if (!q) break;
            q++;
        } else if (strncmp(q, "b=", 2) == 0) {
            b = atoi(q + 2);
            break;
        } else {
            q = strchr(q, '&');
            if (!q) break;
            q++;
        }
    }
    return true;
}

static void sendResponse(int fd, int status, const char* statusText, const char* body) {
    char buf[256];
    int bodyLen = strlen(body);
    int n = snprintf(buf, sizeof(buf),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "%s",
        status, statusText, bodyLen, body);
    write(fd, buf, n);
}

TcpServer::TcpServer(string ip_address, int port, size_t num_threads)
    : m_ip_address(ip_address), m_port(port), m_socket(), m_socketAddress(),
      m_socketAddress_len(sizeof(m_socketAddress)),
      m_num_threads(num_threads),
      m_threadPool(num_threads)
{
    m_socketAddress.sin_family = AF_INET;
    m_socketAddress.sin_port = htons(m_port);
    m_socketAddress.sin_addr.s_addr = INADDR_ANY;

    if (startServer() != 0) {
        cout << "Failed to start server on port " << m_port << endl;
    }
}

TcpServer::~TcpServer() {
    closeServer();
}

int TcpServer::startServer() {
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0) {
        cout << "Cannot create socket" << endl;
        return -1;
    }

    int opt = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cout << "setsockopt failed" << endl;
        return -1;
    }

    if (bind(m_socket, (struct sockaddr*)&m_socketAddress, m_socketAddress_len) < 0) {
        cout << "Cannot bind socket" << endl;
        return -1;
    }
    return 0;
}

void TcpServer::closeServer() {
    close(m_socket);
}

void TcpServer::startListen() {
    if (listen(m_socket, SOMAXCONN) < 0) {
        cout << "Socket listen failed" << endl;
        return;
    }

    cout << "\n*** Listening on " << m_ip_address << ":" << m_port
         << " (" << m_num_threads << " workers) ***\n\n";

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int client_fd = accept(m_socket, (struct sockaddr*)&clientAddr, &clientLen);
        if (client_fd < 0) {
            cout << "accept failed" << endl;
            continue;
        }

        m_threadPool.enqueue([this, client_fd] {
            this->handleClient(client_fd);
        });
    }
}

void TcpServer::handleClient(int client_socket) {
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesRead = read(client_socket, buffer, BUFFER_SIZE - 1);
        if (bytesRead <= 0) break;

        bool closeAfter = wantsClose(buffer);

        char method[16] = {0};
        char path[512] = {0};
        sscanf(buffer, "%15s %511s", method, path);

        int a = 0, b = 0;
        if (!parseSumQuery(path, a, b) || a < 0 || a > MAX_NUM || b < 0 || b > MAX_NUM) {
            sendResponse(client_socket, 400, "Bad Request", "<html><body>bad request</body></html>");
        } else {
            char body[64];
            snprintf(body, sizeof(body), "<html><body>%d</body></html>", a + b);
            sendResponse(client_socket, 200, "OK", body);
        }

        if (closeAfter) break;
    }

    close(client_socket);
}
