/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#define DEBUG_PRINT 0

#include "flow_completion_time_user.hh" 

using namespace std;

FlowCompletionTimeUser::FlowCompletionTimeUser( const std::string &name, const size_t flow_start_time, const size_t num_packets )
: AbstractUser( name ),
    flow_start_time_( flow_start_time ),
    num_packets_( num_packets ),
    done_( false )
{
}

static priority_queue<pair<double, size_t>> idxs_to_buy( const deque<SingleSlot> &order_book, const string &name, size_t start_time, size_t num_packets_to_buy, const size_t latest_time_already_owned )
{
    priority_queue<pair<double, size_t>> idxs_to_buy;
    double idxs_to_buy_cost = 0;

    size_t idx = start_time < order_book.front().time ? 0 : order_book.front().time - start_time;
    while  (idxs_to_buy.size() != num_packets_to_buy) {
        assert(idx < order_book.size() && "can't buy slots we need");
        const SingleSlot &potential_slot = order_book.at( idx );
        bool can_buy = potential_slot.owner != name and potential_slot.has_offers();
        if ( can_buy ) {
            double slot_cost = potential_slot.best_offer().cost;
            idxs_to_buy.push( {slot_cost, idx} );
            idxs_to_buy_cost += slot_cost;
        }
        idx++;
    }

    assert( idx != 0 );
    idx--; // we incremented 1 too many times in while loop

    size_t min_flow_completion_time = max(order_book.at( idx ).time, latest_time_already_owned);
    if (DEBUG_PRINT)
        cout << name << " got min fct " <<  min_flow_completion_time << " and start time " << start_time << " and latest already owned" << latest_time_already_owned << endl;

    auto best_idxs = idxs_to_buy;
    assert(min_flow_completion_time >= start_time);

    double flow_benefit = -((double) min_flow_completion_time - (double) start_time);
    double best_utility = flow_benefit - idxs_to_buy_cost;
    if (DEBUG_PRINT)
        cout << "best utility starting at" << best_utility << " and benefit is " << flow_benefit << endl;

    double most_expensive_slot_cost = idxs_to_buy.top().first;
    for (size_t i = idx + 1; i < order_book.size(); i++) {
        const SingleSlot &potential_slot = order_book.at( i );
        bool can_buy = potential_slot.owner != name and potential_slot.has_offers();
        if ( can_buy and potential_slot.best_offer().cost < most_expensive_slot_cost ) {
            double slot_cost = potential_slot.best_offer().cost;
            idxs_to_buy.push( {slot_cost, i} );
            idxs_to_buy_cost += slot_cost;

            if (DEBUG_PRINT)
                cout << "trying slot " << i << " has cost " << slot_cost << " while most expensive slot in idxs_to_buy is " << most_expensive_slot_cost << endl;
            idxs_to_buy_cost -= idxs_to_buy.top().first;
            idxs_to_buy.pop();

            assert(idxs_to_buy.size() == num_packets_to_buy);
            most_expensive_slot_cost = idxs_to_buy.top().first;

            double current_benefit = - ( (double) max( potential_slot.time, latest_time_already_owned ) - (double) start_time );
            double current_utility = (double) current_benefit - idxs_to_buy_cost;
            if (DEBUG_PRINT)
                cout << "benefit for " << i << " is " << current_benefit << " and utility is " << current_utility << endl;

            if (current_utility > best_utility) {
                if (DEBUG_PRINT)
                    cout << "that is better than current best, " << best_utility << " so swapping" << endl;
                best_idxs = idxs_to_buy;
                best_utility = current_utility;
            }
        }
    }

    assert( best_idxs.size() == num_packets_to_buy );
    return move( best_idxs );
}

template <typename T>
static size_t num_owned( const T &collection, const string &name )
{
    size_t toRet = 0;
    for (auto &item : collection)
    {
        if ( item.owner == name ) {
            toRet++;
        }
    }
    return toRet;
}

