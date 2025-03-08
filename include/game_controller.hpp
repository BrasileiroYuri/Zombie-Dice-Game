/**
 * @file game_controller.hpp
 * @brief Controller principal do jogo Zombie Dice
 * @copyright Copyright (c) 2024 DIMAp/UFRN
 * @license MIT License
 *
 * Este arquivo implementa o controle principal do jogo Zombie Dice, gerenciando:
 * - Fluxo do jogo e estados
 * - Jogadores e pontuações
 * - Interface com o usuário
 * - Configurações do jogo
 *
 * @note Emojis utilizados: 🟩🔸🟨🟥🧠💥👣🧟☣☢
 */

#ifndef GAME_CONTROLLER_HPP
#define GAME_CONTROLLER_HPP

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <string_view>
#include <unordered_map>

#include "../src/ini_parser.cpp"
#include "dice_manager.hpp"

/**
 * @struct Player
 * @brief Representa um jogador no jogo
 *
 * @var Player::name Nome do jogador
 * @var Player::brains Quantidade de cérebros acumulados
 * @var Player::turns Número de turnos jogados
 */
struct Player {
  std::string name;
  size_t brains{ 0 };
  size_t turns{ 0 };

  Player(const std::string& name) : name{ name } {}
};

/**
 * @class GameController
 * @brief Classe principal que controla a lógica do jogo
 *
 * Gerencia todos os aspectos do jogo incluindo:
 * - Máquina de estados do jogo
 * - Controle de jogadores
 * - Interface do usuário
 * - Regras do jogo
 */
class GameController {
public:
  /**
   * @brief Carrega configurações do jogo
   * @param argc Número de argumentos
   * @param argv Vetor de argumentos (espera nome do arquivo de configuração)
   */
  static void parse_config(int argc, char** argv);

  // Funções principais do game loop
  static void process_events();  ///< Processa entrada do usuário
  static void update();          ///< Atualiza estado do jogo
  static void render();          ///< Renderiza interface gráfica
  static bool game_over();       ///< Verifica se o jogo terminou

private:
  // Métodos auxiliares
  static void read_players();            ///< Lê nomes dos jogadores
  static std::string welcome_message();  ///< Mensagem inicial do jogo
  static std::string global_score();     ///< Gera placar global
  static std::string scoreboard();       ///< Gera tabela de rolagens
  static std::string message_area();     ///< Gera área de mensagens

  /**
   * @enum State
   * @brief Estados possíveis da máquina de estados do jogo
   */
  enum State {
    BEGIN,            ///< Estado inicial
    WELCOME_MESSAGE,  ///< Exibindo mensagem de boas-vindas
    READING_SIZE,     ///< Lendo número de jogadores
    PARSING_SIZE,     ///< Validando número de jogadores
    READING_PLAYERS,  ///< Lendo nomes dos jogadores
    INIT,             ///< Inicializando novo turno
    INIT_PLAYER,      ///< Selecionando jogador inicial
    ADDING_TURN,      ///< Incrementando contador de turnos
    PREPARING,        ///< Preparando próximo turno
    CLEANING,         ///< Limpando dados usados
    START,            ///< Início do turno do jogador
    ROLLING,          ///< Rolando dados
    HOLDING,          ///< Jogador optou por parar
    SHOW_DICE,        ///< Mostrando resultado dos dados
    PARSING_DICE,     ///< Processando resultado dos dados
    PARSING,          ///< Validando estado do jogo
    FORCE_QUIT,       ///< Turno terminado por 3+ tiros
    INIT_TIE,         ///< Inicializando desempate
    PARSING_TIE,      ///< Processando critérios de desempate
    PLAYING_TIE,      ///< Em modo de desempate
    SHOW_SCOREBOARD,  ///< Exibindo placar atual
    QUIT,             ///< Jogo finalizado pelo usuário
    END,              ///< Fim natural do jogo
    INVALID_SIZE,     ///< Número inválido de jogadores
    LESS_THAN_TWO,    ///< Menos de 2 jogadores
    INVALID_OPTION    ///< Opção inválida do menu
  };

  // Áreas de armazenamento de dados
  static DiceBag dra;  ///< Área de rolagem (Dice Rolling Area)
  static DiceBag bsa;  ///< Armazenamento de cérebros (Brain Storage Area)
  static DiceBag ssa;  ///< Armazenamento de tiros (Shot Storage Area)

  // Dados do jogo
  static std::vector<Player> players;  ///< Lista de jogadores ativos
  static State state;                  ///< Estado atual do jogo
  static size_t brains_to_win;         ///< Cérebros necessários para vencer
};

#endif  // GAME_CONTROLLER_HPP
