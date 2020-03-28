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

#ifndef PROJECT_TEST_FUNCTION_CALL_HANDLER_HPP
#define PROJECT_TEST_FUNCTION_CALL_HANDLER_HPP

#include "scheduler/scheduler_handlers.hpp"

TEST_F(SchedulerHandlerTest, FunctionCallSuccess){
FunctionCall call;
call.set_name("foo");
string serialized_call;
call.SerializeToString(&serialized_call);

mock_policy.pick_executor_responses_.push_back(pair<Address, unsigned>("tcp://127.0.0.1:6000", 0));


zmq::socket_t func_call_socket(context, ZMQ_REP);

function_call_handler(serialized_call, func_call_socket, pusher_cache, mock_policy, log_);

vector<string> messages = get_zmq_messages();
EXPECT_EQ(messages.size(), 2);

FunctionCall received_call;
received_call.ParseFromString(messages[0]);
EXPECT_NE(received_call.response_key(), "");

GenericResponse response;
response.ParseFromString(messages[1]);
EXPECT_EQ(response.success(), true);
EXPECT_EQ(response.response_id(), received_call.response_key());
}

TEST_F(SchedulerHandlerTest, FunctionCallFail){
FunctionCall call;
call.set_name("foo");
string serialized_call;
call.SerializeToString(&serialized_call);

mock_policy.pick_executor_responses_.push_back(pair<Address, unsigned>("", 0));


zmq::socket_t func_call_socket(context, ZMQ_REP);

function_call_handler(serialized_call, func_call_socket, pusher_cache, mock_policy, log_);

vector<string> messages = get_zmq_messages();
EXPECT_EQ(messages.size(), 1);

GenericResponse response;
response.ParseFromString(messages[0]);
EXPECT_EQ(response.success(), false);
}

#endif //PROJECT_TEST_FUNCTION_CALL_HANDLER_HPP
