
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Estrutura que guarda os dados do investidor
struct Investidor {
    char nome[100];
    char cpf[12];
    char senha[7];

    double reais;
    double bitcoin;
    double ethereum;
    double ripple;
};

// Declaracao das funcoes
int validarDigitos(const char texto[], int tamanho);
int lerTexto(char texto[], int capacidade);
int lerOpcao(void);
int lerValor(double *valor);
int salvarInvestidor(struct Investidor usuario);
int carregarInvestidor(struct Investidor *usuario);
int cadastrarInvestidor(struct Investidor *usuario);
int realizarLogin(struct Investidor usuario);
int confirmarSenha(struct Investidor usuario);
void mostrarMenu(void);
void consultarSaldo(struct Investidor usuario);
int registrarExtrato(const char operacao[], double valor,
                    struct Investidor usuario);
void consultarExtrato(struct Investidor usuario);
void depositar(struct Investidor *usuario);
void sacar(struct Investidor *usuario);

// Confere se um texto contem somente digitos
int validarDigitos(const char texto[], int tamanho) {
    int i;

    if ((int)strlen(texto) != tamanho) {
        return 0;
    }

    for (i = 0; i < tamanho; i++) {
        if (!isdigit((unsigned char)texto[i])) {
            return 0;
        }
    }

    return 1;
}

// Le um texto e remove a quebra de linha
int lerTexto(char texto[], int capacidade) {
    int c;
    size_t tamanho;

    if (fgets(texto, capacidade, stdin) == NULL) {
        return 0;
    }

    tamanho = strlen(texto);

    if (tamanho > 0 && texto[tamanho - 1] == '\n') {
        texto[tamanho - 1] = '\0';
    } else {
        // Descarta caracteres extras de uma linha longa
        while ((c = getchar()) != '\n' && c != EOF) {
        }
        return 0;
    }

    return 1;
}

// Le a opcao escolhida no menu
int lerOpcao(void) {
    char entrada[100];
    char extra;
    int opcao;

    if (!lerTexto(entrada, sizeof(entrada))) {
        return -1;
    }

    if (sscanf(entrada, " %d %c", &opcao, &extra) != 1) {
        return 0;
    }

    return opcao;
}

// Le valores monetarios positivos
// Aceita ponto ou virgula e ate 2 casas decimais
int lerValor(double *valor) {
    char entrada[100];
    char *fim;
    int i;
    int separador = 0;
    int casas = 0;
    int digitos = 0;
    double numero;

    if (!lerTexto(entrada, sizeof(entrada))) {
        return 0;
    }

    for (i = 0; entrada[i] != '\0'; i++) {

        if (isdigit((unsigned char)entrada[i])) {
            digitos++;

            if (separador) {
                casas++;
            }

        } else if (entrada[i] == ',' || entrada[i] == '.') {
            separador++;
            entrada[i] = '.';

        } else {
            return 0;
        }
    }

    if (!digitos || separador > 1 || casas > 2) {
        return 0;
    }

    numero = strtod(entrada, &fim);

    if (*fim != '\0' ||
        !(numero > 0.0) ||
        numero > 1000000.0) {
        return 0;
    }

    *valor = numero;
    return 1;
}

// Salva o cadastro e os saldos no arquivo TXT
int salvarInvestidor(struct Investidor usuario) {
    FILE *arquivo;
    int falhou;

    arquivo = fopen("investidor.txt", "w");

    if (arquivo == NULL) {
        printf("\nNao foi possivel abrir investidor.txt.\n");
        return 0;
    }

    fprintf(arquivo, "%s\n", usuario.nome);
    fprintf(arquivo, "%s\n", usuario.cpf);
    fprintf(arquivo, "%s\n", usuario.senha);

    fprintf(arquivo, "%.2f\n", usuario.reais);
    fprintf(arquivo, "%.8f\n", usuario.bitcoin);
    fprintf(arquivo, "%.8f\n", usuario.ethereum);
    fprintf(arquivo, "%.8f\n", usuario.ripple);

    falhou = ferror(arquivo);

    if (fclose(arquivo) != 0) {
        falhou = 1;
    }

    if (falhou) {
        printf("\nErro ao gravar investidor.txt.\n");
    }

    return !falhou;
}

