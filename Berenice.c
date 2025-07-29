#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>

#include <stdarg.h>
#include <time.h>

void escrever_log(const char *mensagem, ...) {
    time_t agora = time(NULL);
    struct tm *tempo = localtime(&agora);
    va_list args;
    FILE *arquivo = fopen("arquivoMercado.log", "a");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de log");
        return;
    }
    fprintf(arquivo, "[%02d/%02d/%04d %02d:%02d:%02d] ",
            tempo->tm_mday,
            tempo->tm_mon + 1,
            tempo->tm_year + 1900,
            tempo->tm_hour,
            tempo->tm_min,
            tempo->tm_sec);
    va_start(args, mensagem);
    vfprintf(arquivo, mensagem, args);
    va_end(args);
    fprintf(arquivo, "\n");
    fclose(arquivo);
}

void gerarRelatorio(float troco_inicial, float ftLimpeza, float ftAlimentos, float ftPadaria, float vTotal){
    FILE *arquivo;
    time_t agora = time(NULL);
    struct tm *tempo = localtime(&agora);
    arquivo = fopen("relatorio_fechamento.txt", "w");
    if (arquivo == NULL) {
        perror("Erro ao criar o arquivo de relatório");
        return;
    }
       fprintf(arquivo, "=====Relatorio de Fechamento(%02d/%02d/%04d)=====\n\n",
            tempo->tm_mday,
            tempo->tm_mon + 1,
            tempo->tm_year + 1900
            );
       fprintf(arquivo, "- Fechamento realizado: %02d:%02d:%02d\n",
             tempo->tm_hour,
            tempo->tm_min,
            tempo->tm_sec
            );

       fprintf(arquivo, "\n=====SETORES=====\n");
       fprintf(arquivo, "- Limpeza: %.2f\n", ftLimpeza);
       fprintf(arquivo, "- Alimento: %.2f\n", ftAlimentos);
       fprintf(arquivo, "- Padaria: %.2f\n", ftPadaria);

       fprintf(arquivo, "\n=====CAIXA=====\n");
       fprintf(arquivo, "- Abertura(Troco Inicial): %.2f\n", troco_inicial);
       fprintf(arquivo, "- Fechamento(Total): %.2f\n",  vTotal);
   fclose(arquivo);
}


int opcao = 0, vAF = 0, vMenu = 0;
float troco_inicial = 0, trocoCaixa = 0, dinheiroCaixa = 0, ftLimpeza = 0, ftAlimentos = 0, ftPadaria = 0, vTotalDinheiro = 0;
int padaria_quantidade_atualizada = 0;


typedef struct NoProduto {
    int id;
    char nome[50];
    float preco;
    int qtd;
    struct NoProduto* proximo;
} NoProduto;

typedef struct  {
    int id;
    char nome[50];
    float preco;
    int qtd;
} ItemCarrinho;


NoProduto* produtosLimpeza = NULL;
NoProduto* produtosAlimento = NULL;
NoProduto* produtosPadaria = NULL;
NoProduto* produtosTotal = NULL;
NoProduto* produtosVendidos = NULL;


ItemCarrinho carrinho[100];


int contadorLimpeza = 0;
int contadorAlimento = 0;
int contadorCarrinho = 0;
int contadorPadaria = 0;
int contadorTotalProdutos = 0;
int contadorProdutosVendidos = 0;

void salvarProdutosLista(const char *nomeArquivo, NoProduto* cabeca) {
    FILE *arquivo = fopen(nomeArquivo, "w");
    if (arquivo == NULL) {
        perror("Erro ao salvar produtos");
        return;
    }

    NoProduto* atual = cabeca;
    while (atual != NULL) {

        fprintf(arquivo, "%d;%s;%.2f;%d\n",
                atual->id,
                atual->nome,
                atual->preco,
                atual->qtd);
        atual = atual->proximo;
    }
    fclose(arquivo);
}

int carregarProdutosLista(const char *nomeArquivo, NoProduto** cabeca) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL) {
        *cabeca = NULL;
        return 0;
    }

    NoProduto* atual = *cabeca;
    while (atual != NULL) {
        NoProduto* proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    *cabeca = NULL;

    int contagem = 0;
    NoProduto* fim_lista = NULL;

    while (1) {
        NoProduto* novo_no = (NoProduto*) malloc(sizeof(NoProduto));
        if (novo_no == NULL) {
            perror("Erro ao alocar memória para o novo produto ao carregar");
            fclose(arquivo);
            return contagem;
        }

        novo_no->id = 0;
        novo_no->nome[0] = '\0';
        novo_no->preco = 0.0f;
        novo_no->qtd = 0;
        novo_no->proximo = NULL;

        int itens_lidos = fscanf(arquivo, " %d;%49[^;];%f;%d",
                                 &novo_no->id,
                                 novo_no->nome,
                                 &novo_no->preco,
                                 &novo_no->qtd);

        if (itens_lidos != 4) {
            free(novo_no);
            if (feof(arquivo) && contagem > 0) {
                break;
            } else if (itens_lidos == EOF && contagem == 0) {
                 break;
            } else if (itens_lidos != EOF) {
                 perror("Erro de formato inesperado ao carregar produtos");
            }
            break;
        }

        char c;
        while ((c = fgetc(arquivo)) != '\n' && c != EOF);


        if (*cabeca == NULL) {
            *cabeca = novo_no;
            fim_lista = novo_no;
        } else {
            fim_lista->proximo = novo_no;
            fim_lista = novo_no;
        }
        contagem++;
    }

    fclose(arquivo);
    return contagem;
}

void adicionarProdutoAoFinal(NoProduto** cabeca, int ID, const char* nome, float preco, int qtd) {
    NoProduto* novo_no = (NoProduto*) malloc(sizeof(NoProduto));
    if (novo_no == NULL) {
        perror("Erro ao alocar memória para o novo produto");
        return;
    }
    novo_no->id = ID;
    strcpy(novo_no->nome, nome);
    novo_no->preco = preco;
    novo_no->qtd = qtd;
    novo_no->proximo = NULL;

    if (*cabeca == NULL) {
        *cabeca = novo_no;
    } else {
        NoProduto* atual = *cabeca;
        while (atual->proximo != NULL) {
            atual = atual->proximo;
        }
        atual->proximo = novo_no;
    }
}

