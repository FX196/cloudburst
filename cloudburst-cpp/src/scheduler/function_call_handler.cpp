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
#include "scheduler/scheduler_handlers.hpp"

void function_call_handler(string serialized, zmq::socket_t &func_call_socket,
SocketCache &pusher_cache, BaseSchedulerPolicy &policy, logger log){
//    FunctionCall call;
//    string serialized = kZmqUtil->recv_string(&func_call_socket);
//    call.ParseFromString(serialized);
//
//    if (call.response_key() == ""){ //TODO: empty response key value?
//        call.set_response_key(""); // Generate random response key
//    }
//
//    vector<string> refs = call.references();
//    pair<string, unsigned> result = policy.pick_executor(refs);
//
//    GenericResponse response;
//    if
}