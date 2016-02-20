#include <ev++.h>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <strings.h>

struct thread_data
{
    int fd;
    void* data;
};
std::string ip, port, directory;
void process_request(ev::io &watcher, int revents)
{

}
void *thread_function(void *arg)
{
    thread_data data = *((thread_data *) arg);
    char buffer[102400];
    bzero(buffer, sizeof(buffer));
    ssize_t result = recv(data.fd, buffer, sizeof(buffer), MSG_NOSIGNAL);
    std::string request(buffer);
    std::ofstream cheat("/home/box/req.txt");
    cheat << request;
    cheat.close();
    request = request.substr(request.find_first_of(' ') + 1);
    request = request.substr(0, request.find_first_of(' '));
    std::string fname(*(std::string *)data.data);
    std::ifstream file(fname + request);
    std::string response;
    if(file.fail()) response += "HTTP/1.0 404 Not Found\r\n\r\n";
    else
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        response += "HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n<b>" + buffer.str();
        file.close();
    }
    if(result < 0) std::cout << "ERROR" << std::endl;
    else
    {
        send(data.fd, response.c_str(), response.length(), MSG_NOSIGNAL);
    }
    shutdown(data.fd, SHUT_RDWR);
    //watcher.stop();
    //delete &watcher;
    //ev::io *client_watcher = new ev::io(EV_DEFAULT);
    //client_watcher->set<&process_request>(data.data);
    //client_watcher->set(data.fd, ev::READ);
    //client_watcher->start();
}
void accept_connection(ev::io &watcher, int revents)
{
    int client_sd = accept(watcher.fd, 0, 0);
    pthread_t watcher_thread;
    pthread_attr_t opts;
    pthread_attr_init(&opts);
    pthread_attr_setdetachstate(&opts, PTHREAD_CREATE_DETACHED);
    thread_data data;
    data.fd = client_sd;
    data.data = watcher.data;
    pthread_create(&watcher_thread, NULL, &thread_function, &data);
}

int main(int argc, char **argv)
{   
    std::system("sudo service nginx start");
    int option;
    std::string ip, port, directory;
    while((option = getopt(argc, argv, "h:p:d:")) != -1)
    {
        switch(option)
        {
            case 'h':
                ip = optarg;
            break;
            case 'p':
                port = optarg;

            break;
            case 'd':
                directory = optarg;
                std::cout << optarg;
            break;
        }
    }
    daemon(0, 0);
    port="8085";
    directory="/etc";
    directory.erase(std::remove(directory.begin(), directory.end(), '\r'), directory.end());
    directory.erase(std::remove(directory.begin(), directory.end(), '\n'), directory.end());
    int socket_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in socket_config;
    bzero(&socket_config, sizeof(socket_config));
    socket_config.sin_family = AF_INET;
    socket_config.sin_port = htons(std::stoi(port));
    inet_pton(AF_INET, ip.c_str(), &(socket_config.sin_addr));
    bind(socket_descriptor, (sockaddr *) &socket_config, sizeof(socket_config));
    listen(socket_descriptor, SOMAXCONN);
    struct ev_loop *event_loop = ev_default_loop(EVRUN_NOWAIT);
    ev::io socket_watcher(event_loop);
    socket_watcher.set<&accept_connection>(&directory);
    socket_watcher.start(socket_descriptor, ev::READ);
    while(true) ev_run(event_loop, 0);
    return 0;
}