void criarListaTotalDeProdutos() {
    NoProduto* atual_total = produtosTotal;
    NoProduto* proximo_total;
    while (atual_total != NULL) {
        proximo_total = atual_total->proximo;
        free(atual_total);
        atual_total = proximo_total;
    }
    produtosTotal = NULL;
    contadorTotalProdutos = 0;

    NoProduto* atual_cat = produtosLimpeza;
    while (atual_cat != NULL) {
        adicionarProdutoAoFinal(&produtosTotal, atual_cat->id, atual_cat->nome, atual_cat->preco, atual_cat->qtd);
        contadorTotalProdutos++;
        atual_cat = atual_cat->proximo;
    }

    atual_cat = produtosAlimento;
    while (atual_cat != NULL) {
        adicionarProdutoAoFinal(&produtosTotal, atual_cat->id, atual_cat->nome, atual_cat->preco, atual_cat->qtd);
        contadorTotalProdutos++;
        atual_cat = atual_cat->proximo;
    }

    atual_cat = produtosPadaria;
    while (atual_cat != NULL) {
        adicionarProdutoAoFinal(&produtosTotal, atual_cat->id, atual_cat->nome, atual_cat->preco, atual_cat->qtd);
        contadorTotalProdutos++;
        atual_cat = atual_cat->proximo;
    }
    escrever_log("Lista total de produtos criada/atualizada com %d itens.", contadorTotalProdutos);
}

void fOrdenarPorId(NoProduto** cabeca) {
    if (*cabeca == NULL || (*cabeca)->proximo == NULL) {
        return;
    }

    NoProduto *atual, *proximo;
    char temp_nome[50];
    float temp_preco;
    int temp_id, temp_qtd;

    for (atual = *cabeca; atual != NULL; atual = atual->proximo) {
        for (proximo = atual->proximo; proximo != NULL; proximo = proximo->proximo) {
            if (atual->id > proximo->id) {
                strcpy(temp_nome, atual->nome);
                strcpy(atual->nome, proximo->nome);
                strcpy(proximo->nome, temp_nome);

                temp_id = atual->id;
                atual->id = proximo->id;
                proximo->id = temp_id;

                temp_preco = atual->preco;
                atual->preco = proximo->preco;
                proximo->preco = temp_preco;

                temp_qtd = atual->qtd;
                atual->qtd = proximo->qtd;
                proximo->qtd = temp_qtd;
            }
        }
    }
}

void fOrdenarAlfabetica(NoProduto** cabeca) {
    if (*cabeca == NULL || (*cabeca)->proximo == NULL) {
        return;
    }

    NoProduto *atual, *proximo;
    char temp_nome[50];
    float temp_preco;
    int temp_id, temp_qtd;

    for (atual = *cabeca; atual != NULL; atual = atual->proximo) {
        for (proximo = atual->proximo; proximo != NULL; proximo = proximo->proximo) {
            if (strcmp(atual->nome, proximo->nome) > 0) {
                strcpy(temp_nome, atual->nome);
                strcpy(atual->nome, proximo->nome);
                strcpy(proximo->nome, temp_nome);

                temp_id = atual->id;
                atual->id = proximo->id;
                proximo->id = temp_id;

                temp_preco = atual->preco;
                atual->preco = proximo->preco;
                proximo->preco = temp_preco;

                temp_qtd = atual->qtd;
                atual->qtd = proximo->qtd;
                proximo->qtd = temp_qtd;
            }
        }
    }
}

void fExibirLimpeza() {
    NoProduto* atual = produtosLimpeza;

    while (atual != NULL) {
        printf("║ %-4d ║ %-15s ║ R$%-7.2f ║ %-7d ║\n", atual->id, atual->nome, atual->preco, atual->qtd);
        atual = atual->proximo;
    }
}

void fExibirAlimento() {
    NoProduto* atual = produtosAlimento;

    while (atual != NULL) {
        printf("║ %-4d ║ %-15s ║ R$%-7.2f ║ %-7d ║\n", atual->id, atual->nome, atual->preco, atual->qtd);
        atual = atual->proximo;
    }
}

void fExibirPadaria() {
    NoProduto* atual = produtosPadaria;

    while (atual != NULL) {
        printf("║ %-4d ║ %-15s ║ R$%-7.2f ║ %-7d ║\n", atual->id, atual->nome, atual->preco, atual->qtd);
        atual = atual->proximo;
    }
}
void fExibirCarrinho() {
    for (int i = 0; i < contadorCarrinho; i++) {
        printf("║ %-15s ║ R$%-13.2f ║ %-13d ║ ║                ║\n", carrinho[i].nome, carrinho[i].preco, carrinho[i].qtd);
    }
}

void fExibirProdutos(NoProduto* cabeca) {
    NoProduto* atual = cabeca;

    while(atual != NULL){
        printf("║ %-4d ║ %-15s ║ R$%-7.2f ║ %-7d ║\n", atual->id, atual->nome, atual->preco, atual->qtd);
        atual = atual->proximo;
    }
}

void fExibirProdutosOrdenado(NoProduto** cabeca, char* tipoOrdenacao) {
    NoProduto* temp_cabeca = NULL;
    NoProduto* atual_original = *cabeca;
    while (atual_original != NULL) {
        adicionarProdutoAoFinal(&temp_cabeca, atual_original->id, atual_original->nome, atual_original->preco, atual_original->qtd);
        atual_original = atual_original->proximo;
    }

    if (strcmp(tipoOrdenacao, "alfabetica") == 0) {
        fOrdenarAlfabetica(&temp_cabeca);
        escrever_log("Relatório de produtos ordenado alfabeticamente.");
    } else if (strcmp(tipoOrdenacao, "id") == 0) {
        fOrdenarPorId(&temp_cabeca);
        escrever_log("Relatório de produtos ordenado por ID.");
    }

    system("cls");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║               LISTAGEM DE PRODUTOS           ║\n");
    printf("╠═════╦═════════════════╦═════════╦════════════╣\n");
    printf("║ ID  ║ Nome            ║ Preço   ║ Quantidade ║\n");
    printf("╠═════╩═════════════════╩═════════╩════════════╣\n");
    fExibirProdutos(temp_cabeca);
    printf("╚══════════════════════════════════════════════╝\n");

    NoProduto* atual = temp_cabeca;
    while (atual != NULL) {
        NoProduto* proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }

    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║- Pressione ENTER para Voltar                 ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    getchar();
    getchar();
    fRelatorioEstoque();
}

void fExibirProdutosEstoqueZeroOuMinimo() {
    NoProduto* temp_lista_zero = NULL;
    NoProduto* atual = produtosTotal;
    int produtos_encontrados = 0;

    while (atual != NULL) {
        if (atual->qtd <= 5) {
            adicionarProdutoAoFinal(&temp_lista_zero, atual->id, atual->nome, atual->preco, atual->qtd);
            produtos_encontrados++;
        }
        atual = atual->proximo;
    }

    fOrdenarAlfabetica(&temp_lista_zero);

    system("cls");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║       PRODUTOS COM ESTOQUE ZERO OU MÍNIMO    ║\n");
    printf("╠═════╦═════════════════╦═════════╦════════════╣\n");
    printf("║ ID  ║ Nome            ║ Preço   ║ Quantidade ║\n");
    printf("╠═════╩═════════════════╩═════════╩════════════╣\n");
    if (produtos_encontrados == 0) {
        printf("║           Nenhum produto com estoque zero ou mínimo.             ║\n");
    } else {
        fExibirProdutos(temp_lista_zero);
    }
    printf("╚══════════════════════════════════════════════╝\n");

    NoProduto* atual_temp = temp_lista_zero;
    while (atual_temp != NULL) {
        NoProduto* proximo = atual_temp->proximo;
        free(atual_temp);
        atual_temp = proximo;
    }
    escrever_log("Relatório de produtos com estoque zero ou mínimo gerado.");

    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║- Pressione ENTER para Voltar.                ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    getchar();
    getchar();
    fRelatorioEstoque();
}

