#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>



//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                      MACROS                                          //
//                                                                                      //
//--------------------------------------------------------------------------------------//

// ---- Plateau --------------------------------------------------------------------------

// dimensions du plateau
#define LARGEUR_PLATEAU 80	
#define HAUTEUR_PLATEAU 40

// nombre de sortie
#define NB_SORTIES 4

// ---- Serpent et jeu ------------------------------------------------------------------

// taille du serpent
#define TAILLE 10

// position initiale de la tête du serpent
#define X_INITIAL 40
#define Y_INITIAL 20

// temporisation entre deux déplacements du serpent (en microsecondes)
#define ATTENTE 100000

// nombre de pommes à manger pour gagner
#define NB_POMMES 10


// ---- Touches -------------------------------------------------------------------------

// touches de direction
#define NB_DIRECTION 4
#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'

// Touche d'arrêt du jeu
#define STOP 'a'


// ---- Apparences --------------------------------------------------------

// caractères pour représenter le serpent
#define CORPS 'X'
#define TETE 'O'

// caractères pour les éléments du plateau
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'



//--------------------------------------------------------------------------------------//
//                                                                                      //
//                             TYPES ET VARIABLES GLOBALES                              //
//                                                                                      //
//--------------------------------------------------------------------------------------//

// type du plateau
typedef char Plateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1];

// coordonnées prédéfinies des pommes (taille <= NB_POMMES)
int coords_pomme[NB_POMMES][2] = 
{
    {75, 8},
    {75, 39},
    {78, 2},
    {2, 2},
    {8, 5},
    {78, 39},
    {74, 33},
    {2, 38},
    {72, 35},
    {5, 2}
};

// coordonnées prédéfinies des sorties (taille = NB_SORTIES)
int sortie_x[] = {LARGEUR_PLATEAU/2, LARGEUR_PLATEAU, LARGEUR_PLATEAU/2, 1};
int sortie_y[] = {1, HAUTEUR_PLATEAU/2, HAUTEUR_PLATEAU, HAUTEUR_PLATEAU/2};

// variables de statistiques d'exécution du programme
float temps_cpu = 0.0;
int nb_mouvements = 0;





//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                      PROTOTYPES                                      //
//                                                                                      //
//--------------------------------------------------------------------------------------//

void CalculChemin(int *objectif_x, int *objectif_y, int pos_serpent_x, int pos_serpent_y, int pos_pomme_x, int pos_pomme_y);
void CalculDirection(int les_x[], int les_y[], int pos_flag_x, int pos_flag_y, char *direction, Plateau plateau);

void Progresser(int les_x[], int les_y[], char direction, Plateau plateau, bool *collision, bool *pomme);
void InitPlateau(Plateau plateau);
void AjouterPomme(Plateau plateau);
void DessinerPlateau(Plateau plateau);
void DessinerSerpent(int les_x[], int les_y[]);
char CollisionsSerpent(int pos_tete_x, int pos_tete_y, int les_x[], int les_y[], char direction, Plateau plateau);
void CalculPosProchaine(int *pos_prochaine_x, int *pos_prochaine_y, int les_x[], int les_y[], char direction);

void GotoXY(int x, int y);
int Kbhit();
void DisableEcho();
void EnableEcho();
void Afficher(int x, int y, char car);
void Effacer(int x, int y);
int Modulo(int x,int N);





//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                 FONCTION PRINCIPALE                                  //
//                                                                                      //
//--------------------------------------------------------------------------------------//

