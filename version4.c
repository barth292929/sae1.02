
/**
*
* \brief Programme Snake
*
* \author HAMON Gabriel, GOUJON Barthélemy
*
* \version 3
*
* \date 10 Janvier 2025
*
* Ce programme est la 3ere version de la SAE-102
* Objectif : implémenter un système d'obstacles
*
*
* Stratégie utilisée : pour l'objectif que le serpent doit atteindre, on compare les distances en prenant en compte les portails 
* pour trouver le chemin le plus court. Le serpent regarde d'abord l'axe Y, puis l'axe X. Ensuite, il se dirige vers cet objectif, 
* en évitant au besoin les pavés et lui-même, grâce à une procédure qui calcule sa prochaine position. Si son objectif se troyve à
* l'opposé de sa direction actuelle et qu'il ne peut pas tourner, il continue d'avancer jusqu'à ce que se soit possible.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>


// taille du serpent
#define TAILLE 10
// dimensions du plateau
#define LARGEUR_PLATEAU 80  
#define HAUTEUR_PLATEAU 40  
// position initiale de la tête du serpent numéro 1
#define X_INITIAL_SERPENT_1 40
#define Y_INITIAL_SERPENT_1 13
// position intitiale de la tête du serpent numéro 2
#define X_INITIAL_SERPENT_2 40
#define Y_INITIAL_SERPENT_2 27
// nombre de pommes à manger pour gagner
#define NB_POMMES 10
// temporisation entre deux déplacements du serpent (en microsecondes)
#define ATTENTE 100000
// caractères pour représenter le serpent
#define CORPS 'X'
#define CORPS2 'S'
#define TETE_1 '1'
#define TETE_2 '2'
// touches de direction ou d'arrêt du jeu
#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define STOP 'a'
// caractères pour les éléments du plateau
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'
//nombre de directions
#define NB_DIRECTIONS 4
#define NB_SORTIE 4
// caractéristiques des pavés
#define NB_PAVES 6
#define TAILLE_BLOC 5


// définition d'un type pour le plateau : tPlateau
// Attention, pour que les indices du tableau 2D (qui commencent à 0) coincident
// avec les coordonées à l'écran (qui commencent à 1), on ajoute 1 aux dimensions
// et on neutralise la ligne 0 et la colonne 0 du tableau 2D (elles ne sont jamais
// utilisées)
typedef char tPlateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1];


void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau, int nb_pomme);
void ajouterPave(tPlateau plateau);
void afficher(int, int, char);
void effacer(int x, int y);

void dessinerSerpent(int lesX[], int lesY[]);
void dessinerSerpent2(int lesX[], int lesY[]);

void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool * collision, bool * pomme);
void progresser2(int lesX_2[], int lesY_2[], char direction2, tPlateau plateau, bool * collision, bool * pomme);

void calcTraj(tPlateau plateau, int lesX[], int lesY[], int lesX_2[], int lesY_2[], char *direction, char *direction2, int objectif_x, int objectif_y, 
              int *prochaine_position_x, int *prochaine_position_y, int *prochaine_position_x2, int *prochaine_position_y2);
void calcTraj2(tPlateau plateau, int lesX[], int lesY[], int lesX_2[], int lesY_2[], char *direction, char *direction2, int objectif_x2, int objectif_y2, 
              int *prochaine_position_x, int *prochaine_position_y, int *prochaine_position_x2, int *prochaine_position_y2);

bool crash(int lesX[], int lesY[], int lesX_2[], int lesY_2[], char direction, char direction2, int *prochaine_position_x, int *prochaine_position_y, int *prochaine_position_x2, int *prochaine_position_y2, tPlateau plateau);
bool crash2(int lesX[], int lesY[], int lesX_2[], int lesY_2[], char direction, char direction2, int *prochaine_position_x, int *prochaine_position_y, int *prochaine_position_x2, int *prochaine_position_y2, tPlateau plateau);

void trajOpti(int lesX[], int lesY[], int nb_pomme, char direction, int *objectif_x, int *objectif_y);
void trajOpti2(int lesX_2[], int lesY_2[], int nb_pomme, char direction2, int *objectif_x2, int *objectif_y2);

void prochaine_position(int lesX[], int lesY[], int *prochaine_position_x, int *prochaine_position_y, char direction) ;
void prochaine_position2(int lesX[], int lesY[], int *prochaine_position_x2, int *prochaine_position_y2, char direction2);

void gotoxy(int x, int y);
int  kbhit();
void disable_echo();
void enable_echo();
int Modulo(int x,int N);

// coordonnées des pommes
int lesPommesX[NB_POMMES] = {40, 75, 78, 2, 9, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = {20, 38, 2, 2, 5, 38, 32, 38, 32, 2};

// coordonnées des pavés
int lesPavesX[NB_PAVES] = { 4, 73, 4, 73, 38, 38};
int lesPavesY[NB_PAVES] = { 4, 4, 33, 33, 14, 22};

// coordonnées des portails
int portail_X[] = {LARGEUR_PLATEAU/2, LARGEUR_PLATEAU, LARGEUR_PLATEAU/2, 1} ;
int portail_Y[] = {1, HAUTEUR_PLATEAU/2, HAUTEUR_PLATEAU, HAUTEUR_PLATEAU/2} ;

//nombre de pommes mangées par serpent 
int nb_pomme1 ;
int nb_pomme2 ;
//coordonnées de départ du serpent



int main(){
	/************************************************/
	/*      	 PARAMETRES DU SERPENT 1            */
	/************************************************/
    // 2 tableaux contenant les positions des éléments qui constituent le serpent 1
    int lesX[TAILLE];
    int lesY[TAILLE];
    //direction courante du serpent 1 (HAUT, BAS, GAUCHE ou DROITE)
    char direction;
    // compteur de déplacements du serpent 1
    int nbMove = 0;
	//objectif du serpent 1
    int objectif_x ;
    int objectif_y ;
    //position suivante du serpent 1
    int prochaine_position_x, prochaine_position_y;
    //nombre initial de pommes mangés par serpent
    nb_pomme1 = 0 ;
    nb_pomme2 = 0 ;


	/************************************************/
	/*      	 PARAMETRES DU SERPENT 2            */
	/************************************************/
    // 2 tableaux contenant les positions des éléments qui constituent le serpent 2
    int lesX_2[TAILLE];
    int lesY_2[TAILLE];
    //direction courante du serpent 2 (HAUT, BAS, GAUCHE ou DROITE)
    char direction2;
    // compteur de déplacements du serpent 2
    int nbMove2 = 0;
	//objectif du serpent 2
    int objectif_x2 ;
    int objectif_y2 ;
    //position suivante du serpent 2
    int prochaine_position_x2, prochaine_position_y2;

	/************************************************/
	/*      	 			MAIN		            */
	/************************************************/

    int Tour = 0;
	// compteur de pommes mangées
    int nbPommes = 0;

    // représente la touche frappée par l'utilisateur pour l'arrêt
    char touche;

    // le plateau de jeu
    tPlateau lePlateau;

    bool collision=false;
    bool gagne = false;
    bool pommeMangee = false;
   
    // initialisation de la position du serpent : positionnement de la
    // tête en (X_INITIAL, Y_INITIAL), puis des anneaux à sa gauche
    for(int i=0 ; i<TAILLE ; i++){
        lesX[i] = X_INITIAL_SERPENT_1-i;
        lesY[i] = Y_INITIAL_SERPENT_1;
    }

    for(int i=0 ; i<TAILLE ; i++){
        lesX_2[i] = X_INITIAL_SERPENT_2+i;
        lesY_2[i] = Y_INITIAL_SERPENT_2;
    }

    // mise en place du plateau
    initPlateau(lePlateau);
    ajouterPave(lePlateau);
    system("clear");
    dessinerPlateau(lePlateau);


    ajouterPomme(lePlateau, nbPommes);

    // initialisation : le serpent se dirige vers la DROITE
    dessinerSerpent(lesX, lesY);
	dessinerSerpent2(lesX_2, lesY_2);
    disable_echo();
    direction = DROITE;
    direction2 = GAUCHE;
    prochaine_position(lesX, lesY, &prochaine_position_x, &prochaine_position_y, direction) ;
    prochaine_position2(lesX_2, lesY_2, &prochaine_position_x2, &prochaine_position_y2, direction);
    if 
    (
        (crash(lesX, lesY, lesX_2, lesY_2, direction, direction2, &prochaine_position_x, &prochaine_position_y, &prochaine_position_x2, &prochaine_position_y2, lePlateau))
        ||
        (crash2(lesX, lesY, lesX_2, lesY_2, direction, direction2, &prochaine_position_x, &prochaine_position_y, &prochaine_position_x2, &prochaine_position_y2, lePlateau))
    ){
    collision = true ;
    }

    // boucle de jeu. Arret si touche STOP, si collision avec une bordure ou
    // si toutes les pommes sont mangées
    trajOpti(lesX, lesY, nbPommes, direction, &objectif_x, &objectif_y);
    objectif_x2 = 42;
    objectif_y2 = 21;
    do 
    {
        if (Tour == 0){
            /*---------------------------------- POUR LE SERPENT 1 ----------------------------------*/

            calcTraj(lePlateau, lesX, lesY, lesX_2, lesY_2, &direction, &direction2, objectif_x, objectif_y, &prochaine_position_x, &prochaine_position_y, &prochaine_position_x2, &prochaine_position_y2); 
            progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee);
            nbMove++; //compteur de mouvements


            if (pommeMangee){
                nbPommes++; //augmente le nombre de pommes mangées
                nb_pomme1++;//augmente le compteur de pommes du serpent 1
                gagne = (nbPommes==NB_POMMES);
                if (!gagne){
                    ajouterPomme(lePlateau, nbPommes); //ajoute une pomme sur le plateau suite à la pomme mangée
                }   
            }

            if ((lesX[0] == objectif_x && lesY[0] == objectif_y) || (lesX_2[0] == objectif_x && lesY_2[0] == objectif_y)) { //lorsque l'objectif est atteint
            for (int k = 0; k<NB_SORTIE; k++){
                if (!pommeMangee && objectif_x == portail_X[k] && objectif_y == portail_Y[k]) {                              // et que l'objectif n'est pas une pomme
                    progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee);
                    progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee); //on progresse de 2 car le portail est situé au niveau de bordure
                    nbMove += 2;
            }
                }
                trajOpti(lesX, lesY, nbPommes, direction, &objectif_x, &objectif_y) ; //calcul les coordonnées du nouvel objectif
            }

            pommeMangee = false;
        }
        if (Tour == 1){
            /*---------------------------------- POUR LE SERPENT 2 ----------------------------------*/

            calcTraj2(lePlateau, lesX, lesY, lesX_2, lesY_2, &direction, &direction2, objectif_x2, objectif_y2, &prochaine_position_x, &prochaine_position_y, &prochaine_position_x2, &prochaine_position_y2); 
            progresser2(lesX_2, lesY_2, direction2, lePlateau, &collision, &pommeMangee);
            nbMove2++; //compteur de mouvements

            if (pommeMangee){
                nbPommes++; //augmente le nombre de pommes mangées
                nb_pomme2++;//augmente le compteur de pommes du serpent 2
                gagne = (nbPommes==NB_POMMES);
                if (!gagne){
                    ajouterPomme(lePlateau, nbPommes); //ajoute une pomme sur le plateau suite à la pomme mangée
                }   
            }

            if ((lesX_2[0] == objectif_x2 && lesY_2[0] == objectif_y2) || (lesX[0] == objectif_x2 && lesY[0] == objectif_y2)) { //lorsque l'objectif est atteint
            for (int k = 0; k<NB_SORTIE; k++){
                if (!pommeMangee && objectif_x2 == portail_X[k] && objectif_y2 == portail_Y[k]) {                                     // et que l'objectif n'est pas une pomme
                    progresser2(lesX_2, lesY_2, direction2, lePlateau, &collision, &pommeMangee);
                    progresser2(lesX_2, lesY_2, direction2, lePlateau, &collision, &pommeMangee); //on progresse de 2 car le portail est situé au niveau de bordure
                    nbMove += 2;
            }
                }
            trajOpti2(lesX_2, lesY_2, nbPommes, direction2, &objectif_x2, &objectif_y2);
            }

            pommeMangee = false;
            Tour =  Tour-2;
        }
        /*---------------------------------- INDEPENDANT DES SERPENTS ----------------------------------*/
        if (!gagne){
            if (!collision){
                usleep(ATTENTE);
                if (kbhit()==1){
                    touche = getchar();
                }
            }
        }
        Tour ++;
    } while (touche!=STOP && !collision && !gagne);
    enable_echo();
    gotoxy(1, HAUTEUR_PLATEAU+1);
    printf("nombre de déplacements du premier serpent = %d et nombre de pommes mangées : %d\n", nbMove, nb_pomme1);
    printf("nombre de déplacements du deuxième serpent = %d et nombre de pommes mangées : %d\n", nbMove2, nb_pomme2);

    return EXIT_SUCCESS;
}


