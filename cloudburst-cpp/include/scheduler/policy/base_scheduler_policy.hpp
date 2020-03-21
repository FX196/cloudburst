//
// Created by Yuhong Chen on 2020-02-27.
//

#ifndef PROJECT_BASE_SCHEDULER_POLICY_HPP
#define PROJECT_BASE_SCHEDULER_POLICY_HPP

#include "types.hpp"
#include "cloudburst.pb.h"
#include "internal.pb.h"

class BaseSchedulerPolicy {
public:
    virtual pair<string, unsigned> pick_executor(vector<string> refs, string function_name = "") = 0;

    virtual bool pin_function(string dag_name, string function_name) = 0;

    virtual void commit_dag(string dag_name) = 0;

    virtual void discard_dag(Dag dag, bool pending=false) = 0;

    virtual void process_status(const ThreadStatus& status) = 0;

    virtual void update() = 0;

    virtual void update_function_locations(const vector<SchedulerStatus::FunctionLocation>& new_locations) = 0;
};

#endif //PROJECT_BASE_SCHEDULER_POLICY_HPP