// Recupera os dados armazenados no arquivo
int carregarInvestidor(struct Investidor *usuario) {
    FILE *arquivo;
    int resultado;
    size_t tamanho;

    arquivo = fopen("investidor.txt", "r");

    if (arquivo == NULL) {
        return 0;
    }

    if (fgets(usuario->nome,
              sizeof(usuario->nome), arquivo) == NULL) {
        fclose(arquivo);
        return 0;
    }

    tamanho = strlen(usuario->nome);

    if (tamanho && usuario->nome[tamanho - 1] == '\n') {
        usuario->nome[tamanho - 1] = '\0';
    }

    resultado = fscanf(
        arquivo,
        "%11s %6s %lf %lf %lf %lf",
        usuario->cpf,
        usuario->senha,
        &usuario->reais,
        &usuario->bitcoin,
        &usuario->ethereum,
        &usuario->ripple
    );

    fclose(arquivo);

    if (resultado != 6 ||
        !validarDigitos(usuario->cpf, 11) ||
        !validarDigitos(usuario->senha, 6)) {
        return 0;
    }

    return 1;
}

// Cadastra um investidor com saldo zerado
int cadastrarInvestidor(struct Investidor *usuario) {
    char entrada[100];

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

    strcpy(usuario->cpf, entrada);

    printf("Senha (6 numeros): ");

    if (!lerTexto(entrada, sizeof(entrada)) ||
        !validarDigitos(entrada, 6)) {
        printf("Senha invalida.\n");
        return 0;
    }

    strcpy(usuario->senha, entrada);

    usuario->reais = 0.0;
    usuario->bitcoin = 0.0;
    usuario->ethereum = 0.0;
    usuario->ripple = 0.0;

    if (!salvarInvestidor(*usuario)) {
        return 0;
    }

    printf("\nCadastro realizado!\n");
    return 1;
}

// Verifica CPF e senha para permitir o acesso
int realizarLogin(struct Investidor usuario) {
    char cpf[100];
    char senha[100];

    printf("\n========== LOGIN ==========\n");

    printf("CPF: ");

    if (!lerTexto(cpf, sizeof(cpf))) {
        return 0;
    }

    printf("Senha: ");

    if (!lerTexto(senha, sizeof(senha))) {
        return 0;
    }

    if (strcmp(cpf, usuario.cpf) == 0 &&
        strcmp(senha, usuario.senha) == 0) {

        printf("\nBem-vindo, %s!\n", usuario.nome);
        return 1;
    }

    printf("\nCPF ou senha incorretos.\n");
    return 0;
}

// Confere a senha antes de consultar informacoes
int confirmarSenha(struct Investidor usuario) {
    char senha[100];

    printf("Confirme sua senha: ");

    if (!lerTexto(senha, sizeof(senha))) {
        return 0;
    }

    if (strcmp(senha, usuario.senha) != 0) {
        printf("\nSenha incorreta!\n");
        return 0;
    }

    return 1;
}

// Exibe o menu principal do investidor
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

// Consulta a carteira apos confirmar a senha
void consultarSaldo(struct Investidor usuario) {

    if (!confirmarSenha(usuario)) {
        return;
    }

    printf("\n========== SUA CARTEIRA ==========\n");
    printf("Nome: %s\n", usuario.nome);
    printf("CPF: %s\n", usuario.cpf);

    printf("Reais: R$ %.2f\n", usuario.reais);
    printf("Bitcoin: %.8f BTC\n", usuario.bitcoin);
    printf("Ethereum: %.8f ETH\n", usuario.ethereum);
    printf("Ripple: %.8f XRP\n", usuario.ripple);
}

// Adiciona uma movimentacao ao extrato
int registrarExtrato(
    const char operacao[],
    double valor,
    struct Investidor usuario
) {
    FILE *arquivo;
    time_t agora;
    struct tm *data;
    char horario[32];
    int falhou;

    arquivo = fopen("extrato.txt", "a");

    if (arquivo == NULL) {
        return 0;
    }

    // Obtem a data e hora atual
    agora = time(NULL);
    data = localtime(&agora);

    if (data == NULL ||
        strftime(horario, sizeof(horario),
                 "%d-%m-%Y %H:%M", data) == 0) {
        fclose(arquivo);
        return 0;
    }

    // Depositos e saques nao possuem cotacao ou taxa
    fprintf(
        arquivo,
        "%s | %s | R$ %.2f | CT: 0.00 | TX: 0.00 "
        "| REAL: %.2f | BTC: %.8f | ETH: %.8f "
        "| XRP: %.8f\n",
        horario,
        operacao,
        valor,
        usuario.reais,
        usuario.bitcoin,
        usuario.ethereum,
        usuario.ripple
    );

    falhou = ferror(arquivo);

    if (fclose(arquivo) != 0) {
        falhou = 1;
    }

    return !falhou;
}

