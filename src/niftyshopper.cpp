#include <niftyshopper.hpp>

#include "contract_actions.cpp"

void niftyshopper::maintenace_check()
{
    eosio::check(!get_config().get().maintenance, "Contract is in maintenance");
}

void niftyshopper::receive_asset_transfer(
    eosio::name from,
    eosio::name to,
    std::vector<uint64_t> asset_ids,
    std::string memo)
{
}

void niftyshopper::receive_token_transfer(
    eosio::name from,
    eosio::name to,
    eosio::asset token,
    std::string memo)
{
    if (to != get_self() || memo == "ignore_memo")
    {
        return;
    }

    maintenace_check();

    // Check if the memo starts with buy:
    if (memo.rfind("buy:", 0) == 0)
    {
        // Convert memo to asset_id
        uint64_t asset_id = static_cast<uint64_t>(std::stoull(memo.erase(0, 4)));

        // Check if the contract still owns the asset
        auto asset = atomicassets::get_assets(get_self()).require_find(asset_id, "Contract does not own asset");

        // Get store
        auto store = get_store();
        auto entity = store.require_find(asset->template_id, "Asset is not for sale");

        // Check if item can be bought
        eosio::check(entity->token_contract == get_first_receiver(), "Token contract does not match");
        eosio::check(entity->buy_price == token, "Token price does not match");

        // Send item
        eosio::action(
            eosio::permission_level{get_self(), eosio::name("active")},
            atomicassets::ATOMICASSETS_ACCOUNT,
            eosio::name("transfer"),
            std::make_tuple(
                get_self(),
                from,
                std::vector<uint64_t>{asset_id},
                std::string("Bought NFT")))
            .send();
    }
    else
    {
        eosio::check(false, "Invalid memo");
    }
}