#include "market.hh"
#include <algorithm>

using namespace std;

void Market::advance_time()
{
    assert(not order_book.empty());
    struct Slot &front = order_book.front();

    cout << "slot at time " << front.time << " awarded to " << front.owner << endl;
    front.if_packet_sent();

    // delete front slot
    order_book.pop_front();

    // maybe add new slot to back
}

void Market::print_order_book()
{
    cout << "[ ";
    bool is_first = true;
    for (struct Slot &slot : order_book)
    {
        if (is_first) {
            is_first = false;
        } else {
            cout << " | ";
        }
        cout << slot.time << ". ";

        if (slot.owner != "")
            cout << slot.owner;
        else
            cout << "null";

        auto lowest_offer = std::min_element(slot.offers.begin(), slot.offers.end(), compare_two_bidoffers);
        cout << " $";
        if (lowest_offer != slot.offers.end())
            cout << lowest_offer->cost;
        else
            cout << "null";
    }

    cout << " ]" << endl;
}
