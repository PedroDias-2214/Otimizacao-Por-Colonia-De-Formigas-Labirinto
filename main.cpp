#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "Labirinto.h"
#include "Formiga.h"

constexpr int LARGURA_LAB = 75;
constexpr int ALTURA_LAB = 75;
constexpr int N_ITERACOES = 350;
constexpr int N_FORMIGAS = 100;
constexpr double ALFA = 1.0;  // Peso do feromônio
constexpr double BETA = 4.0;  // Peso da heurística
constexpr double TAXA_EVAPORACAO = 0.35; // Taxa de evaporação do feromônio
constexpr double INTENSIDADE_FEROMONIO = LARGURA_LAB * ALTURA_LAB * 0.1; // Valor base do deposito de feromonio
constexpr double MIN_FEROMONIO = 0.01; // Feromônio mínimo no bloco
constexpr int SALVAR_ITERACAO = 2; // de quantas em quantas iterações ele vai criar um arquivo
constexpr bool LABIRINTO_DIFICIL = true; // Labirinto gera com paredes adicionais, tornando o caminho menos previsivel
// a chance de ser gerada uma parede é de 20% para cada chão adjacente a um pilar.
// Isto pode aumentar a complexidade do código, e o código vai gerar labirintos até gerar um labirinto possível

// Switch de segurança, não passa de X passos, esse valor é dinâmico para a largura e a altura do labirinto
constexpr int MAX_PASSOS_TIMEOUT = LARGURA_LAB * ALTURA_LAB * 2;
constexpr int PARADA_POR_ESTAGNACAO = 35; // quantas iterações sem melhora para parar o código, -1 para desativar
constexpr int ELITE = 5; // quantas formigas que vão depositar feromonios

void salvar_iteracao(const Labirinto& lab, const std::string& arquivo) {
    std::ofstream arq_out(arquivo);
    if (!arq_out.is_open()) {
        std::cerr << "ERRO! Nao foi possivel criar o arquivo " << arquivo << std::endl;
        return;
    }

    for (int i=0; i<lab.get_largura(); i++) {
        for (int j=0; j<lab.get_altura(); j++) {
            Pos p = {i, j};

            switch (lab.get_valor_grid(p)) {
                case 1:
                    arq_out << -1;
                    break;

                case 2:
                    arq_out << -2;
                    break;

                case 3:
                    arq_out << -3;
                    break;

                default:
                    arq_out << lab.get_feromonio(p);
                    break;
            }
            if (j < lab.get_altura()-1) arq_out << ',';
        }
        arq_out << '\n';
    }
}