// Mostra as movimentacoes registradas
void consultarExtrato(struct Investidor usuario) {
    FILE *arquivo;
    char linha[512];
    int encontrou = 0;

    if (!confirmarSenha(usuario)) {
        return;
    }

    arquivo = fopen("extrato.txt", "r");

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

// Realiza um deposito e atualiza o saldo
void depositar(struct Investidor *usuario) {
    double valor;
    double anterior;

    printf("\n========== DEPOSITO ==========\n");
    printf("Valor do deposito: R$ ");

    if (!lerValor(&valor)) {
        printf("Valor invalido! Use um numero positivo "
               "com ate 2 casas decimais.\n");
        return;
    }

    anterior = usuario->reais;

    // Adiciona o valor ao saldo
    usuario->reais += valor;

    // Salva o novo saldo no arquivo
    if (!salvarInvestidor(*usuario)) {
        usuario->reais = anterior;
        printf("Deposito nao concluido.\n");
        return;
    }

    // Registra a operacao no extrato
    if (!registrarExtrato("+ DEPOSITO", valor, *usuario)) {
        printf("AVISO: saldo salvo, mas o extrato "
               "nao foi registrado.\n");
    }

    printf("Deposito realizado!\n");
    printf("Saldo: R$ %.2f\n", usuario->reais);
}

// Realiza um saque sem permitir saldo negativo
void sacar(struct Investidor *usuario) {
    double valor;
    double anterior;

    printf("\n========== SAQUE ==========\n");
    printf("Valor do saque: R$ ");

    if (!lerValor(&valor)) {
        printf("Valor invalido! Use um numero positivo "
               "com ate 2 casas decimais.\n");
        return;
    }

    // Confere se ha dinheiro suficiente
    if (valor > usuario->reais) {
        printf("Saldo insuficiente!\n");
        printf("Disponivel: R$ %.2f\n", usuario->reais);
        return;
    }

    anterior = usuario->reais;

    // Retira o valor do saldo
    usuario->reais -= valor;

    // Salva o saldo atualizado
    if (!salvarInvestidor(*usuario)) {
        usuario->reais = anterior;
        printf("Saque nao concluido.\n");
        return;
    }

    // Registra o saque no extrato
    if (!registrarExtrato("- SAQUE", valor, *usuario)) {
        printf("AVISO: saldo salvo, mas o extrato "
               "nao foi registrado.\n");
    }

    printf("Saque realizado!\n");
    
}

// Funcao principal do programa
int main(void) {
    struct Investidor usuario;

    int cadastrado = carregarInvestidor(&usuario);
    int opcaoInicial;
    int opcao;
    int logado = 0;

    // Menu inicial de cadastro e login
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
                if (cadastrado) {
                    printf("\nJa existe um investidor cadastrado!\n");
                } else {
                    cadastrado = cadastrarInvestidor(&usuario);
                }
                break;

            case 2:
                if (!cadastrado) {
                    printf("\nCadastre um investidor primeiro!\n");
                } else {
                    logado = realizarLogin(usuario);
                }
                break;

            case 3:
                printf("\nEncerrando o programa...\n");
                break;

            default:
                printf("\nOpcao invalida!\n");
        }

        // Menu principal apos o login
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
                        printf("\nCompra sera implementada.\n");
                        break;

                    case 6:
                        printf("\nVenda sera implementada.\n");
                        break;

                    case 7:
                        printf("\nCotacao sera implementada.\n");
                        break;

                    case 8:
                        printf("\nSaindo da conta...\n");
                        break;

                    default:
                        printf("\nOpcao invalida!\n");
                }

            } while (opcao != 8);

            // Retorna ao menu de login
            logado = 0;
        }

    } while (opcaoInicial != 3);

    return 0;
}
