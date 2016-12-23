#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <utility>
#include <stdlib.h>   
#include <time.h>
#include <random>
#include <ncurses.h>
#include <iomanip>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;


// rm main.out; g++ -std=c++11  main.cpp -o main.out -Wall -ltinfo -lncurses;  ./main.out
//TODO : Bug options changeemnt difficulté

const string KReset("0");
const string KNoir("30");
const string KRouge("31");
const string KVert("32");
const string KJaune("33");
const string KBleu("34");
const string KMagenta("35");
const string KCyan("36");

typedef vector <char> CVLine;
typedef vector <CVLine> CMatrix;

typedef struct {
	unsigned m_X;
	unsigned m_Y;
	unsigned m_sizeX;
	unsigned m_sizeY;
	char m_token;
} SPlayer;

typedef struct {
	unsigned m_X;
	unsigned m_Y;
	char m_token;
} SObstacle;

typedef struct {
	unsigned m_X;
	unsigned m_Y;
	unsigned m_sizeX;
	unsigned m_sizeY;
	char m_token;
} SBonus;


namespace {
	/* VARIABLES DE CONFIGURATION */
	struct winsize size;

	vector <SObstacle> VObstacle;
	vector <string> VOptionsName;
	vector <string> VOptionValue;

	unsigned KSizeY;
	unsigned KSizeX;
	unsigned KDelay;
	unsigned KDifficult;
	char KBonusY;
	char KBonusX;
	char KBonusZ;
	char KLeft;
	char KTop;
	char KBot;
	char KRight;
	char KEmpty;
	char KObstacle;
	char KBorderLine;
	char KBorderColumn;
	char KFirstPlayer;
	char KSecondPlayer;

	SPlayer FirstPlayer;
	SPlayer SecondPlayer;


	//SCREEN 
	unsigned GetTourMax() {
		unsigned nbrnds;
		cout << "Entrez le nombre de rounds" << endl;
		cin >> nbrnds;
		return nbrnds;
	}

	void Couleur(const string & coul) {
		cout << "\033[" << coul << "m";

	}

	void ClearScreen() {
		cout << "\033[H\033[2J";
	}

	int Rand(int min, int max) {
		random_device randm;
		mt19937 rng(randm());
		uniform_int_distribution<int> uni(min, max);
		return uni(rng);
	}

	void InitCurses() {
		raw();
		keypad(stdscr, TRUE);
		noecho();
	}

	// OPTIONS

	void InitOptions() {

		/*
		All configs go here (InitOptions()) AND in header part (simple declaration) AND in SetConfig()
		All DEFAULT values go here
		*/

		KSizeX = 10;
		KSizeY = 10;
		KDelay = 10;
		KDifficult = 1;
		KBonusX = 'U';
		KBonusY = 'V';
		KBonusZ = 'W';
		KLeft = 'q';
		KTop = 'z';
		KBot = 's';
		KRight = 'd';
		KEmpty = '.';
		KObstacle = '#';
		KBorderLine = '#';
		KBorderColumn = '#';
		KFirstPlayer = 'X';
		KSecondPlayer = 'O';


		VOptionsName = { "KEmpty (background)", "KObstacle (token)", "KBorderLine (token)", "KBorderColumn (token)", "KFirstPlayer (token)", "KSecondPlayer (token)", "KSizeX", "KSizeY", "KDelay (seconds)", "KDifficult (easy: 1 | difficult: 2)" };
		VOptionValue = { ".", "#" ,"#","#", "X" ,"O", "10", "10", "10", "2" };

	}

