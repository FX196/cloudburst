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

void function_create_handler(string serialized,
        zmq::socket_t &func_create_socket,
        KvsClientInterface *kvs,
        logger log,
        ConsistencyType consistency) {
    Function func;
    func.ParseFromString(serialized);

    string name = get_func_kvs_name(func.name());
    log->info("Creating function {}.", name);

    string serialized_val;
    LatticeType type;

    if (consistency == NORMAL) {
        type = LatticeType::LWW;
        LWWPairLattice<string> val = LWWPairLattice<string>(TimestampValuePair<string>(0, func.body()));
        serialized_val = serialize(val);
    } else {
        type = LatticeType::SINGLE_CAUSAL;
        set<string> s;
        s.insert(func.body());
        SingleKeyCausalLattice<SetLattice<string>> val = SingleKeyCausalLattice<SetLattice<string>>(
                VectorClockValuePair<SetLattice<string>>(get_default_vc(), SetLattice<string>(s)));
        serialized_val = serialize(val);
    }
    bool success = kvs_put(kvs, name, serialized_val, log, type);

    if (!success){
        log->error("Function Body PUT Errored!");
    }
    vector<string> funcs = get_func_list(kvs, "", log, true);
    funcs.push_back(FUNC_PREFIX+name);
    put_func_list(kvs, funcs, log);

    GenericResponse ok = GenericResponse();
    ok.set_success(true);

    string serialized_ok;
    ok.SerializeToString(&serialized_ok);

    kZmqUtil->send_string(serialized_ok, &func_create_socket);
}