#pragma once 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <hiredis.h>
#include <unistd.h>
#include <thread>
#include <queue> 
#include <mutex>
#include <functional>
#include "array_lock_free_queue.h"
#include "chrono-util.h"
class ByeRedis {

    public:
        enum class REPLY_TYPE {
            REDIS_REPLY_STRING_=1,
            REDIS_REPLY_ARRAY_,
            REDIS_REPLY_INTEGER_,
            REDIS_REPLY_NIL_,
            REDIS_REPLY_STATUS_,
            REDIS_REPLY_ERROR_,
            REDIS_REPLY_DOUBLE_,
            REDIS_REPLY_BOOL_,
            REDIS_REPLY_MAP_,
            REDIS_REPLY_SET_,
            REDIS_REPLY_ATTR_,
            REDIS_REPLY_PUSH_,
            REDIS_REPLY_BIGNUM_,
            REDIS_REPLY_VERB_
        } ;
        enum CONN_TYPE {
            SUCCESS = 0,
            FAIL = 1
        } ;

        struct ConnectionInfo {
            std::string host;
            int port;
            std::string authkey;
            ConnectionInfo(std::string _host, int _port, std::string _authkey) : host(_host), port(_port), authkey(_authkey) {} 
        };

        std::vector<ConnectionInfo> m_hosts;
        // std::string m_host;
        // int m_port;
        std::string m_pass;
        struct timeval m_timeout = { 1, 500000 }; // 1.5 seconds
        redisContext *m_c;
        int m_errorCount = 0;
        uint64_t m_lastPsub = 0;
        std::string alias; 

        std::vector<std::string> m_psubs;
        ArrayLockFreeQueue<std::string, (2 << 16)> m_jobs;
        std::string m_currentJob;
        virtual ~ByeRedis() { 
            redisFree(m_c);
        }

        template<typename HandlerT, typename HandlerC, typename HandlerS>
        void run(HandlerT handler, HandlerC conn, HandlerS subs) { 
            m_lastPsub = Chrono::unixTimeStamp();
            while(1) {
                // ksooMutex.lock(); 
                if(Chrono::unixTimeStamp() - m_lastPsub >= 1000) {
                    m_lastPsub = Chrono::unixTimeStamp();
                    for(int i=0; i<m_psubs.size(); i++) {
                        m_jobs.push(m_psubs[i]);
                    }
                }
                if (m_currentJob == "") { 
                    if(!m_jobs.pop(m_currentJob)) {
                        continue;
                    }
                } 
                redisReply *reply; 
                do {
                    if (!m_c) {
                        connect();
                        break;
                    }
                    std::string t = m_currentJob; 
                    reply = (redisReply*)redisCommand(m_c, t.c_str()); 
                    if(m_c->err) {
                        CONN_TYPE responseCode;
                        std::string response;
                        response = m_c->errstr; // reply->errstr;
                        responseCode = (CONN_TYPE)m_c->err;
                        if(reply) {
                            // process
                            freeReplyObject(reply);
                        }
                        conn(response, responseCode);
                        connect();
                        break;
                    }else {
                        if(reply) {
                            // process
                            REPLY_TYPE responseCode = (REPLY_TYPE)reply->type;
                            std::string response;
                            if(reply->type == REDIS_REPLY_ERROR) {
                                m_errorCount++;
                                printf("[%s] command: %s error\n", alias.c_str(), t.c_str());
                                if(m_errorCount >= 15) {
                                    m_errorCount = 0;
                                    m_currentJob = "";
                                }
                            } else if(reply->type == REDIS_REPLY_NIL) {
                                m_currentJob = "";
                            } else if(reply->type == REDIS_REPLY_ARRAY) {
                                if(reply->elements >= 4) {
                                    std::vector<std::string> result;
                                    for(int i=0; i<reply->elements; i++) {
                                        if((reply->element)[i]->str == nullptr) {
                                            result.push_back("(null)");
                                        } else {
                                            result.push_back((reply->element)[i]->str);
                                        }
                                    }
                                    subs(result);
                                }
                                m_currentJob = "";
                            } else {
                                if(reply->str) {
                                    response = reply->str; // null -> no data; 
                                } 
                                m_currentJob = "";
                                handler(response, responseCode, t);
                            } 
                            // sleep(1);
                            freeReplyObject(reply);
                        }
                    } 
                } while(0);
                // ksooMutex.unlock();
            }
        }
        void connect() {
            do {
                redisOptions options = {0};
                int randomIndex = rand() % m_hosts.size();
                auto m_host = m_hosts[randomIndex].host;
                auto m_port = m_hosts[randomIndex].port;
                auto m_authKey = m_hosts[randomIndex].authkey;
                printf("[%s] try connect! %s:%d\n", alias.c_str(), m_host.c_str(), m_port);
                REDIS_OPTIONS_SET_TCP(&options, m_host.c_str(), m_port);
                struct timeval timeout = { 1, 500000 }; // 1.5 seconds
                options.connect_timeout = &timeout;
                // m_c = redisConnectWithTimeout(m_host.c_str(), m_port, m_timeout);
                m_c = redisConnectWithOptions(&options); // m_host.c_str(), m_port, m_timeout);
                if (m_c == NULL || m_c->err) {
                    if (m_c) {
                        printf("[%s] Connection error: %s\n", alias.c_str(), m_c->errstr);
                        redisFree(m_c);
                        m_c = nullptr; 
                    } else {
                        printf("[%s] Connection error: can't allocate redis context\n", alias.c_str());
                    }
                    continue;
                }
                redisReply *reply; 
                reply = (redisReply*)redisCommand(m_c, m_authKey.c_str());
                if(reply) {
                    freeReplyObject(reply);
                }
                if(m_c->err) {
                    redisFree(m_c);
                    m_c = nullptr; 
                    continue;
                }
                printf("[%s] connection complete\n", alias.c_str());
            }while(0);
        } 
};