	void SetConfig(string Name, const string &Value) {

		vector<char> valuetochar(Value.c_str(), Value.c_str() + Value.size() + 1u);

		for (unsigned i(0); i < VOptionsName.size(); ++i)
			if (VOptionsName[i] == Name) VOptionValue[i] = Value;


		if ("KLeft" == Name) KLeft = valuetochar[0];
		else if ("KTop" == Name) KTop = valuetochar[0];
		else if ("KBot" == Name) KBot = valuetochar[0];
		else if ("KRight" == Name) KRight = valuetochar[0];
		else if ("KObstacle" == Name) KObstacle = valuetochar[0];
		else if ("KBorderLine" == Name) KBorderLine = valuetochar[0];
		else if ("KBorderColumn" == Name) KBorderColumn = valuetochar[0];
		else if ("KFirstPlayer" == Name) KFirstPlayer = valuetochar[0];
		else if ("KEmpty" == Name) KEmpty = valuetochar[0];
		else if ("KSecondPlayer" == Name) KSecondPlayer = valuetochar[0];
		else if ("KSizeX" == Name) KSizeX = stoul(Value);
		else if ("KSizeY" == Name) KSizeY = stoul(Value);
		else if ("KDelay" == Name) KDelay = stoul(Value);
		else if ("KDifficult" == Name) KDifficult = stoul(Value);

		/*if ((!(KBorderLine == KBorderColumn)) || (!(KBorderLine == KObstacle)) || (!(KBorderColumn == KObstacle))) {
			Couleur(KRouge);
			cout << "[!] KBorderLine, KObstacle et KBorderColumn doivent avoir le même caractère" << endl;
			Couleur(KReset);
		}  // Semble ne peut être dérangeant ? */

	}
	// MATRICE

	CMatrix InitMat(unsigned NbLine, unsigned NbColumn, SPlayer & FirstPlayer, SPlayer & SecondPlayer, bool ShowBorder = true) {
		CMatrix mat;
		mat.resize(NbLine);

		for (unsigned i(0); i < NbLine; ++i)
			for (unsigned j(0); j < NbColumn; ++j)
				mat[i].push_back(KEmpty);



		for (unsigned i(FirstPlayer.m_Y); i < FirstPlayer.m_Y + FirstPlayer.m_sizeY; ++i)
			for (unsigned j(FirstPlayer.m_X); j < FirstPlayer.m_X + FirstPlayer.m_sizeX; ++j)
				mat[i][j] = FirstPlayer.m_token;


		for (unsigned i(SecondPlayer.m_Y); i < SecondPlayer.m_Y + SecondPlayer.m_sizeY; ++i)
			for (unsigned j(SecondPlayer.m_X); j < SecondPlayer.m_X + SecondPlayer.m_sizeX; ++j)
				mat[i][j] = SecondPlayer.m_token;

		if (ShowBorder) {
			for (unsigned i(0); i < NbLine; ++i) {

				mat[i][0] = KBorderColumn;
				mat[i][NbColumn - 1] = KBorderColumn;
			}

			for (unsigned i(0); i < NbColumn; ++i) {
				mat[0][i] = KBorderLine;
				mat[NbLine - 1][i] = KBorderLine;
			}

		}


		return mat;
	}

	void ShowMatrix(const CMatrix & Mat) {

		ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

		ClearScreen();
		cout << endl << endl;
		Couleur(KReset);

		for (unsigned i(0); i < Mat.size(); ++i) {
			cout << setw(round(size.ws_col / 2) - Mat.size());

			for (unsigned a(0); a < Mat[i].size(); ++a) {

				/*GESTION DES COULEURS*/

				if (Mat[i][a] == KObstacle) Couleur(KNoir);
				if (Mat[i][a] == KBonusX) Couleur(KVert);
				if (Mat[i][a] == KBonusY) Couleur(KVert);
				if (Mat[i][a] == KBonusZ) Couleur(KVert);
				if (Mat[i][a] == KFirstPlayer) Couleur(KRouge);
				if (Mat[i][a] == KSecondPlayer)	Couleur(KBleu);
				if (Mat[i][a] == KEmpty) Couleur(KMagenta);

				cout << Mat[i][a];
				Couleur(KReset);

			}
			cout << endl;
		}
	}

