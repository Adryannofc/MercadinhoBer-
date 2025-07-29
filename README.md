**# MercadinhoBer-
MercadinhoBerê
**MerceariaBere - Sistema de Gestão para Mercearia em C
📜 Tabela de Conteúdos
Sobre o Projeto

Funcionalidades

Tecnologias e Conceitos Aplicados

Como Compilar e Executar

Estrutura de Arquivos e Dados

Equipe

📖 Sobre o Projeto
O MerceariaBere é um sistema de linha de comando para gerenciamento de uma mercearia, desenvolvido inteiramente na linguagem C. Este projeto foi concebido como um trabalho acadêmico em grupo  do curso de Análise e Desenvolvimento de Sistemas.

O sistema simula as operações diárias de um pequeno mercado, desde o controle de estoque de produtos em diferentes setores até o processo de venda no caixa. O desenvolvimento foi focado em aplicar conceitos essenciais da programação em C, como listas encadeadas para gerenciamento de produtos, manipulação de arquivos para persistência de dados e alocação dinâmica de memória.

✨ Funcionalidades
O sistema foi implementado com um conjunto robusto de funcionalidades:

Gestão de Produtos por Setores:

Adicionar, listar e gerenciar produtos nos setores de Limpeza, Alimentos e Padaria.

Os IDs dos produtos são gerados de forma incremental e automática para cada setor (100+ para Limpeza, 200+ para Alimentos, etc.).

Controle de Estoque Inteligente:

O estoque é atualizado dinamicamente após cada venda.

O sistema verifica a disponibilidade do produto antes de adicioná-lo ao carrinho.

Função específica para atualizar o estoque da Padaria no início do dia.

Sistema de Carrinho e Pagamento:

Adição de produtos de qualquer setor ao carrinho de compras.

Cálculo de subtotal em tempo real.

Múltiplas formas de pagamento: Dinheiro (com sistema de troco) e Cartão (simulado).

Sistema de desconto progressivo para pagamentos em dinheiro, baseado no valor total da compra.

Persistência de Dados em Arquivos:

Todo o inventário de produtos é salvo em arquivos de texto (limpeza.txt, alimentos.txt, padaria.txt), permitindo que os dados persistam entre as execuções do programa.

O sistema carrega os dados dos arquivos ao ser iniciado.

Geração de Relatórios e Logs:

Relatório de Fechamento de Caixa: Gera um arquivo relatorio_fechamento.txt com o resumo financeiro do dia (faturamento por setor, valor de abertura e fechamento).

Relatórios de Estoque: Listagem de todos os produtos, com opções de ordenação por ID ou ordem alfabética.

Relatório de Estoque Baixo: Exibe produtos com 5 ou menos unidades em estoque.

Log de Atividades: Todas as ações importantes do usuário (navegação, vendas, erros) são registradas com data e hora no arquivo arquivoMercado.log.

Interface de Usuário em Console:

Menus de texto organizados e interativos para facilitar a navegação.

Uso de system("cls") e SetConsoleOutputCP(CP_UTF8) para uma experiência de usuário limpa e com suporte a caracteres especiais no Windows.

🛠️ Tecnologias e Conceitos Aplicados
Linguagem C

Compilador: GCC (MinGW) no Windows.

Bibliotecas Padrão do C:

stdio.h: Para entrada e saída de dados no console e manipulação de arquivos.

stdlib.h: Para alocação dinâmica de memória (malloc, free) e system("cls").

string.h: Para manipulação de strings (nomes dos produtos).

time.h: Para registrar data e hora nos relatórios e logs.

windows.h: Para Sleep() e SetConsoleOutputCP().

Estruturas de Dados:

Listas Encadeadas Simples para armazenar os produtos de cada setor.

Structs para modelar Produtos e Itens do Carrinho.

Arrays para o carrinho de compras.

Conceitos de Programação:

Alocação dinâmica de memória.

Ponteiros.

Manipulação de arquivos (File I/O).

Modularização com funções.

🚀 Como Compilar e Executar
Este projeto foi desenvolvido para o ambiente Windows, devido ao uso da biblioteca windows.h.

Pré-requisitos
Um compilador C, como o GCC (geralmente obtido através do MinGW-w64).

Passos
Clone o repositório:

Bash

git clone https://github.com/SEU_USUARIO/MerceariaBere.git
Navegue até o diretório do projeto:

Bash

cd MerceariaBere
Compile o arquivo Berenice.c:

Bash

gcc Berenice.c -o MerceariaBere.exe
Execute o programa:

Bash

./MerceariaBere.exe
Ou simplesmente:

Bash

MerceariaBere.exe
📁 Estrutura de Arquivos e Dados
Ao executar o programa, ele irá criar e interagir com os seguintes arquivos no mesmo diretório do executável:

MerceariaBere/
├── MerceariaBere.exe      # O programa compilado
├── Berenice.c             # O código-fonte principal
│
├── limpeza.txt            # Banco de dados de produtos de limpeza
├── alimentos.txt          # Banco de dados de produtos alimentícios
├── padaria.txt            # Banco de dados de produtos de padaria
│
├── arquivoMercado.log     # Log de todas as operações realizadas
└── relatorio_fechamento.txt # Relatório gerado ao fechar o caixa
👥 Equipe
Este projeto foi desenvolvido com a colaboração dos seguintes membros:

Nome do Integrante	GitHub
Vitor Cassel 1	@VitorCassel
Adryan Felix 2	@Adryannofc 
luan Augusto 3	@usuario3
Guilherme Rezende 4	@usuario4
Paulo Gustavo 5	@usuario5
