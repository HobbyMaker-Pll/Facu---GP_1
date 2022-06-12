#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define o tipo de mensagem da remessa
#define CABECALHO '1' // cabeçalho da remessa
#define INFO '2' // informações do boleto

// TAMANHO DOS TIPOS DE INFORMAÇÃO
#define CLIENTE_LEN 5 // tamanho de um código de cliente
#define DATA_LEN 8

// TAMANHO DOS TIPOS DE DATA
#define CABECALHO_LEN 9
#define INFO_LEN 44

typedef struct cliente
{
    char *Codigo; // código do cliente no banco
    unsigned long long int SomaTotal; // variavel acumulativa para soma dos boletos
    int QtdBoletos; // variavel acumulativa para contar a quantidade de boletos pagos
    FILE *Arquivo; // arquivo do cliente em questão
} tCliente;


tCliente *AcharCliente(tCliente *clientes, int tamanho, char *cliente)
{
    // essa função returna o endereço da memoria em que o cliente exigido está
    // caso não exista o cliente a função retorna nulo

    if(clientes == NULL) return NULL; // se o vetor de clientes for nulo returna nulo

    int i; // variavel de interação

    // busca pelo array dinamico se o cliente existe caso sim retorna o mesmo
    for (i=0; i<tamanho; i++)
    {
        if(strcmp(cliente, clientes[i].Codigo) == 0 ) return &clientes[i];
    }

    // caso n seja encontrado returna nulo
    return NULL;
}