	// WIN CHECK - WIN STAT

	bool CheckIfWin(SPlayer & FirstPlayer, SPlayer & SecondPlayer) {
		return !((FirstPlayer.m_X > SecondPlayer.m_X + SecondPlayer.m_sizeX - 1) ||
			(FirstPlayer.m_X + FirstPlayer.m_sizeX - 1 < SecondPlayer.m_X) ||
			(SecondPlayer.m_Y > FirstPlayer.m_Y + FirstPlayer.m_sizeY - 1) ||
			(SecondPlayer.m_Y + SecondPlayer.m_sizeY - 1 < FirstPlayer.m_Y));
	}

	char GetWinner(SPlayer& FirstPlayer, SPlayer &SecondPlayer, const unsigned & NbrTour) {
		return (NbrTour % 2 == 0 ? FirstPlayer.m_token : SecondPlayer.m_token);
	}


	// BONUS

	SBonus InitBonus(const unsigned largeur, const unsigned hauteur, const unsigned
		AxeX, const unsigned AxeY, const char Token) {

		SBonus bonus;

		bonus.m_sizeX = largeur;
		bonus.m_sizeY = hauteur;
		bonus.m_X = AxeX;
		bonus.m_Y = AxeY;
		bonus.m_token = Token;

		return bonus;
	}

	void PutBonus(CMatrix & Matrice, SBonus & Bonus) {
		Matrice[Bonus.m_Y][Bonus.m_X] = Bonus.m_token;
	}

	void GetBonus(CMatrix & Mat, SPlayer & Player) {
		for (unsigned i(Player.m_Y); i < Player.m_Y + Player.m_sizeY; ++i) {
			for (unsigned j(Player.m_X); j < Player.m_X + Player.m_sizeX; ++j) {
				//Début de la détéction des bonus

				if (Mat[i][j] == KBonusX) {

					++Player.m_sizeX;
					++Player.m_sizeY;

				}
				if (Mat[i][j] == KBonusY) {

					++Player.m_sizeX;
					++Player.m_sizeY;

				}
				if (Mat[i][j] == KBonusZ) {

					++Player.m_sizeX;
					++Player.m_sizeY;

				}
				//Fin de la détéction des bonus
				for (unsigned i(Player.m_Y); i < Player.m_Y + Player.m_sizeY; ++i)
					for (unsigned j(Player.m_X); j < Player.m_X + Player.m_sizeX; ++j)
						Mat[i][j] = Player.m_token;
			}
		}
	}


	// OBSTACLES
	bool IsSurrounded(CMatrix & Map, SPlayer & Player) {
		return (Map[Player.m_X + 1][Player.m_Y + 1] == KObstacle);
	}

	SObstacle InitObstacle(const unsigned
		AxeX, const unsigned AxeY, const char Token) {

		SObstacle obstacle;

		obstacle.m_X = AxeX;
		obstacle.m_Y = AxeY;
		obstacle.m_token = Token;
		return obstacle;
	}

	void PutObstacle(CMatrix & Matrice, SObstacle & Obstacle) {

		VObstacle.push_back(Obstacle);
		Matrice[Obstacle.m_Y][Obstacle.m_X] = Obstacle.m_token;

	}

	void GenerateRandomObstacles(CMatrix & Matrice, const SObstacle & Obstacle, const unsigned & SizeObs) {
		SObstacle obstacle = Obstacle;

		vector <unsigned> randomValues;

		for (unsigned i(0); i < SizeObs; ++i) randomValues.push_back(Rand(1, 2));


		for (unsigned i(0); i < SizeObs; ++i) {

			switch (randomValues[i])
			{
			case 1:
				++obstacle.m_Y;
				PutObstacle(Matrice, obstacle);

			case 2:
				++obstacle.m_X;
				PutObstacle(Matrice, obstacle);

			}
		}
	}

