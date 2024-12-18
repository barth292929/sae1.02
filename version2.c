/**
*
* \brief Programme Snake
*
* \author HAMON Gabriel, GOUJON Barthélemy
*
* \version 1
*
* \date 6 Décembre 2024
*
* Ce programme est la 1ere version du jeu snake codé en C 
* dans le terminal et automatisé
*
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
// position initiale de la tête du serpent
#define X_INITIAL 40
#define Y_INITIAL 20
// nombre de pommes à manger pour gagner
#define NB_POMMES 10
// temporisation entre deux déplacements du serpent (en microsecondes)
#define ATTENTE 100000
// caractères pour représenter le serpent
#define CORPS 'X'
#define TETE 'O'
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


// définition d'un type pour le plateau : tPlateau
// Attention, pour que les indices du tableau 2D (qui commencent à 0) coincident
// avec les coordonées à l'écran (qui commencent à 1), on ajoute 1 aux dimensions
// et on neutralise la ligne 0 et la colonne 0 du tableau 2D (elles ne sont jamais
// utilisées)
typedef char tPlateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1];


void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau, int nb_pomme);
void afficher(int, int, char);
void effacer(int x, int y);
void dessinerSerpent(int lesX[], int lesY[]);
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool * collision, bool * pomme);
void calcTraj(tPlateau plateau, int lesX[], int lesY[], char *direction, int objectif_x, int objectif_y);
bool crash(int x, int y, int lesX[], int lesY[]);
void trajOpti(tPlateau plateau, int lesX[], int lesY[], int nb_pomme, char direction, int *objectif_x, int *objectif_y);
void gotoxy(int x, int y);
int  kbhit();
void disable_echo();
void enable_echo();

int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = { 8, 39, 2, 2, 5, 39, 33, 38, 35, 2};

int portail_X[] = {LARGEUR_PLATEAU/2, LARGEUR_PLATEAU, LARGEUR_PLATEAU/2, 1} ;
int portail_Y[] = {1, HAUTEUR_PLATEAU/2, HAUTEUR_PLATEAU, HAUTEUR_PLATEAU/2} ;


int main(){
	clock_t begin = clock();
	// 2 tableaux contenant les positions des éléments qui constituent le serpent
    int lesX[TAILLE];
	int lesY[TAILLE];

	// représente la touche frappée par l'utilisateur : touche de direction ou pour l'arrêt
	char touche;

	//direction courante du serpent (HAUT, BAS, GAUCHE ou DROITE)
	char direction;

	// le plateau de jeu
	tPlateau lePlateau;

	bool collision=false;
	bool gagne = false;
	bool pommeMangee = false;

	// compteur de déplacements
	int nbMove = 0;

	// compteur de pommes mangées
	int nbPommes = 0;
	int objectif_x ;
	int objectif_y ;
   
	// initialisation de la position du serpent : positionnement de la
	// tête en (X_INITIAL, Y_INITIAL), puis des anneaux à sa gauche
	for(int i=0 ; i<TAILLE ; i++){
		lesX[i] = X_INITIAL-i;
		lesY[i] = Y_INITIAL;
	}

	int x = 0 ;
	int y = 0 ;

	// mise en place du plateau
	initPlateau(lePlateau);
	system("clear");
	dessinerPlateau(lePlateau);

	ajouterPomme(lePlateau, nbPommes);

	// initialisation : le serpent se dirige vers la DROITE
	dessinerSerpent(lesX, lesY);
	disable_echo();
	direction = DROITE;
	collision = crash(x, y, lesX, lesY) ;

	// boucle de jeu. Arret si touche STOP, si collision avec une bordure ou
	// si toutes les pommes sont mangées
	trajOpti(lePlateau, lesX, lesY, nbPommes, direction, &objectif_x, &objectif_y) ;
	do 
	{
		calcTraj(lePlateau, lesX, lesY, &direction, objectif_x, objectif_y); 
		progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee);
		nbMove++;
		if (pommeMangee){
            nbPommes++;
			gagne = (nbPommes==NB_POMMES);

			if (!gagne){
				ajouterPomme(lePlateau, nbPommes);
				pommeMangee = false;
			}	
			
			if (lesX[0] == objectif_x && lesY[0] == objectif_y) {
				if (!pommeMangee) { 
					progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee);
				}
				trajOpti(lePlateau, lesX, lesY, nbPommes, direction, &objectif_x, &objectif_y) ;
			}
			
		}
		if (!gagne){
			if (!collision){
				usleep(ATTENTE);
				if (kbhit()==1){
					touche = getchar();
				}
			}
		}
	} while (touche!=STOP && !collision && !gagne);
    enable_echo();
	gotoxy(1, HAUTEUR_PLATEAU+1);
	clock_t end = clock();
	double tmpsCPU = ((end - begin)*1.0) / CLOCKS_PER_SEC;
	printf("Temps CPU = %f secondes, nombre de déplacements = %d\n",tmpsCPU, nbMove);
	return EXIT_SUCCESS;
}


/************************************************/
/*		FONCTIONS ET PROCEDURES DU JEU 			*/
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

