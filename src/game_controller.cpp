#include "../include/game_controller.hpp"

// Members:
GameController::State GameController::state{ BEGIN };
std::vector<Player> GameController::players;

size_t GameController::brains_to_win{ 13 };

DiceBag GameController::dra;
DiceBag GameController::bsa;
DiceBag GameController::ssa;

std::unordered_map<std::string, std::pair<size_t, bool>> dm_menber{
  { "weak_dice", { 0, true } },   { "weak_die_faces", { 0, false } },
  { "tough_dice", { 1, true } },  { "tough_die_faces", { 1, false } },
  { "strong_dice", { 2, true } }, { "strong_die_faces", { 2, false } },
};

// Auxiliar function:
std::string trim(const std::string& t_line) {
  auto begin = t_line.find_first_not_of(" \t\r\n");
  auto end = t_line.find_last_not_of(" \t\r\n");
  return (begin != std::string::npos ? t_line.substr(begin, end - begin + 1) : "");
}

std::string get_emoji(char c) {
  switch (c) {
  case BRAIN:
    return "🧠";
  case SHOT:
    return "💥";
  case RUN:
    return "👣";
  default:
    return "";
  }
}

std::string get_color(char c) {
  switch (c) {
  case WEAK:
    return "🟩";
  case TOUGH:
    return "🟨";
  case STRONG:
    return "🟥";
  default:
    return "";
  }
}

// Auxiliar members:
auto idx{ 0 };
char input;
bool tie{ false };
std::string size;
std::vector<ZDie> actual_dice;
std::vector<Player> removed_players;

std::string GameController::welcome_message() {
  std::string message = R"(

           ---> Welcome to the Zombi Dice game (v 0.1) <--
                     -copyright DIMAp/UFRN 2024-

    The object of the jeopardy dice game Zombie Dice is to be the
    first to eat 13 or more human brains in a turn.
    Each player's turn consists of repeatedly rolling 3 dice.
    A die may produce 3 outcomes: you ate a brain, the human escaped,
    or you were shot!
    If you get shot 3 or more times you die and loose all brains you
    ate in that turn; then the turn goes to the next player.
    However, if you decide to stop before get shot 3 times, you keep
    the brains you ate.

)";
  return message;
}

void GameController::parse_config(int argc, char** argv) {

  if (argc == 2) {
    IniParser parser(argv[1]);
    auto btw = parser.get_map().find("brains_to_win");

    if (btw != parser.get_map().end()) {

      bool isdigit = std::all_of(
        btw->second.begin(), btw->second.end(), [](char c) { return std::isdigit(c); });

      if (isdigit) {
        brains_to_win = std::stoi(btw->second);
      }
    }

    for (const auto& p : parser.get_map()) {
      auto it = dm_menber.find(p.first);
      if (it != dm_menber.end()) {
        if (it->second.second) {

          bool isdigit
            = std::all_of(p.second.begin(), p.second.end(), [](char c) { return std::isdigit(c); });
          if (isdigit) {
            std::get<1>(dra.dice_and_faces[it->second.first]) = std::stoi(p.second);
          }
        } else {

          bool isvalid = std::all_of(p.second.begin(), p.second.end(), [](char c) {
            return c == 'b' or c == 'f' or c == 's';
          });

          if (isvalid) {
            std::get<2>(dra.dice_and_faces[it->second.first]) = p.second;
          }
        }
      }
    }
  } else if (argc > 2) {
    std::cout << "Too many arguments!\n";
    exit(1);
  }
}

// Game loop architeture:
void GameController::process_events() {
  switch (state) {
  case INVALID_SIZE:
  case LESS_THAN_TWO:
  case READING_SIZE:
    std::cin >> size;
    break;
  case INVALID_OPTION:
  case INIT:
  case SHOW_DICE:
  case FORCE_QUIT:
  case INIT_TIE:
    std::cin.get();
    break;
  case START:
  case SHOW_SCOREBOARD:
    std::cin.get(input);
    break;
  default:
    break;
  }
};