	bool IsMovementAllowed(const SPlayer & Player, char & Movement) {

		for (unsigned i(0); i < VObstacle.size(); ++i) {
			if (VObstacle[i].m_X == Player.m_X  &&
				VObstacle[i].m_Y == Player.m_Y - 1 && Movement == KTop) return true;
			else if (VObstacle[i].m_X == Player.m_X  &&
				VObstacle[i].m_Y == Player.m_Y + 1 && Movement == KBot) return true;

			else if (VObstacle[i].m_X == Player.m_X - 1 &&
				VObstacle[i].m_Y == Player.m_Y  && Movement == KLeft) return true;

			else if (VObstacle[i].m_X == Player.m_X + 1 &&
				VObstacle[i].m_Y == Player.m_Y  && Movement == KRight) return true;
		}
		return false;
	}

	void GenerateStaticObject(CMatrix & Map, const unsigned Difficulty) {

		SObstacle tmpObs;
		SBonus tmpBonus;
		int rndBX, rndBY;

		unsigned nbobs, nbonus;
		unsigned choix = Rand(1, 3);

		switch (Difficulty)
		{
		case 1: /*EASY*/
			nbobs = Rand(1, 3);
			nbonus = Rand(2, 4);
			for (unsigned i(0); i < nbobs; ++i) {

				int minX = round((KSizeX / 2)) - Rand(1, 3);
				int maxX = round((KSizeX / 2)) + Rand(1, 2);
				int minY = round((KSizeY / 2)) - Rand(3, 4);
				int maxY = round((KSizeY / 2)) + Rand(2, 3);

				int rndX = round(Rand(minX, maxX));
				int rndY = round(Rand(minY, maxY));

				tmpObs = InitObstacle(rndX, rndY, KObstacle);

				if ((FirstPlayer.m_X == tmpObs.m_X && FirstPlayer.m_Y == tmpObs.m_Y)
					&& SecondPlayer.m_X == tmpObs.m_X && SecondPlayer.m_Y == tmpObs.m_Y) {
					--i;
					continue;
				}

				else {
					if (!(IsSurrounded(Map, FirstPlayer)) || !(IsSurrounded(Map, SecondPlayer)))
						GenerateRandomObstacles(Map, tmpObs, Rand(2, 3));
				}
			}

			for (unsigned i(0); i < nbonus; ++i) {
				rndBX = Rand(round(KSizeX - round(KSizeX / 3)) - Rand(1, 3), round(KSizeX - round(KSizeX / 5)) + Rand(1, 3));
				rndBY = Rand(round(KSizeY - round(KSizeY / 3)) - Rand(1, 3), round(KSizeY - round(KSizeY / 5)) + Rand(1, 3));

				if (choix == 1) tmpBonus = InitBonus(1, 1, rndBX - 1, rndBY, KBonusY);
				else if (choix == 2) tmpBonus = InitBonus(1, 1, rndBX - 1, rndBY, KBonusX);
				else if (choix == 3) tmpBonus = InitBonus(1, 1, rndBX - 1, rndBY, KBonusZ);

				PutBonus(Map, tmpBonus);
			}

		case 2: /*HARD*/
			nbobs = Rand(5, 6);

			for (unsigned i(0); i < nbobs; ++i) {

				int minX = round((KSizeX / 2)) - Rand(1, 3);
				int maxX = round((KSizeX / 2)) + Rand(1, 2);
				int minY = round((KSizeY / 2)) - Rand(3, 4);
				int maxY = round((KSizeY / 2)) + Rand(2, 3);

				int rndX = round(Rand(minX, maxX));
				int rndY = round(Rand(minY, maxY));

				tmpObs = InitObstacle(rndX, rndY, KObstacle);


				if ((FirstPlayer.m_X == tmpObs.m_X && FirstPlayer.m_Y == tmpObs.m_Y)
					&& SecondPlayer.m_X == tmpObs.m_X && SecondPlayer.m_Y == tmpObs.m_Y) {
					--i;
					continue;
				}

				else {
					if (!(IsSurrounded(Map, FirstPlayer)) || !(IsSurrounded(Map, SecondPlayer)))
						GenerateRandomObstacles(Map, tmpObs, Rand(2, 3));
				}
			}

			unsigned choix = Rand(1, 3);

			rndBX = Rand(round(KSizeX - round(KSizeX / 3)) - Rand(1, 3), round(KSizeX - round(KSizeX / 5)) + Rand(1, 3));
			rndBY = Rand(round(KSizeY - round(KSizeY / 3)) - Rand(1, 3), round(KSizeY - round(KSizeY / 5)) + Rand(1, 3));

			if (choix == 1) tmpBonus = InitBonus(1, 1, rndBX - 1, rndBY, KBonusY);
			else if (choix == 2) tmpBonus = InitBonus(1, 1, rndBX - 1, rndBY, KBonusX);
			else if (choix == 3) tmpBonus = InitBonus(1, 1, rndBX - 1, rndBY, KBonusZ);

			PutBonus(Map, tmpBonus);
		}





		/*	if (1 == Difficulty) {

				SObstacle FirstObstacle = InitObstacle(2, 5, KObstacle);
				GenerateRandomObstacles(Map, FirstObstacle, 4);

				SBonus BonusOne = InitBonus(1, 1, Rand(round(KSizeX / 3), round(KSizeX / 3) + 2), Rand(round(KSizeX / 3), round(KSizeX / 3) + 2), KBonusX);
				PutBonus(Map, BonusOne);

				SBonus BonusTwo = InitBonus(1, 1, 4, 4, KBonusY);
				PutBonus(Map, BonusTwo);

				SBonus BonusThree = InitBonus(1, 1, Rand(round(KSizeX / 4), round(KSizeX / 2) + 1), Rand(round(KSizeX / 4), round(KSizeX / 2) + 2), KBonusY);
				PutBonus(Map, BonusThree);
			}

			else if (2 == Difficulty) {


				SObstacle ObsOne = InitObstacle(Rand(2, round(KSizeX / 2)), Rand(2, round(KSizeX / 2) + 4), KObstacle);
				GenerateRandomObstacles(Map, ObsOne, Rand(3, 4));

				SObstacle ObsTwo = InitObstacle(Rand(3, round(KSizeX / 3) + 1), Rand(1, round(KSizeX / 2) + 2), KObstacle);
				GenerateRandomObstacles(Map, ObsTwo, Rand(2, 5));

				SObstacle ObsThree = InitObstacle(KSizeX - Rand(1, 3), round(KSizeX / 2) + Rand(0, 2), KObstacle);
				GenerateRandomObstacles(Map, ObsThree, 3);

				SBonus BonusOne = InitBonus(1, 1, Rand(round(KSizeX / 3), round(KSizeX / 3) + 2), Rand(round(KSizeX / 3), round(KSizeX / 3) + 2), KBonusX);
				PutBonus(Map, BonusOne);

				SBonus BonusTwo = InitBonus(1, 1, Rand(round(KSizeX / 4), round(KSizeX / 2) + 1), Rand(round(KSizeX / 4), round(KSizeX / 2) + 2), KBonusY);
				PutBonus(Map, BonusTwo);
				}
		*/
	}

