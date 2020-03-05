//
// Created by Yuhong Chen on 2020-02-27.
//

#ifndef PROJECT_BASE_SCHEDULER_POLICY_HPP
#define PROJECT_BASE_SCHEDULER_POLICY_HPP

#include "types.hpp"

class BaseSchedulerPolicy {
    public:
        virtual pair<string, unsigned> pick_executor() = 0;
};

#endif //PROJECT_BASE_SCHEDULER_POLICY_HPP
