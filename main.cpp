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

/*

TODO :
	- Changer le nom des variables en uppercase
	- Ajouter un timer
	- Supprimer les headers supperflus
	- Régler les problèmes de déplacement du joueur (?)
	- Utiliser les fichiers externes pour l'ASCII-Art dans les Display
	- Enlever le "K" pour les non-constantes variables
*/

//Compilation préconisée : rm main.out; g++ -std=c++11  main.cpp -o main.out -Wall -ltinfo -lncurses;  ./main.out

typedef vector <char> CVLine;
typedef vector <CVLine> CMatricerix;

typedef struct {
	unsigned m_X;
	unsigned m_Y;
	unsigned m_sizeX;
	unsigned m_sizeY;
	int m_score;
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
	string KLog;

	unsigned KSizeY;
	unsigned KSizeX;
	unsigned KDelay;
	unsigned KDifficult;

	/*CoulEURS BASIQUE*/
	const string KReset("0");
	const string KNoir("30");
	const string KRouge("31");
	const string KVert("32");
	const string KJaune("33");
	const string KBleu("34");
	const string KMagenta("35");
	const string KCyan("36");

	/*HIGHTLIGHT*/
	const string KHNoir("40");
	const string KHRouge("41");
	const string KHVert("42");
	const string KHJaune("43");
	const string KHBleu("44");
	const string KHMagenta("45");
	const string KHCyan("46");
	const string KHGris("47");

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

	bool KShowRules;

	SPlayer FirstPlayer;
	SPlayer SecondPlayer;

	void DisplayMenu();
	void DisplaySoloIA();
	void DisplayInfos(const SPlayer & player);


	//SCREEN 
	void Couleur(const string & Coul, const string Hightlight = "") {
		if (Hightlight == "") cout << "\033[" << Coul << "m";
		else cout << "\033[34m\033[" << Hightlight << "m";


	} //Couleur()

	unsigned GetTourMax() {
		unsigned Nbround;
		Couleur(KCyan);
		cout << endl << "[?] Entrez le nombre de rounds : ";
		Couleur(KReset);

		cin >> Nbround;
		return Nbround;
	} //GetTourMax()

	void ClearScreen() {
		cout << "\033[H\033[2J";
	}//ClearScreen()

	int Rand(int Min, int Max) {

		random_device Random;
		mt19937 Rng(Random());
		uniform_int_distribution<int> uni(Min, Max);

		return uni(Rng);
	}//Rand()

	void InitCurses() {
		raw();
		keypad(stdscr, TRUE);
		noecho();
	} //InitCurses()

	bool StrToBool(const string & Chaine) {
		return(Chaine == "true" ? true : false);
	} //StrToBool()

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
		KBonusX = 'W';
		KBonusY = 'V';
		KBonusZ = 'U';
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
		KShowRules = true;

		VOptionsName = { "KEmpty", "KFirstPlayer", "KSecondPlayer", "KSizeX", "KSizeY", "KDelay", "KDifficult", "KShowRules" };

		VOptionValue = { ".",  "X" ,"O", "10", "10", "10", "2", "true" };

		if (1 == KDifficult) VOptionValue[6] = "1";
		else VOptionValue[6] = "0";

	} //InitOptions()

	void SetConfig(string Name, const string &Value) {

		vector<char> Valuetochar(Value.c_str(), Value.c_str() + Value.size() + 1u);

		for (unsigned i(0); i < VOptionsName.size(); ++i)
			if (VOptionsName[i] == Name) VOptionValue[i] = Value;


		if ("KLeft" == Name) KLeft = Valuetochar[0];
		else if ("KTop" == Name) KTop = Valuetochar[0];
		else if ("KBot" == Name) KBot = Valuetochar[0];
		else if ("KRight" == Name) KRight = Valuetochar[0];
		else if ("KFirstPlayer" == Name) KFirstPlayer = Valuetochar[0];
		else if ("KEmpty" == Name) KEmpty = Valuetochar[0];
		else if ("KSecondPlayer" == Name) KSecondPlayer = Valuetochar[0];
		else if ("KSizeX" == Name) KSizeX = stoul(Value);
		else if ("KSizeY" == Name) KSizeY = stoul(Value);
		else if ("KDelay" == Name) KDelay = stoul(Value);
		else if ("KDifficult" == Name) KDifficult = stoul(Value);
		else if ("KShowRules" == Name) KShowRules = StrToBool(Value);


	} //SetConfig();


	// Matrice

	CMatricerix InitMatrice(unsigned NbLine, unsigned NbColumn, SPlayer & FirstPlayer, SPlayer & SecondPlayer, bool ShowBorder = true) {
		CMatricerix Matrice;
		Matrice.resize(NbLine);

		for (unsigned i(0); i < NbLine; ++i)
			for (unsigned j(0); j < NbColumn; ++j)
				Matrice[i].push_back(KEmpty);



		for (unsigned i(FirstPlayer.m_Y); i < FirstPlayer.m_Y + FirstPlayer.m_sizeY; ++i)
			for (unsigned j(FirstPlayer.m_X); j < FirstPlayer.m_X + FirstPlayer.m_sizeX; ++j)
				Matrice[i][j] = FirstPlayer.m_token;


		for (unsigned i(SecondPlayer.m_Y); i < SecondPlayer.m_Y + SecondPlayer.m_sizeY; ++i)
			for (unsigned j(SecondPlayer.m_X); j < SecondPlayer.m_X + SecondPlayer.m_sizeX; ++j)
				Matrice[i][j] = SecondPlayer.m_token;

		if (ShowBorder) {
			for (unsigned i(0); i < NbLine; ++i) {

				Matrice[i][0] = KBorderColumn;
				Matrice[i][NbColumn - 1] = KBorderColumn;
			}

			for (unsigned i(0); i < NbColumn; ++i) {
				Matrice[0][i] = KBorderLine;
				Matrice[NbLine - 1][i] = KBorderLine;
			}

		}


		return Matrice;
	} //InitMatrice()

	void ShowMatricerix(const CMatricerix & Matrice, const bool Clear = true) {

		ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

		if (Clear) ClearScreen();
		cout << endl << endl;
		Couleur(KReset);

		for (unsigned i(0); i < Matrice.size(); ++i) {
			cout << setw(round(size.ws_col / 2) - Matrice.size());

			for (unsigned a(0); a < Matrice[i].size(); ++a) {

				/*GESTION DES CoulEURS*/

				if (Matrice[i][a] == KObstacle) Couleur(KNoir);
				if (Matrice[i][a] == KBonusX) Couleur(KVert, KHVert);
				if (Matrice[i][a] == KBonusY) Couleur(KVert, KHVert);
				if (Matrice[i][a] == KBonusZ) Couleur(KVert, KHVert);
				if (Matrice[i][a] == KFirstPlayer) Couleur(KRouge, KHCyan);
				if (Matrice[i][a] == KSecondPlayer)	Couleur(KBleu, KHJaune);
				if (Matrice[i][a] == KEmpty) Couleur(KMagenta);

				cout << Matrice[i][a];
				Couleur(KReset);

			}
			cout << endl;
		}
	}//ShowMatricerix

	// WIN CHECK - WIN STAT

	bool CheckIfWin(SPlayer & FirstPlayer, SPlayer & SecondPlayer) {
		return !((FirstPlayer.m_X > SecondPlayer.m_X + SecondPlayer.m_sizeX - 1) ||
			(FirstPlayer.m_X + FirstPlayer.m_sizeX - 1 < SecondPlayer.m_X) ||
			(SecondPlayer.m_Y > FirstPlayer.m_Y + FirstPlayer.m_sizeY - 1) ||
			(SecondPlayer.m_Y + SecondPlayer.m_sizeY - 1 < FirstPlayer.m_Y));
	} //CheckIfWin()

	SPlayer GetWinner(const SPlayer& FirstPlayer, const SPlayer &SecondPlayer, const unsigned & NbrTour) {
		return (NbrTour % 2 == 0 ? FirstPlayer : SecondPlayer);
	} //GetWinner()

	void DisplayWin(const unsigned &Tour, const bool &IsBot = true) {

		SPlayer winner = GetWinner(FirstPlayer, SecondPlayer, Tour);
		unsigned margin(3);

		if (IsBot) {

			ClearScreen();
			cout << setw(round(size.ws_col / 2) - 13);
		}

		for (unsigned i(0); i < margin; ++i) cout << endl;
		Couleur(KRouge, KHCyan);

		cout << endl << endl << "[!] Le joueur '" << winner.m_token << "' a gagné avec : ";

		Couleur(KHBleu, KHGris); cout << winner.m_score;
		Couleur(KRouge, KHCyan); cout << " points" << '!' << endl << '\r';

		Couleur(KReset);
		endwin();
		return;
	} //DisplayWin()


	// BONUS

	SBonus InitBonus(const unsigned largeur, const unsigned hauteur, const unsigned
		AxeX, const unsigned AxeY, const char Token) {

		SBonus Bonus;

		Bonus.m_sizeX = largeur;
		Bonus.m_sizeY = hauteur;
		Bonus.m_X = AxeX;
		Bonus.m_Y = AxeY;
		Bonus.m_token = Token;

		return Bonus;
	}//InitBonus()

	void PutBonus(CMatricerix & Matrice, SBonus & Bonus) {
		if ((Bonus.m_Y > 1 && Bonus.m_X > 1) && (Bonus.m_Y < KSizeY - 1 && Bonus.m_X < KSizeX - 1))
			Matrice[Bonus.m_Y][Bonus.m_X] = Bonus.m_token;
	}//PutBonus()

	void GetBonus(CMatricerix & Matrice, SPlayer & Player) {
		for (unsigned i(Player.m_Y); i < Player.m_Y + Player.m_sizeY; ++i) {
			for (unsigned j(Player.m_X); j < Player.m_X + Player.m_sizeX; ++j) {
				//Début de la détéction des bonus

				if (Matrice[i][j] == KBonusX) {
					if (Player.m_X == 1) KLog += "\n\rVous avez déjà prit un bonus de ce type !";
					++Player.m_sizeX;
					++Player.m_sizeY;
					KLog += "\n\rCe bonus vous fait augmenter de taille. \n\rIl vous permet aussi de traverser les obstacles";
					Player.m_score += 25;
					KLog += "\n\r\n\rCe bonus vous a fait GAGNER 25 en score !";
				}
				if (Matrice[i][j] == KBonusY) {


					KLog += "MOI JE FAIS RIEN";
					Player.m_score += 15;
					KLog += "\n\r\n\rCe bonus vous a fait GAGNER 15 en score !";


				}
				if (Matrice[i][j] == KBonusZ) {


					KLog += "MOI JE FAIS RIEN";
					Player.m_score += 10;
					KLog += "\n\r\n\rCe bonus vous a fait GAGNER 10 en score !";


				}
				//Fin de la détéction des bonus
				for (unsigned i(Player.m_Y); i < Player.m_Y + Player.m_sizeY; ++i)
					for (unsigned j(Player.m_X); j < Player.m_X + Player.m_sizeX; ++j)
						Matrice[i][j] = Player.m_token;
			}
		}
	}//GetBonus()

	// OBSTACLES

	bool IsSurrounded(CMatricerix & Map, SPlayer & Player) {
		return (Map[Player.m_X + 1][Player.m_Y + 1] == KObstacle);
	}//IsSurrounded()

	SObstacle InitObstacle(const unsigned
		AxeX, const unsigned AxeY, const char Token) {

		SObstacle Obstacle;

		Obstacle.m_X = AxeX;
		Obstacle.m_Y = AxeY;
		Obstacle.m_token = Token;
		return Obstacle;
	} //InitObstacle()

	void PutObstacle(CMatricerix & Matrice, SObstacle & Obstacle) {

		for (int i = -1; i < 1; i++)

			if ((Matrice[Obstacle.m_Y + i][Obstacle.m_X + i] != (int)Matrice.size())) {

				Matrice[Obstacle.m_Y][Obstacle.m_X] = Obstacle.m_token;
				VObstacle.push_back(Obstacle);

			}
	} //PutObstacle()

	void GenerateRandomObstacles(CMatricerix & Matrice, const SObstacle & Obstacle, const unsigned & Totalsize) {
		SObstacle NewObstacle = Obstacle;

		vector <unsigned> Randomvalues;

		for (unsigned i(0); i < Totalsize; ++i) Randomvalues.push_back(Rand(1, 2));

		for (unsigned i(0); i < Totalsize; ++i) {

			if (1 == Randomvalues[i]) {
				++NewObstacle.m_Y;
				PutObstacle(Matrice, NewObstacle);
			}
			else if (2 == Randomvalues[i]) {
				++NewObstacle.m_X;
				PutObstacle(Matrice, NewObstacle);
			}
		}
	} //GenerateRandomObstacles()

	bool IsMovementForbidden(SPlayer & Player, char & Movement) {
		if (Player.m_sizeX != 1) return false;

		for (unsigned i(0); i < VObstacle.size(); ++i) {
			if (VObstacle[i].m_X == Player.m_X  &&
				VObstacle[i].m_Y == Player.m_Y - 1 && Movement == KTop) {

				KLog += "\n\r\n\r Ce bonus vous a fait PERDRE 8 en score !";
				Player.m_score -= 8;

				return true;
			}
			else if (VObstacle[i].m_X == Player.m_X  &&
				VObstacle[i].m_Y == Player.m_Y + 1 && Movement == KBot) {

				KLog += "\n\r\n\r Ce bonus vous a fait PERDRE 8 en score !";
				Player.m_score -= 8;

				return true;
			}
			else if (VObstacle[i].m_X == Player.m_X - 1 &&
				VObstacle[i].m_Y == Player.m_Y  && Movement == KLeft) {

				KLog += "\n\r\n\r Ce bonus vous a fait PERDRE 8 en score !";
				Player.m_score -= 8;

				return true;
			}

			else if (VObstacle[i].m_X == Player.m_X + 1 &&
				VObstacle[i].m_Y == Player.m_Y  && Movement == KRight) {

				KLog += "\n\r\n\r Ce bonus vous a fait PERDRE 8 en score !";
				Player.m_score -= 8;

				return true;
			}
		}
		return false;
	}//IsMovementForbidden()

	void GenerateStaticObject(CMatricerix & Map, unsigned & Difficulty) {

		SObstacle Tmpobs;
		SBonus tmpBonus;
		int rndBX, rndBY;

		unsigned nbobs, nbonus, choix;

		if (1 == Difficulty) {/*EASY*/
			nbobs = 6;
			nbonus = 5;

			for (unsigned i(0); i < nbobs; ++i) {

				int maxX = round((KSizeX / 2)) + Rand(1, 2);
				int maxY = round((KSizeY / 2)) - Rand(0, 1);

				int rndX = round(Rand(2, maxX));
				int rndY = round(Rand(2, maxY));

				tmpObs = InitObstacle(rndX, rndY, KObstacle);

				if ((FirstPlayer.m_X == tmpObs.m_X && FirstPlayer.m_Y == tmpObs.m_Y)
					&& SecondPlayer.m_X == tmpObs.m_X && SecondPlayer.m_Y == tmpObs.m_Y) {
					--i;

					continue;
				}

				else {
					if (!(IsSurrounded(Map, FirstPlayer)) || !(IsSurrounded(Map, SecondPlayer)))
						GenerateRandomObstacles(Map, tmpObs, Rand(1, 3));
				}
			}

			for (unsigned i(0); i < nbonus; ++i) {
				choix = Rand(1, 3);
				rndBX = Rand(round(KSizeX - round(KSizeX / 3)) - Rand(1, 3), round(KSizeX - round(KSizeX / 5)) + Rand(1, 3));
				rndBY = Rand(round(KSizeY - round(KSizeY / 3)) - Rand(1, 3), round(KSizeY - round(KSizeY / 5)) + Rand(1, 3));

				if (choix == 1) tmpBonus = InitBonus(1, 1, rndBX - 1, rndBY, KBonusY);
				else if (choix == 2) tmpBonus = InitBonus(1, 1, rndBX - 1, rndBY, KBonusX);
				else if (choix == 3) tmpBonus = InitBonus(1, 1, rndBX - 1, rndBY, KBonusZ);

				PutBonus(Map, tmpBonus);
			}
		}

		else if (2 == Difficulty) { /*HARD*/
			nbobs = Rand(7, 9);
			nbonus = 4;

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
						GenerateRandomObstacles(Map, tmpObs, Rand(2, 5));
				}
			}


			rndBX = Rand(round(KSizeX - round(KSizeX / 3)) - Rand(1, 3), round(KSizeX - round(KSizeX / 5)));
			rndBY = Rand(round(KSizeY - round(KSizeY / 3)) - Rand(1, 3), round(KSizeY - round(KSizeY / 5)));

			for (unsigned i(0); i < nbonus; ++i) {
				choix = Rand(1, 3);

				if (1 == choix)  tmpBonus = InitBonus(1, 1, rndBX - 1, rndBY, KBonusY);
				else if (2 == choix) tmpBonus = InitBonus(1, 1, rndBX - 1, rndBY, KBonusX);
				else if (3 == choix) tmpBonus = InitBonus(1, 1, rndBX - 1, rndBY, KBonusZ);

				PutBonus(Map, tmpBonus);
			}
		}
		else cout << '\r' << endl << "[!] Vérifiez vos options, le difficulté doit être comprise entre 1 (facile) et 2 (difficile)" << endl;


	}//GenerateStaticObject

	// PLAYERS

	bool IsBonusTaken(const SPlayer & player) {
		return (player.m_sizeX == 1 ? false : true);
	}//IsBonusTaken()

	SPlayer InitPlayer(const unsigned largeur, const unsigned  hauteur, const unsigned  AxeX, const unsigned  AxeY, const char  Token) {

		SPlayer player;

		player.m_sizeX = largeur;
		player.m_sizeY = hauteur;
		player.m_X = AxeX;
		player.m_Y = AxeY;
		player.m_token = Token;
		player.m_score = 0;

		return player;
	}//InitPlayer()

	void MovePlayer(CMatricerix & Matrice, char Move, SPlayer & player) {

		unsigned additional(0);
		if (IsMovementForbidden(player, Move)) return;

		if (IsBonusTaken(player)) additional = 1;

		if (Move == KTop) {
			if (player.m_Y + player.m_sizeY > 2 + additional)
			{
				player.m_Y -= 1;

				GetBonus(Matrice, player);
				for (unsigned i(player.m_X); i < player.m_X + player.m_sizeX; ++i) {
					Matrice[player.m_Y + player.m_sizeY][i] = KEmpty;
					Matrice[player.m_Y][i] = player.m_token;

				}
			}
		}


		else if (Move == KBot) {

			if (player.m_Y + player.m_sizeY < Matrice.size() - 1) {
				player.m_Y += 1;
				GetBonus(Matrice, player);

				for (unsigned i(player.m_X); i < player.m_X + player.m_sizeX; ++i) {
					Matrice[player.m_Y - 1][i] = KEmpty;
					Matrice[player.m_Y + player.m_sizeY - 1][i] = player.m_token;

				}
			}
		}

		else if (Move == KLeft) {
			if (player.m_X + player.m_sizeY > 2 + additional)
			{
				player.m_X -= 1;
				GetBonus(Matrice, player);

				for (unsigned i(player.m_Y); i < player.m_Y + player.m_sizeY; ++i) {
					Matrice[i][player.m_X + player.m_sizeX] = KEmpty;
					Matrice[i][player.m_X] = player.m_token;
				}
			}
		}

		else if (Move == KRight) {
			if (player.m_X + player.m_sizeX < Matrice[0].size() - 1)
			{
				player.m_X += 1;
				GetBonus(Matrice, player);

				for (unsigned i(player.m_Y); i < player.m_Y + player.m_sizeY; ++i) {
					Matrice[i][player.m_X - 1] = KEmpty;
					Matrice[i][player.m_X + player.m_sizeX - 1] = player.m_token;
				}
			}
		}
	}//MovePlayer()

	void KeyEvent(const int & ch, CMatricerix & Map, SPlayer & player) {

		if (ch == KTop) MovePlayer(Map, KTop, player);
		else if (ch == KBot) MovePlayer(Map, KBot, player);
		else if (ch == KLeft) MovePlayer(Map, KLeft, player);
		else if (ch == KRight) MovePlayer(Map, KRight, player);
		else if (ch == char(3)/*CTRL+C*/) {
			cout << endl;
			endwin();
			exit(0);
		}
		else if (ch == 'r'/*RESTART*/) {
			cout << endl;
			endwin();
			system("rm main.out 2>/dev/null; g++ -std=c++11  main.cpp -o main.out -Wall -ltinfo -lncurses; ./main.out");
			exit(0);
		}

	}//KeyEvent()

	//IA

	void MoveBot(int & ch, CMatricerix & Map, const unsigned & tour) {


		if (tour % 2 == 1) {

			if (!(FirstPlayer.m_X == SecondPlayer.m_X)) {
				if (SecondPlayer.m_X - 1 < FirstPlayer.m_X) MovePlayer(Map, KRight, SecondPlayer);
				else MovePlayer(Map, KLeft, SecondPlayer);
			}
		}

		else {

			if (!(FirstPlayer.m_Y == SecondPlayer.m_Y)) {
				if ((SecondPlayer.m_Y - 1) < FirstPlayer.m_Y) MovePlayer(Map, KBot, SecondPlayer);
				else MovePlayer(Map, KTop, SecondPlayer);
			}
		}

	}//MoveBot()

	// DELAY

	void PrintDelay() { //TODO
		for (unsigned i(0); i < 1000; ++i) {
			cout << i;
		}
	}

	//DISPLAYS

	void DisplayMulti() {

		unsigned Nbround = GetTourMax();
		int ch;

		FirstPlayer = InitPlayer(1, 1, 1, 1, KFirstPlayer);
		SecondPlayer = InitPlayer(1, 1, KSizeX - 1, KSizeY - 1, KSecondPlayer);
		CMatricerix Map = InitMatrice(KSizeX + 1, KSizeY + 1, FirstPlayer, SecondPlayer); // +1 due à la bordure de '#' le long de la Matrice

		GenerateStaticObject(Map, KDifficult);

		initscr();
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

		ShowMatricerix(Map);

		for (unsigned i(0); i < Nbround * 2; ++i) {

			SPlayer &actualplayer = (i % 2 == 0 ? FirstPlayer : SecondPlayer);
			const vector <string> playtitle = {

				"               _   _____   _   _   _____   _____   ",
				"              | | /  _  \\ | | | | | ____| |  _  \\  ",
				"              | | | | | | | | | | | |__   | |_| |  ",
				"           _  | | | | | | | | | | |  __|  |  _  /  ",
				"          | |_| | | |_| | | |_| | | |___  | | \\ \\  ",
				"          \\_____/ \\_____/ \\_____/ |_____| |_|  \\_\\ ",
				"\n"
			};
			for (string Lines : playtitle) cout << endl << Lines;
			ShowMatricerix(Map, false);
			InitCurses();

			DisplayInfos(actualplayer);

			Couleur(KMagenta);
			cout << endl << "[?] Au tour du joueur '"; Couleur(KBleu); cout << actualplayer.m_token << '\'' << endl << '\r';
			Couleur(KReset);

			if (!(0 == i)) ch = getch();

			KeyEvent(ch, Map, actualplayer);


			if (CheckIfWin(FirstPlayer, SecondPlayer)) {
				DisplayWin(i);
				return;
			}

			refresh();
			endwin();

			Couleur(KBleu, KHJaune);
			ClearScreen();
			cout << setw(round(size.ws_col / 2));
			for (unsigned i(0); i < 16; ++i) cout << endl;

			cout << "[!] Egalité !" << endl;
			Couleur(KReset);

			endwin();

		}
	}//DisplayMulti()

	void DisplayLog() {
		if (KLog != "") {
			Couleur(KRouge, KHGris);
			cout << endl << "[!] Dernière inforMatriceion : " << KLog << "\n\r";
			KLog = "";
		}
		Couleur(KReset);
	} // DisplayLog()

	void DisplayInfos(const SPlayer & player) {
		Couleur(KVert);
		cout << endl << "[+] Difficulté : " << KDifficult << flush << endl << '\r';

		Couleur(KCyan);
		cout << endl << "[+] Taille : (" << KSizeX << ", " << KSizeY << ')' << endl << '\r';

		Couleur(KReset);
		if (KShowRules) {
			cout << endl << "[!] Regles :" << endl << '\r' << setw(5) << "Les bonus représentés par des " << KBonusX
				<< ", " << KBonusY << ", " << KBonusZ << " ont différentes propriétés."
				<< endl << setw(5) << "\rA vous de les découvrir !" << endl << '\r'
				<< "Enfin, les obstacles sont représentés par des '" << KObstacle << "'." << endl << '\r'
				<< "Les obstacles et les bonus sont générés de façon"; Couleur(KRouge); cout << " ALEATOIRE. "; Couleur(KReset); cout << endl << '\r' <<
				"Leur nombre dépend de la difficulté choisie. Il se peut qu'aucun bonus n'aparaisse." << endl << '\r';
		}

		Couleur(KJaune);

		cout << endl << "[!] Attention :" << endl << '\r' << "Si vous êtes bloqués, veuillez appuyer sur la touche "; Couleur(KRouge, KHVert);
		cout << 'R'; Couleur(KReset); Couleur(KJaune);
		cout << " (restart)" << endl << '\r';

		Couleur(KCyan); cout << endl << "[+] Votre score est de : " << player.m_score << endl << '\r'; Couleur(KReset);

		DisplayLog();
	}//DisplayInfos

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


		unsigned numero;
		string newparam;

		Couleur(KCyan);
		cout << endl << "[+] Afin de modifier un paramètre, veuillez entrer le numéro correspondant : ";
		cin >> numero;
		if (numero > VOptionsName.size()) {
			KLog += "Vous avez été renvoyé au menu car le paramétre ne correspondait à aucune valeur.";
			DisplayMenu();
			return;
		}
		cout << VOptionsName[numero] << " deviendra : ";
		cin >> newparam;
		SetConfig(VOptionsName[numero], newparam);

		Couleur(KReset);

		DisplayMenu();

		Couleur(KReset);
	}//DisplayOption()

	void DisplayMenu() {
		ClearScreen();


		vector <string> menulist = { "Jouer contre l'ordinateur (IA)", "Jouer à plusieur", "Options", "Quitter" };

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

		Couleur(KCyan, KHJaune); cout << "\n\r[!] Recommandation : agrandissez-la console !" << endl << endl << '\r'; Couleur(KReset);

		for (string option : menulist) {
			++choice;

			cout << ' ';
			Couleur(KRouge);
			cout << "[" << choice << "] ";
			Couleur(KReset);
			cout << option << endl << endl;

		}
		DisplayLog();
		cout << endl << "[?] Choissisez entre [1-" << menulist.size() << "] : ";

		unsigned input;
		cin >> input;

		switch (input) {
		case 1:
			DisplaySoloIA();
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

			Couleur(KRouge, KHCyan);
			cout << endl << "[!] Choix invalide !" << endl;
			Couleur(KReset);
		}
	} //DisplayMenu()

	void DisplaySoloIA() {

		/*TODO IA SOLO*/

		unsigned Nbround = GetTourMax();
		int ch;
		unsigned tourIA, tour(0);


		FirstPlayer = InitPlayer(1, 1, 1, 1, KFirstPlayer);
		SecondPlayer = InitPlayer(1, 1, KSizeX - 1, KSizeY - 1, KSecondPlayer); //le robot
		CMatricerix Map = InitMatrice(KSizeX + 1, KSizeY + 1, FirstPlayer, SecondPlayer); // +1 due à la bordure de '#' le long de la Matrice

		GenerateStaticObject(Map, KDifficult);

		initscr();
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

		ShowMatricerix(Map);

		for (; tour < Nbround * 2; ++tour) {
			/*FirstPlayer = User. SecondPlayer = IA.*/
			SPlayer &actualplayer = (tour % 2 == 0 ? FirstPlayer : SecondPlayer);

			const vector <string> playtitle = {

				"      _____   _____   _       _____  ",
				"     /  ___/ /  _  \\ | |     /  _  \\ ",
				"     | |___  | | | | | |     | | | | ",
				"     \\___  \\ | | | | | |     | | | | ",
				"      ___| | | |_| | | |___  | |_| | ",
				"     /_____/ \\_____/ |_____| \\_____/ ",
				"\n"
			};
			for (string Lines : playtitle) cout << endl << Lines;
			ShowMatricerix(Map, false);

			InitCurses();

			DisplayInfos(FirstPlayer);

			Couleur(KMagenta);
			cout << endl << "[+] A vous de jouer '"; Couleur(KBleu); cout << FirstPlayer.m_token << "' !" << endl << '\r';
			Couleur(KReset);

			if (!(tour % 2 == 0)) ch = getch();

			if (actualplayer.m_token == FirstPlayer.m_token) KeyEvent(ch, Map, FirstPlayer);

			else if (actualplayer.m_token == SecondPlayer.m_token) {
				usleep(3000);

				MoveBot(ch, Map, tourIA);
				++tourIA;

			}

			if (CheckIfWin(FirstPlayer, SecondPlayer)) {
				DisplayWin(tour, false);
				return;
			}

			refresh();
			endwin();

			Couleur(KBleu, KHJaune);
			ClearScreen();
			cout << setw(round(size.ws_col / 2));
			for (unsigned i(0); i < 16; ++i) cout << endl;

			cout << "[!] Egalité !" << endl;
			Couleur(KReset);

			endwin();

		}
	} //DisplaySoloIA()
}

int main() {

	InitOptions();
	DisplayMenu();

	endwin();
	Couleur(KReset);
	cout << endl << endl;
	return 0;
}