/************************************************/
/*      FONCTIONS ET PROCEDURES DU JEU          */
/************************************************/
void initPlateau(tPlateau plateau){
    // initialisation du plateau avec des espaces
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
            plateau[i][j] = VIDE;
        }
    }
    // Mise en place la bordure autour du plateau
    // première ligne
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        plateau[i][1] = BORDURE;
    }
    // lignes intermédiaires
    for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
            plateau[1][j] = BORDURE;
            plateau[LARGEUR_PLATEAU][j] = BORDURE;
        }
    // dernière ligne
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        plateau[i][HAUTEUR_PLATEAU] = BORDURE;
    }

    for(int i = 0 ; i<NB_SORTIE ; i++) {
        plateau[portail_X[i]][portail_Y[i]] = VIDE ;
    }
}

void dessinerPlateau(tPlateau plateau){
    // affiche eà l'écran le contenu du tableau 2D représentant le plateau
    for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
        for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
            afficher(i, j, plateau[i][j]);
        }
    }
}

void ajouterPomme(tPlateau plateau, int nb_pomme){
    // génère aléatoirement la position d'une pomme,
    // vérifie que ça correspond à une case vide
    // du plateau puis l'ajoute au plateau et l'affiche
    plateau[lesPommesX[nb_pomme]][lesPommesY[nb_pomme]]=POMME;
    afficher(lesPommesX[nb_pomme], lesPommesY[nb_pomme], POMME);
}

