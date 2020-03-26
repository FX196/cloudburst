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

MockZmqUtil mock_zmq_util;
ZmqUtilInterface *kZmqUtil = &mock_zmq_util;

KvsMockClient kvs_mock_client;
KvsClientInterface *kvs_mock = &kvs_mock_client;

MockSchedulerPolicy mock_policy;

logger log_ = spdlog::basic_logger_mt("mock_log", "mock_log.txt", true);

class SchedulerHandlerTest : public ::testing::Test {
    // initialization
protected:
    zmq::context_t context;
    SocketCache pushers = SocketCache(&context, ZMQ_PUSH);
public:
    void TearDown() {
        // clear all the logged messages after each test
        mock_zmq_util.sent_messages.clear();
        kvs_mock_client.responses_.clear();
    }

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
};

#endif //PROJECT_SCHEDULER_HANDLER_BASE_HPP
