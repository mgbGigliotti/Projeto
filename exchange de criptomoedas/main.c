
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Dados de cada investidor. Cada CPF possui carteira e extrato proprios.
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
int importarCadastroAntigo(void);
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

int validarDigitos(const char texto[], int tamanho) {
    int i;
    if ((int)strlen(texto) != tamanho) return 0;

    for (i = 0; i < tamanho; i++) {
        if (!isdigit((unsigned char)texto[i])) return 0;
    }

    return 1;
}

// Le uma linha sem deixar caracteres extras no teclado.
int lerTexto(char texto[], int capacidade) {
    int c;
    size_t tamanho;

    if (fgets(texto, capacidade, stdin) == NULL) return 0;

    tamanho = strlen(texto);

    if (tamanho > 0 && texto[tamanho - 1] == '\n') {
        texto[tamanho - 1] = '\0';
    } else {
        // A linha pode ser maior que o espaco disponivel.
        while ((c = getchar()) != '\n' && c != EOF) { }
        return 0;
    }

    return 1;
}

int lerOpcao(void) {
    char entrada[100], extra;
    int opcao;

    if (!lerTexto(entrada, sizeof(entrada))) return -1;
    if (sscanf(entrada, " %d %c", &opcao, &extra) != 1) return 0;

    return opcao;
}

// Le dinheiro positivo com ate duas casas decimais.
// Aceita 125,50 ou 125.50. Limite de R$ 1 milhao por operacao.
int lerValor(double *valor) {
    char entrada[100], *fim;
    int i, separador = 0, casas = 0, digitos = 0;
    double numero;

    if (!lerTexto(entrada, sizeof(entrada))) return 0;

    for (i = 0; entrada[i] != '\0'; i++) {
        if (isdigit((unsigned char)entrada[i])) {
            digitos++;
            if (separador) casas++;
        } else if (entrada[i] == ',' || entrada[i] == '.') {
            separador++;
            entrada[i] = '.';
        } else {
            return 0;
        }
    }

    if (!digitos || separador > 1 || casas > 2) return 0;

    numero = strtod(entrada, &fim);

    if (*fim != '\0' || !(numero > 0.0) ||
        numero > 1000000.0) return 0;

    *valor = numero;
    return 1;
}

// Cada CPF possui seu proprio arquivo de carteira.
void nomeArquivoInvestidor(const char cpf[], char caminho[]) {
    sprintf(caminho, "investidor_%s.txt", cpf);
}

// Cada CPF possui seu proprio arquivo de extrato.
void nomeArquivoExtrato(const char cpf[], char caminho[]) {
    sprintf(caminho, "extrato_%s.txt", cpf);
}

// Grava os dados de um investidor.
int salvarInvestidor(struct Investidor usuario) {
    FILE *arquivo;
    char caminho[64];
    int falhou;

    nomeArquivoInvestidor(usuario.cpf, caminho);
    arquivo = fopen(caminho, "w");

    if (arquivo == NULL) {
        printf("\nNao foi possivel salvar %s.\n", caminho);
        return 0;
    }

    fprintf(arquivo, "%s\n%s\n%s\n",
            usuario.nome, usuario.cpf, usuario.senha);

    fprintf(arquivo, "%.2f\n%.8f\n%.8f\n%.8f\n",
            usuario.reais, usuario.bitcoin,
            usuario.ethereum, usuario.ripple);

    falhou = ferror(arquivo);

    if (fclose(arquivo) != 0) falhou = 1;

    return !falhou;
}

// Busca o cadastro pelo CPF informado.
int carregarInvestidor(const char cpf[], struct Investidor *usuario) {
    FILE *arquivo;
    char caminho[64];
    int resultado;
    size_t tamanho;
    struct Investidor encontrado;

    if (!validarDigitos(cpf, 11)) return 0;

    nomeArquivoInvestidor(cpf, caminho);
    arquivo = fopen(caminho, "r");

    if (arquivo == NULL) return 0;

    if (fgets(encontrado.nome,
              sizeof(encontrado.nome), arquivo) == NULL) {
        fclose(arquivo);
        return 0;
    }

    tamanho = strlen(encontrado.nome);

    if (tamanho && encontrado.nome[tamanho - 1] == '\n') {
        encontrado.nome[tamanho - 1] = '\0';
    }

    resultado = fscanf(
        arquivo, "%11s %6s %lf %lf %lf %lf",
        encontrado.cpf,
        encontrado.senha,
        &encontrado.reais,
        &encontrado.bitcoin,
        &encontrado.ethereum,
        &encontrado.ripple
    );

    fclose(arquivo);

    if (resultado != 6 ||
        strcmp(encontrado.cpf, cpf) != 0 ||
        !validarDigitos(encontrado.senha, 6) ||
        encontrado.reais < 0 ||
        encontrado.bitcoin < 0 ||
        encontrado.ethereum < 0 ||
        encontrado.ripple < 0) {
        return 0;
    }

    *usuario = encontrado;
    return 1;
}

