#include <gtest/gtest.h>

#include <vector>
#include <memory>
#include <optional>
#include <functional>

namespace risk {

namespace rules {

class PlayerNotInTurn : public std::exception
{
public:
};

class Card {};

class Player {
public:
    using Id = int;
    Player(Id id)
        : id_(id)
    {}
    auto id() const { return id_; }
private:
    int id_;
};

class Territory {
public:
    using Id = int;
private:
    std::string_view name;
};

class Board {};

enum class Phase {
    Placing,
    Playing,
};

class State {
public:
    State(Board board, Phase phase, std::vector<Player> players, std::vector<Card> cards)
        : board_(board)
        , phase_(phase)
        , players_(players)
        , cards_(cards)
    {}

    auto board() const { return board_; }
    auto phase() const { return phase_; }
    auto players() const { return players_; }
    auto current_player() const { return players_.at(0); }
    auto cards() const { return cards_; }

private:
    Board board_;
    Phase phase_;
    std::vector<Player> players_;
    std::vector<Card> cards_;
};

using Dice = std::function<int ()>;

class Game {
public:
    Game(std::vector<Player> players, Dice dice)
        : state_(Board{}, Phase::Placing, players, {})
        , dice_(std::move(dice))
    {
        decide_starting_player();
    }

    void decide_starting_player();

    const auto& state() const { return state_; }
    void update(State new_state) { state_ = new_state; }

    int roll_dice() const { return dice_(); }

    void place_unit(Player::Id id, Territory::Id territory);

    template <typename Error>
    void ensure(bool condition, Error err)
    {
        if (!condition) {
            throw err;
        }
    }

    bool is_player_turn(Player::Id id) const
    {
        return state().current_player().id() == id;
    }

    bool player_exists(Player::Id id) const
    {
        auto players = state().players();
        return std::any_of(
            std::begin(players), std::end(players),
            [id] (const Player& player) {
                return player.id() == id;
            }
        );
    }

private:
    State state_;
    Dice dice_;
};

void Game::decide_starting_player()
{
    const int dice = roll_dice();

    auto players = state().players();
    std::rotate(std::begin(players), std::begin(players) + dice - 1, std::end(players));

    auto new_state = State{
        state().board(),
        Phase::Placing,
        players,
        state().cards(),
    };

    update(new_state);
}

void Game::place_unit(Player::Id player_id, Territory::Id territory_id)
{
    // TODO: Correct phase
    // TODO: Player exists

    ensure(player_exists(player_id), std::out_of_range("Player ID not in range"));
    ensure(is_player_turn(player_id), PlayerNotInTurn{});

    // TODO: Territory exists
    // TODO: Any unclaimed territories?
    // TODO:  - Selected territory must be unclaimed
    // TODO: Else:
    // TODO:  - Territory must be claimed by player

    auto next_players_turn = [this] {
        auto players = state().players();
        std::rotate(std::begin(players), std::begin(players) + 1, std::end(players));
        return players;
    };

    auto new_state = State{
        state().board(),
        Phase::Placing,
        next_players_turn(),
        state().cards(),
    };

    update(new_state);
}

}

}

using namespace risk::rules;

struct PlacementPhaseFixture : public ::testing::Test
{
    PlacementPhaseFixture()
        : next_dice_throws(start_with_player(1))
        , game(
            {Player{1}, Player{2}, Player{3}},
            [this] { return throw_dice(); }
          )
    {
    }

protected:
    int throw_dice()
    {
        if (next_dice_throws.empty()) {
            throw std::runtime_error("next_dice_throws is empty");
        }
        auto eyes = next_dice_throws.front();
        next_dice_throws.erase(next_dice_throws.begin());
        return eyes;
    }

    static std::vector<int> start_with_player(Player::Id id)
    {
        return {id};
    }

protected:

    std::vector<int> next_dice_throws;
    risk::rules::Game game;
};

TEST_F(PlacementPhaseFixture, game_starts_in_placing_phase)
{
    ASSERT_EQ(risk::rules::Phase::Placing, game.state().phase());
}

