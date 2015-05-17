/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "brute_force_user.hh" 

using namespace std;

BruteForceUser::BruteForceUser(const std::string &name, const size_t flow_start_time, const size_t num_packets, std::function<int(std::list<size_t>&)> utility_func)
: AbstractUser(name),
    flow_start_time_(flow_start_time),
    num_packets_(num_packets),
    utility_func_(utility_func)
{
}

static list<list<size_t>> potential_idxs(const deque<SingleSlot> &order_book, const string &name, size_t start, size_t num_packets)
{
    if (num_packets == 0)
        return {{}};

    list<list<size_t>> toRet = {};
    for (size_t i = start; i < order_book.size(); i++) {
        if (order_book.at(i).owner != name and not order_book.at(i).offers.empty()) {
            for (list<size_t> &vec : potential_idxs(order_book, name, i+1, num_packets-1)) {
                vec.emplace_front(i);
                toRet.emplace_back(vec);
            }
        }
    }
    return toRet;
}

static uint32_t cost_of_slots(const deque<SingleSlot> &order_book, const list<size_t> &idxs)
{
    uint32_t toRet = 0;
    for (size_t i : idxs) {
        toRet += order_book.at(i).best_offer().cost;
    }
    return toRet;
}


static pair<list<size_t>, int> best_slots(const deque<SingleSlot> &order_book, const string &name, const list<size_t> idxs_for_utility_func, size_t start, size_t num_packets, function<int(list<size_t>&)> utility_func)
{
    list<size_t> best_idxs = {};
    int max_net_utility = INT_MIN;
    for (list<size_t> &idxs : potential_idxs(order_book, name, start, num_packets))
    {
        /*
        cout << "[";
        for (size_t idx : idxs) {
            cout << " " << idx << ",";
        }
        cout << "]" << " costs " << cost_of_slots(order_book, idxs) << " and has utility " << utility_func_(idxs) <<  endl;
        */
        list<size_t> combined_idxs_for_utility_func = idxs_for_utility_func;
        combined_idxs_for_utility_func.insert(combined_idxs_for_utility_func.end(), idxs.begin(), idxs.end());
        int net_utility = utility_func(combined_idxs_for_utility_func) - cost_of_slots(order_book, idxs);
        if (net_utility > max_net_utility) {
            max_net_utility = net_utility;
            best_idxs = idxs;
        }
    }
    return make_pair(best_idxs, max_net_utility);
}

template <typename T>
static size_t num_owned_in_deque(deque<T> deque, const string &name)
{
    size_t toRet = 0;
    for (T &t : deque) {
        if (t.owner == name) {
            toRet++;
        }
    }
    return toRet;
}

static list<size_t> idxs_owned(deque<SingleSlot> order_book, const string &name)
{
    list<size_t> toRet {};
    for (size_t i = 0; i < order_book.size(); i++)
    {
        if (order_book.at(i).owner == name) {
            toRet.emplace_back(i);
        }
    }
    return toRet;
}

void BruteForceUser::take_actions(Market& mkt)
{
    const deque<SingleSlot> &order_book = mkt.order_book();
    if (order_book.empty())
        return;

    size_t num_packets_to_buy = num_packets_ - num_owned_in_deque(mkt.packets_sent(), name_) - idxs_owned(order_book, name_).size(); 

    list<size_t> best_idxs = best_slots(order_book, name_, idxs_owned(order_book, name_), flow_start_time_, num_packets_to_buy, utility_func_).first;

    cout << "buying slots ";
    for (size_t i : best_idxs) {
        cout << i << ", ";
        mkt.add_bid_to_slot( i, { order_book.at(i).best_offer().cost, name_ } );
    }
    cout << endl;

    cout << "making offers for slots ";
    for (size_t i : idxs_owned(order_book, name_)) {
        auto cur_idxs_owned = idxs_owned(order_book, name_);
        //int utility = utility_func_(idxs);

        remove_if( cur_idxs_owned.begin(), cur_idxs_owned.end(), [i](size_t elem){return elem == i;} );

        auto best_backup_slot = best_slots(order_book, name_, cur_idxs_owned, flow_start_time_, 1, utility_func_);
        assert(best_backup_slot.first.size() == 1);
        int utility_delta_to_move_slots = best_backup_slot.second;
        assert(utility_delta_to_move_slots >= 0);
        mkt.add_offer_to_slot( i , { (uint32_t) utility_delta_to_move_slots + 1, name_ } );
    }
}


void BruteForceUser::print_stats(const Market& ) const
{
        cout << "user " << name_ << " started at time " << flow_start_time_ << " and finished at time "
        <<  endl; // need to track money spent
}
