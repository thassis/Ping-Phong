#include <SOIL/SOIL.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

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
    int pontosSet, quantidadeSetsGanho, pontosConsecutivos;
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

const int comprimentoMaximoTela = 600;
const int larguraMaximaTela = 300;

int orientacaoVerticalBola = 1;
int orientacaoHorizontalBola = -1;

int podeColidirDireita = 1;
int podeColidirEsquerda = 1;

int keyboard[256];

int jogoPausado = 0;
int reiniciarJogo = 0;

int quadroAtualBola = 0;
int quadroAtualCoroa = 0;

int bolaColidiuBarra = 0;

int verificarGanhadorSetConsecutivos = 0;

int limiteSetsGanharJogo = 3; //números impar
int quantidadePontosGanharSet = 11;

GLuint idTexturaBolaBranca;
GLuint idTexturaBolaPreta;
GLuint idTexturaBarraEsquerda;
GLuint idTexturaBarraDireita;
GLuint idYinYang;
GLuint idImagePause;
GLuint idTexturaCoroa;

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

    tamanhoBola.largura = 60;
    tamanhoBola.altura = 60;

    float posicaoVerticalBola = rand()%larguraMaximaTela;

    if(posicaoVerticalBola >= 0 && posicaoVerticalBola <= tamanhoBola.altura)
        posicaoVerticalBola += tamanhoBola.altura+5;
    else if(posicaoVerticalBola <= larguraMaximaTela && posicaoVerticalBola >= larguraMaximaTela - tamanhoBola.altura){
        printf("%f",(float) tamanhoBola.altura);
        posicaoVerticalBola = larguraMaximaTela-tamanhoBola.altura-5;
    }

    tamanhoBarraDireita.largura = 15;
    tamanhoBarraDireita.altura = 40;
    posicaoBarraDireita.x = (comprimentoMaximoTela-tamanhoBarraDireita.largura);
    posicaoBarraDireita.y = (larguraMaximaTela-tamanhoBarraDireita.altura)/2;

    tamanhoBarraEsquerda.largura = 15;
    tamanhoBarraEsquerda.altura = 40;
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

    jogoPausado = 0;

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


void desenhaBackgroundPreto(){
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_TRIANGLE_STRIP);
        glVertex2f(0, 0);

        glVertex2f(comprimentoMaximoTela/2, 0);

        glVertex2f(0, larguraMaximaTela);

        glVertex2f(comprimentoMaximoTela/2, larguraMaximaTela);

    glEnd();
}

void verificaGanhadorGame(){
    if(primeiroJogador.quantidadeSetsGanho == limiteSetsGanharJogo){
        //ganhou
        iniciaObjetos();
    } else if(segundoJogador.quantidadeSetsGanho == limiteSetsGanharJogo){
        //ganhou
        iniciaObjetos();
    }
}

void atualizaSet(){
    verificarGanhadorSetConsecutivos = 0;
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

void desenhaQuadroAviso(){
    if(jogoPausado){
        glColor3d(0.50f, 0.50f, 0.50f);
        glBegin(GL_TRIANGLE_STRIP);
            glVertex2f(0, 0);

            glVertex2f(comprimentoMaximoTela, 0);

            glVertex2f(0, larguraMaximaTela);

            glVertex2f(comprimentoMaximoTela, larguraMaximaTela);

        glEnd();
    }
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
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0.0f, 0.0f, 0.0f);
    // Habilita o uso de texturas
    
    desenhaBackgroundPreto();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, idYinYang);
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_PRIMITIVE_RESTART);
    glBegin(GL_POLYGON);
        // Associamos um canto da textura para cada vértice
        glTexCoord2f(0, 0);
        glVertex2f((comprimentoMaximoTela/2) - 0.10*comprimentoMaximoTela, (comprimentoMaximoTela/4) - 0.10*comprimentoMaximoTela);

        glTexCoord2f(1, 0);
        glVertex2f((comprimentoMaximoTela/2) + 0.10*comprimentoMaximoTela, (comprimentoMaximoTela/4) - 0.10*comprimentoMaximoTela);

        glTexCoord2f(1, 1);
        glVertex2f((comprimentoMaximoTela/2) + 0.10*comprimentoMaximoTela, (comprimentoMaximoTela/4) + 0.10*comprimentoMaximoTela);

        glTexCoord2f(0, 1);
        glVertex2f((comprimentoMaximoTela/2) - 0.10*comprimentoMaximoTela, (comprimentoMaximoTela/4) + 0.10*comprimentoMaximoTela);

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

    desenhaQuadroAviso();

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
    glEnable(GL_BLEND );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    idTexturaBolaBranca = carregaTextura("bola_branca.png");
    idTexturaBolaPreta = carregaTextura("bola_preta.png");
    idTexturaBarraEsquerda = carregaTextura("barraEsquerda.png");
    idTexturaBarraDireita = carregaTextura("barraDireita.png");
    idYinYang = carregaTextura("yinyang.png");
    idImagePause = carregaTextura("imagem_pause.png");
    idTexturaCoroa = carregaTextura("crown.png");
    // define o quadrado
    iniciaObjetos();
}

// Callback de redimensionamento
void redimensiona(int w, int h) {
    glViewport(0, 0, w, h);

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
    switch(key) {
        // Tecla ESC
        case 27:
            exit(0);
            break;
        case 'p':
            if(jogoPausado == 1) {
                jogoPausado = 0;
            } else {
                jogoPausado = 1;
            }
            glutPostRedisplay();
            break;
        case 'r':
            iniciaObjetos();
            break;
        default:
            break;
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
    if(jogoPausado == 0){
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

// Rotina principal
int main(int argc, char **argv) {
    // Configuração inicial da janela do GLUT
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
    glutTimerFunc(0, atualizaCena, 33);

    glutTimerFunc(0, atualizaCoroa, 150);   

    glutMainLoop();
    return 0;
}