float fSubtotal() {
    float subtotal = 0;
    for (int i = 0; i < contadorCarrinho; i++) {
        subtotal += (carrinho[i].preco * carrinho[i].qtd);
    }
    return subtotal;
}

void fLimpar() {
    contadorCarrinho = 0;
    fPagamento();
}

void fAtualizarQuantidades() {
    printf("ATUALIZAÇÃO DE ESTOQUE - DIGITE AS NOVAS QUANTIDADES\n\n");

    NoProduto* atual = produtosPadaria;
    if (atual == NULL) {
        printf("Nenhum produto na padaria para atualizar.\n");
        Sleep(1500);
        return;
    }

    int numero_item = 1;
    while (atual != NULL) {
        printf("[%d] Produto: %s\n", numero_item, atual->nome);
        printf("Digite a nova quantidade: ");
        scanf("%d", &atual->qtd);
        getchar();
        printf("---------------------------------\n");
        atual = atual->proximo;
        numero_item++;
    }

    salvarProdutosLista("padaria.txt", produtosPadaria);
    printf("✅ Quantidades atualizadas com sucesso!\n");
}

void fAtualizarSistema() {
    NoProduto* atual_vendido = produtosVendidos;
    NoProduto* proximo_vendido;
    while (atual_vendido != NULL) {
        proximo_vendido = atual_vendido->proximo;
        free(atual_vendido);
        atual_vendido = proximo_vendido;
    }
    produtosVendidos = NULL;
    contadorProdutosVendidos = 0;

    for (int i = 0; i < contadorCarrinho; i++) {
        int id = carrinho[i].id;
        int qtd_comprado = carrinho[i].qtd;

        adicionarProdutoAoFinal(&produtosVendidos, carrinho[i].id, carrinho[i].nome, carrinho[i].preco, carrinho[i].qtd);
        contadorProdutosVendidos++;

        NoProduto* atualAlimento = produtosAlimento;
        while (atualAlimento != NULL) {
            if (atualAlimento->id == id) {
                atualAlimento->qtd -= qtd_comprado;
                dinheiroCaixa += (atualAlimento->preco * qtd_comprado);
                ftAlimentos += (atualAlimento->preco * qtd_comprado);
                break;
            }
            atualAlimento = atualAlimento->proximo;
        }

        NoProduto* atualLimpeza = produtosLimpeza;
        while (atualLimpeza != NULL) {
            if (atualLimpeza->id == id) {
                atualLimpeza->qtd -= qtd_comprado; // Adicionado: subtrair a quantidade também para limpeza
                dinheiroCaixa += (atualLimpeza->preco * qtd_comprado);
                ftLimpeza += (atualLimpeza->preco * qtd_comprado);
                break;
            }
            atualLimpeza = atualLimpeza->proximo;
        }

        NoProduto* atualPadaria = produtosPadaria;
        while (atualPadaria != NULL) {
            if (atualPadaria->id == id) {
                atualPadaria->qtd -= qtd_comprado;
                dinheiroCaixa += (atualPadaria->preco * qtd_comprado);
                ftPadaria += (atualPadaria->preco * qtd_comprado);
                break;
            }
            atualPadaria = atualPadaria->proximo;
        }
    }
    contadorCarrinho = 0;
    salvarProdutosLista("limpeza.txt", produtosLimpeza);
    salvarProdutosLista("alimentos.txt", produtosAlimento);
    salvarProdutosLista("padaria.txt", produtosPadaria);
}

void fMenu() {
    system("cls");
    SetConsoleOutputCP(CP_UTF8);
    if(vAF == 0){
        escrever_log("TELA MENU(ESTADO:FECHADO)");
    }else{
        escrever_log("TELA MENU(ESTADO:ABERTO)");
    }
    contadorLimpeza = carregarProdutosLista("limpeza.txt", &produtosLimpeza);
    contadorAlimento = carregarProdutosLista("alimentos.txt", &produtosAlimento);
    contadorPadaria = carregarProdutosLista("padaria.txt", &produtosPadaria);
    criarListaTotalDeProdutos();
    printf("╔══════════════════════════════════╗\n");
    fEstadoDoCaixa();
    printf("╠══════════════════════════════════╣\n");
    printf("║            MERCEARIA             ║\n");
    printf("╠══════════════════════════════════╣\n");
    printf("║                                  ║\n");
    printf("║ [1] 🧼 Limpeza                   ║\n");
    printf("║ [2] 🥕 Alimentacao               ║\n");
    printf("║ [3] 🍞 Padaria                   ║\n");
    printf("║ [4] 🪙 Pagamento                 ║\n");
    printf("║ [5] 🔑 Caixa                     ║\n");
    printf("║ [0] ❌ Sair                      ║\n");
    printf("║                                  ║\n");
    printf("╚══════════════════════════════════╝\n");
    printf("- Opção: ");
    scanf("%d", &opcao);
    getchar();
    switch (opcao) {
        case 1:
            escrever_log("TELA LIMPEZA");
            fLimpeza();
            break;
        case 2:
            escrever_log("TELA ALIMENTOS");
            fAlimentos();
            break;
        case 3:
            escrever_log("TELA PADARIA");
            fPadaria();
            break;
        case 4:
            escrever_log("TELA PAGAMENTO");
            fPagamento();
            break;
        case 5:
            escrever_log("TELA CAIXA");
            fCaixa();
            break;
        case 0:
        if(vAF == 0){
            escrever_log("TELA SAIR");
            fSair();
            break;
        }else{
            escrever_log("SAIDA VETADA(MOTIVO:CAIXA ABERTO)");
            printf("\n╔════════════════════════════╗");
            printf("\n║ Saida vetada(Caixa Aberto) ║");
            printf("\n╚════════════════════════════╝\n");
            Sleep(1500);
            fMenu();
            break;
        }
        default:
            fMenu();
            break;
    }
}