// Recupera o cadastro da versao anterior, se existir.
int importarCadastroAntigo(void) {
    FILE *arquivo;
    struct Investidor antigo, existente;
    size_t tamanho;
    int resultado;

    arquivo = fopen("investidor.txt", "r");

    if (arquivo == NULL) return 1;

    if (fgets(antigo.nome, sizeof(antigo.nome), arquivo) == NULL) {
        fclose(arquivo);
        return 0;
    }

    tamanho = strlen(antigo.nome);

    if (tamanho && antigo.nome[tamanho - 1] == '\n') {
        antigo.nome[tamanho - 1] = '\0';
    }

    resultado = fscanf(
        arquivo, "%11s %6s %lf %lf %lf %lf",
        antigo.cpf,
        antigo.senha,
        &antigo.reais,
        &antigo.bitcoin,
        &antigo.ethereum,
        &antigo.ripple
    );

    fclose(arquivo);

    if (resultado != 6 ||
        !validarDigitos(antigo.cpf, 11) ||
        !validarDigitos(antigo.senha, 6)) {
        return 0;
    }

    if (!carregarInvestidor(antigo.cpf, &existente)) {

        if (!salvarInvestidor(antigo)) return 0;

        // Copia o extrato antigo caso a nova conta nao tenha um.
        {
            char destino[64];
            FILE *origem, *novo;
            int c;

            nomeArquivoExtrato(antigo.cpf, destino);
            origem = fopen("extrato.txt", "r");

            if (origem != NULL) {
                novo = fopen(destino, "r");

                if (novo != NULL) {
                    fclose(novo);
                } else {
                    novo = fopen(destino, "w");

                    if (novo != NULL) {
                        while ((c = fgetc(origem)) != EOF) {
                            fputc(c, novo);
                        }
                        fclose(novo);
                    }
                }

                fclose(origem);
            }
        }

        printf("\nCadastro antigo importado.\n");
    }

    return 1;
}

// Permite cadastrar varios investidores.
int cadastrarInvestidor(struct Investidor *usuario) {
    char entrada[100];
    struct Investidor existente;

    printf("\n========== CADASTRO ==========\n");

    printf("Nome completo: ");

    if (!lerTexto(entrada, sizeof(entrada)) ||
        !strlen(entrada)) {
        printf("Nome invalido.\n");
        return 0;
    }

    strcpy(usuario->nome, entrada);

    printf("CPF (11 numeros): ");

    if (!lerTexto(entrada, sizeof(entrada)) ||
        !validarDigitos(entrada, 11)) {
        printf("CPF invalido.\n");
        return 0;
    }

    // Impede CPF repetido.
    if (carregarInvestidor(entrada, &existente)) {
        printf("Esse CPF ja esta cadastrado.\n");
        return 0;
    }

    strcpy(usuario->cpf, entrada);

    printf("Senha (6 numeros): ");

    if (!lerTexto(entrada, sizeof(entrada)) ||
        !validarDigitos(entrada, 6)) {
        printf("Senha invalida.\n");
        return 0;
    }

    strcpy(usuario->senha, entrada);

    // Toda carteira nova comeca zerada.
    usuario->reais = 0.0;
    usuario->bitcoin = 0.0;
    usuario->ethereum = 0.0;
    usuario->ripple = 0.0;

    if (!salvarInvestidor(*usuario)) return 0;

    printf("\nCadastro realizado!\n");
    return 1;
}

// Verifica o CPF e a senha para entrar na conta correta.
int realizarLogin(struct Investidor *usuario) {
    char cpf[100], senha[100];

    printf("\n========== LOGIN ==========\n");

    printf("CPF: ");
    if (!lerTexto(cpf, sizeof(cpf))) return 0;

    printf("Senha: ");
    if (!lerTexto(senha, sizeof(senha))) return 0;

    if (!carregarInvestidor(cpf, usuario) ||
        strcmp(senha, usuario->senha) != 0) {
        printf("\nCPF ou senha incorretos.\n");
        return 0;
    }

    printf("\nBem-vindo, %s!\n", usuario->nome);
    return 1;
}

