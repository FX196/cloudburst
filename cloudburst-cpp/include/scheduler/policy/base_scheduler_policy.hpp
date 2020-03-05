//
// Created by Yuhong Chen on 2020-02-27.
//

#ifndef PROJECT_BASE_SCHEDULER_POLICY_HPP
#define PROJECT_BASE_SCHEDULER_POLICY_HPP

#include "types.hpp"
#include "scheduler_utils.hpp"

class BaseSchedulerPolicy {
public:
    virtual pair<string, unsigned> pick_executor(vector <CloudburstReference> refs, string function_name = "") = 0;

    virtual bool pin_function(string dag_name, string function_name) = 0;

    virtual void commit_dag(string dag_name) = 0;

    virtual void discard_dag(Dag dag, bool pending = False) = 0;

    virtual void process_status(ThreadStatus status) = 0;

    virtual void update() = 0;

    virtual void update_function_locations(vector<FunctionLocation> new_locations) = 0;
};

#endif //PROJECT_BASE_SCHEDULER_POLICY_HPP
