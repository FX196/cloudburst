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

#ifndef PROJECT_TEST_CONNECT_HANDLER_HPP
#define PROJECT_TEST_CONNECT_HANDLER_HPP

#include "scheduler/scheduler_handlers.hpp"

TEST_F(SchedulerHandlerTest, ConnectReceive){
vector<string> messages = get_zmq_messages();
EXPECT_EQ(messages.size(), 0);

string serialized = "";
zmq::socket_t connect_socket(context, ZMQ_REP);
string route_addr = "route_addr";

connect_handler(serialized, connect_socket, route_addr, log_);

messages = get_zmq_messages();
EXPECT_EQ(messages.size(), 1);
EXPECT_EQ(messages[0], "route_addr");

std::cout << get_time_since_epoch() << std::endl;
}

#endif //PROJECT_TEST_CONNECT_HANDLER_HPP
