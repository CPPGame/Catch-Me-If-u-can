#include <iostream>		// cout() ...
#include <vector>		// vector<AType> ...
#include <random>		// random_device ...
#include <iomanip>		// setwp() ...
#include <ncurses.h>	// initscr() ...
#include <sys/ioctl.h>	// winsize ...
#include <unistd.h>		// ioctl() ...

/* OPTIONAL INCLUDE
	#include <string>
	#include <string.h>
	#include <utility>
*/

using namespace std;

/*

TODO :

- Historique des déplacements
- Ajouter un timer
- Utiliser les fichiers externes pour l'ASCII-Art dans les Display

*/

/*

Compilation préconisée : rm main.out; g++ -std=c++11  main.cpp -o main.out -Wall -ltinfo -lncurses;  ./main.out
Package à installer : libncurses5-dev (sudo apt-get install libncurses5-dev)

*/

typedef vector <char> CVLine;
typedef vector <CVLine> CMatrice;

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

	/*COULEURS BASIQUE*/
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

	/* VARIABLES DE CONFIGURATION */
	struct winsize size;

	vector <SObstacle> VObstacle;
	vector <string> VOptionsName;
	vector <string> VOptionValue;
	string SLog;

	bool BShowRules;

	unsigned KSizeY;
	unsigned KSizeX;
	unsigned KDelay;
	unsigned KDifficult;

	char BonusY;
	char BonusX;
	char BonusZ;
	char CMouvLeft;
	char CMouvTop;
	char CMouvBot;
	char CMouvRight;
	char CaseEmpty;
	char CaseObstacle;
	char CaseBorder;
	char TokenPlayerX;
	char TokenPlayerY;

	SPlayer PlayerX;
	SPlayer PlayerY;

	void DisplayMenu();
	void DisplaySoloIA();
	void DisplayInfos(const SPlayer & Player);


	//SCREEN - INITS

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
		BonusX = 'W';
		BonusY = 'V';
		BonusZ = 'U';
		CMouvLeft = 'q';
		CMouvTop = 'z';
		CMouvBot = 's';
		CMouvRight = 'd';
		CaseEmpty = '.';
		CaseObstacle = '#';
		CaseBorder = '#';
		TokenPlayerX = 'X';
		TokenPlayerY = 'O';
		BShowRules = true;

		VOptionsName = { "CaseEmpty", "TokenPlayerX", "TokenPlayerY", "KSizeX", "KSizeY", "KDelay", "KDifficult", "BShowRules" };

		VOptionValue = { ".",  "X" ,"O", "10", "10", "10", "2", "true" };

		if (1 == KDifficult) VOptionValue[6] = "1";
		else VOptionValue[6] = "0";

	} //InitOptions()

	void SetConfig(string Name, const string &Value) {

		vector<char> Valuetochar(Value.c_str(), Value.c_str() + Value.size() + 1u);

		for (unsigned i(0); i < VOptionsName.size(); ++i)
			if (VOptionsName[i] == Name) VOptionValue[i] = Value;


		if ("CMouvLeft" == Name) CMouvLeft = Valuetochar[0];
		else if ("CMouvTop" == Name) CMouvTop = Valuetochar[0];
		else if ("CMouvBot" == Name) CMouvBot = Valuetochar[0];
		else if ("CMouvRight" == Name) CMouvRight = Valuetochar[0];
		else if ("TokenPlayerX" == Name) TokenPlayerX = Valuetochar[0];
		else if ("CaseEmpty" == Name) CaseEmpty = Valuetochar[0];
		else if ("TokenPlayerY" == Name) TokenPlayerY = Valuetochar[0];
		else if ("KSizeX" == Name) KSizeX = stoul(Value);
		else if ("KSizeY" == Name) KSizeY = stoul(Value);
		else if ("KDelay" == Name) KDelay = stoul(Value);
		else if ("KDifficult" == Name) KDifficult = stoul(Value);
		else if ("BShowRules" == Name) BShowRules = StrToBool(Value);


	} //SetConfig();
	
	// MATRICE

	CMatrice InitMatrice(unsigned NbLine, unsigned NbColumn, SPlayer & PlayerX, SPlayer & PlayerY, bool ShowBorder = true) {
		CMatrice Matrice;
		Matrice.resize(NbLine);

		for (unsigned i(0); i < NbLine; ++i)
			for (unsigned j(0); j < NbColumn; ++j)
				Matrice[i].push_back(CaseEmpty);



		for (unsigned i(PlayerX.m_Y); i < PlayerX.m_Y + PlayerX.m_sizeY; ++i)
			for (unsigned j(PlayerX.m_X); j < PlayerX.m_X + PlayerX.m_sizeX; ++j)
				Matrice[i][j] = PlayerX.m_token;


		for (unsigned i(PlayerY.m_Y); i < PlayerY.m_Y + PlayerY.m_sizeY; ++i)
			for (unsigned j(PlayerY.m_X); j < PlayerY.m_X + PlayerY.m_sizeX; ++j)
				Matrice[i][j] = PlayerY.m_token;

		if (ShowBorder) {
			for (unsigned i(0); i < NbLine; ++i) {

				Matrice[i][0] = CaseBorder;
				Matrice[i][NbColumn - 1] = CaseBorder;
			}

			for (unsigned i(0); i < NbColumn; ++i) {
				Matrice[0][i] = CaseBorder;
				Matrice[NbLine - 1][i] = CaseBorder;
			}

		}


		return Matrice;
	} //InitMatrice()

	void ShowMatricerix(const CMatrice & Matrice, const bool Clear = true) {

		ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

		if (Clear) ClearScreen();
		cout << endl << endl;
		Couleur(KReset);

		for (unsigned i(0); i < Matrice.size(); ++i) {
			cout << setw(round(size.ws_col / 2) - Matrice.size());

			for (unsigned a(0); a < Matrice[i].size(); ++a) {

				/*GESTION DES CoulEURS*/

				if (Matrice[i][a] == CaseObstacle) Couleur(KNoir);
				if (Matrice[i][a] == BonusX) Couleur(KVert, KHVert);
				if (Matrice[i][a] == BonusY) Couleur(KVert, KHVert);
				if (Matrice[i][a] == BonusZ) Couleur(KVert, KHVert);
				if (Matrice[i][a] == TokenPlayerX) Couleur(KRouge, KHCyan);
				if (Matrice[i][a] == TokenPlayerY)	Couleur(KBleu, KHJaune);
				if (Matrice[i][a] == CaseEmpty) Couleur(KMagenta);

				cout << Matrice[i][a];
				Couleur(KReset);

			}
			cout << endl;
		}
	}//ShowMatricerix

	// WIN CHECK - WIN STAT

	bool CheckIfWin(SPlayer & PlayerX, SPlayer & PlayerY) {
		return !((PlayerX.m_X > PlayerY.m_X + PlayerY.m_sizeX - 1) ||
			(PlayerX.m_X + PlayerX.m_sizeX - 1 < PlayerY.m_X) ||
			(PlayerY.m_Y > PlayerX.m_Y + PlayerX.m_sizeY - 1) ||
			(PlayerY.m_Y + PlayerY.m_sizeY - 1 < PlayerX.m_Y));
	} //CheckIfWin()

	SPlayer GetWinner(const SPlayer& PlayerX, const SPlayer &PlayerY, const unsigned & NbrTour) {
		return (NbrTour % 2 == 0 ? PlayerX : PlayerY);
	} //GetWinner()

	void DisplayWin(const unsigned &Tour, const bool &IsBot = true) {

		SPlayer winner = GetWinner(PlayerX, PlayerY, Tour);
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

	SBonus InitBonus(const unsigned Largeur, const unsigned Hauteur, const unsigned
		AxeX, const unsigned AxeY, const char Token) {

		SBonus Bonus;

		Bonus.m_sizeX = Largeur;
		Bonus.m_sizeY = Hauteur;
		Bonus.m_X = AxeX;
		Bonus.m_Y = AxeY;
		Bonus.m_token = Token;

		return Bonus;
	}//InitBonus()

	void PutBonus(CMatrice & Matrice, SBonus & Bonus) {
		if ((Bonus.m_Y > 1 && Bonus.m_X > 1) && (Bonus.m_Y < KSizeY - 1 && Bonus.m_X < KSizeX - 1))
			Matrice[Bonus.m_Y][Bonus.m_X] = Bonus.m_token;
	}//PutBonus()

	void GetBonus(CMatrice & Matrice, SPlayer & Player) {
		for (unsigned i(Player.m_Y); i < Player.m_Y + Player.m_sizeY; ++i) {
			for (unsigned j(Player.m_X); j < Player.m_X + Player.m_sizeX; ++j) {
				//Début de la détéction des bonus

				if (Matrice[i][j] == BonusX) {
					if (Player.m_X == 1) SLog += "\n\rVous avez déjà prit un bonus de ce type !";
					++Player.m_sizeX;
					++Player.m_sizeY;
					SLog += "\n\rCe bonus vous fait augmenter de taille. \n\rIl vous permet aussi de traverser les obstacles";
					Player.m_score += 25;
					SLog += "\n\r\n\rCe bonus vous a fait GAGNER 25 en score !";
				}
				if (Matrice[i][j] == BonusY) {


					SLog += "MOI JE FAIS RIEN";
					Player.m_score += 15;
					SLog += "\n\r\n\rCe bonus vous a fait GAGNER 15 en score !";


				}
				if (Matrice[i][j] == BonusZ) {


					SLog += "MOI JE FAIS RIEN";
					Player.m_score += 10;
					SLog += "\n\r\n\rCe bonus vous a fait GAGNER 10 en score !";


				}
				//Fin de la détéction des bonus
				for (unsigned i(Player.m_Y); i < Player.m_Y + Player.m_sizeY; ++i)
					for (unsigned j(Player.m_X); j < Player.m_X + Player.m_sizeX; ++j)
						Matrice[i][j] = Player.m_token;
			}
		}
	}//GetBonus()

	// OBSTACLES

	bool IsSurrounded(CMatrice & Map, SPlayer & Player) {
		return (Map[Player.m_X + 1][Player.m_Y + 1] == CaseObstacle);
	}//IsSurrounded()

	SObstacle InitObstacle(const unsigned
		AxeX, const unsigned AxeY, const char Token) {

		SObstacle Obstacle;

		Obstacle.m_X = AxeX;
		Obstacle.m_Y = AxeY;
		Obstacle.m_token = Token;
		return Obstacle;
	} //InitObstacle()

	void PutObstacle(CMatrice & Matrice, SObstacle & Obstacle) {

		for (int i = -1; i < 1; i++)

			if ((Matrice[Obstacle.m_Y + i][Obstacle.m_X + i] != (int)Matrice.size())) {

				Matrice[Obstacle.m_Y][Obstacle.m_X] = Obstacle.m_token;
				VObstacle.push_back(Obstacle);

			}
	} //PutObstacle()

	void GenerateRandomObstacles(CMatrice & Matrice, const SObstacle & Obstacle, const unsigned & Totalsize) {
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
				VObstacle[i].m_Y == Player.m_Y - 1 && Movement == CMouvTop) {

				SLog += "\n\r\n\r Ce bonus vous a fait PERDRE 8 en score !";
				Player.m_score -= 8;

				return true;
			}
			else if (VObstacle[i].m_X == Player.m_X  &&
				VObstacle[i].m_Y == Player.m_Y + 1 && Movement == CMouvBot) {

				SLog += "\n\r\n\r Ce bonus vous a fait PERDRE 8 en score !";
				Player.m_score -= 8;

				return true;
			}
			else if (VObstacle[i].m_X == Player.m_X - 1 &&
				VObstacle[i].m_Y == Player.m_Y  && Movement == CMouvLeft) {

				SLog += "\n\r\n\r Ce bonus vous a fait PERDRE 8 en score !";
				Player.m_score -= 8;

				return true;
			}

			else if (VObstacle[i].m_X == Player.m_X + 1 &&
				VObstacle[i].m_Y == Player.m_Y  && Movement == CMouvRight) {

				SLog += "\n\r\n\r Ce bonus vous a fait PERDRE 8 en score !";
				Player.m_score -= 8;

				return true;
			}
		}
		return false;
	}//IsMovementForbidden()

	void GenerateStaticObject(CMatrice & Map, unsigned & Difficulty) {

		SObstacle TmpObs;
		SBonus TmpBonus;
		int RndBX, rndBY;

		unsigned NbObs, NbBonus, Choix;

		if (1 == Difficulty) {/*EASY*/
			NbObs = 6;
			NbBonus = 5;

			for (unsigned i(0); i < NbObs; ++i) {

				int MaxX = round((KSizeX / 2)) + Rand(1, 2);
				int MaxY = round((KSizeY / 2)) - Rand(0, 1);

				int RndX = round(Rand(2, MaxX));
				int RndY = round(Rand(2, MaxY));

				TmpObs = InitObstacle(RndX, RndY, CaseObstacle);

				if ((PlayerX.m_X == TmpObs.m_X && PlayerX.m_Y == TmpObs.m_Y)
					&& PlayerY.m_X == TmpObs.m_X && PlayerY.m_Y == TmpObs.m_Y) {
					--i;

					continue;
				}

				else {
					if (!(IsSurrounded(Map, PlayerX)) || !(IsSurrounded(Map, PlayerY)))
						GenerateRandomObstacles(Map, TmpObs, Rand(1, 3));
				}
			}

			for (unsigned i(0); i < NbBonus; ++i) {
				Choix = Rand(1, 3);
				RndBX = Rand(round(KSizeX - round(KSizeX / 3)) - Rand(1, 3), round(KSizeX - round(KSizeX / 5)) + Rand(1, 3));
				rndBY = Rand(round(KSizeY - round(KSizeY / 3)) - Rand(1, 3), round(KSizeY - round(KSizeY / 5)) + Rand(1, 3));

				if (Choix == 1) TmpBonus = InitBonus(1, 1, RndBX - 1, rndBY, BonusY);
				else if (Choix == 2) TmpBonus = InitBonus(1, 1, RndBX - 1, rndBY, BonusX);
				else if (Choix == 3) TmpBonus = InitBonus(1, 1, RndBX - 1, rndBY, BonusZ);

				PutBonus(Map, TmpBonus);
			}
		}

		else if (2 == Difficulty) { /*HARD*/
			NbObs = Rand(7, 9);
			NbBonus = 4;

			for (unsigned i(0); i < NbObs; ++i) {

				unsigned MinX = round((KSizeX / 2)) - Rand(1, 3);
				unsigned MaxX = round((KSizeX / 2)) + Rand(1, 2);
				unsigned MinY = round((KSizeY / 2)) - Rand(3, 4);
				unsigned MaxY = round((KSizeY / 2)) + Rand(2, 3);

				unsigned RndX = round(Rand(MinX, MaxX));
				unsigned RndY = round(Rand(MinY, MaxY));

				TmpObs = InitObstacle(RndX, RndY, CaseObstacle);


				if ((PlayerX.m_X == TmpObs.m_X && PlayerX.m_Y == TmpObs.m_Y)
					&& PlayerY.m_X == TmpObs.m_X && PlayerY.m_Y == TmpObs.m_Y) {
					--i;
					continue;
				}

				else {
					if (!(IsSurrounded(Map, PlayerX)) || !(IsSurrounded(Map, PlayerY)))
						GenerateRandomObstacles(Map, TmpObs, Rand(2, 5));
				}
			}


			RndBX = Rand(round(KSizeX - round(KSizeX / 3)) - Rand(1, 3), round(KSizeX - round(KSizeX / 5)));
			rndBY = Rand(round(KSizeY - round(KSizeY / 3)) - Rand(1, 3), round(KSizeY - round(KSizeY / 5)));

			for (unsigned i(0); i < NbBonus; ++i) {
				Choix = Rand(1, 3);

				if (1 == Choix)  TmpBonus = InitBonus(1, 1, RndBX - 1, rndBY, BonusY);
				else if (2 == Choix) TmpBonus = InitBonus(1, 1, RndBX - 1, rndBY, BonusX);
				else if (3 == Choix) TmpBonus = InitBonus(1, 1, RndBX - 1, rndBY, BonusZ);

				PutBonus(Map, TmpBonus);
			}
		}
		else cout << '\r' << endl << "[!] Vérifiez vos options, le difficulté doit être comprise entre 1 (facile) et 2 (difficile)" << endl;


	}//GenerateStaticObject

	// PLAYERS

	bool IsBonusTaken(const SPlayer & Player) {
		return (1 == Player.m_sizeX ? false : true);
	}//IsBonusTaken()

	SPlayer InitPlayer(const unsigned Largeur, const unsigned  Hauteur, const unsigned  AxeX, const unsigned  AxeY, const char  Token) {

		SPlayer Player;

		Player.m_sizeX = Largeur;
		Player.m_sizeY = Hauteur;
		Player.m_X = AxeX;
		Player.m_Y = AxeY;
		Player.m_token = Token;
		Player.m_score = 0;

		return Player;
	}//InitPlayer()

	void MovePlayer(CMatrice & Matrice, char Move, SPlayer & Player) {

		unsigned Additional(0);
		if (IsMovementForbidden(Player, Move)) return;

		if (IsBonusTaken(Player)) Additional = 1;

		if (Move == CMouvTop) {
			if (Player.m_Y + Player.m_sizeY > 2 + Additional)
			{
				Player.m_Y -= 1;

				GetBonus(Matrice, Player);
				for (unsigned i(Player.m_X); i < Player.m_X + Player.m_sizeX; ++i) {
					Matrice[Player.m_Y + Player.m_sizeY][i] = CaseEmpty;
					Matrice[Player.m_Y][i] = Player.m_token;

				}
			}
		}


		else if (Move == CMouvBot) {

			if (Player.m_Y + Player.m_sizeY < Matrice.size() - 1) {
				Player.m_Y += 1;
				GetBonus(Matrice, Player);

				for (unsigned i(Player.m_X); i < Player.m_X + Player.m_sizeX; ++i) {
					Matrice[Player.m_Y - 1][i] = CaseEmpty;
					Matrice[Player.m_Y + Player.m_sizeY - 1][i] = Player.m_token;

				}
			}
		}

		else if (Move == CMouvLeft) {
			if (Player.m_X + Player.m_sizeY > 2 + Additional)
			{
				Player.m_X -= 1;
				GetBonus(Matrice, Player);

				for (unsigned i(Player.m_Y); i < Player.m_Y + Player.m_sizeY; ++i) {
					Matrice[i][Player.m_X + Player.m_sizeX] = CaseEmpty;
					Matrice[i][Player.m_X] = Player.m_token;
				}
			}
		}

		else if (Move == CMouvRight) {
			if (Player.m_X + Player.m_sizeX < Matrice[0].size() - 1)
			{
				Player.m_X += 1;
				GetBonus(Matrice, Player);

				for (unsigned i(Player.m_Y); i < Player.m_Y + Player.m_sizeY; ++i) {
					Matrice[i][Player.m_X - 1] = CaseEmpty;
					Matrice[i][Player.m_X + Player.m_sizeX - 1] = Player.m_token;
				}
			}
		}
	}//MovePlayer()

	void KeyEvent(const int & ch, CMatrice & Map, SPlayer & Player) {

		if (ch == CMouvTop) MovePlayer(Map, CMouvTop, Player);
		else if (ch == CMouvBot) MovePlayer(Map, CMouvBot, Player);
		else if (ch == CMouvLeft) MovePlayer(Map, CMouvLeft, Player);
		else if (ch == CMouvRight) MovePlayer(Map, CMouvRight, Player);
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

	 // IA

	void MoveBot(int & ch, CMatrice & Map, const unsigned & Tour) {


		if (1 == Tour % 2 && (!(PlayerX.m_X == PlayerY.m_X))) {
			
			if (PlayerY.m_X - 1 < PlayerX.m_X) MovePlayer(Map, CMouvRight, PlayerY);
			else MovePlayer(Map, CMouvLeft, PlayerY);
		}

		else {

			if (!(PlayerX.m_Y == PlayerY.m_Y)) {
				if ((PlayerY.m_Y - 1) < PlayerX.m_Y) MovePlayer(Map, CMouvBot, PlayerY);
				else MovePlayer(Map, CMouvTop, PlayerY);
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

		PlayerX = InitPlayer(1, 1, 1, 1, TokenPlayerX);
		PlayerY = InitPlayer(1, 1, KSizeX - 1, KSizeY - 1, TokenPlayerY);
		CMatrice Map = InitMatrice(KSizeX + 1, KSizeY + 1, PlayerX, PlayerY); // +1 due à la bordure de '#' le long de la Matrice

		GenerateStaticObject(Map, KDifficult);

		initscr();
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

		ShowMatricerix(Map);

		for (unsigned i(0); i < Nbround * 2; ++i) {

			SPlayer &actualPlayer = (i % 2 == 0 ? PlayerX : PlayerY);
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

			DisplayInfos(actualPlayer);

			Couleur(KMagenta);
			cout << endl << "[?] Au Tour du joueur '"; Couleur(KBleu); cout << actualPlayer.m_token << '\'' << endl << '\r';
			Couleur(KReset);

			if (!(0 == i)) ch = getch();

			KeyEvent(ch, Map, actualPlayer);


			if (CheckIfWin(PlayerX, PlayerY)) {
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
		if (SLog != "") {
			Couleur(KRouge, KHGris);
			cout << endl << "[!] Dernière inforMatriceion : " << SLog << "\n\r";
			SLog = "";
		}
		Couleur(KReset);
	} // DisplayLog()

	void DisplayInfos(const SPlayer & Player) {
		Couleur(KVert);
		cout << endl << "[+] Difficulté : " << KDifficult << flush << endl << '\r';

		Couleur(KCyan);
		cout << endl << "[+] Taille : (" << KSizeX << ", " << KSizeY << ')' << endl << '\r';

		Couleur(KReset);
		if (BShowRules) {
			cout << endl << "[!] Regles :" << endl << '\r' << setw(5) << "Les bonus représentés par des " << BonusX
				<< ", " << BonusY << ", " << BonusZ << " ont différentes propriétés."
				<< endl << setw(5) << "\rA vous de les découvrir !" << endl << '\r'
				<< "Enfin, les obstacles sont représentés par des '" << CaseObstacle << "'." << endl << '\r'
				<< "Les obstacles et les bonus sont générés de façon"; Couleur(KRouge); cout << " ALEATOIRE. "; Couleur(KReset); cout << endl << '\r' <<
				"Leur nombre dépend de la difficulté choisie. Il se peut qu'aucun bonus n'aparaisse." << endl << '\r';
		}

		Couleur(KJaune);

		cout << endl << "[!] Attention :" << endl << '\r' << "Si vous êtes bloqués, veuillez appuyer sur la touche "; Couleur(KRouge, KHVert);
		cout << 'R'; Couleur(KReset); Couleur(KJaune);
		cout << " (restart)" << endl << '\r';

		Couleur(KCyan); cout << endl << "[+] Votre score est de : " << Player.m_score << endl << '\r'; Couleur(KReset);

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


		unsigned Numero;
		string NewParam;

		Couleur(KCyan);
		cout << endl << "[+] Afin de modifier un paramètre, veuillez entrer le numéro correspondant : ";
		cin >> Numero;
		if (Numero > VOptionsName.size()) {
			SLog += "Vous avez été renvoyé au menu car le paramétre ne correspondait à aucune valeur.";
			DisplayMenu();
			return;
		}
		cout << VOptionsName[Numero] << " deviendra : ";
		cin >> NewParam;
		SetConfig(VOptionsName[Numero], NewParam);

		Couleur(KReset);

		DisplayMenu();

		Couleur(KReset);
	}//DisplayOption()

	void DisplayMenu() {
		ClearScreen();


		vector <string> menulist = { "Jouer contre l'ordinateur (IA)", "Jouer à plusieur", "Options", "Quitter" };

		unsigned Choice(0);

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
			++Choice;

			cout << ' ';
			Couleur(KRouge);
			cout << "[" << Choice << "] ";
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
		unsigned TourIA, Tour(0);


		PlayerX = InitPlayer(1, 1, 1, 1, TokenPlayerX);
		PlayerY = InitPlayer(1, 1, KSizeX - 1, KSizeY - 1, TokenPlayerY); //le robot
		CMatrice Map = InitMatrice(KSizeX + 1, KSizeY + 1, PlayerX, PlayerY); // +1 due à la bordure de '#' le long de la Matrice

		GenerateStaticObject(Map, KDifficult);

		initscr();
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

		ShowMatricerix(Map);

		for (; Tour < Nbround * 2; ++Tour) {
			/*PlayerX = User. PlayerY = IA.*/
			SPlayer &actualPlayer = (Tour % 2 == 0 ? PlayerX : PlayerY);

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

			DisplayInfos(PlayerX);

			Couleur(KMagenta);
			cout << endl << "[+] A vous de jouer '"; Couleur(KBleu); cout << PlayerX.m_token << "' !" << endl << '\r';
			Couleur(KReset);

			if (!(Tour % 2 == 0)) ch = getch();

			if (actualPlayer.m_token == PlayerX.m_token) KeyEvent(ch, Map, PlayerX);

			else if (actualPlayer.m_token == PlayerY.m_token) {
				usleep(3000);

				MoveBot(ch, Map, TourIA);
				++TourIA;

			}

			if (CheckIfWin(PlayerX, PlayerY)) {
				DisplayWin(Tour, false);
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