int main () {
    const auto start = std::chrono::high_resolution_clock::now();
    try {
        Labirinto lab(LARGURA_LAB, ALTURA_LAB, LABIRINTO_DIFICIL);
        std::vector<Formiga> formigas;
        formigas.reserve(N_FORMIGAS); // reserva o número necessário de espaço, é otimizado

        for (int i=0; i<N_FORMIGAS; i++) {
            // emplace_back funciona bem parecido com push_back, mas não recebe uma cópia, é mais otimizado
            // para esse caso
            formigas.emplace_back(lab, ALFA, BETA);
        }

        std::vector<Pos> melhor_caminho_global;
        int menor_tamanho_global = -1;

#ifdef _WIN32
        system("if not exist ..\\visualizacao mkdir ..\\visualizacao");
        system("del /Q ..\\visualizacao\\*.csv 2>nul");
#else
        system("mkdir -p ../visualizacao");
        system("rm -f ../visualizacao/*.csv");
#endif

        {
        std::stringstream ss;
        ss << "../visualizacao/iter_000.csv";
        salvar_iteracao(lab, ss.str()); // salva a inicial antes das formigas andarem nele
    } // fazer {} gera um bloco, como eu uso a variavel ss depois, deixei dentro do bloco pra não interferir

        int iteracao = 0, estagnacao = 0;
        bool atualizou = false;
        while (iteracao<N_ITERACOES) { // a outra condição de parada é a estagnação, no final do loop da pra ver

            #pragma omp parallel for // isso é basicamente um multithreading, dividindo o "loop" entre os nucleos
            for (int i=0; i<N_FORMIGAS; i++) {
                formigas[i].reset();

                int passos_atuais = 0;
                while (!formigas[i].encontrou_comida() && !formigas[i].falhou()) {
                    formigas[i].mover();
                    passos_atuais++;

                    if (passos_atuais > MAX_PASSOS_TIMEOUT) formigas[i].falhar();
                }
            }

            lab.evaporar_feromonios(TAXA_EVAPORACAO, MIN_FEROMONIO);
            int menor_tamanho_iteracao = -1;
            std::vector<Formiga*> formigas_com_sucesso;
            formigas_com_sucesso.reserve(N_FORMIGAS);

            for (Formiga& formiga : formigas) {
                if (formiga.encontrou_comida()) {
                    formigas_com_sucesso.push_back(&formiga);
                    int tamanho_caminho = static_cast<int> (formiga.get_pilha_solucao().size());
                    if (menor_tamanho_iteracao == -1 || tamanho_caminho < menor_tamanho_iteracao) {
                        menor_tamanho_iteracao = tamanho_caminho;
                    }
                }
            }
            std::sort(formigas_com_sucesso.begin(), formigas_com_sucesso.end(), [](const Formiga * a, const Formiga * b) {
                return a->get_pilha_solucao().size() < b->get_pilha_solucao().size();
            });

            int quantidade_deposito = std::min(static_cast<int>(formigas_com_sucesso.size()), ELITE);
            for (int i=0; i<quantidade_deposito; i++) {
                const std::vector<Pos>& caminho = formigas_com_sucesso[i]->get_pilha_solucao();
                lab.depositar_feromonios(caminho, INTENSIDADE_FEROMONIO);

                int tamanho = static_cast<int>(caminho.size());
                if (menor_tamanho_global == -1 || tamanho < menor_tamanho_global) {
                    menor_tamanho_global = tamanho;
                    melhor_caminho_global = caminho;
                    atualizou = true;
                }
            }

            if (!melhor_caminho_global.empty()) {
                lab.depositar_feromonios(melhor_caminho_global, INTENSIDADE_FEROMONIO*ELITE*0.5);
            }

            std::cout << iteracao+1 << '/' << N_ITERACOES << std::endl;
            if (menor_tamanho_iteracao == -1) {
                std::cout << "Nenhuma solução encontrada." << std::endl;
            }
            else {
                std::cout << "Melhor da iteracao: " << menor_tamanho_iteracao << std::endl;
                std::cout << "Melhor caminho global: " << menor_tamanho_global << std::endl;
                std::cout << "\n\n";
            }


            if (((iteracao+1) % SALVAR_ITERACAO == 0 || (iteracao+1) == N_ITERACOES-1) && iteracao!=0) {
                std::stringstream ss;
                ss << "../visualizacao/iter_";
                ss << std::setw(3) << std::setfill('0') << iteracao;
                ss << ".csv";

                salvar_iteracao(lab, ss.str());
            }

            if (!atualizou) estagnacao++;
            else {
                atualizou = false;
                estagnacao = 0;
            }
            if (estagnacao >= PARADA_POR_ESTAGNACAO && PARADA_POR_ESTAGNACAO != -1) {
                std::cout << "PARADA POR ESTAGNACAO: " << estagnacao << " ITERACOES" << std::endl;
                std::cout << "PARADA REALIZADA NA ITERACAO " << iteracao+1 << std::endl;
                break;
            }
            iteracao ++;
        }
        //lab.print_feromonios(); // funcionava pra debug, mas não é necessário e polui os prints finais
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }



    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> duracao = end - start;

    std::cout << "Tempo de simulacao: " << std::fixed << std::setprecision(2) << duracao.count() << std::endl;
    return 0;
}