void FlowCompletionTimeUser::take_actions( Market& mkt )
{
    auto &order_book = mkt.order_book();
    if ( order_book.empty() or order_book.front().time < flow_start_time_)
    {
        return;
    }

    const size_t num_packets_sent = num_owned( mkt.packets_sent(), name_ );

    const size_t num_order_book_slots_owned = num_owned( order_book, name_ );

    assert ( num_packets_ >= ( num_packets_sent + num_order_book_slots_owned ) );

    size_t num_packets_to_buy = num_packets_ - num_packets_sent - num_order_book_slots_owned; 

    if ( num_packets_to_buy > 0 ) {
        size_t current_flow_completion_time = 0;

        if ( num_order_book_slots_owned > 0 ) {
            size_t idx = order_book.size() - 1;
            while ( idx != 0 ) { // skip idx 0 dont care
                if (order_book.at(idx).owner == name_) {
                    current_flow_completion_time = order_book.at(idx).time;
                    break;
                }
                idx--;
            }
        }
        if (DEBUG_PRINT)
            cout << "current_flow_completion_time " << current_flow_completion_time << endl;

        priority_queue<pair<double, size_t>> buying_slots = idxs_to_buy( order_book, name_, flow_start_time_, num_packets_to_buy, current_flow_completion_time );

        if (DEBUG_PRINT)
            cout << name_ << " is buying slots: ";
        while ( not buying_slots.empty() ) {
            const size_t slot_idx_to_buy = buying_slots.top().second;
            const double best_offer_cost = buying_slots.top().first;

            if (DEBUG_PRINT)
                cout << slot_idx_to_buy << ", ";
            mkt.add_bid_to_slot( slot_idx_to_buy, { best_offer_cost, name_ } );
            assert( order_book.at( slot_idx_to_buy ).owner == name_ ); // assert we succesfully got it

            buying_slots.pop();
        }
        if (DEBUG_PRINT)
            cout << "done!" << endl;
    }

    // now price all slots we own, there are two prices, one for the non-last slot, which selling couldnt change flow completion time, and one for the last slot

    if ( num_order_book_slots_owned == 0 and num_packets_to_buy == 0) {
        return;
    }

    // first price last slot
    size_t current_flow_completion_time = 0;
    size_t flow_completion_time_if_sold_back = 0;
    bool has_more_than_one_slot = ( num_order_book_slots_owned + num_packets_to_buy ) > 1;

    size_t idx = order_book.size() - 1;
    while ( idx != 0 ) { // skip idx 0 dont care
        if ( order_book.at(idx).owner == name_ ) {
            if ( current_flow_completion_time == 0 ) {
                current_flow_completion_time = order_book.at(idx).time;
                if ( not has_more_than_one_slot ) {
                    break;
                }
            } else {
                flow_completion_time_if_sold_back = order_book.at(idx).time;
                break;
            }
        }
        idx--;
    }

    num_packets_to_buy = 1;
    pair<double, size_t> back_replacement = idxs_to_buy( order_book, name_, flow_start_time_, num_packets_to_buy, flow_completion_time_if_sold_back ).top();
    pair<double, size_t> non_back_replacement = idxs_to_buy( order_book, name_, flow_start_time_, num_packets_to_buy, current_flow_completion_time ).top();

    double back_benefit_delta = (double) current_flow_completion_time - (double) max( flow_completion_time_if_sold_back, order_book.at( back_replacement.second ).time);
    double non_back_benefit_delta = min( (double) current_flow_completion_time - (double) order_book.at( non_back_replacement.second ).time, 0. );

    double back_sell_price = .01 - ( back_benefit_delta - back_replacement.first );
    double non_back_sell_price = .01 - ( non_back_benefit_delta - non_back_replacement.first );

    idx = 0;
    for ( auto &slot : order_book ) {
        if (slot.owner == name_) {
            if ( slot.time == current_flow_completion_time ) {
                if ( not slot.has_offers() or slot.best_offer().cost != back_sell_price ) {
                    mkt.clear_offers_from_slot( idx, name_ );
                    mkt.add_offer_to_slot( idx, { back_sell_price, name_ } );
                }
                break;
            } else {
                if ( not slot.has_offers() or slot.best_offer().cost != non_back_sell_price ) {
                    mkt.clear_offers_from_slot( idx, name_ );
                    mkt.add_offer_to_slot( idx, { non_back_sell_price, name_ } );
                }
            }
        }
        idx++;
    }
}

bool FlowCompletionTimeUser::done( const Market& mkt )
{
    if (not done_) {
        done_ = num_packets_ == num_owned( mkt.packets_sent(), name_ );
    }
    return done_;
}

void FlowCompletionTimeUser::print_stats( const Market& ) const
{
        cout << "user " << name_ << " started at time " << flow_start_time_ << " and finished at time ?" <<  endl;
}
