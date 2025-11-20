#include "Labirinto.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>

#include "Formiga.h"

constexpr int CHANCE_PAREDE_ADICIONAL = 20; // 5 = 5%, 20 = 20%

Labirinto::Labirinto(const int largura, const int altura, const bool labirinto_dificil) {
    if (largura <= 10 || altura <= 10) {
        throw std::runtime_error("Altura e Largura devem ser maiores que 10!");
    }
    std::cout << "LABIRINTO DIFICIL: " << std::boolalpha << labirinto_dificil << std::endl;
    std::cout << std::noboolalpha;

    this->largura = largura;
    this->altura = altura;
    this->grid.resize(largura, std::vector<int>(altura, 0));
    this->feromonios.resize(largura, std::vector<double>(altura, 0.5));

    if (!labirinto_dificil) Labirinto::criar_grid();
    else Labirinto::criar_grid_dificil();
}

void Labirinto::criar_grid_dificil() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dis(1, 100);

    bool labirinto_valido = false;

    while (!labirinto_valido) {
        for (int i=0; i<this->largura; i++) {
            std::ranges::fill(this->grid[i], 0); // Reseta o labirinto
        }
        for (int i=0; i<this->largura; i++) {
            for (int j=0; j<this->altura; j++) {
                if ((i%2 == 0 && j%2 == 0) || i == 0 || i == this->largura-1 || j == this->altura-1 || j == 0) {
                    this->grid[i][j] = 1; // Gera os pilares iniciais
                }

                else if ((i%2 == 0 && j%2 == 1) || (i%2 == 1 && j%2 == 0)) {
                    if (dis(gen) <= CHANCE_PAREDE_ADICIONAL) {
                        this->grid[i][j] = 1; // Gera os pilares aleatórios
                    }
                }
            }
        }

        this-> pos_ninho = {1, this->altura-2};
        this-> pos_comida = {this->largura-2, 1};

        this-> grid[this-> pos_ninho.x][this-> pos_ninho.y] = 3; // ninho
        this-> grid[this-> pos_comida.x][this-> pos_comida.y] = 2; // comida

        Formiga formiga = Formiga(*this, 0, 5);
        // gera uma formiga que vai utilizar apenas da heurística para resolver o labirinto, o tamanho_volta dela
        // é muito grande, então se ela não encontrar a solução, é porque o problema é impossível

        while (!formiga.encontrou_comida() && !formiga.falhou()) {
            formiga.mover();
        }
        if (formiga.encontrou_comida()) {
            std::cout << "Labirinto valido!" << std::endl;
            labirinto_valido = true;
        }

        else {
            std::cout << "Labirinto invalido, gerando novamente!" << std::endl;
        }

    }
}


void Labirinto::criar_grid() {
    for (int i=0; i<this->largura; i++) {
        for (int j=0; j<this->altura; j++) {
            if ((i%2 == 0 && j%2 == 0) || i == 0 || i == this->largura-1 || j == this->altura-1 || j == 0) {
                this->grid[i][j] = 1;
            }
            // se não cair nesse if, o valor continua a ser 0
        }
    }

    this-> pos_comida = {1, this->altura-2};
    this-> pos_ninho = {this->largura-2, 1};

    this-> grid[this-> pos_ninho.x][this-> pos_ninho.y] = 3; // ninho
    this-> grid[this-> pos_comida.x][this-> pos_comida.y] = 2; // comida
}

void Labirinto::evaporar_feromonios(const double taxa_evaporacao, const double feromonio_minimo) {
    for (int i=0; i<this->largura; i++) {
        for (int j=0; j<this->altura; j++) {
            feromonios[i][j] *= (1.0 - taxa_evaporacao);
            feromonios[i][j] = std::max(feromonios[i][j], feromonio_minimo);
        }
    }
}

void Labirinto::depositar_feromonios(const std::vector<Pos> &caminho, const double intensidade) {
    if (caminho.empty()) {
        return;
    }
    const double deposito = intensidade / static_cast<double>(caminho.size());
    for (const Pos& p : caminho) {
        feromonios [p.x][p.y] += deposito;
    }
}

// Getters

int Labirinto::get_largura() const{
    return this->largura;
}

int Labirinto::get_altura() const {
    return this->altura;
}

Pos Labirinto::get_pos_ninho() const {
    return this->pos_ninho;
}

Pos Labirinto::get_pos_comida() const {
    return this->pos_comida;
}

int Labirinto::get_valor_grid(const Pos &p) const {
    if (p.x < 0 || p.x >= this->largura || p.y < 0 || p.y >= this->altura) {
        throw std::runtime_error("Posicao fora da grid!");
    }
    return this->grid[p.x][p.y];
}

double Labirinto::get_feromonio(const Pos &p) const {
    if (p.x < 0 || p.x >= this->largura || p.y < 0 || p.y >= this->altura) {
        throw std::runtime_error("Posicao fora da grid!");
    }
    return this->feromonios[p.x][p.y];
}

void Labirinto::print_grid() const {
    for (int i=0; i<this->largura; i++) {
        for (int j=0; j<this->altura; j++) {
            std::cout << this->grid[i][j] << ' ';
        }
        std::cout << '\n';
    }
}

void Labirinto::print_feromonios() const {
    // "seta" a precisão para 2, mostrando duas casas decimais apenas
    std::cout << std::fixed << std::setprecision(2) << std::endl;

    for (int i=0; i<this->largura; i++) {
        for (int j=0; j<this->altura; j++) {
            std::cout << this->feromonios[i][j] << ' ';
        }
        std::cout << '\n';
    }
    // volta a printar do mesmo jeito de antes
    std::cout.unsetf(std::ios_base::floatfield);
}