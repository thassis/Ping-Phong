#include <SOIL/SOIL.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

typedef struct ponto {
    float x, y;
} Ponto;

//errorLarguraImagem: é a tentativa de dizer o tamanho da imagem com espaços em branco
typedef struct tamanho {
    float largura, altura;
} Tamanho;

typedef struct velocidade {
    float x, y;
} Velocidade;

typedef struct pontuacao{
    int pontosSet, quantidadeSetsGanho, pontosConsecutivos, pontuacaoMaxima;
} Pontuacao;

Pontuacao primeiroJogador, segundoJogador;

Ponto posicaoBola;
Tamanho tamanhoBola;

Ponto posicaoBarraEsquerda;
Tamanho tamanhoBarraEsquerda;

Ponto posicaoBarraDireita;
Tamanho tamanhoBarraDireita;

Ponto posicaoMouse;

Velocidade velocidadeBola;
Velocidade velocidadeBarra;

int comprimentoMaximoTela = 600;
int larguraMaximaTela = 300;

int orientacaoVerticalBola = 1;
int orientacaoHorizontalBola = -1;

int podeColidirDireita = 1;
int podeColidirEsquerda = 1;

int keyboard[256];

int jogoPausado = 0;
int reiniciarJogo = 0;
int jogoFinalizado = 0;

int quadroAtualBola = 0;
int quadroAtualCoroa = 0;
int quadroAtualSegundos = 0;
int bolaColidiuBarra = 0;

int verificarGanhadorSetConsecutivos = 0;

int limiteSetsGanharJogo = 3 ; //números impar
int quantidadePontosGanharSet = 11;

int reservarTecladoParaNome = 0;

char exibirQuadroAviso = 'n';

struct nomeGanhador {
    char nome[50];
    int indexNome;
    int numeroVencedor;
} ganhador;

GLuint idTexturaBolaBranca;
GLuint idTexturaBolaPreta;
GLuint idTexturaBarraEsquerda;
GLuint idTexturaBarraDireita;
GLuint idYinYang;
GLuint idImagePause;
GLuint idTexturaCoroa;
GLuint idTexturaVencedor1;
GLuint idTexturaVencedor2;
GLuint idTexturaRecorde;
GLuint idTexturaReiniciar;

FILE *arquivo;

const int salvarNmelhoresRanking = 12;

struct ranking{
    int *posicoes;
    char **nomes;
    int *pontuacoes;
} ranking;

int checaColisaoComBola(Ponto posicaoObj1, Tamanho tamanhoObj1){
    // Collision x-axis
    int collisionX = posicaoObj1.x + tamanhoObj1.largura >= posicaoBola.x &&
        posicaoBola.x + tamanhoBola.largura >= posicaoObj1.x;
    // Collision y-axis
    int collisionY = posicaoObj1.y + tamanhoObj1.altura >= posicaoBola.y &&
        posicaoBola.y + tamanhoBola.altura >= posicaoObj1.y;
    // Collision only if on both axes
    return collisionX && collisionY;
}

GLuint carregaTextura(const char* arquivo) {
    GLuint idTextura = SOIL_load_OGL_texture(
                           arquivo,
                           SOIL_LOAD_AUTO,
                           SOIL_CREATE_NEW_ID,
                           SOIL_FLAG_INVERT_Y
                       );

    if (idTextura == 0) {
        printf("Erro do SOIL: '%s'\n", SOIL_last_result());
    }

    return idTextura;
}