int main(){
	// initialisation du la graine de génération de l'aléatoire
	srand(time(NULL));
	// désactivation de l'écho du terminal
	DisableEcho();


	// Initialisations serpent / plateau ------------------------------------------------
	int les_x[TAILLE];
	int les_y[TAILLE];

	for(int i=0 ; i<TAILLE ; i++){
		les_x[i] = X_INITIAL-i;
		les_y[i] = Y_INITIAL;
	}

	Plateau plateau;

	InitPlateau(plateau);

	// ajout de la pomme initiale
	AjouterPomme(plateau);


	// Premier dessin du jeu ------------------------------------------------------------
	system("clear");
	DessinerPlateau(plateau);
	DessinerSerpent(les_x, les_y);


	// Initialisations des variables de la boucle principale de jeu ---------------------
	char touche;
	char direction = DROITE;

	// variables booléennes de gestion de lexécution du jeu
	bool collision = false;
	bool gagne = false;
	bool pomme_mangee = false;

	// compteur de pommes mangées
	int nb_pommes = 0;


	// Init partie automatisation #######################################################
	int objectif_x;
	int objectif_y;

	clock_t start;
	clock_t end;

	// calcul de l'objectif initial
	CalculChemin(&objectif_x, &objectif_y, les_x[0], les_y[0], coords_pomme[nb_pommes][0], coords_pomme[nb_pommes][1]);





	// boucle principale du jeu ---------------------------------------------------------
	do {

		// Gestion des directions #######################################################
		start = clock();
		CalculDirection(les_x, les_y, objectif_x, objectif_y, &direction, plateau);
		end = clock();
		temps_cpu += (((end-start)*1.0) / CLOCKS_PER_SEC);



		// ------------------------------------------------------------------------------
		Progresser(les_x, les_y, direction, plateau, &collision, &pomme_mangee);

		if (pomme_mangee){
            nb_pommes++;
			gagne = (nb_pommes==NB_POMMES);
			if (!gagne){
				AjouterPomme(plateau);
			}	
			
		}


		// Gestion des objectifs ########################################################
		start = clock();
		if (les_x[0] == objectif_x && les_y[0] == objectif_y) {
			if (!pomme_mangee) {
				// progresse dans le cas ou l'objectif atteint n'est pas une pomme (un portail) pour atteindre l'autre côté du portail
				Progresser(les_x, les_y, direction, plateau, &collision, &pomme_mangee);
			}
			// calcul d'un nouvel objectif car l'objectif est atteint
			CalculChemin(&objectif_x, &objectif_y, les_x[0], les_y[0], coords_pomme[nb_pommes][0], coords_pomme[nb_pommes][1]);
		}
		end = clock();
		temps_cpu += (((end-start)*1.0) / CLOCKS_PER_SEC);


		// ------------------------------------------------------------------------------
		pomme_mangee = false;


		if (Kbhit()==1){
			touche = getchar();
		}

		usleep(ATTENTE);

	} while (touche != STOP && !collision && !gagne);

	GotoXY(1, HAUTEUR_PLATEAU+1);
    EnableEcho();


	// affichage des nb_mouvements et temps_cpu ##########################################
	printf("Le serpent à fait %d mouvements pour obtenir toutes les pommes\n", nb_mouvements);
	printf("Le programme à pris %f secondes du temps CPU\n", temps_cpu);


	return EXIT_SUCCESS;
}





//--------------------------------------------------------------------------------------//
//                                                                                      //
//                       FONCTIONS ET PROCEDURE D'AUTOMATISATION                        //
//                                                                                      //
//--------------------------------------------------------------------------------------//

