#ifndef ACO_LABIRINTO_LABIRINTO_H
#define ACO_LABIRINTO_LABIRINTO_H
#include <vector>

struct Pos {
    int x, y;

    bool operator==(const Pos& other) const {
        if (x==other.x && y==other.y) return true;

        return false;
    } // Isso funciona para mudar o '==' da struct, basicamente a forma de comparação entre duas structs iguais
};

class Labirinto {
public:
    Labirinto(int largura, int altura, bool labirinto_dificil);
    void print_grid() const;
    void print_feromonios() const;
    void evaporar_feromonios(double taxa_evaporacao, double feromonio_minimo);
    void depositar_feromonios(const std::vector<Pos>& caminho, double intensidade);

    // nodiscard significa que quando a função for chamada, o valor que ela retorna é importante, então ela precisa ser
    // uma atribuição, como "variavel = Labirinto::get_largura()" ou algo do tipo
    [[nodiscard]] int get_largura() const;
    [[nodiscard]] int get_altura() const;
    [[nodiscard]] Pos get_pos_ninho() const;
    [[nodiscard]] Pos get_pos_comida() const;

    // passar como & faz com que não crie uma cópia da struct Pos no método. isso ajuda na otimização
    // como Pos é minúsculo, não tem tanta diferença assim, mas é mais seguro e uma boa prática de c++ de qualquer forma
    [[nodiscard]] int get_valor_grid(const Pos& p) const;
    [[nodiscard]] double get_feromonio(const Pos& p) const;

private:
    int altura{}, largura{}; // tem o {} para não criar lixo na memória
    Pos pos_ninho{}, pos_comida{};

    std::vector<std::vector<int>> grid;
    std::vector<std::vector<double>> feromonios;
    void criar_grid();
    void criar_grid_dificil();
};


#endif //ACO_LABIRINTO_LABIRINTO_H