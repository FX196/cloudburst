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
    FunctionCall call;
    call.ParseFromString(serialized);

    if (call.response_key().empty()){
        call.set_response_key(get_random_response_key());
    }



    // pick a node for this request.
    pair<Address, unsigned> result = policy.pick_executor(call);

    GenericResponse response;
    if (result.first == ""){
        response.set_success(false);
        response.set_error(CloudburstError::NO_RESOURCES);
        string serialized_response;
        response.SerializeToString(&serialized_response);
        kZmqUtil->send_string(serialized_response, &func_call_socket);
        return;
    }

    // Forward the request on to the chosen executor node.
    Address ip = result.first;
    unsigned tid = result.second;
    string serialized_send;
    call.SerializeToString(&serialized_send);
    kZmqUtil->send_string(serialized_send, &pusher_cache[ip]);


    response.set_success(true);
    response.set_response_id(call.response_key());
    string serialized_response;
    response.SerializeToString(&serialized_response);
    kZmqUtil->send_string(serialized_response, &func_call_socket);
}