/*
Cette procédure détermine l'objectif optimal en termes de déplacements pour atteindre la pomme ou une sortie.

Algorithme :
1. Calcule la distance Manhattan entre la tête du serpent et la pomme via un chemin direct.
2. Pour chaque sortie, calcule la distance Manhattan entre le serpent et la sortie + la sortie opposé et la pomme.
3. Compare toutes les distances calculées pour trouver le chemin nécessitant le moins de déplacements.
4. Renvoie les coordonnées de l'objectif correspondant (pomme ou sortie).
*/
void CalculChemin(int *objectif_x, int *objectif_y, int pos_serpent_x, int pos_serpent_y, int pos_pomme_x, int pos_pomme_y) {

	// initialise le minimum au nombre de déplacement du chemin direct
	int nombre_deplacement_min = abs(pos_pomme_x - pos_serpent_x) + abs(pos_pomme_y - pos_serpent_y);
	// considère la pomme comme la sortie -1
	int id_sortie_min = -1;

	// trouve le minimum de déplacement entre les chemins des NB_SORITES sorties et la pomme
	int id_sortie_oppose;
	int nombre_deplacement;
	for (int id_sortie = 0; id_sortie < NB_SORTIES; id_sortie++) {
		id_sortie_oppose = (id_sortie+(NB_SORTIES/2)) % NB_SORTIES;

		nombre_deplacement = (
			(abs(sortie_x[id_sortie] - pos_serpent_x) + abs(sortie_y[id_sortie] - pos_serpent_y))
			+ 1 +
			(abs(pos_pomme_x - sortie_x[id_sortie_oppose]) + abs(pos_pomme_y - sortie_y[id_sortie_oppose])));

		if (nombre_deplacement_min > nombre_deplacement) {
			nombre_deplacement_min = nombre_deplacement;
			id_sortie_min = id_sortie;
		}
	}

	if (id_sortie_min == -1) {
		*objectif_x = pos_pomme_x;
		*objectif_y = pos_pomme_y;
	} else {
		*objectif_x = sortie_x[id_sortie_min];
		*objectif_y = sortie_y[id_sortie_min];
	}
}

/*
Cette procédure détermine la direction que doit emprunter le serpent pour se rapprocher d'un objectif (pomme ou sortie), 
en évitant les collisions avec lui-même ou avec les obstacles. 

Algorithme :
1. Classe les directions possibles (haut, bas, gauche, droite) en fonction de leur capacité à rapprocher la tête du serpent
   de l'objectif, en priorisant l'axe X puis Y.
2. Remplit les directions restantes en complétant celles non prioritaires.
3. Vérifie chaque direction dans l'ordre du classement pour détecter une collision (avec le serpent ou d'autres obstacles).
4. Renvoie la première direction valide.
*/
void CalculDirection(int les_x[], int les_y[], int pos_flag_x, int pos_flag_y, char *direction, Plateau plateau) {
	int pos_serpent_x = les_x[0];
	int pos_serpent_y = les_y[0];

	char classement[NB_DIRECTION];
	int index_classement = 0;

    // Priorité sur l'axe X
    if (pos_flag_x > pos_serpent_x) {
        classement[index_classement++] = DROITE;
    } else if (pos_flag_x < pos_serpent_x) {
        classement[index_classement++] = GAUCHE;
    }

    // Priorité sur l'axe Y
    if (pos_flag_y > pos_serpent_y) {
        classement[index_classement++] = BAS;
    } else if (pos_flag_y < pos_serpent_y) {
        classement[index_classement++] = HAUT;
    }

    // Ajout des directions restantes (non prioritaires)
    if (index_classement < NB_DIRECTION) {
        if (pos_flag_x <= pos_serpent_x) classement[index_classement++] = DROITE;
        if (pos_flag_x >= pos_serpent_x) classement[index_classement++] = GAUCHE;
        if (pos_flag_y <= pos_serpent_y) classement[index_classement++] = BAS;
        if (pos_flag_y >= pos_serpent_y) classement[index_classement++] = HAUT;
    }

	int pos_prochaine_x;
	int pos_prochaine_y;
	index_classement = 0;
	char colision;
	do {
		CalculPosProchaine(&pos_prochaine_x, &pos_prochaine_y, les_x, les_y, classement[index_classement]);
		colision = CollisionsSerpent(pos_prochaine_x, pos_prochaine_y, les_x, les_y, classement[index_classement], plateau);
	}
	while (colision != VIDE && colision != POMME && index_classement++ < NB_DIRECTION);
	
	*direction = classement[index_classement];
}



//--------------------------------------------------------------------------------------//
//                                                                                      //
//                            FONCTIONS ET PROCEDURES DU JEU                            //
//                                                                                      //
//--------------------------------------------------------------------------------------//