void fLimpeza() {
    vMenu = 1;
    system("cls");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║               SETOR LIMPEZA                  ║\n");
    printf("╠═════╦═════════════════╦═════════╦════════════╣\n");
    printf("║ ID  ║ Nome            ║ Preço   ║ Quantidade ║\n");
    printf("╠═════╩═════════════════╩═════════╩════════════╣\n");
    fExibirLimpeza();
    printf("╚══════════════════════════════════════════════╝\n");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║- Digite [1] 🏠 Menu:                         ║\n");
    printf("║- Digite [2] 🛒 Carrinho:                     ║\n");
    printf("║- Digite [3] ➕ Novo Produto:                 ║\n");
    printf("║- Digite [4] 🪙 Pagamento:                    ║\n");
    printf("║- Digite [5] 🔢 Ordenanacao ID                ║\n");
    printf("║- Digite [6] 🔠 Ordenanacao Alfabetica        ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");
    printf(" - Opção: ");
    scanf("%d", &opcao);
    getchar();
    switch (opcao) {
        case 1:
            escrever_log("RETORNOU AO MENU");
            fMenu();
            break;
        case 2:
            escrever_log("CARRINHO CHAMADO(LIMPEZA)");
            fVerificacaoDeEstado();
            escrever_log("PRODUTO NO CARRINHO");
            adicionarCarrinhoLimpeza();
            break;
        case 3:
            escrever_log("NOVO PRODUTO ADICIONADO");
            fAdicionarLimpeza();
            break;
        case 4:
            escrever_log("CHAMANDO PAGAMENTO(LIMPEZA)");
            fVerificacaoDeEstado();
            escrever_log("MENU PAGAMENTO)");
            fPagamento();
            break;
        case 5:
            fOrdenarPorId(&produtosLimpeza);
            system("cls");
            fLimpeza();
            break;
        case 6:
            fOrdenarAlfabetica(&produtosLimpeza);
            system("cls");
            fLimpeza();
            break;
        default: fLimpeza();
            break;
    }
}

void fAdicionarLimpeza() {
    SetConsoleOutputCP(CP_UTF8);
    int novo_id;
    char novo_nome[50];
    float novo_preco;
    int nova_qtd;

    if (produtosLimpeza == NULL) {
        novo_id = 100;
    } else {
        NoProduto* atual = produtosLimpeza;
        int id_maximo = 0;
        while (atual != NULL) {
            if (atual->id > id_maximo) {
                id_maximo = atual->id;
            }
            atual = atual->proximo;
        }
        novo_id = id_maximo + 1;
    }

    printf("Digite o Nome do Produto: ");
    fgets(novo_nome, sizeof(novo_nome), stdin);
    novo_nome[strcspn(novo_nome, "\n")] = 0;

    printf("Digite o PRECO do Produto: ");
    scanf("%f", &novo_preco);
    getchar();

    printf("Digite a QUANTIDADE do Produto:");
    scanf("%d", &nova_qtd);
    getchar();

    adicionarProdutoAoFinal(&produtosLimpeza, novo_id, novo_nome, novo_preco, nova_qtd);
    escrever_log("| %d | %s | %.2f | %d |", novo_id, novo_nome, novo_preco, nova_qtd);
    contadorLimpeza++;
    salvarProdutosLista("limpeza.txt", produtosLimpeza);
    system("cls");
    fLimpeza();
}

void adicionarCarrinhoLimpeza(){
    int id, qtd;

    printf("- Digite o ID do Produto: ");
    scanf("%d", &id);
    getchar();
    printf("- Digite a Quantidade do Produto: ");
    scanf("%d", &qtd);
    getchar();

    NoProduto* atual = produtosLimpeza;
    while (atual != NULL) {
        if (atual->id == id) {
            if (atual->qtd >= qtd) {
                carrinho[contadorCarrinho].id = atual->id;
                strcpy(carrinho[contadorCarrinho].nome, atual->nome);
                carrinho[contadorCarrinho].preco = atual->preco;
                carrinho[contadorCarrinho].qtd = qtd;
                escrever_log("| %d | %s | %.2f | %d |", carrinho[contadorCarrinho].id, carrinho[contadorCarrinho].nome, carrinho[contadorCarrinho].preco, carrinho[contadorCarrinho].qtd);
                contadorCarrinho++;
                system("cls");
                fLimpeza();
                return;
            } else {
                escrever_log("Quantidade Indisponivel do %d", id);
                printf("Quantidade Indisponivel! Estoque Atual: %d", atual->qtd);
                getchar();
                getchar();
                fLimpeza();
                return;
            }
        }
        atual = atual->proximo;
    }
    escrever_log("ID %d não Encontrado", id);
    printf("ID do produto não encontrado! Tente novamente.");
    Sleep(2000);
    fLimpeza();
}

void fAlimentos() {
    vMenu = 2;
    system("cls");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║               SETOR ALIMENTOS                ║\n");
    printf("╠═════╦═════════════════╦═════════╦════════════╣\n");
    printf("║ ID  ║ Nome            ║ Preço   ║ Quantidade ║\n");
    printf("╠═════╩═════════════════╩═════════╩════════════╣\n");
    fExibirAlimento();
    printf("╚══════════════════════════════════════════════╝\n");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║- Digite [1] 🏠 Menu:                         ║\n");
    printf("║- Digite [2] 🛒 Carrinho:                     ║\n");
    printf("║- Digite [3] ➕ Novo Produto:                 ║\n");
    printf("║- Digite [4] 🪙 Pagamento:                    ║\n");
    printf("║- Digite [5] 🔢 Ordenanacao ID                ║\n");
    printf("║- Digite [6] 🔠 Ordenanacao Alfabetica        ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");
    printf(" - Opção: ");
    scanf("%d", &opcao);
    getchar();
    switch (opcao) {
        case 1:
            escrever_log("RETORNOU AO MENU");
            fMenu();
            break;
        case 2:
            escrever_log("CARRINHO CHAMADO(ALIMENTOS)");
            fVerificacaoDeEstado();
            escrever_log("PRODUTO NO CARRINHO");
            adicionarCarrinhoAlimentos();
            break;
        case 3:
            escrever_log("NOVO PRODUTO ADICIONADO(ALIMENTOS)");
            fAdicionarAlimento();
            break;
        case 4:
            escrever_log("CHAMANDO PAGAMENTO(ALIMENTOS)");
            fVerificacaoDeEstado();
            escrever_log("MENU PAGAMENTO");
            fPagamento();
            break;
        case 5:
            fOrdenarPorId(&produtosAlimento);
            system("cls");
            fAlimentos();
            break;
        case 6:
            fOrdenarAlfabetica(&produtosAlimento);
            system("cls");
            fAlimentos();
        default: fAlimentos();
            break;
    }
}