void GameController::update() {
  switch (state) {
  case BEGIN:
    state = WELCOME_MESSAGE;
    break;
  case WELCOME_MESSAGE:
    state = READING_SIZE;
    break;
  case INVALID_SIZE:
  case LESS_THAN_TWO:
  case READING_SIZE:

    state = std::all_of(size.begin(), size.end(), [](char c) { return std::isdigit(c); })
              ? PARSING_SIZE
              : INVALID_SIZE;

    break;
  case PARSING_SIZE: {

    auto sz{ std::stoi(size) };

    if (sz > 1) {
      players.reserve(sz);
      state = READING_PLAYERS;
    } else {
      state = LESS_THAN_TWO;
    }

    break;
  }

  case READING_PLAYERS:

    std::cin.ignore();
    read_players();
    state = INIT_PLAYER;

    break;

  case INIT_PLAYER: {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(0, players.size() - 1);
    idx = distrib(gen);
    state = INIT;
    break;
  }
  case ADDING_TURN: {
    players[idx].turns++;

    auto turn = std::all_of(
      players.begin(), players.end(), [](auto p) { return p.turns == players[0].turns; });
    auto btw = std::any_of(
      players.begin(), players.end(), [](auto p) { return p.brains >= brains_to_win; });

    if (turn and btw) {
      state = PARSING_TIE;
      break;
    }
  }
  case PREPARING:
    idx = idx + 1 == players.size() ? 0 : ++idx;
  case CLEANING:

    bsa.get_dice().clear();
    ssa.get_dice().clear();
    actual_dice.clear();

  case INIT:
    dra.init();
    state = START;
    break;

  case PARSING_TIE: {
    auto max = std::max_element(players.begin(), players.end(), [](auto p1, auto p2) {
                 return p1.brains < p2.brains;
               })->brains;

    for (auto i{ 0 }; i < players.size(); i++) {
      if (players[i].brains < (tie ? max : brains_to_win)) {
        removed_players.push_back(players[i]);
        players.erase(players.begin() + i);
      }
    }

    if (players.size() > 1) {
      state = INIT_TIE;
      dra.init();
      tie = true;
    } else {
      state = END;
    }
    actual_dice.clear();
    bsa.get_dice().clear();
    ssa.get_dice().clear();

    break;
  }
  case INIT_TIE:
    state = INIT_PLAYER;
    break;

  case SHOW_SCOREBOARD:
  case INVALID_OPTION:
  case START:

    switch (input) {
    case '\n':
      state = ROLLING;
      break;
    case 'h':
      state = HOLDING;
      break;
    case 'q':
      state = QUIT;
      break;
    default:
      state = INVALID_OPTION;
    }

    break;
  case ROLLING:

    if (dra.get_dice().size() < 3) {
      dra.get_dice().insert(dra.get_dice().end(), bsa.get_dice().begin(), bsa.get_dice().end());
      bsa.get_dice().clear();
    }

    for (auto rit{ dra.get_dice().rbegin() }; rit != dra.get_dice().rbegin() + 3; rit++) {
      rit->roll();
      actual_dice.push_back(*rit);
    }

    dra.get_dice().erase(dra.get_dice().end() - 3, dra.get_dice().end());
    state = SHOW_DICE;

    break;
  case HOLDING:

    std::cin.ignore();
    players[idx].brains += bsa.get_dice().size();
    state = ADDING_TURN;

    break;
  case SHOW_DICE:
    state = PARSING_DICE;
    break;
  case PARSING_DICE:

    for (const auto& d : actual_dice) {
      if (d.face == 'b') {
        bsa.get_dice().push_back(d);
      } else if (d.face == 's') {
        ssa.get_dice().push_back(d);
      } else {
        dra.get_dice().push_back(d);
      }
    }

    state = PARSING;
    break;
  case PARSING:

    if (ssa.get_dice().size() > 2) {
      state = FORCE_QUIT;
    } else {
      state = SHOW_SCOREBOARD;
      actual_dice.clear();
    }

    break;
  case FORCE_QUIT:
    state = ADDING_TURN;
    break;
  default:
    break;
  }
};