void iniciaObjetos(){
    glEnable(GL_BLEND);
    glColor3d(1.0f, 1.0f, 1.0f);
    exibirQuadroAviso = 'n';
    reiniciarJogo = 0;
    tamanhoBola.largura = 60;
    tamanhoBola.altura = 60;

    float posicaoVerticalBola = rand()%larguraMaximaTela;

    if(posicaoVerticalBola >= 0 && posicaoVerticalBola <= tamanhoBola.altura)
        posicaoVerticalBola += tamanhoBola.altura+5;
    else if(posicaoVerticalBola <= larguraMaximaTela && posicaoVerticalBola >= larguraMaximaTela - tamanhoBola.altura){
        posicaoVerticalBola = larguraMaximaTela-tamanhoBola.altura-5;
    }

    tamanhoBarraDireita.largura = 15;
    tamanhoBarraDireita.altura = 80;
    posicaoBarraDireita.x = (comprimentoMaximoTela-tamanhoBarraDireita.largura);
    posicaoBarraDireita.y = (larguraMaximaTela-tamanhoBarraDireita.altura)/2;

    tamanhoBarraEsquerda.largura = 15;
    tamanhoBarraEsquerda.altura = 80;
    posicaoBarraEsquerda.x = 0;
    posicaoBarraEsquerda.y = (larguraMaximaTela-tamanhoBarraEsquerda.altura)/2;

    posicaoBola.x = comprimentoMaximoTela/2;
    posicaoBola.y = posicaoVerticalBola;

    velocidadeBola.x = 15;
    velocidadeBola.y = 20;

    velocidadeBarra.y = 60;
    velocidadeBarra.x = 0;
    orientacaoVerticalBola = 1;
    orientacaoHorizontalBola = -1;

    podeColidirDireita = 1;
    podeColidirEsquerda = 1;

    primeiroJogador.pontosSet = 0;
    segundoJogador.pontosSet = 0;
    
    primeiroJogador.quantidadeSetsGanho = 0;
    segundoJogador.quantidadeSetsGanho = 0;
    
    primeiroJogador.pontosConsecutivos = 0;
    segundoJogador.pontosConsecutivos = 0;

    primeiroJogador.pontuacaoMaxima = 0;
    segundoJogador.pontuacaoMaxima = 0;

    ganhador.indexNome = 0;
    ganhador.numeroVencedor = 0;
    jogoPausado = 0;

    jogoFinalizado = 0;

    quadroAtualSegundos = 0;
    for(int i=0;i<256;i++)
        keyboard[i] = 0;
}

void desenhaBola(){
    GLuint idTexturaBola = idTexturaBolaBranca;
    
    if(posicaoBola.x + tamanhoBola.largura >= comprimentoMaximoTela/2) idTexturaBola = idTexturaBolaPreta;
    
    glBindTexture(GL_TEXTURE_2D, idTexturaBola);
    glEnable(GL_PRIMITIVE_RESTART);
    glBegin(GL_POLYGON);
        // Associamos um canto da textura para cada vértice
        glTexCoord2f((float) (quadroAtualBola)/19, 0);
        glVertex2f(posicaoBola.x, posicaoBola.y);

        glTexCoord2f((float) (quadroAtualBola+1)/19, 0);
        glVertex2f(posicaoBola.x + tamanhoBola.largura, posicaoBola.y);

        glTexCoord2f((float) (quadroAtualBola+1)/19, 1);
        glVertex2f(posicaoBola.x + tamanhoBola.largura, posicaoBola.y + tamanhoBola.altura);

        glTexCoord2f((float) (quadroAtualBola)/19, 1);
        glVertex2f(posicaoBola.x, posicaoBola.y + tamanhoBola.altura);

    glEnd();
}

char* retornaPontuacaoString(int pontuacao){
    char *pontuacaoString;
    pontuacaoString = (char*)malloc(2*sizeof(char));
    if(pontuacao < 10){
        // sprintf(pontuacaoString[0], "%d", 0);
        
    } else {
        sprintf(pontuacaoString, "%d", pontuacao);
    }
    return pontuacaoString;
}

void desenhaPlacar(){
    if(jogoFinalizado == 0){
        char pontuacaoString[2];
        sprintf(pontuacaoString, "%d", primeiroJogador.pontosSet);
        glColor3d(1.0, 1.0, 1.0);
        glRasterPos2d(comprimentoMaximoTela*0.20, (float)larguraMaximaTela*0.1);
        for (int n=0; n<2; ++n) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, pontuacaoString[n]);
        }
        sprintf(pontuacaoString, "%d", segundoJogador.pontosSet);
        glColor3d(0.0, 0.0, 0.0);
        glRasterPos2d(comprimentoMaximoTela*0.70, (float)larguraMaximaTela*0.1);
        for (int n=0; n<2; ++n) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, pontuacaoString[n]);
        }
    }
    glClearColor(1.0f,1.0f,1.0f,1.0f);
}


void desenhaBackgroundPreto(){
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_TRIANGLE_STRIP);
        glVertex2f(0, 0);

        glVertex2f(comprimentoMaximoTela/2, 0);

        glVertex2f(0, larguraMaximaTela);

        glVertex2f(comprimentoMaximoTela/2, larguraMaximaTela);

    glEnd();
    glColor3d(1.0, 1.0, 1.0);
    glEnable(GL_PRIMITIVE_RESTART);
    glBegin(GL_TRIANGLE_STRIP);
        glVertex2f(comprimentoMaximoTela/2, 0);

        glVertex2f(comprimentoMaximoTela, 0);

        glVertex2f(comprimentoMaximoTela/2, larguraMaximaTela);

        glVertex2f(comprimentoMaximoTela, larguraMaximaTela);

    glEnd();
}