TEST_F(PlacementPhaseFixture, highest_dice_throwing_player_starts)
{
    EXPECT_EQ(Player::Id{1}, game.state().current_player().id());

    next_dice_throws = start_with_player(2);
    game = Game({Player{1}, Player{2}, Player{3}}, [this] { return throw_dice(); } );

    EXPECT_EQ(Player::Id{2}, game.state().current_player().id());

    next_dice_throws = start_with_player(3);
    game = Game({Player{1}, Player{2}, Player{3}}, [this] { return throw_dice(); } );

    EXPECT_EQ(Player::Id{3}, game.state().current_player().id());
}

TEST_F(PlacementPhaseFixture, player1_places_a_unit)
{
    EXPECT_EQ(Player::Id{1}, game.state().current_player().id());
    ASSERT_NO_THROW(game.place_unit(Player::Id{1}, Territory::Id{0}));

    ASSERT_EQ(risk::rules::Phase::Placing, game.state().phase());
    EXPECT_EQ(Player::Id{2}, game.state().current_player().id());
}

TEST_F(PlacementPhaseFixture, player2_tries_to_place_a_unit_when_not_in_turn)
{
    EXPECT_EQ(Player::Id{1}, game.state().current_player().id());
    ASSERT_THROW(game.place_unit(Player::Id{2}, Territory::Id{0}), risk::rules::PlayerNotInTurn);

    ASSERT_EQ(risk::rules::Phase::Placing, game.state().phase());
    EXPECT_EQ(Player::Id{1}, game.state().current_player().id());
}

TEST_F(PlacementPhaseFixture, player1_places_a_unit_then_player2)
{
    EXPECT_EQ(Player::Id{1}, game.state().current_player().id());
    ASSERT_NO_THROW(game.place_unit(Player::Id{1}, Territory::Id{0}));
    ASSERT_NO_THROW(game.place_unit(Player::Id{2}, Territory::Id{0}));

    ASSERT_EQ(risk::rules::Phase::Placing, game.state().phase());
    EXPECT_EQ(Player::Id{3}, game.state().current_player().id());
}

TEST_F(PlacementPhaseFixture, player1_places_a_unit_then_player2_then_player3)
{
    EXPECT_EQ(Player::Id{1}, game.state().current_player().id());
    ASSERT_NO_THROW(game.place_unit(Player::Id{1}, Territory::Id{0}));
    ASSERT_NO_THROW(game.place_unit(Player::Id{2}, Territory::Id{0}));
    ASSERT_NO_THROW(game.place_unit(Player::Id{3}, Territory::Id{0}));

    ASSERT_EQ(risk::rules::Phase::Placing, game.state().phase());
    EXPECT_EQ(Player::Id{1}, game.state().current_player().id());
}

TEST_F(PlacementPhaseFixture, unknown_player_tries_to_place_a_unit)
{
    ASSERT_THROW(game.place_unit(Player::Id{4}, Territory::Id{0}), std::out_of_range);
    // TODO: Assert game state unchanged
}

TEST_F(PlacementPhaseFixture, player_tries_to_place_unit_in_unknown_territory)
{
    EXPECT_EQ(Player::Id{1}, game.state().current_player().id());
    // TODO:
    // ASSERT_THROW(game.place_unit(Player::Id{1}, Territory::Id{0}), std::out_of_range);
}

TEST_F(PlacementPhaseFixture, player_tries_to_place_unit_in_territory_claimed_by_other_player)
{
    EXPECT_EQ(Player::Id{1}, game.state().current_player().id());
    ASSERT_NO_THROW(game.place_unit(Player::Id{1}, Territory::Id{1}));
    // TODO:
    // ASSERT_THROW(game.place_unit(Player::Id{1}, Territory::Id{1}), IllegalMove{});
}

TEST_F(PlacementPhaseFixture, player_places_unit_in_territory_claimed_by_same_player)
{
    EXPECT_EQ(Player::Id{1}, game.state().current_player().id());
    ASSERT_NO_THROW(game.place_unit(Player::Id{1}, Territory::Id{1}));
    // TODO:
    // ASSERT_THROW(game.place_unit(Player::Id{1}, Territory::Id{1}), IllegalMove{});
}

TEST_F(PlacementPhaseFixture, placement_phase_ends_when_no_player_has_units_left_to_place)
{

}