	// PLAYERS

	SPlayer InitPlayer(const unsigned  largeur, const unsigned  hauteur, const unsigned  AxeX, const unsigned  AxeY, const char  Token) {

		SPlayer player;

		player.m_sizeX = largeur;
		player.m_sizeY = hauteur;
		player.m_X = AxeX;
		player.m_Y = AxeY;
		player.m_token = Token;

		return player;
	}

	void MovePlayer(CMatrix & Mat, char Move, SPlayer & player) {

		if (IsMovementAllowed(player, Move)) return;


		if (Move == KTop) {
			if (player.m_Y > 1) //Verifie les bordures
			{
				player.m_Y -= 1;

				GetBonus(Mat, player);
				for (unsigned i(player.m_X); i < player.m_X + player.m_sizeX; ++i) {
					Mat[player.m_Y + player.m_sizeY][i] = KEmpty;
					Mat[player.m_Y][i] = player.m_token;

				}
			}
		}


		else if (Move == KBot) {

			if (player.m_Y + player.m_sizeY < Mat.size() - 1) {
				player.m_Y += 1;
				GetBonus(Mat, player);

				for (unsigned i(player.m_X); i < player.m_X + player.m_sizeX; ++i) {
					Mat[player.m_Y - 1][i] = KEmpty;
					Mat[player.m_Y + player.m_sizeY - 1][i] = player.m_token;

				}
			}
		}

		else if (Move == KLeft) {
			if (player.m_X > 1)
			{
				player.m_X -= 1;
				GetBonus(Mat, player);

				for (unsigned i(player.m_Y); i < player.m_Y + player.m_sizeY; ++i) {
					Mat[i][player.m_X + player.m_sizeX] = KEmpty;
					Mat[i][player.m_X] = player.m_token;
				}
			}
		}

		else if (Move == KRight) {
			if (player.m_X + player.m_sizeX < Mat[0].size() - 1)
			{
				player.m_X += 1;
				GetBonus(Mat, player);

				for (unsigned i(player.m_Y); i < player.m_Y + player.m_sizeY; ++i) {
					Mat[i][player.m_X - 1] = KEmpty;
					Mat[i][player.m_X + player.m_sizeX - 1] = player.m_token;
				}
			}
		}
	}

