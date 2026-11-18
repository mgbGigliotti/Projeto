#ifndef FUNCOES_H
#define FUNCOES_H

struct Investidor {
    char nome[100];
    char cpf[12];
    char senha[7];
    double reais;
    double bitcoin;
    double ethereum;
    double ripple;
};

// Prototipos: permitem chamar funcoes escritas mais abaixo.
int validarDigitos(const char texto[], int tamanho);
int lerTexto(char texto[], int capacidade);
int lerOpcao(void);
int lerValor(double *valor);
int salvarInvestidor(struct Investidor usuario);
int carregarInvestidor(const char cpf[], struct Investidor *usuario);
void nomeArquivoInvestidor(const char cpf[], char caminho[]);
void nomeArquivoExtrato(const char cpf[], char caminho[]);
int cadastrarInvestidor(struct Investidor *usuario);
int realizarLogin(struct Investidor *usuario);
int confirmarSenha(struct Investidor usuario);
void mostrarMenu(void);
void consultarSaldo(struct Investidor usuario);
int registrarExtrato(const char operacao[], double valor, struct Investidor usuario);
void consultarExtrato(struct Investidor usuario);
void depositar(struct Investidor *usuario);
void sacar(struct Investidor *usuario);
int salvarCotacoes(const double cotacoes[3]);
void carregarCotacoes(double cotacoes[3]);
void atualizarCotacoes(double cotacoes[3]);
void mostrarCotacoes(const double cotacoes[3]);
void comprar(struct Investidor *usuario, const double cotacoes[3]);
void vender(struct Investidor *usuario, const double cotacoes[3]);
int registrarCripto(const char operacao[], double quantidade, int moeda,
                   double cotacao, double taxa, struct Investidor usuario);
int lerQuantidade(double *quantidade);
int escolherMoeda(const double cotacoes[3]);
void mostrarSaldos(struct Investidor usuario);

#endif
