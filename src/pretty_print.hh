/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef PRETTY_PRINT_HH
#define PRETTY_PRINT_HH

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "flow.hh"
#include "interval.hh"
#include "opportunity.hh"
#include "transaction.hh"


static std::string interval_to_string( const Interval &i )
{
    return std::string("[") + std::to_string(i.start) + "," + std::to_string(i.end) + "]";
}

static std::string uid_to_string( const size_t uid )
{
    if ( uid ) {
        return std::string( 1, 'A' + uid - 1 );
    } else {
        return std::string("ccast");
    }
}

__attribute__ ((unused)) static void print_transactions( const std::vector<Transaction> &transactions )
{
    for ( auto &t : transactions ) {
        std::cout << t.amount << " from " << uid_to_string( t.from ) << " to " << uid_to_string( t.to ) << std::endl;
    }
}

__attribute__ ((unused)) static void print_flows( const std::vector<Flow> &flows )
{
    for ( auto &flow : flows ) {
        std::cout << "User " << uid_to_string( flow.uid ) << " flow start time: " << flow.start << " num packets: " << flow.num_packets << std::endl;
    }
}

__attribute__ ((unused)) static void print_opportunities( const std::vector<Opportunity> opportunities )
{
    for ( auto &o : opportunities ) {
        if ( o.owner != 0 ) { // XXX temp not printing ccast right now
            std::cout << interval_to_string( o.interval ) << " owned by " << uid_to_string( o.owner ) << std::endl;
        }
    }
}

#endif /* PRETTY_PRINT_HH */