void ajouterPave(tPlateau plateau){
    int x, y;

    for (int k = 0; k<NB_PAVES; k++){
        x = lesPavesX[k];
        y = lesPavesY[k];
            for (int i = 0; i < TAILLE_BLOC; i++) {
                for (int j = 0; j < TAILLE_BLOC; j++) {
                    plateau[x + i][y + j] = BORDURE; // Placer le pavé
                }
            }
    }
}

void effacer(int x, int y){
    gotoxy(x, y);
    printf(" ");
    gotoxy(1,1);
}

void afficher(int x, int y, char car){
    gotoxy(x, y);
    printf("%c", car);
    gotoxy(1,1);
}

void dessinerSerpent(int lesX[], int lesY[]){
    // affiche les anneaux puis la tête
    for(int i=1 ; i<TAILLE ; i++){
        afficher(lesX[i], lesY[i], CORPS);
    }
    afficher(lesX[0], lesY[0],TETE_1);
}

void dessinerSerpent2(int lesX[], int lesY[]){
    // affiche les anneaux puis la tête
    for(int i=1 ; i<TAILLE ; i++){
        afficher(lesX[i], lesY[i], CORPS2);
    }
    afficher(lesX[0], lesY[0],TETE_2);
}

void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool * collision, bool * pomme){
    // efface le dernier élément avant d'actualiser la position de tous les 
    // éléments du serpent avant de le  redessiner et détecte une
    // collision avec une pomme ou avec une bordure
    effacer(lesX[TAILLE-1], lesY[TAILLE-1]);

    for(int i=TAILLE-1 ; i>0 ; i--){
        lesX[i] = lesX[i-1];
        lesY[i] = lesY[i-1];
    }
    //faire progresser la tete dans la nouvelle direction
    switch(direction){
        case HAUT : 
            lesY[0] = lesY[0] - 1;
            break;
        case BAS:
            lesY[0] = lesY[0] + 1;
            break;
        case DROITE:
            lesX[0] = lesX[0] + 1;
            break;
        case GAUCHE:
            lesX[0] = lesX[0] - 1;
            break;
    }

    lesX[0] = Modulo(lesX[0]-1, LARGEUR_PLATEAU) +1;
    lesY[0] = Modulo(lesY[0]-1, HAUTEUR_PLATEAU) +1;

    *pomme = false;
    // détection d'une "collision" avec une pomme
    if (plateau[lesX[0]][lesY[0]] == POMME){
        *pomme = true;
        // la pomme disparait du plateau
        plateau[lesX[0]][lesY[0]] = VIDE;
    }
    // détection d'une collision avec la bordure
    else if (plateau[lesX[0]][lesY[0]] == BORDURE){
        *collision = true;
    }

    dessinerSerpent(lesX, lesY);
}

