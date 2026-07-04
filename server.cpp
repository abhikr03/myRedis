#include <thread>
#include "common.h"
#include "RESPParser.h"
#include "cmds.h"
#include "redisstore.h"
#include "config.h"

class RObject;

void periodic_snapshot() {

    while (true) {
        redis_save({"save"});
        std::this_thread::sleep_for(
            std::chrono::minutes(redis::GlobalConfig.snapshot_period));
    }
}

void process_request(const std::vector<std::string>& req, int client_fd) {

    if (req.size() == 0) {
        return;
    }

    if (req[0] == "COMMAND") {
        const char* response = "*1\r\n$4\r\nPING\r\n";
        write_exactly(client_fd, response, strlen(response));
    } else {
        redis_func_type redis_func = get_redis_func(req[0]);
        std::unique_ptr<redis::RObject> output = redis_func(req);
        if (output == nullptr) {
            throw RedisServerError("Redis cmd failed to return a valid RObject!");
        }
        std::string response = output->serialize();
        write_exactly(client_fd, response.c_str(), response.size());
    }
}


void handle_client(int client_fd) {

    RESPParser parser(client_fd);
    while(true) {
        try {
            std::vector<std::string> req = parser.read_new_request();
            if (req.size() == 0) {
                throw RedisServerError("Read empty request!");
            }
            req[0] = to_lower(req[0]);
            process_request(req, client_fd);
        }  
        catch (std::runtime_error& e) {
            break;
        }
    }
    if (close(client_fd)) {
        die("close");
    }
}
int setup_server() {
    int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { 
        die("socket");
    }
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        die("setsockopt");
    }
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(redis::GlobalConfig.port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_fd, (const sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        die("bind");
    }
    if (listen(server_fd, SOMAXCONN) == -1) {
        die("listen");
    }

    std::cout << "Server listening on port: " << redis::GlobalConfig.port << std::endl;
    return server_fd;
}

void handle_clients(int server_fd) {

    std::vector<std::thread> client_threads;

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int client_fd;
        if ((client_fd = accept(server_fd, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
            die("accept");
        }
        client_threads.emplace_back(handle_client, client_fd);
    }
    for (auto& thread : client_threads) {
        thread.join();
    }
}

int main() {

    if (!redis::read_config()) {
        std::cout << "Unable to read config\n" 
                  << "Please ensure config.json exist and is correctly setup"
                  << std::endl;
        return 0;
    }

    if (!RedisStore::getInstance().restore()) {
        std::cout << "State restoral failed! Continuing with empty state..." << std::endl;
    } else {
        std::cout << "Previous state restored!" << std::endl;
    }

    std::thread snapshot_thread(periodic_snapshot);

    int server_fd = setup_server();    
    handle_clients(server_fd);
    if (close(server_fd)) {
        die("close");
    }

    snapshot_thread.join();
    RedisStore::delInstance();
}