	// DELAY

	void PrintDelay() {
		for (unsigned i(0); i < 1000; ++i) {
			cout << i;
		}
	}

	//DISPLAYS

	void DisplayMenu();

	void DisplayMulti() {

		unsigned NbRnds = GetTourMax();


		FirstPlayer = InitPlayer(1, 1, 1, 1, KFirstPlayer);
		SecondPlayer = InitPlayer(1, 1, KSizeX - 1, KSizeY - 1, KSecondPlayer);
		CMatrix Map = InitMat(KSizeX + 1, KSizeY + 1, FirstPlayer, SecondPlayer); // +1 due à la bordure de '#' le long de la matrice

		GenerateStaticObject(Map, KDifficult);

		initscr();
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

		int ch;
		ShowMatrix(Map);

		for (unsigned i(0); i < NbRnds * 2; ++i)
		{

			SPlayer &actualplayer = (i % 2 == 0 ? FirstPlayer : SecondPlayer);
			ShowMatrix(Map);
			InitCurses();

			Couleur(KVert);
			cout << endl << "[+] Difficulté : " << KDifficult << flush << endl << '\r';

			Couleur(KCyan);
			cout << endl << "[+] Taille : (" << KSizeX << ", " << KSizeY << ')' << endl << '\r';

			Couleur(KReset);

			cout << endl << "[!] Regles :" << endl << '\r' << setw(5) << "Les bonus représentés par des " << KBonusX
				<< ", " << KBonusY << ", " << KBonusZ << " ont différentes propriétés."
				<< endl << setw(5) << "\rA vous de les découvrir !" << endl << '\r';


			Couleur(KMagenta);
			cout << endl << "[?] Au tour de "; Couleur(KBleu); cout << actualplayer.m_token << endl << '\r';



			Couleur(KReset);
			if (!(i == 0)) ch = getch();

			if (ch == KTop) MovePlayer(Map, KTop, actualplayer);
			else if (ch == KBot) MovePlayer(Map, KBot, actualplayer);
			else if (ch == KLeft) MovePlayer(Map, KLeft, actualplayer);
			else if (ch == KRight) MovePlayer(Map, KRight, actualplayer);
			else if (ch == char(3)/*CTRL+C*/) {
				endwin();
				exit(0);
			}

			if (CheckIfWin(FirstPlayer, SecondPlayer)) {

				Couleur(KRouge);
				cout << endl << "[!] Le joueur '" << GetWinner(FirstPlayer, SecondPlayer, i) << "' a gagné ! " << endl << '\r';
				Couleur(KReset);
				endwin();
				return;
			}

			refresh();
			endwin();

			Couleur(KBleu);
			cout << "[!] Egalité !" << endl;
			Couleur(KReset);
			endwin();

		}
	}

