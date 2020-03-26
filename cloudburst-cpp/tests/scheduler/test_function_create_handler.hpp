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

#ifndef PROJECT_TEST_FUNCTION_CREATE_HANDLER_HPP
#define PROJECT_TEST_FUNCTION_CREATE_HANDLER_HPP

#include "scheduler/scheduler_handlers.hpp"

TEST_F(SchedulerHandlerTest, FunctionCreateReceive){
Function func;
func.set_name("foo");
func.set_body("foo body");
string serialized;
func.SerializeToString(&serialized);

((KvsMockClient *) kvs_mock)->responses_.push_back(get_func_list_response());

zmq::socket_t create_socket(context, ZMQ_REP);

function_create_handler(serialized, create_socket, kvs_mock, log_);

EXPECT_EQ(((KvsMockClient *) kvs_mock)->keys_get_.size(), 1);
EXPECT_EQ(((KvsMockClient *) kvs_mock)->keys_get_[0], FUNCOBJ);
EXPECT_EQ(((KvsMockClient *) kvs_mock)->keys_put_.size(), 2);
EXPECT_EQ(((KvsMockClient *) kvs_mock)->keys_put_[0], FUNC_PREFIX + "foo");
EXPECT_EQ(((KvsMockClient *) kvs_mock)->keys_put_[1], FUNCOBJ);

vector<string> messages = get_zmq_messages();
EXPECT_EQ(messages.size(), 1);
string serialized_ok = messages[0];
GenericResponse ok;
ok.ParseFromString(serialized_ok);
EXPECT_EQ(ok.success(), true);
}

#endif //PROJECT_TEST_FUNCTION_CREATE_HANDLER_HPP