void desenhaTelaAdicionarNome(){

}

void desenhaRanking(){
    if(jogoFinalizado == 1 && exibirQuadroAviso == 'b'){
        // glDisable(GL_BLEND);
        // printf("aqui %c\n", exibirQuadroAviso);
        glClearColor(1.0f,1.0f,1.0f,1.0f);
        glEnable(GL_PRIMITIVE_RESTART);
        glBegin(GL_POLYGON);
            // Associamos um canto da textura para cada vértice
            
            glVertex2f(0, larguraMaximaTela);

            glVertex2f(comprimentoMaximoTela, larguraMaximaTela);

            glVertex2f(comprimentoMaximoTela, 0);

            glVertex2f(0, 0);

        glEnd();
        glColor3d(0.0, 0.0, 0.0);
        glRasterPos2d(0, (float)larguraMaximaTela*0.1);
        char titulo[] = "Ranking dos melhores";
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, titulo);
        glRasterPos2d(0, (float)larguraMaximaTela*0.2);
        char subtitulo[] = "Posicao Nome Pontuacao";
        glutBitmapString(GLUT_BITMAP_HELVETICA_18, subtitulo);
        int aux = (float)larguraMaximaTela*0.2;
        for(int i=0;i<salvarNmelhoresRanking;i++){
            aux += 18;
            glRasterPos2d(0, aux);
            char strPontuacao[50];
            char strPosicao[50];
            char strRanking[200];
            sprintf(strPosicao, "%d", ranking.posicoes[i]);
            sprintf(strPontuacao, "%d", ranking.pontuacoes[i]);

            strcat(strPosicao, " ");
            strcpy(strRanking, strPosicao);
            strcat(strRanking, ranking.nomes[i]);
            strcat(strRanking, " ");
            strcat(strRanking, strPontuacao);

            glutBitmapString(GLUT_BITMAP_HELVETICA_18, strRanking);
        }
    }
}

void desenhaTexturaAviso(GLuint idTextura){
    if(exibirQuadroAviso == 'w' || exibirQuadroAviso == 'p' || exibirQuadroAviso == 'r' || exibirQuadroAviso == 'v')
    jogoPausado = 1;
    glColor3d(1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, idTextura);
    glEnable(GL_PRIMITIVE_RESTART);
    glBegin(GL_POLYGON);
        // Associamos um canto da textura para cada vértice
        glTexCoord2f(0, 0);
        glVertex2f(0, larguraMaximaTela);

        glTexCoord2f(1, 0);
        glVertex2f(comprimentoMaximoTela, larguraMaximaTela);

        glTexCoord2f(1, 1);
        glVertex2f(comprimentoMaximoTela, 0);

        glTexCoord2f(0, 1);
        glVertex2f(0, 0);

    glEnd();
    glClearColor(1.0f,1.0f,1.0f,1.0f);
}

void reiniciarTela(){
    jogoPausado = 1;
    if(keyboard['s'] == 1){
        iniciaObjetos();
    } else if(keyboard['n'] == 1){
        reiniciarJogo = 0;
        jogoPausado = 0;
        exibirQuadroAviso = 'n';
    }
}

/*
    Opcao da tela a ser desenhada;
    opcao == 'p' => tela de pause
    opcao == 'r' => tela de reiniciar
    opcao == 'v' => tela de vencedor
    opcao == 'w' => tela de digitar o ranking
    opcao == 'b' => tela de exibir o Ranking de todos
    opcao == default => nao exibir
*/
void desenhaQuadroAviso(char opcao){
    
    switch (opcao){
        case 'w':
            jogoFinalizado = 1;
            reservarTecladoParaNome = 1;
            desenhaTexturaAviso(idTexturaRecorde);
            break;
        case 'v':
            jogoFinalizado = 1;
            // iniciaObjetos();
            if(ganhador.numeroVencedor == 1)
                desenhaTexturaAviso(idTexturaVencedor1);
            else 
                desenhaTexturaAviso(idTexturaVencedor2);
            break;
        case 'b':
            desenhaRanking();
            break;
        case 'r':
            desenhaTexturaAviso(idTexturaReiniciar);
            reiniciarTela();
            break;
        case 'p':
            desenhaTexturaAviso(idImagePause);
            break;
        default:
            break;
    }   
    glClearColor(1.0f,1.0f,1.0f,1.0f);
}

