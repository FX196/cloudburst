//
// Created by Yuhong Chen on 2020-02-27.
//

#ifndef PROJECT_DEFAULT_SCHEDULER_POLICY_HPP
#define PROJECT_DEFAULT_SCHEDULER_POLICY_HPP

#include "base_scheduler_policy.hpp"

class DefaultSchedulerPolicy : public BaseSchedulerPolicy {
private:
    // This scheduler's IP address.
    string ip_;

    zmq::socket_t pin_accept_socket_;

    SocketCache pusher_cache_;

    KvsClientInterface kvs_client_;

    map<pair<string, string>, > running_counts_; // TODO: type for time?

};

#endif //PROJECT_DEFAULT_SCHEDULER_POLICY_HPP
