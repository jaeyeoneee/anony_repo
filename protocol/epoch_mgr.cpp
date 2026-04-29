// protocol/epoch_mgr.cpp

#include "protocol/epoch_mgr.h"
#include "protocol/distjoin.h"
#include "protocol/softrefresh.h"
#include "protocol/hardrefresh.h"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace epochess {

EpochManager::EpochManager(const ESSConfig& cfg,
                           const std::vector<uint64_t>& initial_U,
                           Bus& bus)
    : cfg_(cfg), bus_(bus), epoch_(1), U_(initial_U) {
    segment_secret_ = random_secret(cfg_.lambda);
    auto shs = cond_share(cfg_, U_, epoch_, segment_secret_);
    install_shares(shs);
}

const Share* EpochManager::get_share(uint64_t id) const {
    auto it = shares_.find(id);
    if (it == shares_.end()) return nullptr;
    return &it->second;
}

bool EpochManager::dist_join_one(uint64_t joiner_id) {
    if (std::find(U_.begin(), U_.end(), joiner_id) != U_.end()) return false;

    if (U_.size() < cfg_.k + 1) return false;

    std::mt19937_64 rng(0xB007 ^ joiner_id);
    for (int attempt = 0; attempt < 8; attempt++) {
        std::vector<uint64_t> shuffled = U_;
        std::shuffle(shuffled.begin(), shuffled.end(), rng);
        std::vector<Share> committee;
        for (size_t i = 0; i < static_cast<size_t>(cfg_.k) + 1; i++) {
            committee.push_back(shares_.at(shuffled[i]));
        }
        auto r = dist_join(cfg_, joiner_id, epoch_, committee, bus_);
        if (r.success) {
            shares_[joiner_id] = *r.share_for_joiner;
            U_.push_back(joiner_id);
            return true;
        }
    }
    return false;
}

bool EpochManager::soft_transition(const std::vector<uint64_t>& J_next,
                                    const std::vector<uint64_t>& R_next,
                                    unsigned k_next) {
    std::vector<uint64_t> U_next;
    for (uint64_t id : U_) {
        if (std::find(R_next.begin(), R_next.end(), id) == R_next.end()) {
            U_next.push_back(id);
        }
    }
    for (uint64_t id : J_next) U_next.push_back(id);

    std::vector<uint64_t> overlap_ids;
    for (uint64_t id : U_) {
        if (std::find(R_next.begin(), R_next.end(), id) == R_next.end()) {
            overlap_ids.push_back(id);
        }
    }
    if (overlap_ids.size() < cfg_.k) return false;

    std::vector<Share> overlap_committee;
    for (size_t i = 0; i < cfg_.k; i++) {
        overlap_committee.push_back(shares_.at(overlap_ids[i]));
    }

    ESSConfig cfg_next = cfg_;
    cfg_next.k = k_next;

    auto r = soft_refresh(cfg_, cfg_next, overlap_committee, U_next,
                          epoch_ + 1, bus_);
    if (!r.success) return false;

    shares_.clear();
    U_ = U_next;
    epoch_++;
    cfg_ = cfg_next;
    install_shares(r.new_shares);
    return true;
}

bool EpochManager::hard_transition(const std::vector<uint64_t>& J_next,
                                    const std::vector<uint64_t>& R_next,
                                    unsigned k_next) {
    std::vector<uint64_t> U_next;
    for (uint64_t id : U_) {
        if (std::find(R_next.begin(), R_next.end(), id) == R_next.end()) {
            U_next.push_back(id);
        }
    }
    for (uint64_t id : J_next) U_next.push_back(id);

    ESSConfig cfg_next = cfg_;
    cfg_next.k = k_next;

    auto r = hard_refresh(cfg_next, U_next, epoch_ + 1, bus_);
    if (!r.success) return false;

    shares_.clear();
    U_ = U_next;
    epoch_++;
    cfg_ = cfg_next;
    install_shares(r.new_shares);
    std::vector<Share> committee;
    for (size_t i = 0; i < cfg_.k; i++) committee.push_back(r.new_shares[i]);
    segment_secret_ = reconstruct(cfg_, committee);
    return true;
}

Poly EpochManager::reconstruct_epoch_secret(
    const std::vector<uint64_t>& committee_ids) const {
    if (committee_ids.size() < cfg_.k) {
        throw std::runtime_error("insufficient committee size");
    }
    std::vector<Share> committee;
    for (size_t i = 0; i < cfg_.k; i++) {
        auto it = shares_.find(committee_ids[i]);
        if (it == shares_.end()) {
            throw std::runtime_error("unknown identity in committee");
        }
        committee.push_back(it->second);
    }
    return reconstruct(cfg_, committee);
}

void EpochManager::install_shares(const std::vector<Share>& shs) {
    for (const auto& sh : shs) shares_[sh.identity] = sh;
}

} 