// Confirma a senha para consultar informacoes e negociar.
int confirmarSenha(struct Investidor usuario) {
    char senha[100];

    printf("Confirme sua senha: ");

    if (!lerTexto(senha, sizeof(senha))) return 0;

    if (strcmp(senha, usuario.senha) != 0) {
        printf("\nSenha incorreta!\n");
        return 0;
    }

    return 1;
}

// Menu da Exchange.
void mostrarMenu(void) {
    printf("\n==================================\n");
    printf("      EXCHANGE DE CRIPTOMOEDAS\n");
    printf("==================================\n");

    printf("1 - Consultar saldo\n");
    printf("2 - Consultar extrato\n");
    printf("3 - Depositar reais\n");
    printf("4 - Sacar reais\n");
    printf("5 - Comprar criptomoedas\n");
    printf("6 - Vender criptomoedas\n");
    printf("7 - Atualizar cotacao\n");
    printf("8 - Sair\n");
}

// Exibe o saldo apenas depois de confirmar a senha.
void consultarSaldo(struct Investidor usuario) {
    if (!confirmarSenha(usuario)) return;

    printf("\n========== SUA CARTEIRA ==========\n");
    printf("Nome: %s\n", usuario.nome);
    printf("CPF: %s\n", usuario.cpf);
    printf("Reais: R$ %.2f\n", usuario.reais);
    printf("Bitcoin: %.8f BTC\n", usuario.bitcoin);
    printf("Ethereum: %.8f ETH\n", usuario.ethereum);
    printf("Ripple: %.8f XRP\n", usuario.ripple);
}

// Registra depositos e saques no extrato individual.
int registrarExtrato(
    const char operacao[],
    double valor,
    struct Investidor usuario
) {
    char caminho[64];
    FILE *arquivo;
    time_t agora;
    struct tm *data;
    char horario[32];
    int falhou;

    nomeArquivoExtrato(usuario.cpf, caminho);
    arquivo = fopen(caminho, "a");

    if (arquivo == NULL) return 0;

    agora = time(NULL);
    data = localtime(&agora);

    if (data == NULL ||
        strftime(horario, sizeof(horario),
                 "%d-%m-%Y %H:%M", data) == 0) {
        fclose(arquivo);
        return 0;
    }

    fprintf(
        arquivo,
        "%s | %s | R$ %.2f | CT: 0.00 | TX: 0.00 "
        "| REAL: %.2f | BTC: %.8f | ETH: %.8f | XRP: %.8f\n",
        horario,
        operacao,
        valor,
        usuario.reais,
        usuario.bitcoin,
        usuario.ethereum,
        usuario.ripple
    );

    falhou = ferror(arquivo);
    if (fclose(arquivo) != 0) falhou = 1;

    return !falhou;
}

// Consulta somente o extrato do investidor logado.
void consultarExtrato(struct Investidor usuario) {
    FILE *arquivo;
    char caminho[64];
    char linha[512];
    int encontrou = 0;

    if (!confirmarSenha(usuario)) return;

    nomeArquivoExtrato(usuario.cpf, caminho);
    arquivo = fopen(caminho, "r");

    printf("\n========== EXTRATO ==========\n");
    printf("Nome: %s\n", usuario.nome);
    printf("CPF: %s\n\n", usuario.cpf);

    if (arquivo == NULL) {
        printf("Nenhuma movimentacao registrada ainda.\n");
        return;
    }

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        printf("%s", linha);
        encontrou = 1;
    }

    if (ferror(arquivo)) {
        printf("\nErro ao ler o extrato.\n");
    }

    fclose(arquivo);

    if (!encontrou) {
        printf("Nenhuma movimentacao registrada ainda.\n");
    }
}

// Adiciona dinheiro a carteira.
void depositar(struct Investidor *usuario) {
    double valor, anterior;

    printf("\n========== DEPOSITO ==========\n");
    printf("Valor do deposito: R$ ");

    if (!lerValor(&valor)) {
        printf("Valor invalido! Use um numero positivo "
               "com ate 2 casas decimais.\n");
        return;
    }

    anterior = usuario->reais;
    usuario->reais += valor;

    if (!salvarInvestidor(*usuario)) {
        usuario->reais = anterior;
        printf("Deposito nao concluido.\n");
        return;
    }

    if (!registrarExtrato("+ DEPOSITO", valor, *usuario)) {
        printf("AVISO: saldo salvo, mas houve erro no extrato.\n");
    }

    printf("Deposito realizado!\n");
    printf("Saldo: R$ %.2f\n", usuario->reais);
}