void fAdicionarAlimento() {
    SetConsoleOutputCP(CP_UTF8);
    int novo_id;
    char novo_nome[50];
    float novo_preco;
    int nova_qtd;

    if (produtosAlimento == NULL) {
        novo_id = 200;
    } else {
        NoProduto* atual = produtosAlimento;
        int id_maximo = 0;
        while (atual != NULL) {
            if (atual->id > id_maximo) {
                id_maximo = atual->id;
            }
            atual = atual->proximo;
        }
        novo_id = id_maximo + 1;
    }

    printf("Digite o NOME do Produto: ");
    fgets(novo_nome, sizeof(novo_nome), stdin);
    novo_nome[strcspn(novo_nome, "\n")] = 0;

    printf("Digite o PRECO do Produto: ");
    scanf("%f", &novo_preco);
    getchar();

    printf("Digite a QUANTIDADE do Produto:");
    scanf("%d", &nova_qtd);
    getchar();

    adicionarProdutoAoFinal(&produtosAlimento, novo_id, novo_nome, novo_preco, nova_qtd);
    escrever_log("| %d | %s | %.2f | %d |", novo_id, novo_nome, novo_preco, nova_qtd);
    contadorAlimento++;
    salvarProdutosLista("alimentos.txt", produtosAlimento);
    system("cls");
    fAlimentos();
}
void adicionarCarrinhoAlimentos(){
    int id, qtd;

    printf("- Digite o ID do Produto: ");
    scanf("%d", &id);
    getchar();
    printf("- Digite a Quantidade do Produto: ");
    scanf("%d", &qtd);
    getchar();

    NoProduto* atual = produtosAlimento;
    while (atual != NULL) {
        if (atual->id == id) {
            if (atual->qtd >= qtd) {
                carrinho[contadorCarrinho].id = atual->id;
                strcpy(carrinho[contadorCarrinho].nome, atual->nome);
                carrinho[contadorCarrinho].preco = atual->preco;
                carrinho[contadorCarrinho].qtd = qtd;
                escrever_log("| %d | %s | %.2f | %d |", carrinho[contadorCarrinho].id, carrinho[contadorCarrinho].nome, carrinho[contadorCarrinho].preco, carrinho[contadorCarrinho].qtd);
                contadorCarrinho++;
                system("cls");
                fAlimentos();
                return;
            } else {
                escrever_log("Quantidade Indisponivel! Estoque Atual: %d", atual->qtd);
                getchar();
                getchar();
                fAlimentos();
                return;
            }
        }
        atual = atual->proximo;
    }
    escrever_log("ID %d não Encontrado", id);
    printf("ID do produto não encontrado! Tente novamente.");
    Sleep(2000);
    fAlimentos();
}

void fPadaria() {
    if(vAF == 1){
        if (padaria_quantidade_atualizada == 0){
            fAtualizarQuantidades();
        }
        padaria_quantidade_atualizada = 1;
        system("cls");
        printf("╔══════════════════════════════════════════════╗\n");
        printf("║               SETOR PADARIA                  ║\n");
        printf("╠═════╦═════════════════╦═════════╦════════════╣\n");
        printf("║ ID  ║ Nome            ║ Preço   ║ Quantidade ║\n");
        printf("╠═════╩═════════════════╩═════════╩════════════╣\n");
        fExibirPadaria();
        printf("╚══════════════════════════════════════════════╝\n");
        printf("╔══════════════════════════════════════════════╗\n");
        printf("║- Digite [1] 🏠 Menu:                         ║\n");
        printf("║- Digite [2] 🛒 Carrinho:                     ║\n");
        printf("║- Digite [3] ➕ Novo Produto:                 ║\n");
        printf("║- Digite [5] 🔢 Ordenanacao ID                ║\n");
        printf("║- Digite [6] 🔠 Ordenanacao Alfabetica        ║\n");
        printf("╚══════════════════════════════════════════════╝\n\n");
        printf(" - Opção: ");
        scanf("%d", &opcao);
        getchar();
        switch (opcao) {
            case 1:
                escrever_log("RETORNOU AO MENU");
                fMenu();
                break;
            case 2:
                escrever_log("CARRINHO CHAMADO(PADARIA)");
                fVerificacaoDeEstado();
                escrever_log("PRODUTO NO CARRINHO");
                adicionarCarrinhoPadaria();
                break;
            case 3:
                if(vAF == 1) {
                    escrever_log("NOVO PRODUTO ADICIONADO(PADARIA)");
                    fAdicionarPadaria();
                } else {
                    fPadaria();
                }
                break;
            case 5:
                fOrdenarPorId(&produtosPadaria);
                system("cls");
                fPadaria();
                break;
            case 6:
                fOrdenarAlfabetica(&produtosPadaria);
                system("cls");
                fPadaria();
                break;
            default: fPadaria();
                break;
        }
    }else{
        vMenu = 0;
        fVerificacaoDeEstado();
    }
}

void fAdicionarPadaria() {
    SetConsoleOutputCP(CP_UTF8);
    int novo_id;
    char novo_nome[50];
    float novo_preco;
    int nova_qtd;

    if (produtosPadaria == NULL) {
        novo_id = 300;
    } else {
        NoProduto* atual = produtosPadaria;
        int id_maximo = 0;
        while (atual != NULL) {
            if (atual->id > id_maximo) {
                id_maximo = atual->id;
            }
            atual = atual->proximo;
        }
        novo_id = id_maximo + 1;
    }

    printf("Digite o Nome do Produto: ");
    fgets(novo_nome, sizeof(novo_nome), stdin);
    novo_nome[strcspn(novo_nome, "\n")] = 0;

    printf("Digite o PRECO do Produto: ");
    scanf("%f", &novo_preco);
    getchar();

    printf("Digite a QUANTIDADE do Produto:");
    scanf("%d", &nova_qtd);
    getchar();

    adicionarProdutoAoFinal(&produtosPadaria, novo_id, novo_nome, novo_preco, nova_qtd);
    escrever_log("| %d | %s | %.2f | %d |", novo_id, novo_nome, novo_preco, nova_qtd);
    contadorPadaria++;
    salvarProdutosLista("padaria.txt", produtosPadaria);
    system("cls");
    fPadaria();
}

void adicionarCarrinhoPadaria(){
    int id, qtd;

    printf("- Digite o ID do Produto: ");
    scanf("%d", &id);
    getchar();
    printf("- Digite a Quantidade do Produto: ");
    scanf("%d", &qtd);
    getchar();

    NoProduto* atual = produtosPadaria;
    while (atual != NULL) {
        if (atual->id == id) {
            if (atual->qtd >= qtd) {
                carrinho[contadorCarrinho].id = atual->id;
                strcpy(carrinho[contadorCarrinho].nome, atual->nome);
                carrinho[contadorCarrinho].preco = atual->preco;
                carrinho[contadorCarrinho].qtd = qtd;
                contadorCarrinho++;
                escrever_log("| %d | %s | %.2f | %d |", carrinho[contadorCarrinho-1].id, carrinho[contadorCarrinho-1].nome, carrinho[contadorCarrinho-1].preco, carrinho[contadorCarrinho-1].qtd);
                system("cls");
                fPadaria();
                return;
            } else {
                escrever_log("Quantidade Indisponivel! Estoque Atual: %d\n", atual->qtd);
                getchar();
                getchar();
                fPadaria();
                return;
            }
        }
        atual = atual->proximo;
    }
    escrever_log("ID %d não Encontrado", id);
    printf("ID do produto não encontrado! Tente novamente.");
    Sleep(2000);
    fPadaria();
}


