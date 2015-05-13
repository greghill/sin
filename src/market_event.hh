struct PacketSent
{
    char* owner;
    uint64_t time;
};

struct MoneyExchanged
{
    char* from;
    char* to;
    uint32_t amount;
    uint64_t time;
};

struct MarketEvent
{
    enum {PACKET_SENT, MONEY_EXCHANGED} type;

    union {
        PacketSent packet_sent;
        MoneyExchanged money_exchanged;
    };

    inline bool operator==( const MarketEvent& ) const {
        return true;
    }
};