void Progresser(int les_x[], int les_y[], char direction, Plateau plateau, bool * collision, bool * pomme){
	int pos_prochaine_x;
	int pos_prochaine_y;
	CalculPosProchaine(&pos_prochaine_x, &pos_prochaine_y, les_x, les_y, direction);
    char colision = CollisionsSerpent(pos_prochaine_x, pos_prochaine_y, les_x, les_y, direction, plateau);


	Effacer(les_x[TAILLE-1], les_y[TAILLE-1]);
	for(int i=TAILLE-1 ; i>0 ; i--){
		les_x[i] = les_x[i-1];
		les_y[i] = les_y[i-1];
	}
	les_x[0] = pos_prochaine_x;
	les_y[0] = pos_prochaine_y;


	*pomme = false;
	// détection d'une "collision" avec une pomme
	if (colision == POMME){
		*pomme = true;
		// la pomme disparait du plateau
		plateau[les_x[0]][les_y[0]] = VIDE;
	}
	// détection d'une collision avec la bordure ou le serpent
	else if (colision == BORDURE || colision == CORPS){
		*collision = true;
	}

   	DessinerSerpent(les_x, les_y);

	nb_mouvements++;
}

void InitPlateau(Plateau plateau){
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
	for (int i = 0; i < NB_SORTIES; i++) {
		plateau[sortie_x[i]][sortie_y[i]] = VIDE;
	}
}

void AjouterPomme(Plateau plateau){
    static int index = 0;
    if (index < NB_POMMES) {

        plateau[coords_pomme[index][0]][coords_pomme[index][1]] = POMME;
        Afficher(coords_pomme[index][0], coords_pomme[index][1], POMME);
        index++;
    } 
}

void DessinerPlateau(Plateau plateau){
	// affiche eà l'écran le contenu du tableau 2D représentant le plateau
	for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
		for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
			Afficher(i, j, plateau[i][j]);
		}
	}
}

void DessinerSerpent(int les_x[], int les_y[]){
	// affiche les anneaux puis la tête
	for(int i=1 ; i<TAILLE ; i++){
		Afficher(les_x[i], les_y[i], CORPS);
	}
	Afficher(les_x[0], les_y[0],TETE);
}

char CollisionsSerpent(int pos_tete_x, int pos_tete_y, int les_x[], int les_y[], char direction, Plateau plateau) {
	for (int i = 0; i < TAILLE-1; i++) {
		if (pos_tete_x == les_x[i] && pos_tete_y == les_y[i]) {
			return CORPS;
		}
	}
	return plateau[pos_tete_x][pos_tete_y];
}

void CalculPosProchaine(int *pos_prochaine_x, int *pos_prochaine_y, int les_x[], int les_y[], char direction) {
	switch(direction){
		case HAUT : 
			*pos_prochaine_y = les_y[0] - 1;
			*pos_prochaine_x = les_x[0];
			break;
		case BAS:
			*pos_prochaine_y = les_y[0] + 1;
			*pos_prochaine_x = les_x[0];
			break;
		case DROITE:
			*pos_prochaine_y = les_y[0];
			*pos_prochaine_x = les_x[0] + 1;
			break;
		case GAUCHE:
			*pos_prochaine_y = les_y[0];
			*pos_prochaine_x = les_x[0] - 1;
			break;
	}

    // Oblige le serpent à être dans les limites du plateau lorsqu'il en sort
    *pos_prochaine_x = Modulo(*pos_prochaine_x-1, LARGEUR_PLATEAU) +1;
    *pos_prochaine_y = Modulo(*pos_prochaine_y-1, HAUTEUR_PLATEAU) +1;
}



//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                FONCTIONS UTILITAIRES                                 //
//                                                                                      //
//--------------------------------------------------------------------------------------//

void GotoXY(int x, int y) { 
    printf("\033[%d;%df", y, x);
}

int Kbhit(){
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
void DisableEcho() {
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
void EnableEcho() {
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

void Afficher(int x, int y, char car){
	GotoXY(x, y);
	printf("%c", car);
	GotoXY(1,1);
}

void Effacer(int x, int y){
	GotoXY(x, y);
	printf(" ");
	GotoXY(1,1);
}

int Modulo(int x,int N){
    return (x % N + N) %N;
}