// Retira dinheiro sem deixar saldo negativo.
void sacar(struct Investidor *usuario) {
    double valor, anterior;

    printf("\n========== SAQUE ==========\n");
    printf("Valor do saque: R$ ");

    if (!lerValor(&valor)) {
        printf("Valor invalido! Use um numero positivo "
               "com ate 2 casas decimais.\n");
        return;
    }

    if (valor > usuario->reais) {
        printf("Saldo insuficiente!\n");
        printf("Disponivel: R$ %.2f\n", usuario->reais);
        return;
    }

    anterior = usuario->reais;
    usuario->reais -= valor;

    if (!salvarInvestidor(*usuario)) {
        usuario->reais = anterior;
        printf("Saque nao concluido.\n");
        return;
    }

    if (!registrarExtrato("- SAQUE", valor, *usuario)) {
        printf("AVISO: saldo salvo, mas houve erro no extrato.\n");
    }

    printf("Saque realizado!\n");
    printf("Saldo: R$ %.2f\n", usuario->reais);
}

// Dados das tres criptomoedas.
const char *nomes[3] = {"Bitcoin", "Ethereum", "Ripple"};
const char *siglas[3] = {"BTC", "ETH", "XRP"};

const double taxasCompra[3] = {0.02, 0.01, 0.01};
const double taxasVenda[3] = {0.03, 0.02, 0.01};

// Recupera as cotacoes salvas no arquivo TXT.
void carregarCotacoes(double cotacoes[3]) {
    FILE *arquivo = fopen("cotacoes.txt", "r");
    double btc, eth, xrp;

    if (arquivo != NULL) {
        if (fscanf(arquivo, "%lf %lf %lf",
                   &btc, &eth, &xrp) == 3 &&
            btc > 0 && eth > 0 && xrp > 0 &&
            btc < 1000000000000.0 &&
            eth < 1000000000000.0 &&
            xrp < 1000000000000.0) {

            cotacoes[0] = btc;
            cotacoes[1] = eth;
            cotacoes[2] = xrp;

            fclose(arquivo);
            return;
        }

        fclose(arquivo);
        printf("\nAviso: cotacoes.txt invalido.\n");
    }

    // Valores ficticios usados na simulacao.
    cotacoes[0] = 350000.00;
    cotacoes[1] = 16000.00;
    cotacoes[2] = 2.57;

    if (!salvarCotacoes(cotacoes)) {
        printf("\nErro ao salvar cotacoes iniciais.\n");
    }
}

// Salva os precos das criptomoedas.
int salvarCotacoes(const double cotacoes[3]) {
    FILE *arquivo = fopen("cotacoes.txt", "w");
    int falhou;

    if (arquivo == NULL) return 0;

    fprintf(arquivo, "%.8f\n%.8f\n%.8f\n",
            cotacoes[0], cotacoes[1], cotacoes[2]);

    falhou = ferror(arquivo);

    if (fclose(arquivo) != 0) falhou = 1;

    return !falhou;
}

// Mostra o preco de cada criptomoeda.
void mostrarCotacoes(const double cotacoes[3]) {
    int i;

    printf("\n========== COTACOES (SIMULADAS) ==========\n");

    for (i = 0; i < 3; i++) {
        printf("%d - %s (%s): R$ %.2f\n",
               i + 1, nomes[i], siglas[i], cotacoes[i]);
    }
}

// Atualiza as cotacoes aleatoriamente em ate 5%.
void atualizarCotacoes(double cotacoes[3]) {
    double novas[3], variacao;
    int i;

    for (i = 0; i < 3; i++) {
        variacao = (double)rand() / RAND_MAX * 0.10 - 0.05;

        novas[i] = cotacoes[i] * (1.0 + variacao);

        novas[i] = (double)(
            (long long)(novas[i] * 100000000.0 + 0.5)
        ) / 100000000.0;
    }

    if (!salvarCotacoes(novas)) {
        printf("\nErro ao salvar novas cotacoes.\n");
        return;
    }

    for (i = 0; i < 3; i++) {
        cotacoes[i] = novas[i];
    }

    printf("\nCotacoes atualizadas!\n");
    mostrarCotacoes(cotacoes);
}