void GameController::render() {
  std::ostringstream oss;
  switch (state) {
  case WELCOME_MESSAGE:
    oss << welcome_message();
    break;
  case READING_SIZE:
    oss << ">>> How many players (min 2)?\n";
    break;
  case INVALID_SIZE:
    oss << ">>> Invalid size! Try again:\n";
    break;
  case LESS_THAN_TWO:
    oss << ">>> At least two players! Try again:\n";
    break;
  case INIT:
    oss << "\n>>> The player who will start the game is \"" << players[idx].name
        << "\"\nPress <Enter> to start the match.";
    break;
  case INVALID_OPTION:
    oss << ">>> Invalid option! Try again:\n";
    break;
  case QUIT:
  case END:
  case START:
  case INIT_TIE:
    oss << global_score();
  case SHOW_DICE:
  case FORCE_QUIT:
  case SHOW_SCOREBOARD:
    oss << scoreboard();
    oss << message_area();
  default:
    break;
  }
  std::cout << oss.str();
  oss.clear();
}

void GameController::read_players() {
  std::string names;
  std::vector<std::string> vec;

  while (true) {
    std::cout << ">>> Enter the name of the " << players.capacity()
              << " players, separated by comma (ex.: \"player #1, player #2\" etc.):\n";
    std::getline(std::cin, names);
    names = trim(names);

    if (names.empty()) {
      std::cout << ">>> Entrada vazia! Try again:\n";
      continue;
    }

    std::stringstream ss(names);
    std::string token;
    vec.clear();

    while (std::getline(ss, token, ',')) {
      token = trim(token);
      if (token.empty()) {
        std::cout << ">>> Um dos nomes está vazio. Certifique-se de que não há "
                     "campos vazios!\n";
        vec.clear();
        break;
      }
      vec.push_back(token);
    }

    if (vec.empty())
      continue;

    if (vec.size() != players.capacity()) {
      std::cout << ">>> Você deve digitar exatamente " << players.capacity()
                << " nomes. Foi encontrado " << vec.size() << " nome(s).\n";
      continue;
    }

    break;
  }

  for (const auto& n : vec) {
    players.push_back(n);
  }
}

std::string GameController::global_score() {
  std::ostringstream oss;

  oss << R"(
      ->💥[🧟] Zombie Dice Delux, v 0.1 [🧟]💥<-

┌────────────────────────┐
│      Global Score      │
└────────────────────────┘
)";

  auto repeat_str = [](std::string_view str, size_t count) -> std::string {
    std::string result;
    result.reserve(str.size() * count);
    for (size_t i = 0; i < count; ++i)
      result.append(str);
    return result;
  };

  const auto max_name_len
    = std::max_element(players.begin(), players.end(), [](const auto& p1, const auto& p2) {
        return p1.name.size() < p2.name.size();
      })->name.size();

  const auto max_brains_player
    = std::max_element(players.begin(), players.end(), [](const auto& p1, const auto& p2) {
        return p1.brains < p2.brains;
      });

  const size_t field_width = max_name_len + 2;

  for (size_t i = 0; i < players.size(); ++i) {
    const auto& player = players[i];
    auto sz = max_brains_player->brains;

    oss << (i == idx or state == END ? ">" : "") << std::setw(field_width - (i == idx ? 1 : 0))
        << std::right << player.name << ": " << repeat_str("🧠", player.brains)
        << repeat_str("🔸",
                      (brains_to_win <= max_brains_player->brains ? max_brains_player->brains + 5
                                                                  : brains_to_win)
                        - player.brains)
        << "│ (" << player.brains << "), # turns played: " << player.turns << "\n";
  }

  return oss.str();
}
std::string GameController::scoreboard() {

  std::ostringstream oss;

  if (state != END) {
    oss << "\nPlayer: \"" << players[idx].name << "\" │ Turn #: " << players[idx].turns + 1
        << " │ Bag has: " << dra.get_dice().size() << " 🎲.\n";
  } else {
    oss << "\nPlayer: \"" << players[0].name << "\" 🎲.\n";
  }

  oss << R"(
┌──────────────────────────┐
│      Rolling Table       │
├────────┬────────┬────────┤
)";

  if (state == SHOW_DICE) {
    oss << "│";
    for (const auto& d : actual_dice) {
      oss << " " << get_emoji(d.face) << "(" << get_color(d.type) << ") │";
    }
  } else {
    oss << "│        │        │        │";
  }
  oss << "\n└────────┴────────┴────────┘\n";

  oss << "🧠: ";
  for (const auto& d : bsa.get_dice()) {
    oss << get_color(d.type) << " ";
  }
  oss << "(" << bsa.get_dice().size() << ")\n";

  oss << "💥: ";
  for (const auto& d : ssa.get_dice()) {
    oss << get_color(d.type) << " ";
  }
  oss << "(" << ssa.get_dice().size() << ")\n\n";
  return oss.str();
}

