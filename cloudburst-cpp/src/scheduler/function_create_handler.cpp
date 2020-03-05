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

void function_create_handler(
        zmq::socket_t &func_create_socket,
        KvsClient *kvs,
        logger log,
        ConsistencyType consistency = NORMAL) {
    string serialized = kZmqUtil->recv_string(&func_create_socket);
    Function func;
    func.ParseFromString(serialized);

    string name = get_func_kvs_name(func.name());
    log->info("Creating function {}.", name);

    if (consistency == NORMAL) {
        LWWPairLattice<TimestampValuePair<string>> val = LWWPairLattice<TimestampValuePair<string>>(TimestampValuePair<string>(
                generate_timestamp(0), func.body()
        ));
    } else {
        set<string> s;
        s.insert(func.body());
        SingleKeyCausalLattice<VectorClockValuePair<SetLattice<string>>> val = SingleKeyCausalLattice<VectorClockValuePair<SetLattice<string>>>(
                VectorClockValuePair<SetLattice<string>>(DEFAULT_VC, SetLattice<string>(s)));
    }
    bool success = kvs_put(kvs, name, serialize(val), log);

    if (!success){
        log->error("Function Body PUT Errored!");
    }
    vector<string> funcs = get_func_list(kvs, "", fullname=true); // TODO: get_func_list/put_func_list
    funcs.push_back(name);
    put_func_list(kvs, funcs);
    kZmqUtil->send_string(serialized_ok, &func_create_socket);
}