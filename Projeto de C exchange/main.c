#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "funcoes.h"

// Funcao principal do programa.
int main(void) {
    struct Investidor usuario;
    double cotacoes[3];

    int opcaoInicial;
    int opcao;
    int logado = 0;

    // Inicializa os numeros aleatorios.
    srand((unsigned int)time(NULL));

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