// Escolhe a criptomoeda para negociar.
int escolherMoeda(const double cotacoes[3]) {
    int opcao;

    mostrarCotacoes(cotacoes);

    printf("Escolha a criptomoeda (1 a 3): ");
    opcao = lerOpcao();

    if (opcao < 1 || opcao > 3) {
        printf("Moeda invalida.\n");
        return -1;
    }

    return opcao - 1;
}

// Le quantidades com ate oito casas decimais.
int lerQuantidade(double *quantidade) {
    char entrada[100], *fim;
    int i, ponto = 0, casas = 0, digitos = 0;
    double numero;

    if (!lerTexto(entrada, sizeof(entrada))) return 0;

    for (i = 0; entrada[i] != '\0'; i++) {
        if (isdigit((unsigned char)entrada[i])) {
            digitos++;
            if (ponto) casas++;
        } else if (entrada[i] == '.' || entrada[i] == ',') {
            ponto++;
            entrada[i] = '.';
        } else {
            return 0;
        }
    }

    if (!digitos || ponto > 1 || casas > 8) return 0;

    numero = strtod(entrada, &fim);

    if (*fim != '\0' ||
        numero < 0.00000001 ||
        numero > 1000000.0) {
        return 0;
    }

    *quantidade = numero;
    return 1;
}

// Registra compras e vendas no extrato.
int registrarCripto(
    const char operacao[],
    double quantidade,
    int moeda,
    double cotacao,
    double taxa,
    struct Investidor usuario
) {
    char caminho[64];
    FILE *arquivo;
    time_t agora;
    struct tm *data;
    char horario[32];
    int falhou;

    nomeArquivoExtrato(usuario.cpf, caminho);
    arquivo = fopen(caminho, "a");

    if (arquivo == NULL) return 0;

    agora = time(NULL);
    data = localtime(&agora);

    if (data == NULL ||
        strftime(horario, sizeof(horario),
                 "%d-%m-%Y %H:%M", data) == 0) {
        fclose(arquivo);
        return 0;
    }

    fprintf(
        arquivo,
        "%s | %s %.8f %s | CT: %.8f | TX: %.2f%% "
        "| REAL: %.2f | BTC: %.8f | ETH: %.8f | XRP: %.8f\n",
        horario,
        operacao,
        quantidade,
        siglas[moeda],
        cotacao,
        taxa * 100.0,
        usuario.reais,
        usuario.bitcoin,
        usuario.ethereum,
        usuario.ripple
    );

    falhou = ferror(arquivo);

    if (fclose(arquivo) != 0) falhou = 1;

    return !falhou;
}

// Compra uma criptomoeda usando o saldo em reais.
void comprar(struct Investidor *usuario, const double cotacoes[3]) {
    int moeda;
    double valor, taxa, quantidade, *saldoCripto;
    struct Investidor anterior;

    printf("\n========== COMPRA ==========\n");

    if (!confirmarSenha(*usuario)) return;

    moeda = escolherMoeda(cotacoes);
    if (moeda == -1) return;

    printf("Valor TOTAL a gastar em reais (inclui taxa): R$ ");

    if (!lerValor(&valor)) {
        printf("Valor invalido.\n");
        return;
    }

    if (valor > usuario->reais + 0.0000001) {
        printf("Saldo insuficiente!\n");
        return;
    }

    taxa = taxasCompra[moeda];

    // Desconta o efeito da taxa no valor convertido.
    quantidade = (valor / (1.0 + taxa)) / cotacoes[moeda];

    quantidade = (double)(
        (long long)(quantidade * 100000000.0 + 0.0000001)
    ) / 100000000.0;

    if (quantidade < 0.00000001) {
        printf("Valor muito pequeno para comprar esta moeda.\n");
        return;
    }

    saldoCripto = moeda == 0 ? &usuario->bitcoin :
                  moeda == 1 ? &usuario->ethereum :
                               &usuario->ripple;

    anterior = *usuario;

    usuario->reais -= valor;
    *saldoCripto += quantidade;

    if (!salvarInvestidor(*usuario)) {
        *usuario = anterior;
        printf("Compra nao concluida: falha ao salvar.\n");
        return;
    }

    if (!registrarCripto("+ COMPRA", quantidade, moeda,
                         cotacoes[moeda], taxa, *usuario)) {
        printf("AVISO: carteira salva, mas houve erro no extrato.\n");
    }

    printf("Compra efetuada: %.8f %s (taxa %.0f%%).\n",
           quantidade, siglas[moeda], taxa * 100.0);

    printf("Saldo em reais: R$ %.2f\n", usuario->reais);
}