void progresser2(int lesX_2[], int lesY_2[], char direction2, tPlateau plateau, bool * collision, bool * pomme){
    // efface le dernier élément avant d'actualiser la position de tous les 
    // éléments du serpent avant de le  redessiner et détecte une
    // collision avec une pomme ou avec une bordure
    effacer(lesX_2[TAILLE-1], lesY_2[TAILLE-1]);

    for(int i=TAILLE-1 ; i>0 ; i--){
        lesX_2[i] = lesX_2[i-1];
        lesY_2[i] = lesY_2[i-1];
    }
    //faire progresser la tete dans la nouvelle direction
    switch(direction2){
        case HAUT : 
            lesY_2[0] = lesY_2[0] - 1;
            break;
        case BAS:
            lesY_2[0] = lesY_2[0] + 1;
            break;
        case DROITE:
            lesX_2[0] = lesX_2[0] + 1;
            break;
        case GAUCHE:
            lesX_2[0] = lesX_2[0] - 1;
            break;
    }

    lesX_2[0] = Modulo(lesX_2[0]-1, LARGEUR_PLATEAU) +1;
    lesY_2[0] = Modulo(lesY_2[0]-1, HAUTEUR_PLATEAU) +1;

    *pomme = false;
    // détection d'une "collision" avec une pomme
    if (plateau[lesX_2[0]][lesY_2[0]] == POMME){
        *pomme = true;
        // la pomme disparait du plateau
        plateau[lesX_2[0]][lesY_2[0]] = VIDE;
    }
    // détection d'une collision avec la bordure
    else if (plateau[lesX_2[0]][lesY_2[0]] == BORDURE){
        *collision = true;
    }

    dessinerSerpent2(lesX_2, lesY_2);
}