void fPagamento() {
    if(vAF == 1){
        SetConsoleOutputCP(CP_UTF8);
        system("cls");
        float subtotal;
        subtotal = fSubtotal();
        printf("╔═══════════════════════════════════════════════════╗ ╔════════════════╗\n");
        printf("║                     PAGAMENTO                     ║ ║    CODIGOS     ║\n");
        printf("╠═════════════════╦═════════════════╦═══════════════╣ ╠════╦═══════════╣\n");
        printf("║ PRODUTO         ║ PRECO           ║ QUANTIDADE    ║ ║ ID ║ PRODUTO   ║\n");
        printf("╠═════════════════╩═════════════════╩═══════════════╣ ╠════╩═══════════╣\n");
        fExibirCarrinho();
        printf("║                                                   ║ ║ 101 Sabao      ║\n");
        printf("╠═══════════════════════════════════════════════════╣ ║ 102 Detergente ║\n");
        printf("║SUBTOTAL: R$%-6.2f                                 ║ ║ 103 Shampoo    ║\n", subtotal);
        printf("║                                                   ║ ║ 102 Detergente ║\n");
        printf("╚═══════════════════════════════════════════════════╝ ╚════════════════╝\n");
        printf("╔═══════════════════════════════════════════════════╗\n");
        printf("║- Digite [1] 🏠 Menu:                              ║\n");
        printf("║- Digite [2] 🛒 Adicionar Produto:                 ║\n");
        printf("║- Digite [3] 💵 Dinheiro(Com desconto)             ║\n");
        printf("║- Digite [4] 💳 Cartão                             ║\n");
        printf("║- Digite [5] 🗑  Limpar                             ║\n");
        printf("╚═══════════════════════════════════════════════════╝\n\n");
        printf("- Opção: ");
        scanf("%d", &opcao);
        getchar();
        switch (opcao) {
            case 1:
                escrever_log("RETORNOU AO MENU");
                fMenu();
                break;
            case 2:
                escrever_log("ADICIONAR PRODUTO AO CARRINHO");
                fAdicionarProduto();
                break;
            case 3:
                escrever_log("DINHEIRO SELECIONADO");
                fPagamentoDinheiro();
                break;
            case 4:
                escrever_log("CARTÃO SELECIONADO");
                fPagamentoCartao();
                break;
            case 5:
                escrever_log("LIMPOU CARRINHO");
                fLimpar();
                break;
            default: fPagamento();
            break;
        }
    }else{
        vMenu = 0;
        fVerificacaoDeEstado();
    }
}

void fAdicionarProduto() {
    int id, qtd;
    printf("- Digite o ID do Produto: ");
    scanf("%d", &id);
    getchar();
    printf("- Digite a Quantidade do Produto: ");
    scanf("%d", &qtd);
    getchar();

    NoProduto* atualAlimento = produtosAlimento;
    while (atualAlimento != NULL) {
        if (atualAlimento->id == id) {
            if (atualAlimento->qtd >= qtd) {
                carrinho[contadorCarrinho].id = atualAlimento->id;
                strcpy(carrinho[contadorCarrinho].nome, atualAlimento->nome);
                carrinho[contadorCarrinho].preco = atualAlimento->preco;
                carrinho[contadorCarrinho].qtd = qtd;
                escrever_log("| %d | %s | %.2f | %d |", carrinho[contadorCarrinho].id, carrinho[contadorCarrinho].nome, carrinho[contadorCarrinho].preco, carrinho[contadorCarrinho].qtd);
                contadorCarrinho++;
                system("cls");
                fPagamento();
                return;
            } else {
                escrever_log("Quantidade Indisponivel do %d", id);
                printf("Quantidade Indisponivel! Estoque Atual: %d", atualAlimento->qtd);
                getchar();
                getchar();
                fPagamento();
                return;
            }
        }
        atualAlimento = atualAlimento->proximo;
    }

    NoProduto* atualLimpeza = produtosLimpeza;
    while (atualLimpeza != NULL) {
        if (atualLimpeza->id == id) {
            if (atualLimpeza->qtd >= qtd) {
                carrinho[contadorCarrinho].id = atualLimpeza->id;
                strcpy(carrinho[contadorCarrinho].nome, atualLimpeza->nome);
                carrinho[contadorCarrinho].preco = atualLimpeza->preco;
                carrinho[contadorCarrinho].qtd = qtd;
                escrever_log("| %d | %s | %.2f | %d |", carrinho[contadorCarrinho].id, carrinho[contadorCarrinho].nome, carrinho[contadorCarrinho].preco, carrinho[contadorCarrinho].qtd);
                contadorCarrinho++;
                system("cls");
                fPagamento();
                return;
            } else {
                escrever_log("Quantidade Indisponivel do %d", id);
                printf("Quantidade Indisponivel! Estoque Atual: %d", atualLimpeza->qtd);
                getchar();
                getchar();
                fPagamento();
                return;
            }
        }
        atualLimpeza = atualLimpeza->proximo;
    }

    NoProduto* atualPadaria = produtosPadaria;
    while (atualPadaria != NULL) {
        if (atualPadaria->id == id) {
            if (atualPadaria->qtd >= qtd) {
                carrinho[contadorCarrinho].id = atualPadaria->id;
                strcpy(carrinho[contadorCarrinho].nome, atualPadaria->nome);
                carrinho[contadorCarrinho].preco = atualPadaria->preco;
                carrinho[contadorCarrinho].qtd = qtd;
                contadorCarrinho++;
                escrever_log("| %d | %s | %.2f | %d |", carrinho[contadorCarrinho-1].id, carrinho[contadorCarrinho-1].nome, carrinho[contadorCarrinho-1].preco, carrinho[contadorCarrinho-1].qtd);
                system("cls");
                fPagamento();
                return;
            } else {
                escrever_log("Quantidade Indisponivel do %d", id);
                printf("Quantidade Indisponivel! Estoque Atual: %d\n", atualPadaria->qtd);
                getchar();
                getchar();
                fPagamento();
                return;
            }
        }
        atualPadaria = atualPadaria->proximo;
    }
    escrever_log("ID %d não Encontrado", id);
    printf("ID do produto não encontrado! Tente novamente.");
    Sleep(2000);
    fPagamento();
}

float fDesconto() {
    float desconto = 0, subtotal = 0;
    subtotal = fSubtotal();
    if (subtotal <= 50) {
        desconto = 5;
    } else if (subtotal < 200) {
        desconto = 10;
    } else {
        printf("Valor superior a R$200, digite a porcentagem de desconto a ser aplicada: ");
        scanf("%f", &desconto);
        getchar();
        system("cls");
        escrever_log("DESCONTO ADICIONADO");
    }
    return desconto;
}

