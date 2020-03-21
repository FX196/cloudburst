//
// Created by Yuhong Chen on 2020-03-05.
//

#ifndef PROJECT_MOCK_SCHEDULER_POLICY_HPP
#define PROJECT_MOCK_SCHEDULER_POLICY_HPP

#include "base_scheduler_policy.hpp"

class MockSchedulerPolicy : public BaseSchedulerPolicy {
public:
    pair<string, unsigned> pick_executor(vector<string> refs, string function_name = ""){
        return pair<string, unsigned>("", 0);
    }

    bool pin_function(string dag_name, string function_name){
        std::cout << "Pinning function " << function_name << " of Dag" << dag_name << std::endl;
        return false;
    }

    void commit_dag(string dag_name){
        std::cout << "Committing Dag " << dag_name << std::endl;
    }

    void discard_dag(Dag dag, bool pending=false){
        std::cout << "Discarding Dag " << dag.name() << std::endl;
    }

    void process_status(const ThreadStatus& status){

    }

    void update(){

    }

    void update_function_locations(const vector<SchedulerStatus::FunctionLocation>& new_locations){

    }

};

#endif //PROJECT_MOCK_SCHEDULER_POLICY_HPP
