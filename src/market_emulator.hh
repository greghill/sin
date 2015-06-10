/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef MARKET_EMULATOR_HH
#define MARKET_EMULATOR_HH

#include <iostream>
#include <deque>
#include <queue>
#include <list>
#include <algorithm>
#include <cassert>
#include <memory>
#include <functional>

#include "abstract_user.hh"
#include "market.hh"

class MarketEmulator {
    Market mkt_;
    std::vector<std::unique_ptr<AbstractUser>> users_;
    const bool verbose_;
    const bool random_user_order_;

    size_t next_idx( size_t last_idx );
    void users_take_actions_until_finished();

    public:
    MarketEmulator( std::vector<std::unique_ptr<AbstractUser>> &&users, bool verbose, bool random_user_order );

    void run_to_completion();

    void print_slots();

    const std::vector<PacketSent> &packets_sent() const { return mkt_.packets_sent(); };

    void print_packets_sent();

    void print_money_exchanged();

    void print_user_stats();
};

#endif /* MARKET_EMULATOR_HH */
