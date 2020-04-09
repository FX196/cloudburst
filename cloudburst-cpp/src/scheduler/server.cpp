//  Copyright 2019 U.C. Berkeley RISE Lab
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "common.hpp"
#include "client/kvs_client.hpp"
#include "scheduler/scheduler_handlers.hpp"
#include "scheduler/policy/mock_scheduler_policy.hpp"
#include "yaml-cpp/yaml.h"

const unsigned METADATA_THRESHOLD = 5; // TODO: in seconds?
const unsigned REPORT_THRESHOLD = 5;
const unsigned kRoutingThreadNum = 4;

ZmqUtil zmq_util;
ZmqUtilInterface *kZmqUtil = &zmq_util;

void run_scheduler(string ip, string mgmt_ip, string route_addr, string metric_addr){

    // set up logging
    string log_file = "log_" + ip + ".txt";
    string log_name = "scheduler_log_" + ip;
    auto log = spdlog::basic_logger_mt(log_name, log_file, true);
    log->flush_on(spdlog::level::info);

    bool local = mgmt_ip.compare("") == 0;

    vector<UserRoutingThread> threads;
    for (unsigned i = 0; i < kRoutingThreadNum; i++) {
        threads.push_back(UserRoutingThread(route_addr, i));
    }

    KvsClient client(threads, ip, 0, 10000);
    KvsClientInterface *kvs = &client;

    string scheduler_id = get_random_id();

    // map from Dag's name to its protobuf and its sources
    map <string, pair<Dag, set<string>>> dags;

    // Tracks how many requests for each function is received.
    map<string, unsigned > call_frequency;

    // Tracks the most recent arrival for each DAG -- used to calculate interarrival times.
    map<string, TimePoint> last_arrivals;

    // Tracks the time interval between successive requests for a particular DAG.
    map<string, vector<unsigned long long>> interarrivals;

    // Maintains a list of all other schedulers in the system, so we can propagate metadata to them.
    set<string> schedulers;

    // ZMQ set up
    zmq::context_t context(1);
    SocketCache pusher_cache = SocketCache(&context, ZMQ_PUSH);
    SocketCache requestor_cache = SocketCache(&context, ZMQ_REQ);

    zmq::socket_t connect_socket(context, ZMQ_REP);
    connect_socket.bind(get_bind_address(CONNECT_PORT));

    zmq::socket_t func_create_socket(context, ZMQ_REP);
    func_create_socket.bind(get_bind_address(FUNC_CREATE_PORT));

    zmq::socket_t func_call_socket(context, ZMQ_REP);
    func_call_socket.bind(get_bind_address(FUNC_CALL_PORT));

    zmq::socket_t dag_create_socket(context, ZMQ_REP);
    dag_create_socket.bind(get_bind_address(DAG_CREATE_PORT));

    zmq::socket_t dag_call_socket(context, ZMQ_REP);
    dag_call_socket.bind(get_bind_address(DAG_CALL_PORT));

    zmq::socket_t dag_delete_socket(context, ZMQ_REP);
    dag_delete_socket.bind(get_bind_address(DAG_DELETE_PORT));

    zmq::socket_t list_socket(context, ZMQ_PULL);
    list_socket.bind(get_bind_address(LIST_PORT));

    zmq::socket_t exec_status_socket(context, ZMQ_PULL);
    exec_status_socket.bind(get_bind_address(STATUS_PORT));

    zmq::socket_t sched_update_socket(context, ZMQ_PULL);
    sched_update_socket.bind(get_bind_address(SCHED_UPDATE_PORT));

    zmq::socket_t pin_accept_socket(context, ZMQ_PULL);
    int timeout = 500;
    zmq_setsockopt(pin_accept_socket, ZMQ_RCVTIMEO, &timeout, sizeof(int)); // TODO:Check docs if this line fails to compile
    pin_accept_socket.bind(get_bind_address(PIN_ACCEPT_PORT));

    //  Initialize poll set
    vector<zmq::pollitem_t> pollitems = {
            {static_cast<void *>(connect_socket), 0, ZMQ_POLLIN, 0},
            {static_cast<void *>(func_create_socket), 0, ZMQ_POLLIN, 0},
            {static_cast<void *>(func_call_socket), 0, ZMQ_POLLIN, 0},
            {static_cast<void *>(dag_create_socket), 0, ZMQ_POLLIN, 0},
            {static_cast<void *>(dag_call_socket), 0, ZMQ_POLLIN, 0},
            {static_cast<void *>(dag_delete_socket), 0, ZMQ_POLLIN, 0},
            {static_cast<void *>(list_socket), 0, ZMQ_POLLIN, 0},
            {static_cast<void *>(exec_status_socket), 0, ZMQ_POLLIN, 0},
            {static_cast<void *>(sched_update_socket), 0, ZMQ_POLLIN, 0}};

    // Initialize Policy
    // TODO: policy initiallization
    MockSchedulerPolicy mock_policy;
    kSchedulerPolicy = &mock_policy;

    // Enter event loop
    while(true){
        auto work_start = std::chrono::system_clock::now();

        kZmqUtil->poll(0, &pollitems);

        // connect socket
        if (pollitems[0].revents & ZMQ_POLLIN) {
            string serialized = kZmqUtil->recv_string(&connect_socket);
            connect_handler(serialized, connect_socket, route_addr, log);
        }

        // receives a function create request
        if (pollitems[1].revents & ZMQ_POLLIN) {
            string serialized = kZmqUtil->recv_string(&func_create_socket);
            function_create_handler(serialized, func_create_socket, kvs, log);
        }

        // receives a function call request
        if (pollitems[2].revents & ZMQ_POLLIN) {
            string serialized = kZmqUtil->recv_string(&func_call_socket);
            function_call_handler(serialized, func_call_socket, pusher_cache, kSchedulerPolicy, log);
        }

        // receives a dag create request
        if (pollitems[3].revents & ZMQ_POLLIN) {
            string serialized = kZmqUtil->recv_string(&dag_create_socket);
            dag_create_handler(serialized, dag_create_socket, pusher_cache,
                    kvs, dags, kSchedulerPolicy, call_frequency, log);
        }

        // receives a dag call request
        if (pollitems[4].revents & ZMQ_POLLIN) {
            string serialized = kZmqUtil->recv_string(&dag_call_socket);
            dag_call_handler(serialized, dag_call_socket, pusher_cache,
                    last_arrivals, interarrivals, dags,
                    kSchedulerPolicy, call_frequency, log);
        }

        // receives a dag delete request
        if (pollitems[5].revents & ZMQ_POLLIN) {
            string dag_name = kZmqUtil->recv_string(&dag_delete_socket);
            dag_delete_handler(dag_name, dag_delete_socket, dags,
                    kSchedulerPolicy, call_frequency, log);
        }

        // receives list request
        if (pollitems[6].revents & ZMQ_POLLIN) {
            string serialized = kZmqUtil->recv_string(&list_socket);
            // TODO: list handler
        }

        // receives exec_status request
        if (pollitems[7].revents & ZMQ_POLLIN) {
            string serialized = kZmqUtil->recv_string(&exec_status_socket);
            // TODO: exec_status handler
        }

        // receives sched_update request
        if (pollitems[8].revents & ZMQ_POLLIN) {
            string serialized = kZmqUtil->recv_string(&sched_update_socket);
            // TODO: sched_update handler
        }

        auto work_end = std::chrono::system_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                work_end - work_start)
                .count();

        if(duration > METADATA_THRESHOLD){
            // TODO: update policy

            if(!local){
                schedulers = get_ip_set(get_scheduler_list_address(mgmt_ip),
                        requestor_cache);
            }
        }

        if(duration > REPORT_THRESHOLD){
            SchedulerStatus status;
            for(auto dag_name_info_pair : dags){
                auto* dag_name_ptr = status.add_dags();
                *dag_name_ptr = dag_name_info_pair.first;
            }

            // TODO: function locations

            string msg;
            status.SerializeToString(&msg);
            for(string sched_ip : schedulers){
                if(sched_ip.compare(ip) != 0){
                    kZmqUtil->send_string(msg, &pusher_cache[
                            get_scheduler_update_address(sched_ip)
                            ]);
                }
            }

            ExecutorStatistics stats;
            for(auto func_freq_pair : call_frequency){
                auto* fstats = stats.add_functions();
                fstats->set_name(func_freq_pair.first);
                fstats->set_call_count(func_freq_pair.second);
                log->info("Reporting %d calls for function %s", func_freq_pair.second, func_freq_pair.first);
                func_freq_pair.second = 0;
            }

            for(auto dname_time_pair : interarrivals){
                auto* dstats = stats.add_dags();
                dstats->set_name(dname_time_pair.first);
                dstats->set_call_count(dname_time_pair.second.size() + 1);
                for(auto runtime : dname_time_pair.second){
                    dstats->add_interarrival(runtime);
                }
                dname_time_pair.second.clear();
            }

            if(!local){
                string serialized_stats;
                stats.SerializeToString(&serialized_stats);
                kZmqUtil->send_string(serialized_stats, &pusher_cache[
                        get_statistics_report_address(mgmt_ip)
                        ]);
            }

            work_start = std::chrono::system_clock::now();
        }
    }
}

int main(int argc, char *argv[]){
    string conf_file;
    if(argc > 1){
        conf_file = argv[1];
    }
    else {
        conf_file = "conf/cloudburst-config.yml";
    }

    YAML::Node conf = YAML::LoadFile(conf_file);
    YAML::Node sched_conf = conf["scheduler"];
    string metric_address = "http://" + sched_conf["metric_address"].as<string>() + ":3000/publish";

    string mgmt_ip = conf["mgmt_ip"].as<string>();
    std::cout << mgmt_ip << std::endl;

//    run_scheduler(conf["ip"].as<string>(),
//            conf["mgmt_ip"].as<string>(),
//            sched_conf["routing_address"].as<string>(),
//            metric_address);
}