void verificaRanking(int pontuacao){
    int menorRanking = ranking.pontuacoes[salvarNmelhoresRanking-1];
    if(pontuacao >= menorRanking){
        ranking.pontuacoes[salvarNmelhoresRanking-1] = pontuacao;
        exibirQuadroAviso = 'w';
        jogoFinalizado = 1;
        desenhaQuadroAviso('w');
    } else {
        jogoFinalizado = 1;
        exibirQuadroAviso = 'v';
        desenhaQuadroAviso('v');
    }
}

void verificaGanhadorGame(){
    if(primeiroJogador.quantidadeSetsGanho == limiteSetsGanharJogo){
        //ganhou
        ganhador.numeroVencedor = 1;
        verificaRanking(primeiroJogador.pontuacaoMaxima);
    } else if(segundoJogador.quantidadeSetsGanho == limiteSetsGanharJogo){
        //ganhou
        ganhador.numeroVencedor = 2;
        verificaRanking(segundoJogador.pontuacaoMaxima);
    }
}

void atualizaSet(){
    verificarGanhadorSetConsecutivos = 0;
    if(primeiroJogador.pontuacaoMaxima < primeiroJogador.pontosSet) 
        primeiroJogador.pontuacaoMaxima = primeiroJogador.pontosSet;
    else if(segundoJogador.pontuacaoMaxima < segundoJogador.pontosSet) 
        segundoJogador.pontuacaoMaxima = segundoJogador.pontosSet;
    primeiroJogador.pontosSet = 0;
    primeiroJogador.pontosConsecutivos = 0;
    segundoJogador.pontosSet = 0;
    segundoJogador.pontosConsecutivos = 0;
    verificaGanhadorGame();
}

float novaPosicaoVerticalBola(){
    float posicaoVerticalBola = rand()%larguraMaximaTela;

    if(posicaoVerticalBola >= 0 && posicaoVerticalBola <= tamanhoBola.altura)
        posicaoVerticalBola += tamanhoBola.altura+5;
    else if(posicaoVerticalBola <= larguraMaximaTela && posicaoVerticalBola >= larguraMaximaTela - tamanhoBola.altura){
        posicaoVerticalBola = larguraMaximaTela-tamanhoBola.altura-5;
    }
    return posicaoVerticalBola;
}

void verificaGanhadorSet(){
    if(verificarGanhadorSetConsecutivos == 1){
        if(primeiroJogador.pontosConsecutivos == 2 ){
            primeiroJogador.quantidadeSetsGanho++;
            atualizaSet();
        } else if(segundoJogador.pontosConsecutivos == 2){
            segundoJogador.quantidadeSetsGanho++;
            atualizaSet();
        }
    } else if(primeiroJogador.pontosSet == quantidadePontosGanharSet-1 && segundoJogador.pontosSet == quantidadePontosGanharSet-1){
        primeiroJogador.pontosConsecutivos = 0;
        segundoJogador.pontosConsecutivos = 0;
        verificarGanhadorSetConsecutivos = 1;
    } else {
        if(primeiroJogador.pontosSet == quantidadePontosGanharSet){
            primeiroJogador.quantidadeSetsGanho++;
            atualizaSet();
        } else if (segundoJogador.pontosSet == quantidadePontosGanharSet){
            segundoJogador.quantidadeSetsGanho++;
            atualizaSet();
        }
    }
}

//recebe como parametro o jogador que venceu a rodada;
void atualizaVencedorSet(int numeroJogador){
    posicaoBola.x = comprimentoMaximoTela/2;
    posicaoBola.y = novaPosicaoVerticalBola(); 
    podeColidirDireita = 1;
    podeColidirEsquerda = 1;
    if(numeroJogador == 1){
        primeiroJogador.pontosSet++;
        primeiroJogador.pontosConsecutivos++;
        segundoJogador.pontosConsecutivos = 0;
    }
    else{ 
        segundoJogador.pontosSet++;
        segundoJogador.pontosConsecutivos++;
        primeiroJogador.pontosConsecutivos = 0;
    }
    verificaGanhadorSet();
}

void salvaNovoRecorde(){
    ranking.nomes[salvarNmelhoresRanking-1][0] = '\0';
    strcpy(ranking.nomes[salvarNmelhoresRanking-1], ganhador.nome);
    int i, j, auxPontuacao;
    char auxNome[50];
    for(i = 1; i < salvarNmelhoresRanking; i++) {
       for (j = 0; j < salvarNmelhoresRanking - 1; j++) {
            if (ranking.pontuacoes[j] < ranking.pontuacoes[j + 1]) {
                auxPontuacao = ranking.pontuacoes[j];
                ranking.pontuacoes[j] = ranking.pontuacoes[j + 1];
                ranking.pontuacoes[j + 1] = auxPontuacao;

                strcpy(auxNome, ranking.nomes[j]);
                strcpy(ranking.nomes[j], ranking.nomes[j + 1]);
                strcpy(ranking.nomes[j + 1], auxNome);
            }
        }
    }
    fclose(arquivo);
    arquivo = fopen("ranking.txt", "w+");
    for(int i=0;i<salvarNmelhoresRanking;i++){
        fprintf(arquivo, "%d;%s;%d;\n", ranking.posicoes[i], ranking.nomes[i], ranking.pontuacoes[i]);
    }
    fclose(arquivo);
    exibirQuadroAviso = 'b';
}

