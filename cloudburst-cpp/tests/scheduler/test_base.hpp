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

#ifndef PROJECT_SCHEDULER_HANDLER_BASE_HPP
#define PROJECT_SCHEDULER_HANDLER_BASE_HPP

#include "scheduler/scheduler_utils.hpp"
#include "mock_zmq_utils.hpp"
#include "kvs_mock_client.hpp"
#include "scheduler/policy/mock_scheduler_policy.hpp"
#include "scheduler/policy/default_scheduler_policy.hpp"

logger log_ = spdlog::basic_logger_mt("mock_log", "mock_log.txt", true);

MockZmqUtil mock_zmq_util;
ZmqUtilInterface *kZmqUtil = &mock_zmq_util;

KvsMockClient kvs_mock_client;
KvsClientInterface *kvs_mock = &kvs_mock_client;

SchedulerPolicyInterface *kSchedulerPolicy;

zmq::context_t context;
SocketCache pusher_cache = SocketCache(&context, ZMQ_PUSH);

zmq::socket_t pin_accept_socket = zmq::socket_t(context, ZMQ_PULL);
int timeout = 500;
int rc = zmq_setsockopt(pin_accept_socket, ZMQ_RCVTIMEO, &timeout, sizeof(int)); // TODO:Check docs if this line fails to compile
int bind = zmq_bind(pin_accept_socket, get_bind_address(PIN_ACCEPT_PORT).c_str());
//pin_accept_socket.bind(get_bind_address(PIN_ACCEPT_PORT).c_str());

DefaultSchedulerPolicy default_policy(pin_accept_socket, pusher_cache, kvs_mock, "127.0.0.1", log_, 0, true);

class TestBase : public ::testing::Test {
protected:
    map <string, pair<Dag, set<string>>> dags;
    map<string, unsigned > call_frequency;
    map<string, TimePoint> last_arrivals;
    map<string, vector<unsigned long long>> interarrivals;

public:
    vector<string> get_zmq_messages() { return mock_zmq_util.sent_messages;}

    KeyResponse get_func_list_response(){
        KeyResponse response;
        KeyTuple *tp = response.add_tuples();
        response.set_type(RequestType::GET);

        tp->set_key(FUNCOBJ);
        tp->set_lattice_type(LatticeType::SET);

        set<string> funcs;
        funcs.insert(FUNC_PREFIX+"fee");
        funcs.insert(FUNC_PREFIX+"fi");
        funcs.insert(FUNC_PREFIX+"fo");
        SetLattice<string> func_lattice(funcs);

        tp->set_payload(serialize(func_lattice));
        return response;
    }

    KeyResponse get_put_ok_response(unsigned rid){
        KeyResponse response;
        response.set_response_id(std::to_string(rid));
        KeyTuple *tp = response.add_tuples();
        tp->set_error(AnnaError::NO_ERROR);
        return response;
    }
};

class SchedulerHandlerTest : public TestBase {
    // initialization

public:
    MockSchedulerPolicy mock_policy;

    void SetUp() {
        kSchedulerPolicy = &mock_policy;
    }

    void TearDown() {
        // clear all the logged messages after each test
        mock_zmq_util.sent_messages.clear();
        kvs_mock_client.clear();
        mock_policy.pick_executor_responses_.clear();
    }
};

class PolicyTest : public TestBase {
public:
    void SetUp() {
        kSchedulerPolicy = &default_policy;
        (((DefaultSchedulerPolicy*) kSchedulerPolicy))->unpinned_executors_.insert({"127.0.0.1", 1});
    }

    void TearDown() {
        // clear all the logged messages after each test
        mock_zmq_util.sent_messages.clear();
        kvs_mock_client.clear();
    }
};

#endif //PROJECT_SCHEDULER_HANDLER_BASE_HPP
