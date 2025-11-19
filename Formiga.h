#ifndef ACO_LABIRINTO_FORMIGA_H
#define ACO_LABIRINTO_FORMIGA_H

#include "Labirinto.h"
#include <vector>
#include <random>

class Formiga {
public:
    Formiga(Labirinto& labirinto, double alfa, double beta);
    void mover();
    void reset();
    void falhar();

    [[nodiscard]] bool encontrou_comida() const;
    [[nodiscard]] bool falhou() const;
    [[nodiscard]] const std::vector<Pos>& get_pilha_solucao() const;

private:
    [[nodiscard]] std::vector<Pos> get_caminhos_validos() const;
    [[nodiscard]] double get_distancia_heuristica(const Pos& p) const;
    [[nodiscard]] std::vector<double> calcular_chance(const std::vector<Pos>& caminhos) const;

    Pos escolher_proximo(const std::vector<Pos>& caminhos, const std::vector<double>& chances);

    Labirinto& lab;

    // HIPERPARÂMETROS
    int tamanho_volta;
    double alfa;
    double beta;

    Pos pos_ninho;
    Pos pos_comida;


    Pos pos_atual;
    bool m_encontrou_comida;
    bool m_fracassou;

    std::vector<Pos> pilha_solucao; // Usado para backtracking
    std::vector<std::vector<bool>> matriz_exploracao;

    // Gerador de números aleatórios
    std::mt19937 gen;
};


#endif //ACO_LABIRINTO_FORMIGA_H