std::string GameController::message_area() {
  std::ostringstream oss;
  oss << "┌─[Message area]─────────────────────────┐\n";
  switch (state) {
  case START:
  case SHOW_SCOREBOARD:
    oss << "│ Ready to play?                         │\n"
        << "│   <enter> - roll dices                 │\n"
        << "│   H + <enter> - hold turn              │\n"
        << "│   Q + <enter> - quit game              │";
    break;
  case SHOW_DICE: {
    auto b{ 0 }, s{ 0 };
    for (const auto& d : actual_dice) {
      if (d.face == 'b') {
        b++;
      } else if (d.face == 's') {
        s++;
      }
    }
    auto size_b = std::to_string(b).size();
    auto size_s = std::to_string(s).size();
    oss << "│ Rolling outcome:                       │\n";
    oss << "│   # brains you ate: " << b << std::setw(23 - size_b) << std::right << "│\n"
        << "│   # shots that hit you: " << s << std::setw(19 - size_s) << std::right << "│\n"
        << "│ Press <enter> to continue              │";
    break;
  }
  case INIT_TIE: {
    oss << "│ Tie break!                             │\n";

    oss << "│  Tie break players:                    │\n";
    for (const auto& p : players) {
      oss << "│  " << p.name << std::setw(42 - p.name.size()) << std::right << "│\n";
    }
    oss << "│  Removed players:                      │\n";
    for (const auto& p : removed_players) {
      oss << "│  " << p.name << std::setw(42 - p.name.size()) << std::right << "│\n";
    }
    auto size = std::to_string(brains_to_win).size();
    oss << "│ Let's play the tie break!              │\n";
    oss << "│ Press <enter> to continue              │";
    break;
  }
  case FORCE_QUIT: {
    auto s{ 0 };
    for (const auto& d : ssa.get_dice()) {
      if (d.face == 's') {
        s++;
      }
    }
    auto size = std::to_string(s).size();
    auto name = players[(idx + 1) % players.size()].name;
    oss << "│ You lost!                              │\n"
        << "│   You got " << s << " shots! 💥" << std::setw(23 - size) << std::right << "│\n"
        << "│   Next player: " << name << std::setw(28 - name.size()) << std::right << "│\n"
        << "│ Press <enter> to continue              │";
  } break;
  case END: {
    oss << "│ Game Over!                             │\n";
    auto p = players[0];
    oss << "│ Winner: " << p.name << std::setw(35 - p.name.size()) << std::right << "│\n";
    auto size = std::to_string(p.name.size()).size();
    oss << "│   Rounds played: " << p.turns << std::setw(26 - size) << std::right << "│\n";
    oss << "│   Thanks for play!                     │";
    break;
  }
  case QUIT: {

    auto size = std::to_string(players[0].turns).size();
    oss << "│  Game Over!                            │\n"
        << "│   The game has no winners!             │\n"
        << "│   Rounds played: " << players[0].turns << std::setw(26 - size) << std::right << "│\n"
        << "│   Thanks for play!                     │";

    break;
  }
  default:
    break;
  }
  oss << "\n└────────────────────────────────────────┘\n🧟>";
  return oss.str();
}
bool GameController::game_over() { return state == END or state == QUIT; }
