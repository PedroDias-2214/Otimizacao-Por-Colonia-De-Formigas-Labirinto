#include "Formiga.h"
#include <cmath>
#include <numeric>
#include <algorithm>



Formiga::Formiga(Labirinto &labirinto, const double alfa, const double beta)
    :   lab(labirinto),
        alfa(alfa),
        beta(beta),
        pos_ninho(lab.get_pos_ninho()),
        pos_comida(lab.get_pos_comida()),
        gen(std::random_device{}())
{
    matriz_exploracao.resize(labirinto.get_largura(), std::vector<bool>(lab.get_altura(), false));
    Formiga::reset();
}

void Formiga::reset() {
    m_encontrou_comida = false;
    m_fracassou = false;
    pos_atual = pos_ninho;

    // Limpa os vetores usados e joga a primeira variável neles
    pilha_solucao.clear();
    for(auto &coluna : matriz_exploracao) {
        std::fill(coluna.begin(), coluna.end(), false);
    }
    matriz_exploracao[pos_ninho.x][pos_ninho.y] = true;
    pilha_solucao.push_back(pos_ninho);
}

void Formiga::mover() {
    if (m_encontrou_comida || m_fracassou) {
        return; // sai do método
    }

    const std::vector<Pos> caminhos = this->get_caminhos_validos();

    if (caminhos.empty()) {
        if (pilha_solucao.size() < 2) {
            this->m_fracassou = true;
            return;
        }

        this->pilha_solucao.pop_back(); // remove o ultimo elemento (pos atual)
        this->pos_atual = pilha_solucao.back(); // move a formiga pra pos anterior
        // isso mostra que entrou num lugar sem saída
    }
    else {
        const std::vector<double> chances = Formiga::calcular_chance(caminhos);

        this->pos_atual = Formiga::escolher_proximo(caminhos, chances);
        this->pilha_solucao.push_back(this->pos_atual);

        matriz_exploracao[pos_atual.x][pos_atual.y] = true;

        if (this->pos_atual == this->pos_comida) {
            this->m_encontrou_comida = true;
        }
    }
}

void Formiga::falhar() {
    this->m_fracassou = true;
}

bool Formiga::encontrou_comida() const {
    return m_encontrou_comida;
}

bool Formiga::falhou() const {
    return m_fracassou;
}

const std::vector<Pos>& Formiga::get_pilha_solucao() const {
    return pilha_solucao;
}

std::vector<Pos> Formiga::get_caminhos_validos() const {
    std::vector<Pos> caminhos;
    caminhos.reserve(4);
    const std::vector<Pos> movimentos = {
        {this-> pos_atual.x - 1, this-> pos_atual.y},
        {this-> pos_atual.x + 1, this-> pos_atual.y},
        {this-> pos_atual.x, this-> pos_atual.y - 1},
        {this-> pos_atual.x, this-> pos_atual.y + 1}
    };

    for (const Pos& p : movimentos) {

        // verifica se está dentro do grid
        if (p.x < 0 || p.x >= this-> lab.get_largura()
            || p.y < 0 || p.y >= this-> lab.get_altura()) {
            continue;
        }

        if (lab.get_valor_grid(p) != 1 && !matriz_exploracao[p.x][p.y]) {
            caminhos.push_back(p);
        }
    }
    return caminhos;
}

double Formiga::get_distancia_heuristica(const Pos& p) const {
    // dist é a distância que ela está da comida, como se fosse para sentir o "cheiro" da comida
    const int dist = std::abs(p.x - pos_comida.x) + std::abs(p.y - pos_comida.y);

    return 1.0 / (static_cast<double>(dist) + 1e-5); // +1e-5 é para evitar que o valor seja 0
}

std::vector<double> Formiga::calcular_chance(const std::vector<Pos>& caminhos) const {
    std::vector<double> valores_atratividade;
    valores_atratividade.reserve(caminhos.size()); // parte de otimização (alocando a memória só 1x)

    for (const Pos& p : caminhos) {
        double feromonio = lab.get_feromonio(p);
        const double heuristica = get_distancia_heuristica(p);

        if (feromonio < 1e-10) feromonio = 1e-10; // evita que feromonio seja 0

        valores_atratividade.push_back(std::pow(feromonio, alfa)
            * std::pow(heuristica, beta));
    }
    const double soma_valores = std::accumulate(valores_atratividade.begin(),
        valores_atratividade.end(), 0.0); // soma todos os valores do primeiro até o último de
    // valores_atratividade, 0,0 diz que ele começa com esse valor

    if (soma_valores == 0.0) {
        return std::vector<double>(caminhos.size(), 1.0 / caminhos.size());
    }

    // sintaxe esquisita, mas esse comando vai somar os valores e dividir cada um dos valores pela soma deles
    // fazendo com que a soma de tudo vire 1,0, ajustando para a porcentagem
    std::ranges::transform(valores_atratividade,
                           valores_atratividade.begin(),
                           [soma_valores](const double v) { return v / soma_valores; });

    return valores_atratividade;
}

Pos Formiga::escolher_proximo(const std::vector<Pos> &caminhos, const std::vector<double> &chances) {
    std::discrete_distribution<> dist(chances.begin(), chances.end());
    int const indice_escolhido = dist(gen);
    return caminhos[indice_escolhido];
}