void fPagamentoDinheiro() {
    system("cls");
    float dinheiro_recebido = 0, troco = 0;
    float subtotal_calculado = fSubtotal();
    float desconto = fDesconto();
    subtotal_calculado -= (subtotal_calculado * desconto / 100);
    printf("╔═══════════════════════════════════════════════════╗\n");
    printf("║               PAGAMENTO EM DINHEIRO               ║\n");
    printf("╠═══════════════════════════════════════════════════╣\n");
    printf("║ Desconto aplicado: %%%-7.1f                       ║\n", desconto);
    printf("║ Total com desconto: R$%-6.2f                      ║\n", subtotal_calculado);
    printf("╚═══════════════════════════════════════════════════╝\n");
    do {
        printf("- Digite o Dinheiro Recebido: ");
        scanf("%f", &dinheiro_recebido);
        getchar();
        if (dinheiro_recebido < subtotal_calculado) {
            printf("╔═══════════════════════════════════════════════════╗\n");
            printf("║ Dinheiro insuficiente! Faltam R$%-6.2f            ║\n", subtotal_calculado - dinheiro_recebido);
            printf("╚═══════════════════════════════════════════════════╝\n");
        }
    } while (dinheiro_recebido < subtotal_calculado);
    troco = dinheiro_recebido - subtotal_calculado;
    printf("╔═══════════════════════════════════════════════════╗\n");
    printf("║ Troco: R$%-6.2f                                   ║\n", troco);
    printf("║ Pagamento realizado com sucesso!                  ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n");
    escrever_log("Subtotal: %.2f", subtotal_calculado);
    escrever_log("Dinheiro Recebido %.2f", dinheiro_recebido);
    escrever_log("Troco: %.2f", troco);
    fAtualizarSistema();
    escrever_log("Sistema Atualizado");
    escrever_log("Voltou ao Menu");
    vTotalDinheiro += subtotal_calculado;
    printf("╔═══════════════════════════════════════════════════╗\n");
    printf("║ Pressione ENTER para Voltar ao Menu:              ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n");
    getchar();
    fMenu();
}

void fPagamentoCartao() {
    system("cls");
    opcao = 0;
    do {
        printf("Passe, insira ou aproxime o cartão\n");
        getchar();
        printf("Pressione ENTER para simular a transação...\n");
        getchar();

        printf("Transação aceita(1)\n");
        printf("Transação recusada(0)\n");
        printf("Selecione: ");
        scanf("%d", &opcao);
        getchar();
        if (opcao == 0) {
            printf("Digite (1) para tentar novamente\n");
            printf("Digite (2) para retornar as opções de pagamento\n");
            printf("Selecione: ");
            scanf("%d", &opcao);
            getchar();
            switch (opcao) {
                case 1: fPagamentoCartao();
                    break;
                case 2: fPagamento();
                    break;
                default: printf("Opcao invalida!\n");
            }
        }
    } while (opcao != 1);
    printf("Pagamento concluido.\n");
    fAtualizarSistema();
    printf("Pressione ENTER para Voltar ao Menu:\n ");
    getchar();
    system("cls");
    fMenu();
}

void fCaixa() {
    vMenu = 5;
    system("cls");
    SetConsoleOutputCP(CP_UTF8);
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║              DINHEIRO DISPONIVEL             ║\n");
    printf("╠══════════════════════════════════════════════╣\n");
    printf("║ Troco: R$%-6.2f                              ║\n", vTotalDinheiro + trocoCaixa);
    printf("╚══════════════════════════════════════════════╝\n");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║- Digite [1] 🏠 Menu:                         ║\n");
    printf("║- Digite [2] 💰 Adicionar Troco:              ║\n");
    if(vAF == 0){
        printf("║- Digite [3] 🔑 Abrir Caixa:                  ║\n");
        } else {
            printf("║- Digite [3] 🔒 Fechar Caixa:                 ║\n");
        }
    printf("║- Digite [4] 📊 Relatórios                    ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");
    printf("- Opção: ");
    scanf("%d", &opcao);
    getchar();
    switch (opcao) {
        case 1:
            escrever_log("RETORNOU AO MENU");
            fMenu();
            break;
        case 2:
            fVerificacaoDeEstado();
            escrever_log("ADICIONAR TROCO CHAMADO");
            fAdicionarTrocoCaixa();
            break;
        case 3:
            if(vAF == 0){
                fAbrirCaixa();
            }else{
                fFecharCaixa();
            }
            break;
        case 4:
            escrever_log("MENU RELATORIOS");
            fRelatorios();
            break;
        default: fCaixa();
            break;
    }
}

void fRelatorios() {
    system("cls");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║                 RELATÓRIOS                   ║\n");
    printf("╠══════════════════════════════════════════════╣\n");
    printf("║- Digite [1] ↩️ Voltar:                       ║\n");
    printf("║- Digite [2] 📦 Relatórios de Estoque         ║\n");
    printf("║- Digite [3] 📈 Relatórios de Vendas          ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");
    printf("- Opção: ");
    scanf("%d", &opcao);
    getchar();
    switch (opcao) {
        case 1:
            fCaixa();
            break;
        case 2:
            fRelatorioEstoque();
            break;
        case 3:
            fRelatorioProdutosVendidos();
            break;
        default:
            fRelatorios();
            break;
    }
}

void fRelatorioEstoque(){
    system("cls");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║             RELATÓRIOS DE ESTOQUE            ║\n");
    printf("╠══════════════════════════════════════════════╣\n");
    printf("║- Digite [1] ↩️ Voltar:                       ║\n");
    printf("║- Digite [2] 🔠 Produtos em Ordem Alfabética  ║\n");
    printf("║- Digite [3] 🔢 Produtos em Ordem de Código   ║\n");
    printf("║- Digite [4] ⚠️ Produtos com Estoque Mínimo   ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");
    printf("- Opção: ");
    scanf("%d", &opcao);
    getchar();
    switch (opcao) {
        case 1:
            fRelatorios();
            break;
        case 2:
            fExibirProdutosOrdenado(&produtosTotal, "alfabetica");
            break;
        case 3:
            fExibirProdutosOrdenado(&produtosTotal, "id");
            break;
        case 4:
            fExibirProdutosEstoqueZeroOuMinimo();
            break;
        default:
            fRelatorioEstoque();
            break;
    }
}

