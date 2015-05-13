/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "market.hh"
#include <algorithm>

using namespace std;

void Market::advance_time()
{
    assert(not order_book_.empty());
    struct Slot &front = order_book_.front();

    cout << "slot at time " << front.time << " awarded to " << front.owner << endl;

    // add slot to sent_slots and delete from order book
    //completed_events.emplace_back(front.owner, 
    order_book_.pop_front();
}
