/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef FLOW_COMPLETION_TIME_USER
#define FLOW_COMPLETION_TIME_USER

#include <iostream>
#include "abstract_user.hh"
#include "market.hh"
#include "flow.hh"


class FlowCompletionTimeUser : public AbstractUser
{
    const size_t num_packets_;
    bool done_ = false;

    public:
    FlowCompletionTimeUser( Flow flow )
        : AbstractUser( flow.uid, flow.start ),
        num_packets_( flow.num_packets )
    {
    }

    void take_actions( Market& mkt ) override
    {
        if ( not done_ ) {
            size_t best_interval_length = 1;
            double best_interval_cost = 0;

            //for ( size_t interval_length = 1; interval_length <= 1024; interval_length <<= 1 ) {
            for ( size_t interval_length = num_packets_; interval_length <= 512; interval_length++ ) {
                double cost = mkt.cost_for_intervals( uid_, start_, start_+interval_length, num_packets_ );
                if ( mkt.verbose() ) {
                //    std::cout << uid_to_string( uid_ ) << ": $" << cost << " for interval length " << interval_length << std::endl;
                }
                if ( cost > 0 and ( best_interval_cost == 0 or -best_interval_length-best_interval_cost < -interval_length-cost ) ) {
                    best_interval_length = interval_length;
                    best_interval_cost = cost;
                }
            }
            if ( best_interval_cost > 0 ) {
                std::vector<std::pair<size_t, double>> offers { }; 
                //for ( size_t extra_length = 1; extra_length <= 64; extra_length <<= 1 ) {
                for ( size_t extra_length = 1; extra_length <= 256; extra_length++ ) {
                    //std::cout << "start_ " << start_ << "best_interval_length " <<  best_interval_length << "extra_length " << extra_length << std::endl;
                    offers.push_back( { start_ + best_interval_length + extra_length, (double) extra_length + .01 } );
                }

                double spent = mkt.buy_intervals( uid_, start_, start_+best_interval_length, num_packets_, best_interval_cost, offers );
                //std::cout << uid_to_string( uid_ ) << " cost was " << best_interval_cost << " and spent " << spent << " for interval length " << best_interval_length << std::endl;
                if ( spent > 0 ) {
                    done_ = true;
                }
            }
        }
        done_ = true; // only run once regardless for now
    }

    bool done( const Market& mkt ) override
    {
        return done_;
    }
};

#endif /* FLOW_COMPLETION_TIME_USER */
