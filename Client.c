#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    char buffer[256];

    if (argc < 3)
    {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        return 1;
    }

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        return 2;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(
            (char*)server->h_addr,
            (char*)&serv_addr.sin_addr.s_addr,
            server->h_length
    );
    serv_addr.sin_port = htons(atoi(argv[2]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 3;
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error connecting to socket");
        return 4;
    }
    ////////////////////////////////////////////////////////////////////
    bzero(buffer,256);
    read(sockfd,buffer,255);

    if(buffer[0] != 'R') {
        return -1;
    }
    int gameSize = buffer[1];
    char gameBoard[gameSize][gameSize];
    char gameBoardResult[gameSize][gameSize];
    for(int i = 0; i < gameSize; i++) {
        for (int j = 0; j < gameSize; ++j) {
            gameBoardResult[i][j] = '?';
        }
    }

    int buffCounter = 2;
    for(int i = 0; i < gameSize; i++) {
        for (int j = 0; j < gameSize; ++j) {
            gameBoard[i][j] = buffer[buffCounter];
            buffCounter++;
        }
    }
    printf("Game Loaded!\n");

    int player1 = 0;
    int player2 = 0;
    int winningPoints = ((gameSize * gameSize) / 2) + 1;

    while(1) {
        int x, y;
        int cardCount = 0;
        int broken;
        char firstCard, secondCard, firstX, firstY;

        while(cardCount < 2) { // Herny cyklus

            broken = 0;
            if(cardCount == 0) {
                for(int i = 0; i < gameSize; i++) { // Print board
                    for (int j = 0; j < gameSize; ++j) {
                        printf("%c ",gameBoardResult[i][j]);
                    }
                    printf("\n");
                }
            }

            printf("Zvolte ktoru kartu chcete otocit\n");

            printf("Suradnica X: \n");
            scanf("%d",&x);

            if(x < 1 || x > 8) {
                printf("Zadali ste nespravnu suradnicu, zadajte znova!\n");
                broken = 1;
                break;
            }

            printf("Suradnica Y: \n");
            scanf("%d",&y);

            if(y < 1 || y > 8) {
                printf("Zadali ste nespravnu suradnicu, zadajte znova!\n");
                broken = 1;
                break;
            }

            if(gameBoardResult[x-1][y-1] != '?') {
                firstX = 0;
                firstY = 0;
                printf("Tato karta je uz otocena, vyberte znova!\n");
                broken = 1;
                break;
            }
            if(cardCount == 1) {
                if(x == firstX && y == firstY) {
                    firstX = 0;
                    firstY = 0;
                    printf("Tato karta je uz otocena, vyberte znova!\n");
                    broken = 1;
                    break;
                }
            }

            if(cardCount == 0) {
                firstX = x;
                firstY = y;
            }

            gameBoardResult[x-1][y-1] =  gameBoard[x-1][y-1];

            for(int i = 0; i < gameSize; i++) { // Print board
                for (int j = 0; j < gameSize; ++j) {
                    printf("%c ",gameBoardResult[i][j]);
                }
                printf("\n");
            }

            if(cardCount == 0) {
                firstCard = gameBoard[x-1][y-1];
            } else {
                secondCard = gameBoard[x-1][y-1];
            }

            cardCount++;
        }
        cardCount = 0;

        if(broken == 1) {
            continue;
        }

        sleep(3);
        system("clear");
        printf("\n");

        bzero(buffer,256);

        if(firstCard == secondCard) {
            player2++;
            buffer[0] = '+';
            buffer[1] = firstX;
            buffer[2] = firstY;
            buffer[3] = x;
            buffer[4] = y;
            gameBoardResult[firstX-1][firstY-1] = firstCard;
            gameBoardResult[x-1][y-1] = secondCard;
            if(player2 == winningPoints) {
                buffer[0] = '~';
                n = write(sockfd, buffer, strlen(buffer));
                if (n < 0)
                {
                    perror("Error writing to socket");
                    return 5;
                }
                printf("Vyhrali ste!\n");
                return 0;
            } else {
                n = write(sockfd, buffer, strlen(buffer));
                if (n < 0)
                {
                    perror("Error writing to socket");
                    return 5;
                }
            }
        } else {
            buffer[0] = '-';
            gameBoardResult[firstX-1][firstY-1] = '?';
            gameBoardResult[x-1][y-1] = '?';
            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0)
            {
                perror("Error writing to socket");
                return 5;
            }
        }
        for(int i = 0; i < gameSize; i++) { // Print board
            for (int j = 0; j < gameSize; ++j) {
                printf("%c ",gameBoardResult[i][j]);
            }
            printf("\n");
        }
        printf("Hrac 1 je na tahu!\n");

        bzero(buffer,256);
        n = read(sockfd, buffer, 255);
        if (n < 0)
        {
            perror("Error reading from socket");
            return 6;
        }
        if(buffer[0] == '+') {
            gameBoardResult[buffer[1]-1][buffer[2]-1] =   gameBoard[buffer[1]-1][buffer[2]-1];
            gameBoardResult[buffer[3]-1][buffer[4]-1] =  gameBoard[buffer[3]-1][buffer[4]-1];
            player1++;
        }
        if(buffer[0] == '~') {
            printf("Prehrali ste!\n");
            return 0;
        }

    }
    close(sockfd);

    return 0;
}