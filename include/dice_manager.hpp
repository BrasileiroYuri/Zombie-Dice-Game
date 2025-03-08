/**
 * @file DiceBag.hpp
 * @brief Definições para o sistema de dados do Zombie Dice
 * @copyright Copyright (c) 2024
 * @license MIT License
 *
 * Este arquivo implementa a estrutura básica dos dados e o saco de dados usado no jogo,
 * incluindo tipos de dados, faces e mecânicas de rolagem.
 */

#ifndef DICE_BAG_HPP
#define DICE_BAG_HPP

#include <algorithm>
#include <random>
#include <string>
#include <utility>
#include <vector>

/**
 * @enum DieFace
 * @brief Representa os possíveis resultados de uma rolagem de dado
 */
enum DieFace {
  BRAIN = 'b',  ///< Jogador ganha um cérebro
  SHOT = 's',   ///< Jogador leva um tiro
  RUN = 'f'     ///< Vítima foge (footprint)
};

/**
 * @enum DieType
 * @brief Tipos de dados disponíveis no jogo
 */
enum DieType {
  WEAK,   ///< Dado verde (fraco) - Maior chance de cérebros
  TOUGH,  ///< Dado amarelo (resistente) - Equilíbrio médio
  STRONG  ///< Dado vermelho (forte) - Maior chance de tiros
};

/**
 * @struct ZDie
 * @brief Representa um dado individual do jogo
 *
 * @var ZDie::type Tipo do dado (WEAK/TOUGH/STRONG)
 * @var ZDie::faces Sequência de faces configuráveis
 * @var ZDie::face Resultado atual da rolagem
 */
struct ZDie {
  DieType type;
  std::string faces;
  char face;

  /**
   * @brief Constrói um novo dado
   * @param type Tipo do dado
   * @param faces Sequência de 6 caracteres representando as faces (b/s/f)
   */
  ZDie(const DieType& type, const std::string& faces) : type{ type }, faces{ faces } {}

  /**
   * @brief Rola o dado e atualiza a face atual
   */
  void roll() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(0, faces.size() - 1);
    face = faces[distrib(gen)];
  }
};

/**
 * @class DiceBag
 * @brief Gerenciador de dados do jogo
 *
 * Responsável por:
 * - Armazenar a configuração dos diferentes tipos de dados
 * - Criar e embaralhar os dados
 * - Gerenciar o ciclo de vida dos dados durante o jogo
 */
class DiceBag {
private:
  /// @brief Cria os dados baseado na configuração dice_and_faces
  void create_dice() {
    for (const auto& t : dice_and_faces) {
      dice.insert(dice.end(), std::get<1>(t), { std::get<0>(t), std::get<2>(t) });
    }
  }

  /// @brief Embaralha os dados usando algoritmo Fisher-Yates
  void shuffle_dice() {
    std::shuffle(dice.begin(), dice.end(), std::mt19937{ std::random_device{}() });
  }

  std::vector<ZDie> dice;  ///< Vetor de dados ativos

public:
  /**
   * @brief Configuração padrão dos dados
   *
   * Tuple contendo:
   * 1. Tipo do dado
   * 2. Quantidade deste tipo
   * 3. Sequência de faces (6 caracteres)
   *
   * Configuração padrão:
   * - 6 dados fracos:   3🧠 2👣 1💥
   * - 3 dados resistentes: 1🧠 2👣 3💥
   * - 4 dados fortes:   2🧠 2👣 2💥
   */
  std::vector<std::tuple<DieType, size_t, std::string>> dice_and_faces{
    { WEAK, 6, "bbbffs" },
    { TOUGH, 3, "bffsss" },
    { STRONG, 4, "bbffss" },
  };

  /**
   * @brief Reinicializa o saco de dados
   *
   * 1. Limpa dados existentes
   * 2. Recria dados baseado na configuração
   * 3. Embaralha os dados
   */
  void init() {
    dice.clear();
    create_dice();
    shuffle_dice();
  }

  /**
   * @brief Acessa os dados do saco
   * @return Referência para o vetor de dados
   */
  inline std::vector<ZDie>& get_dice() { return dice; }
};

#endif  // !DICE_BAG_HPP
