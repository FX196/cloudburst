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

void list_handler(string serialized, zmq::socket_t &list_socket, KvsClientInterface *kvs, logger log){
    StringSet resp;
    for(string func : get_func_list(kvs, serialized, log)){
        string* key_ptr = resp.add_keys();
        *key_ptr = func;
    }
    string serialized_resp;
    resp.SerializeToString(&serialized_resp);
    kZmqUtil->send_string(serialized_resp, &list_socket);
}