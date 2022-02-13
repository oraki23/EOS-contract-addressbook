#include <eosio/eosio.hpp>
#include "abcounter.cpp"

using namespace eosio;

/*
    "name" is a special type of eosio (a uint64_t integer)

    [[ ... ]] are ABI Action Declarations
*/
class [[eosio::contract("addressbook")]] addressbook : public eosio::contract
{
    public:

    //Constructor
    // code: account on the blockchain that the contract is being deployed to.
    addressbook(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds) {}

    [[eosio::action]]
    void upsert(name user, std::string first_name, std::string last_name, uint64_t age, std::string street, std::string city, std::string state){
        require_auth(user);
        /*
            To instantiate a table, two parameters are required, 
            1. parameter "code", which specifies the owner of this table.
                The account will be charged for storage costs
            2. parameter "scope" which ensures the uniqueness of the table in the scope of this contract.
                We can use the get_first_receiver function for our case.
                    The value returned from the get_first_receiver function is the account name on which this contract is deployed to.
        */
        address_index addresses(get_self(), get_first_receiver().value);
        auto iterator = addresses.find(user.value);
        // "end" is an alias for "null"
        if(iterator == addresses.end()){
            //The user isn't in the table
            addresses.emplace(user, [&](auto& row){
                row.key = user;
                row.first_name = first_name;
                row.last_name = last_name;
                row.age = age;
                row.street = street;
                row.city = city;
                row.state = state;
            });

            increment_counter(user, "emplace");
            send_summary(user, "successfully emplaced record to addressbook");
        } else{
            std::string changes;
            //The user is in the table
            addresses.modify(iterator, user, [&](auto& row){
                row.key = user;
                if(row.first_name != first_name) {
                    row.first_name = first_name;
                    changes += "first name ";
                }

                if(row.last_name != last_name) {
                    row.last_name = last_name;
                    changes += "last name ";
                }

                if(row.age != age) {
                    row.age = age;
                    changes += "age ";
                }

                if(row.street != street) {
                    row.street = street;
                    changes += "street ";
                }

                if(row.city != city) {
                    row.city = city;
                    changes += "city ";
                }

                if(row.state != state) {
                    row.state = state;
                    changes += "state ";
                }
            });
            if(!changes.empty()) {
                increment_counter(user, "modify");
                send_summary(user, " successfully modified record in addressbook. Fields changed: " + changes);
            } else {
                send_summary(user, " called upsert, but request resulted in no changes.");
            }
        }
    }

    [[eosio::action]]
    void erase(name user){
        require_auth(user);
        address_index addresses(get_self(), get_first_receiver().value);
        auto iterator = addresses.find(user.value);
        check(iterator != addresses.end(), "Record does not exist");
        addresses.erase(iterator);

        increment_counter(user, "erase");
        send_summary(user, "successfully erased record from addressbook");
    }

    [[eosio::action]]
    void notify(name user, std::string msg) {
        // Validate that the call is from the contract itself
        require_auth(get_self());

        // Ensures that these accounts
        // receive a notification of the action being executed. 
        require_recipient(user);
    }

    private:
        struct [[eosio::table]] person{
            name key;
            std::string first_name;
            std::string last_name;
            std::string street;
            std::string city;
            std::string state;
            uint64_t age;

            uint64_t primary_key() const { return key.value; }
            uint64_t get_secondary_1() const { return age; }
        };
        using address_index = eosio::multi_index<"people"_n, person,
            indexed_by<"byage"_n, const_mem_fun<person, uint64_t, &person::get_secondary_1>>
            >;

        void send_summary(name user, std::string message) { 
            action(
                //permission_level,
                permission_level{get_self(), "active"_n},
                //code (AKA account where contract is deployed),
                get_self(),
                //action,
                "notify"_n,
                //data
                std::make_tuple(user, name{user}.to_string() + message)
            ).send();
         }

         void increment_counter(name user, std::string type){
             abcounter::count_action count("abcounter"_n, {get_self(), "active"_n});
             count.send(user, type);
         }

};

