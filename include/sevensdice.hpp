#include "types.hpp"

class sevensdice : public contract {
public:
    using contract::contract;
    sevensdice(account_name self) : contract(self), tbets(_self, _self), tlogs(_self, _self), tenvironments(_self, _self){};

    /// @abi action
    void launch(public_key pub_key, uint8_t casino_fee, double ref_bonus, double player_bonus);

    /// @abi action
    void resolvebet(const uint64_t& bet_id, const signature& sig);

    template<typename T>
    void apply_transfer(T data);

    /// @abi action
    void receipt(const results& result);

    ///@abi action
    void cleanlog(uint64_t bet_id);
private:
    _tbet tbets;
    _tlogs tlogs;
    _tenvironments tenvironments;

    uint8_t get_random_roll(checksum256 hash) {
        uint64_t mix_hash = hash.hash[0] + hash.hash[2] * hash.hash[4] + hash.hash[6] * hash.hash[8] + hash.hash[10] * hash.hash[12] + hash.hash[14] * hash.hash[16];
        return (mix_hash % 100) + 1;
    }

    void check_game_id(uint64_t& game_id) {
        auto iter = tbets.begin();
        while (iter != tbets.end()) {
            eosio_assert(iter->game_id != game_id, "GameID collision detected");
            iter++;
        }
        auto exist = tlogs.find(game_id);
        eosio_assert(exist == tlogs.end(), "GameID collision detected");
    }

    void check_roll_under(uint64_t& roll_under, asset& quantity, uint8_t& fee) {
        eosio_assert(roll_under >= 2 && roll_under <= 96, "Rollunder must be >= 2, <= 96.");
        asset player_win_amount = calc_payout(quantity, roll_under, fee) - quantity;
        eosio_assert(player_win_amount <= max_win(), "Available fund overflow");
    }

    void check_quantity(asset quantity) {
        eosio_assert(quantity.is_valid(), "Invalid token");
        eosio_assert(quantity.symbol == EOS_SYMBOL, "EOS token required");
        eosio_assert(quantity.amount >= MINBET, "Minimum transfer quantity is 0.1");
    }

    asset calc_payout(const asset& quantity, const uint64_t& roll_under, const double& fee) {
        const double rate = (100.0 - fee) / (roll_under - 1);
        return asset(rate * quantity.amount, quantity.symbol);
    }

    asset max_win() {
        return available_balance() / 100;
    }

    asset available_balance() {
        auto token_contract = eosio::token(N(eosio.token));
        const asset balance = token_contract.get_balance(_self, symbol_type(EOS_SYMBOL).name());
        auto stenvironments = tenvironments.get();
        const asset available = balance - stenvironments.locked;
        eosio_assert(available.amount >= 0, "Liabilities pool overdraw");
        return available;
    }

    void unlock(const asset& amount) {
        auto stenvironments = tenvironments.get();
        asset locked = stenvironments.locked - amount;
        eosio_assert(locked.amount >= 0, "Fund lock error");
        stenvironments.locked = locked;
        tenvironments.set(stenvironments, _self);
    }

    void lock(const asset& amount) {
        auto stenvironments = tenvironments.get();
        stenvironments.locked += amount;
        tenvironments.set(stenvironments, _self);
    }

    uint64_t available_bet_id() {
        auto stenvironments = tenvironments.get();
        uint64_t bet_id = stenvironments.next_id;
        stenvironments.next_id += 1;
        tenvironments.set(stenvironments, _self);
        return bet_id;
    }

    string winner_msg(const bets& bet) {
        string msg = "Bet id: ";
        string id = to_string(bet.id);
        msg += id;
        msg += " Player: ";
        string player = name{bet.player}.to_string();
        msg += player;
        msg += "  *** Winner *** Play: eos777.io/dice";
        return msg;
    }

    string ref_msg(const bets& bet) {
        string msg = "Referrer: ";
        string referrer = name{bet.referrer}.to_string();
        msg += referrer;
        msg += " Bet id: ";
        string id = to_string(bet.id);
        msg += id;
        msg += " Player: ";
        string player = name{bet.player}.to_string();
        msg += player;
        msg += "  *** Referral reward *** Play: eos777.io/dice";
        return msg;
    }

    void parse_game_params(const string data,
                    uint64_t* roll_under,
                    account_name* referrer,
                    string* player_seed,
                    uint64_t* game_id) {
        size_t sep_count = count(data.begin(), data.end(), '-');
        eosio_assert(sep_count == 3, "Invalid memo");

        size_t pos;
        string part;
        pos = str_to_sep(data, &part, '-', 0);
        eosio_assert(!part.empty(), "No rollunder");
        *roll_under = stoi(part);
        pos = str_to_sep(data, &part, '-', ++pos);
        eosio_assert(!part.empty(), "No referrer");
        *referrer = eosio::string_to_name(part.c_str());
        pos = str_to_sep(data, &part, '-', ++pos);
        *player_seed = part;
        part = data.substr(++pos);
        eosio_assert(!part.empty(), "No gameID");
        *game_id = stoi(part); // Попробовать передать строку!
    }
};

extern "C" {
void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    sevensdice thiscontract(receiver);

    if ((code == N(eosio.token)) && (action == N(transfer))) {
        thiscontract.apply_transfer(unpack_action_data<eosio::token::transfer_args>());
    }

    if (code != receiver) return;

    switch(action) { EOSIO_API(sevensdice, (launch)(resolvebet)(cleanlog)(receipt)) }
    eosio_exit(0);
}
}
