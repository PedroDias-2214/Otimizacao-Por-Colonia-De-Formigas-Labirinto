import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import glob
import sys
import os

PASTA_VISUALIZACAO = 'visualizacao' # nome da pasta com os arquivos csv q o c++ cria
# VMIN deve ser igual ao 'MIN_FEROMONIO' definido no C++
VMAX_FEROMONIO = 5.0
VMIN_FEROMONIO = 0.01 

def extrair_numero_iteracao(nome_arquivo): 
    # Pega o número da iteração do arquivo em csv, se for (iter_050), extrai o numero 50
    nome_base = os.path.basename(nome_arquivo)
    iteracao = nome_base.split('_')[-1].split('.')[0]
    return int(iteracao)

def encontrar_arquivos(): # Pega todos os arquivos da pasta e ordena baseado no numero da iteracao
    padrao_busca = os.path.join(PASTA_VISUALIZACAO, 'iter_*.csv')
    arquivos = glob.glob(padrao_busca)
    
    if not arquivos:
        print(f"Erro: Nenhum arquivo .csv encontrado em '{PASTA_VISUALIZACAO}'")
        print("Dica: Certifique-se de que o programa C++ foi executado e gerou os dados.")
        sys.exit() # Se der erro, mostra mensagem de erro e encerra o programa

    # Ordena todos os arquivos por numero de iteracao
    arquivos.sort(key=extrair_numero_iteracao)
    iteracoes = [extrair_numero_iteracao(f) for f in arquivos]
    
    return arquivos, iteracoes # Retorna os arquivos e as iteracoes, arquivo iter_050.csv, iteracao 50

def carregar_dados(arquivos):
    frames = []
    for f in arquivos:
        # genfromtxt carrega o CSV em uma matriz numpy
        frames.append(np.genfromtxt(f, delimiter=','))
    return frames # retorna a lista com todas as matrizes

def configurar_cores():
    cores_mapa = { # Os numeros são negativos pois numeros positivos são feromonios
        -3: (0.1, 0.1, 1.0),  # Azul -> Ninho
        -2: (0.5, 1.0, 0.5),  # Verde claro -> Comida 
        -1: (0.2, 0.2, 0.2)   # Cinza Escuro -> Parede
    }
    
    # Cria o colormap
    cores_lista = [cores_mapa[i] for i in range(-3, 0)]
    cmap_labirinto = mcolors.ListedColormap(cores_lista)
    norm_labirinto = mcolors.Normalize(vmin=-3, vmax=-1)
    
    # Quanto maior o feromonio, mais perto de vermelho escuro fica a grid do local
    cores_gradiente = ["white", "yellow", "red", "darkred"]
    
    cmap_feromonio = mcolors.LinearSegmentedColormap.from_list("mapa_feromonio", cores_gradiente)
    
    # Normalização Logarítmica pra poder visualizar melhor com feromonios maiores
    norm_log = mcolors.LogNorm(vmin=VMIN_FEROMONIO, vmax=VMAX_FEROMONIO, clip=True)
    
    return cmap_labirinto, norm_labirinto, cmap_feromonio, norm_log

class InteractiveVisualizer:
    
    def __init__(self, fig, ax, all_frames, all_iteracoes, cmaps):
        self.fig = fig
        self.ax = ax
        
        self.frames = all_frames
        self.iteracoes = all_iteracoes
        
        # Desempacota os mapas de cores recebidos
        self.cmap_lab, self.norm_lab, self.cmap_fero, self.norm_log = cmaps
        
        self.quadro_atual = 0
        
        # Conecta a função on_key_press ao evento de apertar tecla
        self.fig.canvas.mpl_connect('key_press_event', self.on_key_press)
        
        # desenha o estado inicial
        self.draw_frame()

    def draw_frame(self):
        data = self.frames[self.quadro_atual]
        
        # Reserva o labirinto, ignorando tudo que for maior ou igual a 0 (ignora caminhos/feromonios)
        lab_data = np.ma.masked_where(data >= 0, data)
        
        # Mesma coisa do labirinto, mas agora usa mascara nas paredes
        feromonio_data = np.ma.masked_where(data < 0, data.copy()).filled(0)

        # Calcula as cores e a transparência
        dados_normalizados = self.norm_log(feromonio_data)
        cores_rgba = self.cmap_fero(dados_normalizados)
        
        cores_rgba[..., 3] = np.power(dados_normalizados, 0.75)

        self.ax.clear()
        
        self.ax.imshow(lab_data.T, cmap=self.cmap_lab, norm=self.norm_lab, interpolation='nearest')
        self.ax.imshow(cores_rgba.transpose(1, 0, 2), interpolation='nearest')
        

        # Configuração do titulo e visual basico
        titulo = f"Iteração: {self.iteracoes[self.quadro_atual]}\n"
        titulo += "[ESPAÇO/DIR]: Avançar | [ESQ]: Voltar"
        
        self.ax.set_title(titulo)
        self.ax.set_aspect('equal')
        self.ax.axis('off')
        
        self.fig.canvas.draw()

    def on_key_press(self, event): 
        # Função que implementa a logica para apertar o botão do teclado (se fosse gif ficaria ruim)
        if event.key == 'right' or event.key == ' ': # Se apertar tecla pra direita, avança X frames
            self.quadro_atual = (self.quadro_atual + 1) % len(self.frames)
        
        elif event.key == 'left': # Se apertar tecla para esquerda, volta X frames
            self.quadro_atual = (self.quadro_atual - 1) % len(self.frames)
            
        else:
            return # Tecla não mapeada, não faz nada
            
        self.draw_frame() # Desenha o frame novamente depois de avançar ou voltar


def main():
    arquivos, iteracoes = encontrar_arquivos() # Se não houverem arquivos, ele encerra o programa na função

    # Carrega os dados e define as cores
    frames_de_dados = carregar_dados(arquivos)
    cmaps = configurar_cores() 


    fig, ax = plt.subplots(figsize=(10, 10))
    viz = InteractiveVisualizer(fig, ax, frames_de_dados, iteracoes, cmaps)
    
    plt.show()

if __name__ == '__main__':
    main()