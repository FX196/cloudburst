//
// Created by Yuhong Chen on 2020-02-27.
//

#ifndef PROJECT_DEFAULT_SCHEDULER_POLICY_HPP
#define PROJECT_DEFAULT_SCHEDULER_POLICY_HPP

#include "scheduler_policy_interface.hpp"

class DefaultSchedulerPolicy : public SchedulerPolicyInterface {
private:
    // This scheduler's IP address.
    string ip_;

    zmq::socket_t pin_accept_socket_;

    SocketCache pusher_cache_;

    KvsClientInterface kvs_client_;

    map<pair<string, string>, std::chrono::time_point> running_counts_;

    map<pair<string, string>, std::chrono::time_point> backoff_;

    map<string, set<string>> key_locations_;

    set<pair<string, string>> unpinned_executors_;

};

#endif //PROJECT_DEFAULT_SCHEDULER_POLICY_HPP
