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
#include "scheduler_handlers.hpp"

const unsigned METADATA_THRESHOLD = 5;
const unsigned REPORT_THRESHOLD = 5;

ZmqUtil zmq_util;
ZmqUtilInterface *kZmqUtil = &zmq_util;

void run_scheduler(string ip, string mgmt_ip, string route_addr, string metric_addr){

    // set up logging
    string log_file = "log_" + std::to_string(ip) + ".txt";
    string log_name = "scheduler_log_" + std::to_string(ip);
    auto log = spdlog::basic_logger_mt(log_name, log_file, true);
    log->flush_on(spdlog::level::info);

    bool local = mgmt_ip.compare("") == 0;
    KvsClient kvs(); //TODO: fill in constructor

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
    vector<string> schedulers;

    // ZMQ set up
    zmq::context_t context(1);
    SocketCache pusher_cache = SocketCache(&context, ZMQ_PUSH);
    SocketCache requestor_cache = SocketCache(context, zmq.REQ);

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
    zmq_setsockopt(pin_accept_socket, zmq.RCVTIMEO, 500); // TODO:Check docs if this line fails to compile
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
            {static_cast<void *>(sched_update_socket), 0, ZMQ_POLLIN, 0},
            {static_cast<void *>(pin_accept_socket), 0, ZMQ_POLLIN, 0}};

    // Initialize Policy
    // TODO: policy

    auto work_start = std::chrono::system_clock::now();

    // Enter event loop



}