/******************************************************/
/*      FONCTIONS ET PROCEDURES DES SERPENTS          */
/******************************************************/

void calcTraj(tPlateau plateau, int lesX[], int lesY[], int lesX_2[], int lesY_2[], char *direction, char *direction2, int objectif_x, int objectif_y, 
              int *prochaine_position_x, int *prochaine_position_y, int *prochaine_position_x2, int *prochaine_position_y2) {

    if (lesX[0] < objectif_x && *direction != GAUCHE) {
        *direction = DROITE; // Emmène le serpent vers la droite
    } else if (lesX[0] > objectif_x && *direction != DROITE) {
        *direction = GAUCHE; // Emmène le serpent vers la gauche
    }

    if (lesY[0] < objectif_y && *direction != HAUT) {
        *direction = BAS; // Emmène le serpent vers le bas
    } else if (lesY[0] > objectif_y && *direction != BAS) {
        *direction = HAUT; // Emmène le serpent vers le haut
    }
    
    // On vérifie si la direction actuelle est toujours valide
    if (!crash(lesX, lesY, lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
        // Si la direction actuelle est valide, on continue dans cette direction
        return;
    }

    // Si la direction n'est pas valide, on essaye une autre direction
    if (crash(lesX, lesY, lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {

        switch (*direction) {
            case HAUT: // essaye 1 par 1 les directions possibles
                *direction = DROITE;
                if (crash(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                    *direction = GAUCHE;
                    if (crash(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                        *direction = BAS;
                    }
                }
            break;

            case BAS: // essaye 1 par 1 les directions possibles
                *direction = DROITE;
                if (crash(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                    *direction = GAUCHE;
                    if (crash(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                        *direction = HAUT;
                    }
                }
            break;

            case GAUCHE: // essaye 1 par 1 les directions possibles
                *direction = HAUT;
                if (crash(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                    *direction = BAS;
                    if (crash(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                        *direction = DROITE;
                    }
                }
            break;

            case DROITE: // essaye 1 par 1 les directions possibles
                *direction = HAUT;
                if (crash(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                    *direction = BAS;
                    if (crash(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                        *direction = GAUCHE;
                    }
                }
            break;
        }
    }
}

void calcTraj2(tPlateau plateau, int lesX[], int lesY[], int lesX_2[], int lesY_2[], char *direction, char *direction2, int objectif_x2, int objectif_y2, 
              int *prochaine_position_x, int *prochaine_position_y, int *prochaine_position_x2, int *prochaine_position_y2) {

    if (lesX_2[0] < objectif_x2 && *direction2 != GAUCHE) {
        *direction2 = DROITE; // Emmène le serpent vers la droite
    } else if (lesX_2[0] > objectif_x2 && *direction2 != DROITE) {
        *direction2 = GAUCHE; // Emmène le serpent vers la gauche
    }

    if (lesY_2[0] < objectif_y2 && *direction2 != HAUT) {
        *direction2 = BAS; // Emmène le serpent vers le bas
    } else if (lesY_2[0] > objectif_y2 && *direction2 != BAS) {
        *direction2 = HAUT; // Emmène le serpent vers le haut
    }
    
    // On vérifie si la direction actuelle est toujours valide
    if (!crash2(lesX, lesY, lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
        // Si la direction actuelle est valide, on continue dans cette direction
        return;
    }

    // Si la direction n'est pas valide, on essaye une autre direction
    if (crash2(lesX, lesY, lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {

        switch (*direction2) {
            case HAUT: // essaye 1 par 1 les directions possibles
                *direction2 = DROITE;
                if (crash2(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                    *direction2 = GAUCHE;
                    if (crash(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                        *direction2 = BAS;
                    }
                }
            break;

            case BAS: // essaye 1 par 1 les directions possibles
                *direction2 = DROITE;
                if (crash2(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                    *direction2 = GAUCHE;
                    if (crash2(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                        *direction2 = HAUT;
                    }
                }
            break;

            case GAUCHE: // essaye 1 par 1 les directions possibles
                *direction2 = HAUT;
                if (crash2(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                    *direction2 = BAS;
                    if (crash2(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                        *direction2 = DROITE;
                    }
                }
            break;

            case DROITE: // essaye 1 par 1 les directions possibles
                *direction2 = HAUT;
                if (crash2(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                    *direction2 = BAS;
                    if (crash2(lesX, lesY,  lesX_2, lesY_2, *direction, *direction2, prochaine_position_x, prochaine_position_y, prochaine_position_x2, prochaine_position_y2, plateau)) {
                        *direction2 = GAUCHE;
                    }
                }
            break;
        }
    }
}

bool crash(int lesX[], int lesY[], int lesX_2[], int lesY_2[], char direction,  char direction2, int *prochaine_position_x, int *prochaine_position_y, int *prochaine_position_x2, int *prochaine_position_y2, tPlateau plateau){

    int i ;
    bool ok = false;
    prochaine_position(lesX, lesY, prochaine_position_x, prochaine_position_y, direction);

    // détecte à l'avance une collision avec les serpents
    for (i=1 ; i<TAILLE ; i++) {
        if
		( 
		(*prochaine_position_x == lesX[i] && *prochaine_position_y==lesY[i])
		||
		(*prochaine_position_x == lesX_2[i] && *prochaine_position_y==lesY_2[i])	
		){
            ok = true;
        }
    }

    // détecte à l'avance une collision avec un pavé ou la bordure
    for (int x = 0; x < LARGEUR_PLATEAU+1; x++) // +1 pour prendre en compte la bordure
    {
        for (int y = 0; y < HAUTEUR_PLATEAU+1; y++) // +1 pour prendre en compte la bordure
        {
            if (plateau[x][y] == BORDURE)
            {
                if (*prochaine_position_x == x && *prochaine_position_y == y)
                {
                    ok = true;
                }
            }
		}
	}
    return ok;
}

bool crash2(int lesX[], int lesY[], int lesX_2[], int lesY_2[], char direction,  char direction2, int *prochaine_position_x, int *prochaine_position_y, int *prochaine_position_x2, int *prochaine_position_y2, tPlateau plateau){

    int i ;
    bool ok = false;
    prochaine_position2(lesX_2, lesY_2, prochaine_position_x2, prochaine_position_y2, direction2);

    // détecte à l'avance une collision avec lui-même
    for (i=1 ; i<TAILLE ; i++) {
        if
		( 
		(*prochaine_position_x2 == lesX[i] && *prochaine_position_y2 ==lesY[i])
		||
		(*prochaine_position_x2 == lesX_2[i] && *prochaine_position_y2 ==lesY_2[i])	
		){
            ok = true;
        }
    }

    // détecte à l'avance une collision avec un pavé ou la bordure
    for (int x = 0; x < LARGEUR_PLATEAU+1; x++) // +1 pour prendre en compte la bordure
    {
        for (int y = 0; y < HAUTEUR_PLATEAU+1; y++) // +1 pour prendre en compte la bordure
        {
            if (plateau[x][y] == BORDURE)
            {
                if (*prochaine_position_x2 == x && *prochaine_position_y2 == y)
                {
                    ok = true;
                }
            }
		}
	}
    return ok;
}

void trajOpti(int lesX[], int lesY[], int nb_pomme, char direction, int *objectif_x, int *objectif_y){
    int Distance_pomme, distance_min;
    int sortie_oppose, sortie, sortie_min ;
    int distance ;

    sortie_min = -1 ;

    // calcul de la distance serpent->pomme
    Distance_pomme = (abs(lesX[0]-lesPommesX[nb_pomme]) + abs(lesY[0]-lesPommesY[nb_pomme])); //abs pour absolute

    distance_min = Distance_pomme ;

    for (sortie = 0 ; sortie < NB_DIRECTIONS ; sortie++) {
        sortie_oppose = (sortie + (NB_DIRECTIONS / 2)) % NB_DIRECTIONS ; //pour trouver l'opposé en deux
        distance = (abs(lesX[0] - portail_X[sortie]) + 
                    abs(lesY[0] - portail_Y[sortie]) + 
                    abs(lesPommesX[nb_pomme] - portail_X[sortie_oppose]) + 
                    abs(lesPommesY[nb_pomme] - portail_Y[sortie_oppose])
                    ) ;
                    //cela calcul la distance serpent -> portail + portail -> pomme

        if (distance_min > distance) { //cherche la distance minimale
            distance_min = distance ;
            sortie_min = sortie ;
        }
    }

    if (sortie_min == -1) { //si le chemin le plus court est vers la pomme
        *objectif_x = lesPommesX[nb_pomme] ;
        *objectif_y = lesPommesY[nb_pomme] ;
    } else { //si le chemin le plus court est vers le portail
        *objectif_x = portail_X[sortie_min] ;
        *objectif_y = portail_Y[sortie_min] ;
    }

}

void trajOpti2(int lesX_2[], int lesY_2[], int nb_pomme, char direction2, int *objectif_x2, int *objectif_y2){
    int Distance_pomme, distance_min;
    int sortie_oppose, sortie, sortie_min ;
    int distance ;

    sortie_min = -1 ;

    // calcul de la distance serpent->pomme
    Distance_pomme = (abs(lesX_2[0]-lesPommesX[nb_pomme]) + abs(lesY_2[0]-lesPommesY[nb_pomme])); //abs pour absolute

    distance_min = Distance_pomme ;

    for (sortie = 0 ; sortie < NB_DIRECTIONS ; sortie++) {
        sortie_oppose = (sortie + (NB_DIRECTIONS / 2)) % NB_DIRECTIONS ; //pour trouver l'opposé en deux
        distance = (abs(lesX_2[0] - portail_X[sortie]) + 
                    abs(lesY_2[0] - portail_Y[sortie]) + 
                    abs(lesPommesX[nb_pomme] - portail_X[sortie_oppose]) + 
                    abs(lesPommesY[nb_pomme] - portail_Y[sortie_oppose])
                    ) ;
                    //cela calcule la distance serpent -> portail + portail -> pomme

        if (distance_min > distance) { //cherche la distance minimale
            distance_min = distance ;
            sortie_min = sortie ;
        }
    }

    if (sortie_min == -1) { //si le chemin le plus court est vers la pomme
        *objectif_x2 = lesPommesX[nb_pomme] ;
        *objectif_y2 = lesPommesY[nb_pomme] ;
    } else { //si le chemin le plus court est vers le portail
        *objectif_x2 = portail_X[sortie_min] ;
        *objectif_y2 = portail_Y[sortie_min] ;
    }
}

void prochaine_position(int lesX[], int lesY[], int *prochaine_position_x, int *prochaine_position_y, char direction) {
    switch (direction)
    {
    case HAUT:
        *prochaine_position_x = lesX[0] ;
        *prochaine_position_y = lesY[0] - 1 ;
        break;
    
    case BAS:
        *prochaine_position_x = lesX[0] ;
        *prochaine_position_y = lesY[0] + 1 ;   
        break;
    
    case GAUCHE:
        *prochaine_position_x = lesX[0] -1;
        *prochaine_position_y = lesY[0] ;
        break;

    case DROITE:
        *prochaine_position_x = lesX[0] + 1;
        *prochaine_position_y = lesY[0] ;

        break;
    }
}

void prochaine_position2(int lesX_2[], int lesY_2[], int *prochaine_position_x2, int *prochaine_position_y2, char direction2) {
    switch (direction2)
    {
    case HAUT:
        *prochaine_position_x2 = lesX_2[0] ;
        *prochaine_position_y2 = lesY_2[0] - 1 ;
        break;
    
    case BAS:
        *prochaine_position_x2 = lesX_2[0] ;
        *prochaine_position_y2 = lesY_2[0] + 1 ;   
        break;
    
    case GAUCHE:
        *prochaine_position_x2 = lesX_2[0] -1;
        *prochaine_position_y2 = lesY_2[0] ;
        break;

    case DROITE:
        *prochaine_position_x2 = lesX_2[0] + 1;
        *prochaine_position_y2 = lesY_2[0] ;

        break;
    }
}

/************************************************/
/*               FONCTIONS UTILITAIRES          */
/************************************************/
void gotoxy(int x, int y) { 
    printf("\033[%d;%df", y, x);
}

int kbhit(){
    // la fonction retourne :
    // 1 si un caractere est present
    // 0 si pas de caractere présent
    int unCaractere=0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    // mettre le terminal en mode non bloquant
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
    ch = getchar();

    // restaurer le mode du terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
 
    if(ch != EOF){
        ungetc(ch, stdin);
        unCaractere=1;
    } 
    return unCaractere;
}

// Fonction pour désactiver l'echo
void disable_echo() {
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Désactiver le flag ECHO
    tty.c_lflag &= ~ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

// Fonction pour réactiver l'echo
void enable_echo() {
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Réactiver le flag ECHO
    tty.c_lflag |= ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

int Modulo(int x,int N){
    return (x % N + N) %N;
}
