#include "funcoes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

int validarDigitos(const char texto[], int tamanho) {
    int i;
    if ((int)strlen(texto) != tamanho) return 0;

    for (i = 0; i < tamanho; i++) {
        if (!isdigit((unsigned char)texto[i])) return 0;
    }

    return 1;
}

// le uma linha sem deixar caracteres extras no teclado
int lerTexto(char texto[], int capacidade) {
    int c;
    size_t tamanho;

    if (fgets(texto, capacidade, stdin) == NULL) return 0;

    tamanho = strlen(texto);

    if (tamanho > 0 && texto[tamanho - 1] == '\n') {
        texto[tamanho - 1] = '\0';
    } else {
        // deixa linha pode ser maior que o espaco disponivel
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

// le o dinheiro positivo com ate duas casas decimais.
// faz aceitar 0,0 e 0.0 e limita em 1 milhao por vez
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

// salva o arquivo do investidor para escrever no txt depois
void nomeArquivoInvestidor(const char cpf[], char caminho[]) {
    sprintf(caminho, "investidor_%s.txt", cpf);
}

// salva o extrato do investidor pra escrever depois
void nomeArquivoExtrato(const char cpf[], char caminho[]) {
    sprintf(caminho, "extrato_%s.txt", cpf);
}

// grava os dados dos investidores
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

// busca os dados dos investidores para carrega-lo
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

// faaz o cadastro das pessoas
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

    // nao repete cpf
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
	//copia o conteudo da string esse strcpy
    strcpy(usuario->senha, entrada);

    // toda carteira vem zerada ja
    usuario->reais = 0.0;
    usuario->bitcoin = 0.0;
    usuario->ethereum = 0.0;
    usuario->ripple = 0.0;

    if (!salvarInvestidor(*usuario)) return 0;

    printf("\nCadastro realizado!\n");
    return 1;
}

// faz a verificacao da senha e do cpf pra entrar na conta
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

// pede a senha novamente para poder confirmar em algumas funcoes
int confirmarSenha(struct Investidor usuario) {
    char senha[100];

    printf("Confirme sua senha: ");

    if (!lerTexto(senha, sizeof(senha))) return 0;
	// o strcmp compara duas strings
    if (strcmp(senha, usuario.senha) != 0) {
        printf("\nSenha incorreta!\n");
        return 0;
    }

    return 1;
}

// menu do exchange
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

// serve pra exibir saldo pos confirmacao de senha
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

// salva extrato e saque no registro
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

// consulta o extrato do investidor logado
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

// mostra os saldos da carteira (reais e as tres criptomoedas)
void mostrarSaldos(struct Investidor usuario) {
    printf("Reais: R$ %.2f\n", usuario.reais);
    printf("Bitcoin: %.8f BTC\n", usuario.bitcoin);
    printf("Ethereum: %.8f ETH\n", usuario.ethereum);
    printf("Ripple: %.8f XRP\n", usuario.ripple);
}

// adiciona dinheiro na carteira
void depositar(struct Investidor *usuario) {
    double valor, anterior;

    printf("\n========== DEPOSITO ==========\n");
    printf("Valor do deposito: R$ ");

    if (!lerValor(&valor)) {
        printf("Valor invalido! Use um numero positivo "
               "com ate 2 casas decimais.\n");
        return;
    }

    anterior = usuario->reais; // guarda o saldo antes da operacao
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
    printf("\nSaldos atualizados:\n");
	mostrarSaldos(*usuario);
}

// tira dinheiro sem deixar o saldo negativo
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
    printf("\nSaldos atualizados:\n");
	mostrarSaldos(*usuario);
}

// dados das tres criptomoedas.
const char *nomes[3] = {"Bitcoin", "Ethereum", "Ripple"};
const char *siglas[3] = {"BTC", "ETH", "XRP"};

const double taxasCompra[3] = {0.02, 0.01, 0.01};
const double taxasVenda[3] = {0.03, 0.02, 0.01};

// recupera as cotacoes salvas no arquivo txt
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

    // valores das criptomoedas
    cotacoes[0] = 350000.00;
    cotacoes[1] = 16000.00;
    cotacoes[2] = 2.57;

    if (!salvarCotacoes(cotacoes)) {
        printf("\nErro ao salvar cotacoes iniciais.\n");
    }
}

// salva os precos das criptomoedas
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

// mostra o preco de cada uma das criptomoeda
void mostrarCotacoes(const double cotacoes[3]) {
    int i;

    printf("\n========== COTACOES (SIMULADAS) ==========\n");

    for (i = 0; i < 3; i++) {
        printf("%d - %s (%s): R$ %.2f\n",
               i + 1, nomes[i], siglas[i], cotacoes[i]);
    }
}

// serve para atualiza as cotacoes aleatoriamente
void atualizarCotacoes(double cotacoes[3]) {
    double novas[3], variacao;
    int i;

    for (i = 0; i < 3; i++) {
        variacao = (double)rand() / RAND_MAX * 0.10 - 0.05;

        novas[i] = cotacoes[i] * (1.0 + variacao);
		//long long porque o numero passa do limite do int
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

// faz vc escolher a criptomoeda para negociar
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

// le quantidades com ate oito casas decimais
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

// registra compras e vendas no extrato
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

// compra uma criptomoeda usando o saldo em reais
void comprar(struct Investidor *usuario, const double cotacoes[3]) {
    int moeda;
    double valor, taxa, quantidade, *saldoCripto; //saldocripto guarda o endereco do campo da moeda escolhida
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

    // desconta o efeito da taxa no valor convertido da moeda
    quantidade = (valor / (1.0 + taxa)) / cotacoes[moeda];
	//long long porque o numero passa do limite
    quantidade = (double)(
        (long long)(quantidade * 100000000.0 + 0.0000001)
    ) / 100000000.0;

    if (quantidade < 0.00000001) {
        printf("Valor muito pequeno para comprar esta moeda.\n");
        return;
    }

    saldoCripto = moeda == 0 ? &usuario->bitcoin : //saldocripto guarda o endereco do campo da moeda escolhida
                  moeda == 1 ? &usuario->ethereum :
                               &usuario->ripple;

    anterior = *usuario; // guarda uma copia da carteira inteira antes de alterar

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

// vende a criptomoeda do saldo do investidor
void vender(struct Investidor *usuario, const double cotacoes[3]) {
    int moeda;
    double quantidade, bruto, recebido, taxa, *saldoCripto; //saldocripto guarda o endereco do campo da moeda escolhida
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

    // o usuario recebe o valor apos descontar a taxa / long long porque o numero passa do limite do int
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