	void DisplayOption() {

		ClearScreen();
		const vector <string> optiontitle = {
			" _____   _____   _____   _   _____   __   _   _____  ",
			"/  _  \\ |  _  \\ |_   _| | | /  _  \\ |  \\ | | /  ___/ ",
			"| | | | | |_| |   | |   | | | | | | |   \\| | | |___  ",
			"| | | | |  ___/   | |   | | | | | | | |\\   | \\___  \\ ",
			"| |_| | | |       | |   | | | |_| | | | \\  |  ___| | ",
			"\\_____/ |_|       |_|   |_| \\_____/ |_|  \\_| /_____/ ",
			"\n"
		};
		for (string Lines : optiontitle) cout << ' ' << Lines << endl;

		cout << "Liste des options par defaut : " << endl;

		for (unsigned i(0); i < VOptionsName.size(); ++i) cout << i << ". " << VOptionsName[i] << " : '" << VOptionValue[i] << '\'' << endl;

		Couleur(KRouge);

		cout << endl << "[!] Attention, ces configurations ne seront présentes jusqu'à la fermeture complète du jeu." << endl;

		Couleur(KVert);

		cout << endl << "[?] Voulez-vous modifier une option ? [Y/n] ";

		char choix;
		unsigned numero;
		string newparam;

		cin >> choix;

		if (choix == 'y') {

			cout << endl << "[+] Afin de modifier un paramètre, veuillez entrer le numéro correspondant : ";
			cin >> numero;
			cout << VOptionsName[numero] << " deviendra : ";
			cin >> newparam;
			SetConfig(VOptionsName[numero], newparam);
			Couleur(KReset);

		}
		else exit(0);

		DisplayMenu();

		Couleur(KReset);
	}

	void DisplaySolo() {

		//TODO

	}

	void DisplayMenu() {
		ClearScreen();

		vector <string> menulist = { "Singleplayer (IA)", "Multiplayer", "Options", "Exit" };

		unsigned choice(0);

		const vector <string>  menutitle = {
			"     ___  ___   _____   __   _   _   _  ",
			"    /   |/   | | ____| |  \\ | | | | | | ",
			"   / /|   /| | | |__   |   \\| | | | | | ",
			"  / / |__/ | | |  __|  | |\\   | | | | | ",
			" / /       | | | |___  | | \\  | | |_| | ",
			"/_/        |_| |_____| |_|  \\_| \\_____/ ",
			"\n"
		};

		for (string line : menutitle) cout << ' ' << line << endl;

		for (string option : menulist) {
			++choice;

			cout << ' ';
			Couleur(KRouge);
			cout << "[" << choice << "] ";
			Couleur(KReset);
			cout << option << endl << endl;

		}
		cout << endl << "Choose between [1-" << menulist.size() << "] : ";

		unsigned input;
		cin >> input;

		switch (input) {
		case 1:
			DisplaySolo();
			break;
		case 2:
			DisplayMulti();
			break;

		case 3:
			DisplayOption();
			break;

		case 4:
			exit(0);

		default:

			Couleur(KRouge);
			cout << "[!] Choix invalide !" << endl;
			Couleur(KReset);
		}
	}

}

int main() {

	InitOptions();
	DisplayMenu();

	endwin();
	Couleur(KReset);

	return 0;
}