void afficher(int x, int y, char car){
	gotoxy(x, y);
	printf("%c", car);
	gotoxy(1,1);
}

void effacer(int x, int y){
	gotoxy(x, y);
	printf(" ");
	gotoxy(1,1);
}

void dessinerSerpent(int lesX[], int lesY[]){
	// affiche les anneaux puis la tête
	for(int i=1 ; i<TAILLE ; i++){
		afficher(lesX[i], lesY[i], CORPS);
	}
	afficher(lesX[0], lesY[0],TETE);
}

void calcTraj(tPlateau plateau, int lesX[], int lesY[], char *direction, int objectif_x, int objectif_y){
    if (lesX[0] < objectif_x) {
        *direction = DROITE;
    } else if (lesX[0] > objectif_x) {
        *direction = GAUCHE;
    }
	if (lesY[0] > objectif_y) {
        *direction = HAUT;
    } else if (lesY[0] < objectif_y) {
        *direction = BAS;
    }

}



bool crash(int x, int y, int lesX[], int lesY[]){ //vérifie si il y a une collision avec le corps du serpent
	int i ;
	bool ok = false;
	for (i=1 ; i<TAILLE ; i++) {
		if (lesX[i] == x && lesY[i] == y) {
			ok = true ;
		}
	}
	return ok;
}

void trajOpti(tPlateau plateau, int lesX[], int lesY[], int nb_pomme, char direction, int *objectif_x, int *objectif_y){
	int Distance_pomme, distance_min;
	int sortie_oppose, sortie, sortie_min ;
	int distance ;

	sortie_min = -1 ;

	// calcul de la distance serpent->pomme
	Distance_pomme = (abs(lesX[0]-lesPommesX[nb_pomme]) + abs(lesY[0]-lesPommesY[nb_pomme])); //abs pour absolute

	distance_min = Distance_pomme ;

	for (sortie = 0 ; sortie < NB_DIRECTIONS ; sortie++) {
		sortie_oppose = (sortie + NB_DIRECTIONS / 2) % NB_DIRECTIONS ; //pour trouver l'opposé en deux
		distance = (abs(lesX[0] - portail_X[sortie]) + 
					abs(lesY[0] - portail_Y[sortie]) + 
					abs(lesPommesX[nb_pomme] - portail_X[sortie_oppose]) + 
					abs(lesPommesY[nb_pomme] - portail_Y[sortie_oppose])
					) ;
		if (distance_min > distance) {
			distance_min = distance ;
			sortie_min = sortie ;
		}
	}

	if (sortie_min == -1) {
		*objectif_x = lesPommesX[nb_pomme] ;
		*objectif_y = lesPommesY[nb_pomme] ;
	} else {
		*objectif_x = portail_X[sortie_min] ;
		*objectif_y = portail_Y[sortie_min] ;
	}

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

	if (lesY[0] == 1 && lesX[0] == LARGEUR_PLATEAU / 2)
    {
        lesY[0] = HAUTEUR_PLATEAU - 2;
    }
    else if (lesY[0] == HAUTEUR_PLATEAU - 1 && lesX[0] == LARGEUR_PLATEAU / 2)
    {
        lesY[0] = 2;
    }
    else if (lesX[0] == 1 && lesY[0] == HAUTEUR_PLATEAU / 2)
    {
        lesX[0] = LARGEUR_PLATEAU - 2;
    }
    else if (lesX[0] == LARGEUR_PLATEAU - 1 && lesY[0] == HAUTEUR_PLATEAU / 2)
    {
        lesX[0] = 2; 
    }
   	dessinerSerpent(lesX, lesY);
}



/************************************************/
/*				 FONCTIONS UTILITAIRES 			*/
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