void fRelatorioProdutosVendidos(){
    system("cls");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║            RELATÓRIOS DE VENDAS              ║\n");
    printf("╠══════════════════════════════════════════════╣\n");
    printf("║- Digite [1] ↩️ Voltar:                       ║\n");
    printf("║- Digite [2] 🔠 Produtos Vendidos (Alfabética)║\n");
    printf("║- Digite [3] 🔢 Produtos Vendidos (Por Código)║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");
    printf("- Opção: ");
    scanf("%d", &opcao);
    getchar();
    switch (opcao) {
        case 1:
            fRelatorios();
            break;
        case 2:
            fExibirProdutosOrdenado(&produtosVendidos, "alfabetica");
            break;
        case 3:
            fExibirProdutosOrdenado(&produtosVendidos, "id");
            break;
        default:
            fRelatorioProdutosVendidos();
            break;
    }
    printf("\nOBS: Este relatório exibe apenas os itens da última venda. Para um histórico completo de vendas, seria necessária uma implementação de persistência de dados de vendas.");
    escrever_log("Relatório de produtos vendidos exibido (última transação).");

    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║- Pressione ENTER para Voltar.                  ║\n");
    printf("╚════════════════════════════════════════════════╝\n");
    getchar();
    getchar();
    fRelatorios();
}


void fAdicionarTrocoCaixa() {
    float valor_adicionar;
    if(vAF == 1){
        printf("Digite o Valor que Deseja Colocar: ");
        scanf("%f", &valor_adicionar);
        getchar();
        trocoCaixa = trocoCaixa + valor_adicionar;
        fCaixa();
    } else{
        vMenu = 5;
        fVerificacaoDeEstado();
    }
}

void fEstadoDoCaixa(){
    if(vAF == 0){
        printf("║ 🟥       CAIXA FECHADO        🟥 ║\n");
    }else if(vAF == 1){
        printf("║ 🟩       CAIXA ABERTO         🟩 ║\n");
    }
 }

void fVerificacaoDeEstado(){
    if(vAF == 0){
        printf("\n╔════════════════╗\n");
        printf("║ CAIXA FECHADO! ║\n");
        printf("╚════════════════╝\n");
        Sleep(1500);
        if(vMenu == 0){
            escrever_log("ATIVIDADE VETADA(CAIXA FECHADO)");
            escrever_log("TELA MENU(RETORNO)");
            fMenu();
        }else if(vMenu == 1 ){
            escrever_log("ATIVIDADE VETADA(CAIXA FECHADO)");
            escrever_log("TELA LIMPEZA(RETORNO)");
            fLimpeza();
        }else if(vMenu == 2){
            escrever_log("ATIVIDADE VETADA(CAIXA FECHADO)");
            escrever_log("TELA ALIMENTOS(RETORNO)");
            fAlimentos();

        }else if(vMenu == 5){
            escrever_log("ATIVIDADE VETADA(CAIXA FECHADO)");
            escrever_log("TELA CAIXA(RETORNO)");
            fCaixa();
        }
    }
}

void fAbrirCaixa(){
    vAF = 1;
    printf("╔═════════════════════════╗\n");
    printf("║  ABERTURA EM ANDAMENTO  ║\n");
    printf("╚═════════════════════════╝\n");
    printf("\nValor de Abertura: ");
    scanf("%f", &troco_inicial);
    getchar();
    trocoCaixa = troco_inicial;
    Sleep(1000);
    printf("\n╔═════════════════╗\n");
    printf("║  CAIXA ABERTO!  ║\n");
    printf("╚═════════════════╝\n");
    escrever_log("ABRIU CAIXA: %.2f", trocoCaixa);
    Sleep(1000);
    fMenu();
}

void fFecharCaixa() {
    vAF = 0;
    system("cls");
    printf("╔══════════════════════════════════╗\n");
    printf("║           FATURAMENTO            ║\n");
    printf("╠══════════════════════════════════╣\n");
    printf("║  Abertura: R$%-6.2f              ║\n", troco_inicial);
    printf("║ 🧼 Limpeza: R$%-6.2f             ║\n", ftLimpeza);
    printf("║ 🥕 Alimentos: R$%-6.2f           ║\n", ftAlimentos);
    printf("║ 🍞 Padaria: R$%-6.2f             ║\n",  ftPadaria);
    printf("║ 💸 Total: R$%-6.2f               ║\n", vTotalDinheiro + trocoCaixa);
    printf("╚══════════════════════════════════╝\n");
    gerarRelatorio(troco_inicial, ftLimpeza, ftAlimentos, ftPadaria, vTotalDinheiro + trocoCaixa);
    escrever_log("RELATÓRIO DE FECHAMENTO ESCRITO");
    escrever_log("FECHAMENTO");
    escrever_log("Abertura: %.2f", troco_inicial);
    escrever_log("Limpeza: %.2f", ftLimpeza);
    escrever_log("Alimententos: %.2f", ftAlimentos);
    escrever_log("Padaria: %.2f", ftPadaria);
    escrever_log("Total: %.2f", vTotalDinheiro + trocoCaixa);
    printf("Pressione ENTER para voltar ao menu...\n");
    padaria_quantidade_atualizada = 0;
    getchar();

    fMenu();
}


void fSair() {
    escrever_log("FINALIZAR PROGRAMA");
    system("cls");
    printf("\n╔══════════════════════════════════╗\n");
    Sleep(150);
    printf("║        DESENVOLVEDORES:          ║\n");
    Sleep(150);
    printf("╠══════════════════════════════════╣\n");
    Sleep(150);
    printf("║- ADRYANN FELIX                   ║\n");
    Sleep(150);
    printf("║- GUILHERME REZENDE               ║\n");
    Sleep(150);
    printf("║- LUAN AUGUSTO                    ║\n");
    Sleep(150);
    printf("║- PAULO GUSTAVO                   ║\n");
    Sleep(150);
    printf("║- VITOR CASSEL                    ║\n");
    Sleep(150);
    printf("╚══════════════════════════════════╝\n");

    NoProduto* prod_atual;
    NoProduto* proximo_prod;

    prod_atual = produtosLimpeza;
    while (prod_atual != NULL) {
        proximo_prod = prod_atual->proximo;
        free(prod_atual);
        prod_atual = proximo_prod;
    }
    produtosLimpeza = NULL;

    prod_atual = produtosAlimento;
    while (prod_atual != NULL) {
        proximo_prod = prod_atual->proximo;
        free(prod_atual);
        prod_atual = proximo_prod;
    }
    produtosAlimento = NULL;

    prod_atual = produtosPadaria;
    while (prod_atual != NULL) {
        proximo_prod = prod_atual->proximo;
        free(prod_atual);
        prod_atual = proximo_prod;
    }
    produtosPadaria = NULL;

    prod_atual = produtosTotal;
    while (prod_atual != NULL) {
        proximo_prod = prod_atual->proximo;
        free(prod_atual);
        prod_atual = proximo_prod;
    }
    produtosTotal = NULL;

    prod_atual = produtosVendidos;
    while (prod_atual != NULL) {
        proximo_prod = prod_atual->proximo;
        free(prod_atual);
        prod_atual = proximo_prod;
    }
    produtosVendidos = NULL;
}

int main() {
    escrever_log("INICIAR PROGRAMA");

    fMenu();
    return 0;
}