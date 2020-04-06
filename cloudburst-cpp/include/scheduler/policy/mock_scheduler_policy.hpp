//
// Created by Yuhong Chen on 2020-03-05.
//

#ifndef PROJECT_MOCK_SCHEDULER_POLICY_HPP
#define PROJECT_MOCK_SCHEDULER_POLICY_HPP

#include "base_scheduler_policy.hpp"

class MockSchedulerPolicy : public BaseSchedulerPolicy {
public:
    pair<Address, unsigned> pick_executor(const vector<string>& references, string function_name = ""){
        if(pick_executor_responses_.size() == 0) {
            std::cout << "Ran out of responses! Fix your test" << std::endl;
            return pair<Address, unsigned>("", 0);
        }
        pair<Address, unsigned> response = pick_executor_responses_[0];
        pick_executor_responses_.erase(pick_executor_responses_.begin());
        return response;
    }

    bool pin_function(string dag_name, string function_name){
        if(pin_function_responses_.size() == 0) {
            std::cout << "Ran out of responses! Fix your test" << std::endl;
            return false;
        }
        bool response = pin_function_responses_[0];
        pin_function_responses_.erase(pin_function_responses_.begin());
        return response;
    }

    void commit_dag(string dag_name){
        std::cout << "Committing Dag " << dag_name << std::endl;
    }

    void discard_dag(const Dag& dag, bool pending=false){
        std::cout << "Discarding Dag " << dag.name() << std::endl;
    }

    void process_status(const ThreadStatus& status){

    }

    void update(){

    }

    void update_function_locations(const vector<SchedulerStatus::FunctionLocation>& new_locations){

    }

    void clear(){
        pick_executor_responses_.clear();
        pin_function_responses_.clear();
    }

    vector<pair<Address, unsigned>> pick_executor_responses_;
    vector<bool> pin_function_responses_;

};

#endif //PROJECT_MOCK_SCHEDULER_POLICY_HPP