int main()
{
    tCliente *clientes = NULL; // Array dinamico para guardar os arquivos de cada cliente
    char RemessaData[DATA_LEN]; // Data de envio da remessa
    int tamanho = 0; // quantidade de clientes lidos
    
    // Abrindo arquivo de Remessa e checando se realmente abriu
    FILE *Remessa = fopen("Remessa.txt", "r");
    if(Remessa == NULL) {perror("Erro ao abrir o arquivo"); exit(1);}

    // Criando ponteiro para se necessário criar arquivo de log
    FILE *Error = NULL;
    int ContagemLinha = 1; // contador de linhas para informar o erro se necessário

    // Criando variavel temporaria para guardar o caractere e flag de novo cliente
    char chr; // variavel que recebe novos caracteres lidos do arquivo
    int NovaInfo = 1; // aviso que a próxima mensagem está chegando
    int PosInfo = 0; // informação de quantas letras a mensagem atual possuí
    int TipoInfo; // define qual o tipo de informação que está sendo lida
    tCliente *ClienteAtual = NULL; // Aponta para o cliente atual em que o programa está escrevendo
    char *tLinha = NULL; // variavel que armazena temporariamente a linha a ser lida
    char *fileName = malloc(sizeof(char) * (DATA_LEN + CLIENTE_LEN + 6)); // variavel de buffer para armazenar o nome do arquivo
    int i; // interador para fechar os arquivos de cada cliente
    
    do {
        chr = fgetc(Remessa); // pega o caracter do arquivo
        PosInfo++; // conta uma posição de leitura

        if (NovaInfo) // caso seja uma nova informação
        {
            TipoInfo = chr; // define o tipo da nova informação

            if(TipoInfo == CABECALHO) // se for do tipo cabeçalho 
            {
                tLinha = realloc(tLinha, sizeof(char) * CABECALHO_LEN + 1); // aloca um tamanho de linha equivalente
                tLinha[0] = '\0'; // insere o finalizador na string para definir como sendo de tamanho 0
            }

            else if( TipoInfo == INFO) 
            {
                if(RemessaData[0] == '\0') exit(1); // caso o programa não tenha cabeçalho n sera possivel adicionar clientes
                tLinha = realloc(tLinha, sizeof(char) * INFO_LEN + 1); // aloca um tamanho de linha equivalente
                tLinha[0] = '\0'; // insere o finalizador na string para definir como sendo de tamanho 0
            }

            NovaInfo = 0; // reseta o bit de nova informação
            PosInfo = 0; // reseta a posição da informação
            continue; // pula o caractere atual
        }

        if (chr == '\n' || chr == EOF) // Caso o novo caractere sejá uma nova lina é necessario escrever no arquivo do cliente ou atualizar a data da remessa
        {
            switch (TipoInfo)
            {
            case CABECALHO:
                if (PosInfo != CABECALHO_LEN) // se o cabeçalho passar do tamanho correto é emitido um erro
                {
                    if( Error == NULL) Error = fopen("Error.txt", "w"); // caso o arquivo de erro n exista cria um novo
                    fprintf(Error, "Erro lendo o cabecalho na linha %i no arquivo de remessa \n", ContagemLinha); // escreve o erro junto com a linha do erro
                }
                else
                {
                    strcpy(RemessaData, tLinha); // caso esteja tudo correto atualisa a data da remessa
                }
                break;
            
            case INFO:
                if(PosInfo != INFO_LEN) // se a informação passar do tamanho correto é emitido um erro
                {
                    if( Error == NULL) Error = fopen("Error.txt", "w"); // caso o arquivo de erro n exista cria um novo
                    fprintf(Error, "Erro lendo as informações na linha %i no arquivo de remessa \n", ContagemLinha);// escreve o erro junto com a linha do erro
                }
                else
                {
                    fprintf(ClienteAtual->Arquivo, "2%s\n", tLinha);   // caso tudo esteja certo escreve no arquivo do cliente a informação
                    ClienteAtual->QtdBoletos++; // adiciona mais um na contagem de boletos pago do cliente
                    ClienteAtual->SomaTotal += atoi((tLinha+22)); // realiza a somatoria do pagamento dos boletos do cliente
                }
                
                break;
            }
            
            NovaInfo = 1; // define que o próximo caracter será fará parte de uma nova informação
            PosInfo = 0; // reseta a possição de caracteres
            ContagemLinha++; // acrescenta mais um na contagem de linhas para futuros erros
            continue; // passa para o proximo caractere
        }


        switch (TipoInfo)
        {
        case CABECALHO: // caso a nova informação sejá um cabeçalho
            if (PosInfo > INFO_LEN) continue; // ignora novos caracteres que tenham passado da quantidade maxima permitida
            strncat(tLinha, &chr, 1); // concatena o novo caractere com a string da linha
            break;
        
        case INFO: // caso a nova informção seja um boleto

            if (PosInfo > 18 && PosInfo < 27) continue; // Pula a data de vencimento que n é util para o cliente
            if (PosInfo > INFO_LEN) continue; // ignora novos caracteres que tenham passado da quantidade maxima permitida

            strncat(tLinha, &chr, 1); // concatena o novo caractere com a string da linha

            if(PosInfo == CLIENTE_LEN) // caso a quantidade de informaçãoes do boleto sejá igual a quantidade de digito do cliente
            {
                ClienteAtual = AcharCliente(clientes, tamanho, tLinha); // procura pelos clientes já abertos
                if (ClienteAtual == NULL) // caso n seja encontrado um cliente
                {
                    
                    clientes = realloc(clientes, sizeof(tCliente) * (tamanho+1)); // realoca a memoria para adequar a nova  quantidade de clientes
                    ClienteAtual = clientes + tamanho; // define o cliente atual como sendo o novo cliente criado
                    tamanho++; // aumenta a quantidade de clientes criados

                    sprintf(fileName, "%s_%s.txt", RemessaData, tLinha); // cria o nome do arquivo do cliente
                    ClienteAtual->Arquivo = fopen(fileName, "w"); // abre o arquivo do cliente
                    fprintf(ClienteAtual->Arquivo, "1%s\n", RemessaData); // escreve o cabeçalho do cliente

                    ClienteAtual->Codigo = malloc(sizeof(char) * (CLIENTE_LEN+1)); // aloca a memoria para guardar o código do cliente
                    strcpy(ClienteAtual->Codigo, tLinha); // copia a linha atual para o código do cliente
                    ClienteAtual->QtdBoletos = 0; // define o valor inicial da quantidade de boletos
                    ClienteAtual->SomaTotal = 0; // define o valor inicial da soma dos boletos
                    
                }
                
                tLinha[0] = '\0'; // limpa a linha atual
            }

            break;
        }

    } while (chr != EOF); // caso encontre o fim do arquivo termina o while loop

    fclose(Remessa); // fecha o arquivo de remessa

    for (i = 0; i < tamanho; i++) // passa pelos clientes criados
    {
        fprintf(clientes[i].Arquivo, "3%06i%010i\n", clientes[i].QtdBoletos, clientes[i].SomaTotal); // fecha o rodapé de todos os arquivos de cliente
        fclose(clientes[i].Arquivo); // fecha o arquivo do cliente
    }

}