void desenhaCoroa(Ponto posicaoCoroa, Tamanho tamanhoCoroa){
    glColor3f(1.0, 1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, idTexturaCoroa);
    glEnable(GL_PRIMITIVE_RESTART);
    glBegin(GL_POLYGON);
        // Associamos um canto da textura para cada vértice
        glTexCoord2f((float) (quadroAtualCoroa)/7, 0);
        glVertex2f(posicaoCoroa.x, posicaoCoroa.y + tamanhoCoroa.altura);

        glTexCoord2f((float) (quadroAtualCoroa+1)/7, 0);
        glVertex2f(posicaoCoroa.x + tamanhoCoroa.largura, posicaoCoroa.y + tamanhoCoroa.altura);

        glTexCoord2f((float) (quadroAtualCoroa+1)/7, 1);
        glVertex2f(posicaoCoroa.x + tamanhoCoroa.largura, posicaoCoroa.y);

        glTexCoord2f((float) (quadroAtualCoroa)/7, 1);
        glVertex2f(posicaoCoroa.x, posicaoCoroa.y);

    glEnd();
}

void definePosicaoCoroa(){
    Ponto posicaoCoroa;
    posicaoCoroa.x = (float)(comprimentoMaximoTela*0.20)+4;
    posicaoCoroa.y = (float)larguraMaximaTela*0.05;
    Tamanho tamanhoCoroa;
    tamanhoCoroa.largura = 20;
    tamanhoCoroa.altura = 20;
    
    for(int i=0;i<primeiroJogador.quantidadeSetsGanho;i++){
        posicaoCoroa.x += tamanhoCoroa.largura;
        desenhaCoroa(posicaoCoroa, tamanhoCoroa);
    }
    posicaoCoroa.x = (float)(comprimentoMaximoTela*0.70)+4;
    for(int i=0;i<segundoJogador.quantidadeSetsGanho;i++){
        posicaoCoroa.x += tamanhoCoroa.largura;
        desenhaCoroa(posicaoCoroa, tamanhoCoroa);
    }


    /*Atualiza coroa em cima das barras que estás prestes a completar um set*/
    Ponto posicaoBarra = posicaoBarraEsquerda;
    posicaoBarra.y += tamanhoBarraEsquerda.altura/2 - 4;
    if(primeiroJogador.pontosSet == quantidadePontosGanharSet-1){
        desenhaCoroa(posicaoBarra, tamanhoCoroa);
    }
    if(segundoJogador.pontosSet == quantidadePontosGanharSet-1){
        posicaoBarra = posicaoBarraDireita;
        posicaoBarra.y += tamanhoBarraDireita.altura/2 - 4;
        desenhaCoroa(posicaoBarra, tamanhoCoroa);
    }
    
    if(verificarGanhadorSetConsecutivos == 1 && primeiroJogador.pontosConsecutivos == 1){
        desenhaCoroa(posicaoBarra, tamanhoCoroa);
    }
    if(verificarGanhadorSetConsecutivos == 1 && segundoJogador.pontosConsecutivos == 1){
        posicaoBarra = posicaoBarraDireita;
        posicaoBarra.y += tamanhoBarraDireita.altura/2 - 4;
        desenhaCoroa(posicaoBarra, tamanhoCoroa);
    }
}
void desenhaCena() {
    // Limpa a tela (com a cor definida por glClearColor(r,g,b)) para que
    // possamos desenhar
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "text");
    glEnable(GL_PRIMITIVE_RESTART);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 1.0f, 1.0f);
    // Habilita o uso de texturas
    
    if(exibirQuadroAviso != 'b'){
        desenhaBackgroundPreto();

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, idYinYang);
        glColor3f(1.0f, 1.0f, 1.0f);
        glEnable(GL_PRIMITIVE_RESTART);
        glBegin(GL_POLYGON);
            // Associamos um canto da textura para cada vértice
            glTexCoord2f(0, 0);
            glVertex2f((comprimentoMaximoTela/2) - 0.10*comprimentoMaximoTela, (larguraMaximaTela/2) - 0.10*comprimentoMaximoTela);

            glTexCoord2f(1, 0);
            glVertex2f((comprimentoMaximoTela/2) + 0.10*comprimentoMaximoTela, (larguraMaximaTela/2) - 0.10*comprimentoMaximoTela);

            glTexCoord2f(1, 1);
            glVertex2f((comprimentoMaximoTela/2) + 0.10*comprimentoMaximoTela, (larguraMaximaTela/2) + 0.10*comprimentoMaximoTela);

            glTexCoord2f(0, 1);
            glVertex2f((comprimentoMaximoTela/2) - 0.10*comprimentoMaximoTela, (larguraMaximaTela/2) + 0.10*comprimentoMaximoTela);

        glEnd();

        desenhaBola();
        // Começa a usar a textura que criamos
        glBindTexture(GL_TEXTURE_2D, idTexturaBarraDireita);
        glEnable(GL_PRIMITIVE_RESTART);
        glBegin(GL_POLYGON);
            // Associamos um canto da textura para cada vértice
            glTexCoord2f(0, 0);
            glVertex2f(posicaoBarraDireita.x, posicaoBarraDireita.y);

            glTexCoord2f(1, 0);
            glVertex2f( posicaoBarraDireita.x + tamanhoBarraDireita.largura, posicaoBarraDireita.y);

            glTexCoord2f(1, 1);
            glVertex2f( posicaoBarraDireita.x + tamanhoBarraDireita.largura, posicaoBarraDireita.y + tamanhoBarraDireita.altura);

            glTexCoord2f(0, 1);
            glVertex2f(posicaoBarraDireita.x, posicaoBarraDireita.y + tamanhoBarraDireita.altura);

        glEnd();

        glBindTexture(GL_TEXTURE_2D, idTexturaBarraEsquerda);    
        glEnable(GL_PRIMITIVE_RESTART);
        glBegin(GL_POLYGON);
            // Associamos um canto da textura para cada vértice
            glTexCoord2f(0, 0);
            glVertex2f(posicaoBarraEsquerda.x, posicaoBarraEsquerda.y);

            glTexCoord2f(1, 0);
            glVertex2f( posicaoBarraEsquerda.x + tamanhoBarraEsquerda.largura, posicaoBarraEsquerda.y);

            glTexCoord2f(1, 1);
            glVertex2f( posicaoBarraEsquerda.x + tamanhoBarraEsquerda.largura, posicaoBarraEsquerda.y + tamanhoBarraEsquerda.altura);

            glTexCoord2f(0, 1);
            glVertex2f(posicaoBarraEsquerda.x, posicaoBarraEsquerda.y + tamanhoBarraEsquerda.altura);

        glEnd();

        definePosicaoCoroa();

        desenhaQuadroAviso(exibirQuadroAviso);
    } else {
        desenhaRanking();
    }

    glDisable(GL_TEXTURE_2D);
    
    desenhaPlacar();

    // Diz ao OpenGL para colocar o que desenhamos na tela
    glutSwapBuffers();
}