// Vende uma criptomoeda do saldo do investidor.
void vender(struct Investidor *usuario, const double cotacoes[3]) {
    int moeda;
    double quantidade, bruto, recebido, taxa, *saldoCripto;
    struct Investidor anterior;

    printf("\n========== VENDA ==========\n");

    if (!confirmarSenha(*usuario)) return;

    moeda = escolherMoeda(cotacoes);
    if (moeda == -1) return;

    saldoCripto = moeda == 0 ? &usuario->bitcoin :
                  moeda == 1 ? &usuario->ethereum :
                               &usuario->ripple;

    printf("Quantidade de %s a vender (ate 8 casas): ",
           siglas[moeda]);

    if (!lerQuantidade(&quantidade)) {
        printf("Quantidade invalida.\n");
        return;
    }

    if (quantidade > *saldoCripto + 0.000000001) {
        printf("Saldo de %s insuficiente.\n", siglas[moeda]);
        return;
    }

    taxa = taxasVenda[moeda];

    bruto = quantidade * cotacoes[moeda];

    // O usuario recebe o valor apos descontar a taxa.
    recebido = (double)(
        (long long)(bruto * (1.0 - taxa) * 100.0 + 0.5)
    ) / 100.0;

    if (recebido <= 0.0) {
        printf("Valor muito pequeno para gerar reais.\n");
        return;
    }

    anterior = *usuario;

    *saldoCripto -= quantidade;

    if (*saldoCripto < 0.000000001) {
        *saldoCripto = 0.0;
    }

    usuario->reais += recebido;

    if (!salvarInvestidor(*usuario)) {
        *usuario = anterior;
        printf("Venda nao concluida: falha ao salvar.\n");
        return;
    }

    if (!registrarCripto("- VENDA", quantidade, moeda,
                         cotacoes[moeda], taxa, *usuario)) {
        printf("AVISO: carteira salva, mas houve erro no extrato.\n");
    }

    printf("Venda efetuada: %.8f %s (taxa %.0f%%).\n",
           quantidade, siglas[moeda], taxa * 100.0);

    printf("Recebido: R$ %.2f\n", recebido);
    printf("Saldo: R$ %.2f\n", usuario->reais);
}

// Funcao principal do programa.
int main(void) {
    struct Investidor usuario;
    double cotacoes[3];

    int opcaoInicial;
    int opcao;
    int logado = 0;

    // Inicializa os numeros aleatorios.
    srand((unsigned int)time(NULL));

    // Tenta recuperar o cadastro da versao antiga.
    if (!importarCadastroAntigo()) {
        printf("Aviso: nao foi possivel importar o cadastro antigo.\n");
    }

    // Carrega os precos das criptomoedas.
    carregarCotacoes(cotacoes);

    do {
        printf("\n==================================\n");
        printf("      EXCHANGE DE CRIPTOMOEDAS\n");
        printf("==================================\n");

        printf("1 - Cadastrar investidor\n");
        printf("2 - Fazer login\n");
        printf("3 - Sair\n");

        printf("\nEscolha uma opcao: ");
        opcaoInicial = lerOpcao();

        switch (opcaoInicial) {

            case 1:
                cadastrarInvestidor(&usuario);
                break;

            case 2:
                logado = realizarLogin(&usuario);
                break;

            case 3:
                printf("\nEncerrando o programa...\n");
                break;

            default:
                printf("\nOpcao invalida!\n");
        }

        // Menu principal do investidor logado.
        if (logado) {
            do {
                mostrarMenu();

                printf("\nEscolha uma opcao: ");
                opcao = lerOpcao();

                switch (opcao) {

                    case 1:
                        consultarSaldo(usuario);
                        break;

                    case 2:
                        consultarExtrato(usuario);
                        break;

                    case 3:
                        depositar(&usuario);
                        break;

                    case 4:
                        sacar(&usuario);
                        break;

                    case 5:
                        comprar(&usuario, cotacoes);
                        break;

                    case 6:
                        vender(&usuario, cotacoes);
                        break;

                    case 7:
                        atualizarCotacoes(cotacoes);
                        break;

                    case 8:
                        printf("\nSaindo da conta...\n");
                        break;

                    default:
                        printf("\nOpcao invalida!\n");
                }

            } while (opcao != 8);

            logado = 0;
        }

    } while (opcaoInicial != 3);

    return 0;
}
