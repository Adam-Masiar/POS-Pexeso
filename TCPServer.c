#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct data {
    int *gameSize;
    char **gameBoard;
    int *score_PLAYER1;
    int *score_PLAYER2;
    pthread_mutex_t *mutex;
    pthread_cond_t *COND_GENERATING;
    pthread_cond_t *COND_PLAYING;
    int *generation;
    int *gameState;
    int *socket;
} DATA;

void *game(void *data) {

    int x, y;
    int n;
    int cardCount = 0;
    int broken;
    char firstCard, secondCard, firstX, firstY;
    DATA *dat = data;

    int winningPoints = ((*dat->gameSize * *dat->gameSize) / 2) + 1;

    char gameBoardResult[*dat->gameSize][*dat->gameSize];
    for(int i = 0; i < *dat->gameSize; i++) {
        for (int j = 0; j < *dat->gameSize; ++j) {
            gameBoardResult[i][j] = '?';
        }
    }

    char buffer[256];
    pthread_mutex_lock(dat->mutex);
    while (*dat->gameState == 0) {
        while(*dat->generation == 0) {
            pthread_cond_wait(dat->COND_GENERATING, dat->mutex);
        }
        bzero(buffer,256);
        n = read(*dat->socket, buffer, 255);
        if (n < 0)
        {
            perror("Error reading from socket");
            return 6;
        }
        if(buffer[0] == '+') {
            gameBoardResult[buffer[1]-1][buffer[2]-1] =  *(*(dat->gameBoard + (buffer[2]-1)) + (buffer[1]-1));
            gameBoardResult[buffer[3]-1][buffer[4]-1] =  *(*(dat->gameBoard + (buffer[4]-1)) + (buffer[3]-1));
            *dat->score_PLAYER2++;
        }
        if(buffer[0] == '~') {
            printf("Prehrali ste!\n");
            return 0;
        }

        while(cardCount < 2) {

            broken = 0;
            if(cardCount == 0) {
                for(int i = 0; i < *dat->gameSize; i++) { // Print board
                    for (int j = 0; j < *dat->gameSize; ++j) {
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
            gameBoardResult[x-1][y-1] =  *(*(dat->gameBoard + (y-1)) + (x-1));

            for(int i = 0; i < *dat->gameSize; i++) { // Print board
                for (int j = 0; j < *dat->gameSize; ++j) {
                    printf("%c ",gameBoardResult[i][j]);
                }
                printf("\n");
            }
            if(cardCount == 0) {
                firstCard = *(*(dat->gameBoard + (y-1)) + (x-1));
            } else {
                secondCard = *(*(dat->gameBoard + (y-1)) + (x-1));
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
            *dat->score_PLAYER1++;
            buffer[0] = '+';
            buffer[1] = firstX;
            buffer[2] = firstY;
            buffer[3] = x;
            buffer[4] = y;
            gameBoardResult[firstX-1][firstY-1] = firstCard;
            gameBoardResult[x-1][y-1] = secondCard;
            if(*dat->score_PLAYER1 == winningPoints) {
                buffer[0] = '~';
                n = write(*dat->socket, buffer, strlen(buffer));
                if (n < 0)
                {
                    perror("Error writing to socket");
                    return 5;
                }
                printf("Vyhrali ste!\n");
                pthread_mutex_unlock(dat->mutex);
                return 0;
            } else {
                n = write(*dat->socket, buffer, strlen(buffer));
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
            n = write(*dat->socket, buffer, strlen(buffer));
            if (n < 0)
            {
                perror("Error writing to socket");
                return 5;
            }
        }
        for(int i = 0; i < *dat->gameSize; i++) { // Print board
            for (int j = 0; j < *dat->gameSize; ++j) {
                printf("%c ",gameBoardResult[i][j]);
            }
            printf("\n");
        }
        printf("Hrac 2 je na tahu!\n");

    }
}

void *generation(void *data) {
    srand(time(NULL));
    DATA *dat = data;
    int character = 'A';
    char buffer[256];
    pthread_mutex_lock(dat->mutex);
    for (int i = 0; i < *dat->gameSize; i++) {
        for (int j = 0; j < *dat->gameSize / 2; j++) {
            *(*(dat->gameBoard + j) + i) = character;
            character++;
            if(character == 91)
                character = 48;
        }
    }
    character = 'A';
    for (int i = 0; i < *dat->gameSize; i++) {
        for (int j = *dat->gameSize / 2; j < *dat->gameSize; j++) {
            *(*(dat->gameBoard + j) + i) = character;
            character++;
            if(character == 91)
                character = 48;
        }
    }

    for(int i = 0; i < *dat->gameSize; i++) {
        for(int j = 0; j < *dat->gameSize; j++) {
            int r = rand() % (*dat->gameSize - i) + i;
            int temp = *(*(dat->gameBoard + j) + i);
            *(*(dat->gameBoard + j) + i) = *(*(dat->gameBoard + j) + r);
            *(*(dat->gameBoard + j) + r) = temp;
        }
    }

    *dat->generation = 1;
    pthread_mutex_unlock(dat->mutex);
    pthread_cond_signal(dat->COND_GENERATING);
    bzero(buffer,256);
    buffer[0] = 'R';
    buffer[1] = *dat->gameSize;
    int buffCounter = 2;
    for(int i = 0; i < *dat->gameSize; i++) {
        for(int j = 0; j < *dat->gameSize; j++) {
            buffer[buffCounter] = *(*(dat->gameBoard + j) + i);
            buffCounter++;
        }
    }
    write(*dat->socket,buffer,255);
}


int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    char buffer[256];

    if (argc < 2) {
        fprintf(stderr, "usage %s port\n", argv[0]);
        return 1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding socket address");
        return 2;
    }

    listen(sockfd, 5);
    cli_len = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
    if (newsockfd < 0) {
        perror("ERROR on accept");
        return 3;
    }
    /////////////////////////////////////////////////////////////////////////////////////
    pthread_t Tplay, Tgenerate;
    pthread_mutex_t mutex;
    pthread_cond_t COND_GENERATING, COND_PLAYING;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&COND_GENERATING, NULL);
    pthread_cond_init(&COND_PLAYING, NULL);

    printf("Zadajte velkost pexesa (4 6 alebo 8)\n");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);
    int size = atoi(buffer);
    bool correctSize = false;
    if (size < 4 || size > 8 || size % 2 != 0) {
        while (correctSize == false) {
            printf("Zadali ste zle cislo\n");
            printf("Zadajte velkost pexesa (4 6 alebo 8)\n");
            bzero(buffer, 256);
            fgets(buffer, 255, stdin);
            if (size >= 4 && size <= 8 && size % 2 == 0)
                correctSize = true;
        }
    }
    printf("Cakajte na kolo Hraca 2\n");
    char **gameBoard = (char **) malloc(size * sizeof(char *));
    for (int i = 0; i < size; i++) {
        gameBoard[i] = (char *) malloc(size * sizeof(char));
    }
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            gameBoard[i][j] = '#';
        }
    }
    int score1 = 0;
    int score2 = 0;
    int gen = 0;
    int gameS = 0;
    DATA data = {&size, gameBoard, &score1, &score2, &mutex, &COND_GENERATING, &COND_PLAYING, &gen, &gameS, &newsockfd};
    pthread_create(&Tplay, NULL, &game, &data);
    pthread_create(&Tgenerate, NULL, &generation, &data);
    pthread_join(Tgenerate, NULL);
    pthread_join(Tplay, NULL);

    for (int i = 0; i < size; i++) {
        free(gameBoard[i]);
    }
    free(gameBoard);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&COND_PLAYING);
    pthread_cond_destroy(&COND_GENERATING);
    close(newsockfd);
    close(sockfd);

    return 0;
}