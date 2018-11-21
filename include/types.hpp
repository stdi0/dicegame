#include <eosiolib/singleton.hpp>
#include "utils.hpp"

#define MINBET 1000
#define EOS_SYMBOL symbol("EOS", 4)
#define CASINOSEVENS name("sevenscasino")

struct results {
    uint64_t id;
    uint64_t game_id;
    name player;
    asset amount;
    uint64_t roll_under;
    uint64_t random_roll;
    asset payout;
    string player_seed;
    checksum256 house_seed_hash;
    signature sig;
    name referrer;
};

struct [[eosio::table, eosio::contract("sevensdice")]] bets {
        uint64_t id;
        uint64_t game_id;
        name player;
        uint64_t roll_under;
        asset amount;
        string player_seed;
        checksum256 house_seed_hash;
        uint64_t created_at;
        name referrer;
        uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(bets, (id)(game_id)(player)(roll_under)(amount)(player_seed)(house_seed_hash)(created_at)(referrer))
    };

    struct [[eosio::table, eosio::contract("sevensdice")]] environments {
        public_key pub_key;
        uint8_t casino_fee;
        double ref_bonus;
        double player_bonus;
        asset locked;
        uint64_t next_id;
        uint64_t primary_key() const { return 0; }

        EOSLIB_SERIALIZE(environments, (pub_key)(casino_fee)(ref_bonus)(player_bonus)(locked)(next_id))
    };

    struct [[eosio::table, eosio::contract("sevensdice")]] logs {
        uint64_t game_id;
        asset amount;
        asset payout;
        uint64_t created_at;
        uint64_t primary_key() const { return game_id; }

        EOSLIB_SERIALIZE(logs, (game_id)(amount)(payout)(created_at))
    };

    typedef multi_index<"bets"_n, bets> _tbet;
    typedef multi_index<"logs"_n, logs> _tlogs;
    typedef multi_index<"envs"_n, environments> _tenvironments;