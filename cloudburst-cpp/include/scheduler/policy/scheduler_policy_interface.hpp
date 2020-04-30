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


#ifndef PROJECT_SCHEDULER_POLICY_INTERFACE_HPP
#define PROJECT_SCHEDULER_POLICY_INTERFACE_HPP

#include "server_utils.hpp"
#include "types.hpp"
#include "cloudburst.pb.h"
#include "internal.pb.h"

using ThreadLocation = pair<Address, unsigned>;

class SchedulerPolicyInterface {
public:
    virtual ThreadLocation pick_executor(const vector<string>& references, string function_name = "") = 0;

    virtual bool pin_function(string dag_name, string function_name) = 0;

    virtual void commit_dag(string dag_name) = 0;

    virtual void discard_dag(const Dag& dag, bool pending=false) = 0;

    virtual void process_status(const ThreadStatus& status) = 0;

    virtual void update() = 0;

    virtual void update_function_locations(SchedulerStatus& status) = 0;
};

extern SchedulerPolicyInterface* kSchedulerPolicy;

#endif //PROJECT_SCHEDULER_POLICY_INTERFACE_HPP