// Inicia algumas variáveis de estado
void inicializa() {
    srand(time(NULL));
    // cor para limpar a tela
    glClearColor(1, 1, 1, 1);      // branco
    
    // habilita mesclagem de cores, para termos suporte a texturas
    // com transparência
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    idTexturaBolaBranca = carregaTextura("bola_branca.png");
    idTexturaBolaPreta = carregaTextura("bola_preta.png");
    idTexturaBarraEsquerda = carregaTextura("barraEsquerda.png");
    idTexturaBarraDireita = carregaTextura("barraDireita.png");
    idYinYang = carregaTextura("yinyang.png");
    idImagePause = carregaTextura("pausado.png");
    idTexturaCoroa = carregaTextura("crown.png");
    idTexturaVencedor1 = carregaTextura("vencedor1.png");
    idTexturaVencedor2 = carregaTextura("vencedor2.png");
    idTexturaRecorde = carregaTextura("recorde.png");
    idTexturaReiniciar = carregaTextura("reiniciar.png");
    // define o quadrado
    iniciaObjetos();
}

// Callback de redimensionamento
void redimensiona(int w, int h) {
    comprimentoMaximoTela = w;
    larguraMaximaTela = h;
    glViewport(0, 0, w, h);
    
    posicaoBarraDireita.x = (comprimentoMaximoTela-tamanhoBarraDireita.largura);
    posicaoBarraDireita.y = (larguraMaximaTela-tamanhoBarraDireita.altura)/2;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // repare que o sistema de coordenadas do mundo foi configurado para
    // ser bem parecido com o sistema da janela: 0 a w, 0 a h, começando
    // do canto esquerdo-superior
    glOrtho(0, w, h, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Callback de evento de teclado
void teclaPressionada(unsigned char key, int x, int y) {
    keyboard[key] = 1;
    if(reservarTecladoParaNome == 0)
        switch(key) {
            // Tecla ESC
            case 27:
                exit(0);
                fclose(arquivo);
                break;
            case 'p':
                if(jogoPausado == 1) {
                    exibirQuadroAviso = 'n';
                    jogoPausado = 0;
                } else {
                    exibirQuadroAviso = 'p';
                    jogoPausado = 1;
                }
                glutPostRedisplay();
                break;
            case 'r':
                exibirQuadroAviso = 'r';
                break;
            default:
                break;
        }
    else{
        if(key != 13){
            ganhador.nome[ganhador.indexNome] = key;
            glClearColor(0.0f,0.0f,0.0f,0.0f);
            glDisable(GL_BLEND);
            glEnable(GL_PRIMITIVE_RESTART);
            glRasterPos2d(24+ganhador.indexNome, (float)larguraMaximaTela*0.8);
            glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, ganhador.nome);
            ganhador.indexNome++;
        } else {
            reservarTecladoParaNome = 0;
            salvaNovoRecorde();
        }
    }
    
    
    // posicaoBarraDireita.y +=5; 
}

// Callback de evento de teclado
void teclaSolta(unsigned char key, int x, int y) {
    keyboard[key] = 0;
}

// Callback de evento de movimento do mouse
void movimentoMouse(int x, int y) {
    posicaoMouse.x = x;
    posicaoMouse.y = y;
}

int verificaSePassouVertical(Ponto posicao, Tamanho tamanho){
    if(posicao.y + tamanho.altura > larguraMaximaTela){
        return 1;
    } else if(posicao.y < 0){
        return 1; 
    }

    return 0;
}

// Callback do evento timer
void atualizaCena(int periodo) {
    
    if(orientacaoHorizontalBola == 1){
        quadroAtualBola++;
        if(quadroAtualBola >= 19){
            quadroAtualBola = 0;
        }
    } else {
        quadroAtualBola--;
        if(quadroAtualBola <= 0){
            quadroAtualBola = 18;
        }
    }
    

    // faz o quadrado andar na direção do ponteiro
    if(jogoPausado == 0 && jogoFinalizado == 0){
        if(verificaSePassouVertical(posicaoBola, tamanhoBola) == 1){
            orientacaoVerticalBola *= -1;
        }  

        if(posicaoBola.x >= comprimentoMaximoTela){
            atualizaVencedorSet(1);
        } else if(posicaoBola.x <= 0){
            atualizaVencedorSet(2);
        }

        posicaoBola.x += orientacaoHorizontalBola*velocidadeBola.x;
        posicaoBola.y += orientacaoVerticalBola*velocidadeBola.y;

        if(keyboard['w'] == 1){
            posicaoBarraEsquerda.y -= velocidadeBarra.y;
            if(verificaSePassouVertical(posicaoBarraEsquerda, tamanhoBarraEsquerda) == 1)
                posicaoBarraEsquerda.y = 0;
        }
        if(keyboard['s'] == 1){
            posicaoBarraEsquerda.y += velocidadeBarra.y;
            if(verificaSePassouVertical(posicaoBarraEsquerda, tamanhoBarraEsquerda) == 1)
                posicaoBarraEsquerda.y = larguraMaximaTela - tamanhoBarraEsquerda.altura;
        }

        if(keyboard['o'] == 1){
            posicaoBarraDireita.y -= velocidadeBarra.y;
            if(verificaSePassouVertical(posicaoBarraDireita, tamanhoBarraDireita) == 1)
                posicaoBarraDireita.y = 0;
        }
        if(keyboard['l'] == 1){
            posicaoBarraDireita.y += velocidadeBarra.y;
            if(verificaSePassouVertical(posicaoBarraDireita, tamanhoBarraDireita) == 1)
                posicaoBarraDireita.y = larguraMaximaTela - tamanhoBarraDireita.altura;
        }

        int colisaoBarraDireitaBola = checaColisaoComBola(posicaoBarraDireita, tamanhoBarraDireita);

        if(colisaoBarraDireitaBola == 1 && podeColidirDireita == 1){
            orientacaoHorizontalBola *= -1;
            podeColidirDireita = -1;
            podeColidirEsquerda = 1;
        } else {
            int colisaoBarraEsquerdaBola = checaColisaoComBola(posicaoBarraEsquerda, tamanhoBarraEsquerda);
            if(colisaoBarraEsquerdaBola == 1 && podeColidirEsquerda == 1){
                orientacaoHorizontalBola *= -1; 
                podeColidirEsquerda = -1;
                podeColidirDireita = 1;
            }
        }
    }

    // Pede ao GLUT para redesenhar a tela, assim que possível
    glutPostRedisplay();

    // Se registra novamente, para que fique sempre sendo chamada (30 FPS)
    
    glutTimerFunc(periodo, atualizaCena, periodo);
}

void atualizaCoroa(int periodo){
    quadroAtualCoroa++;

    if(quadroAtualCoroa == 7){ 
        quadroAtualCoroa = 0;
    }

    // Pede ao GLUT para redesenhar a tela, assim que possível
    glutPostRedisplay();

    // Se registra novamente, para que fique sempre sendo chamada (30 FPS)
    
    glutTimerFunc(periodo, atualizaCoroa, periodo);
}

void leRanking(){
    ranking.pontuacoes = (int*)malloc(salvarNmelhoresRanking*sizeof(int));
    ranking.posicoes = (int*)malloc(salvarNmelhoresRanking*sizeof(int));
    ranking.nomes = (char**)malloc(salvarNmelhoresRanking * sizeof(char*));
    for(int i = 0; i < salvarNmelhoresRanking; i++){
        ranking.nomes[i] = (char *)malloc(50 * sizeof(char));
        ranking.nomes[i][0] = 'x';
        ranking.posicoes[i] = i+1;
        ranking.pontuacoes[i] = 0;
    }

    char carecterArquivo;
    char strPosicao[1];
    char strNome[50];
    char strPontuacao[50];
    int aux = 0;
    int posicaoPontoVirgula = 0;
    int linha = 0;
    arquivo = fopen("ranking.txt", "r");
    /*
        lê os caracteres e os separa a cada ;, para salvar nas variáveis
        1 coluna (posicaoPontoVirgula == 0) significa que estou lendo as posicoes;
        2 coluna (posicaoPontoVirgula == 1) significa que estou lendo os nomes;
        3 coluna (posicaoPontoVirgula == 2) significa que estou lendo as pontuacoes;
    */
    while (carecterArquivo != EOF){
        //faz a leitura do caracter no arquivo apontado por pont_arq
        carecterArquivo = getc(arquivo);
        if(carecterArquivo!=';'){
            if(carecterArquivo != '\n' && carecterArquivo != EOF){
                if (posicaoPontoVirgula == 0)
                    strPosicao[aux] = carecterArquivo;
                else if(posicaoPontoVirgula == 1)
                    strNome[aux] = carecterArquivo;
                else if(posicaoPontoVirgula == 2)
                    strPontuacao[aux] = carecterArquivo;
                aux++;
            }
        }else{
            switch (posicaoPontoVirgula){
                case 0:
                    sscanf(strPosicao, "%d", &ranking.posicoes[linha]);
                    memset(strPosicao,0,sizeof(strPosicao));
                    break;
                case 1:
                    strcpy(ranking.nomes[linha], strNome);
                    ranking.nomes[linha][aux] = '\0';
                    memset(strNome,0,sizeof(strNome));
                    break;
                case 2:
                    sscanf(strPontuacao, "%d", &ranking.pontuacoes[linha]);
                    memset(strPontuacao,0,sizeof(strPontuacao));
                    break;
                default:
                    break;
            }
            aux=0;
            posicaoPontoVirgula++;
            if(posicaoPontoVirgula==3) {
                posicaoPontoVirgula = 0; // reiniciou a linha
                linha++;
            }
        }
        //exibe o caracter lido na tela
    }
}

// Rotina principal
int main(int argc, char **argv) {
    // Configuração inicial da janela do GLUT
    //abrindo o arquivo
    leRanking();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(comprimentoMaximoTela, larguraMaximaTela);

    glutCreateWindow("Segue Mouse com Atraso");
    inicializa();

    glutDisplayFunc(desenhaCena);
    glutReshapeFunc(redimensiona);
    glutKeyboardFunc(teclaPressionada);
    glutKeyboardUpFunc(teclaSolta); // Tell GLUT to use the method "keyUp" for key up events  
    // Registra a função "movimentoMouse" para executar sempre que o mouse mexer
    glutPassiveMotionFunc(movimentoMouse);
    // Registra a função "atualiza" para executar daqui a 0 milissegundos
    glutTimerFunc(0, atualizaCena, 40);

    glutTimerFunc(0, atualizaCoroa, 150);   

    glutMainLoop();
    return 0;
}