# Deploy contract

cleos create account eosio addressbook EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV -p eosio@active

cleos set contract addressbook /home/anthony/edev/contracts-eos/addressbook -p addressbook@active

# Test Contract

cleos push action addressbook upsert '["alice", "alice", "liddell", "123 drink me way", "wonderland", "amsterdam"]' -p alice@active

<!-- Fails -->
cleos push action addressbook upsert '["bob", "bob", "is a loser", "doesnt exist", "somewhere", "someplace"]' -p alice@active

cleos get table addressbook addressbook people --lower alice --limit 1

cleos push action addressbook erase '["alice"]' -p alice@active