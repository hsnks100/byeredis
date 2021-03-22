//
// g++ async_example.cpp -o async_example -I.. -lboost_program_options -lhiredis -lev
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <queue> 
#include <mutex>
#include <functional>
#include <hiredis.h>
#include <byeredis.h>

void redisCallback(std::string a, ByeRedis::REPLY_TYPE b, std::string command) {
    // std::cout << "================ call back ===========\n";
    std::cout << "execute: " << command << ", " << a << (int)b << std::endl;
    std::cout << "================ call end ===========\n";
}

void redisCallback2(std::string a, ByeRedis::REPLY_TYPE b, std::string command) {
    // std::cout << "================ call back ===========\n";
    std::cout << "SSSSSSSSSS execute: " << command << ", " << a << (int)b << std::endl;
    std::cout << "================ call end ===========\n";
}
void connectionCallback(std::string a, ByeRedis::CONN_TYPE b) {
    // std::cout << "================ connection issue ===========\n";
    std::cout << a << b << std::endl;
    // std::cout << "================ connection issue end ===========\n";
}

int main(int argc, char **argv) {

    ByeRedis redis;
    ByeRedis subscribeRedis;
    redis.m_hosts.emplace_back(
        "103.238.128.141", 6379, "auth @pass1"); 
    redis.m_hosts.emplace_back(
        "103.238.128.142", 6379, "auth @pass1"); 
    redis.alias = "MAIN";
    subscribeRedis.m_hosts.emplace_back(
        "103.238.128.141", 6379, "auth @pass1"); 
    subscribeRedis.m_hosts.emplace_back(
        "103.238.128.142", 6379, "auth @pass1"); 
    subscribeRedis.alias = "SUB";
#if 0
    std::thread([&redis]() {
        redis.run([](std::string a, ByeRedis::REPLY_TYPE b, std::string command) {
            std::cout << "================ call back ===========\n";
            std::cout << "execute: " << command << ", " << a << (int)b << std::endl;
            std::cout << "================ call end ===========\n";

        }, [](std::string a, ByeRedis::CONN_TYPE b) {
            std::cout << "================ connection issue ===========\n";
            std::cout << a << b << std::endl;
            std::cout << "================ connection issue end ===========\n";
        });
    }).detach(); 
#else 
    std::thread([&redis]() {
        // redis.run(std::bind(&redisCallback, std::placeholders::_1, std::placeholders::_2)); 
        redis.run(&redisCallback, &connectionCallback, [](std::vector<std::string> result) { 
            for(int i=0; i<result.size(); i++) {
                printf(".. %s\n", result[i].c_str());
            }
            std::cout << "================ call end ===========\n";
        });

    }).detach();

    std::thread([&subscribeRedis]() {
        // redis.run(std::bind(&redisCallback, std::placeholders::_1, std::placeholders::_2)); 
        subscribeRedis.run(&redisCallback2, &connectionCallback, [](std::vector<std::string> result) { 
            for(int i=0; i<result.size(); i++) {
                printf(".. %s\n", result[i].c_str());
            }
            std::cout << "================ call end ===========\n";
        }); 
    }).detach();
#endif 


    // std::string sub = std::string("psubscribe __keyspace@0__:ksoo*");
// "config set notify-keyspace-events KEA", "
    subscribeRedis.m_jobs.push("config set notify-keyspace-events KEA"); 
    subscribeRedis.m_psubs.push_back("psubscribe __keyspace@0__:ksoo*");
    subscribeRedis.m_psubs.push_back("psubscribe __keyspace@0__:gsoo*");

    std::vector<std::string> commands = {"ping", "get ksoobabo", "sibal"};
    for(int i=0; i<commands.size(); i++) {
        redis.m_jobs.push(commands[i].c_str());
    }

    // 종료방지
    while(1) {
        std::string name;
        std::getline(std::cin, name);
        printf("job size1: %d\n", redis.m_jobs.size());
        redis.m_jobs.push(name.c_str());
        printf("job size2: %d\n", redis.m_jobs.size());
    } 
    return 0;
}
