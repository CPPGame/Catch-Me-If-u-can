/*!
* \file   main.cpp
* \authors Alain Casali, Marc Laporte
* \date January 17 2017
* \brief   Terminal's color management
*          beginning of the project titled "catch me if you can"
*/

#include <iostream>		// cout() ...
#include <vector>		// vector<AType> ...
#include <random>		// random_device ...
#include <iomanip>		// setwp() ...
#include <ncurses.h>	// initscr() ...
#include <sys/ioctl.h>	// winsize ...
#include <unistd.h>		// ioctl() ...
#include <fstream>		// ifstream() ...
#include <utility>		// make_pair() ...


using namespace std;

/*

Compilation préconisée :  rm main.out; g++ -std=c++11  CMIFUC.cpp -o main.out -Wall -ltinfo -lncurses;  ./main.out
Package à installer : libncurses5-dev (sudo apt-get install libncurses5-dev)

*/

typedef vector <vector <char>> CMatrice;

typedef struct {
	unsigned m_X;
	unsigned m_Y;
	unsigned m_sizeX;
	unsigned m_sizeY;
	int m_score;
	char m_token;
	vector<char> m_history;
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
	const string KUnderLine("4");
	const string KBold("1");
	const string KOverLine("53");
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
	bool BShowHistory;

	unsigned KSizeY;
	unsigned KSizeX;
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

	void DisplayLog();
	void DisplayMenu();
	void DisplaySoloIA();
	void DisplayHistory();
	void DisplayEditor();
	void DisplayInfos(const SPlayer & Player);
	void GenerateStaticObject(CMatrice & Map, unsigned & Difficulty, const unsigned & NbObs, const unsigned & NbBonus);

	//SCREEN - INITS

	void Couleur(const string & Coul, const string Hightlight = "") {

		if (Hightlight != "")
			cout << "\033[34m\033[" << Hightlight << "m";
		else
			cout << "\033[" << Coul << "m";


	} //Couleur()

	unsigned GetTourMax() {
		unsigned Nbround;

		Couleur(KCyan);
		cout << endl << "[?] Entrez le nombre de rounds : ";

		while (!(cin >> Nbround))
		{
			Couleur(KRouge);
			cout << "\n\r[!] Le nombre de tour est incorrect. Veillez réessayer : \n\r";
			cin.clear();
			break;

		}
		Couleur(KReset);

		return Nbround;
	} //GetTourMax()

	void SetTextMiddle() {
		cout << setw(round(size.ws_col / 2));

	} //SetTextMiddle()

	void ClearScreen() {
		cout << "\033[H\033[2J";
	}//ClearScreen()

	int Rand(int Min, int Max) {

		random_device Random;
		mt19937 Rng(Random());
		uniform_int_distribution<int> uni(Min, Max);

		return uni(Rng);
	}//Rand()

	void ListenKeyboard() {
		raw();
		keypad(stdscr, TRUE);
		noecho();
	} //ListenKeyboard()

	void InitCurses() {

		initscr();
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	} //InitCurses()

	void PrintLines(const unsigned & LineNumber) {
		for (unsigned i(0); i < LineNumber; ++i) cout << endl;
	} //PrintLines

	bool StrToBool(const string & Chaine) {
		return(Chaine == "true" ? true : false);
	} //StrToBool()

	void ShowTitle(const string & FileName, const bool Clear = true) {

		ClearScreen();
		string StrTitle;
		string StrDir = "asciititle";
		ifstream IFSTitle(StrDir + '/' + FileName);

		cout << endl;

		while (true) {
			if (IFSTitle.eof()) break;
			getline(IFSTitle, StrTitle);
			cout << StrTitle << endl;
		}

		IFSTitle.clear();
	}//ShowTitle()

	// OPTIONS

	void InitOptions() {

		/*
		All configs go here (InitOptions()) AND in header part (simple declaration) AND in SetConfig()
		All DEFAULT values go here
		*/

		KSizeX = 10;
		KSizeY = 10;
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
		BShowHistory = true;
		BShowRules = true;

		VOptionsName = { "CaseEmpty", "TokenPlayerX", "TokenPlayerY", "KSizeX", "KSizeY",  "KDifficult", "BShowRules", "BShowHistory" };

		VOptionValue = { ".",  "X" ,"O", "10", "10", "2", "true", "true" };

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
		else if ("KDifficult" == Name) KDifficult = stoul(Value);
		else if ("BShowRules" == Name) BShowRules = StrToBool(Value);
		else if ("BShowHistory" == Name) BShowHistory = StrToBool(Value);


	} //SetConfig();

	// MATRICE

	unsigned GetMaxLine(const string & File) {
		string Line;
		unsigned NbLine(0);
		ifstream ReadedMap(File);

		while (!ReadedMap.eof()) {
			getline(ReadedMap, Line);
			++NbLine;
		}

		return NbLine;
	}//GetMaxLine

	unsigned GetMaxColumn(const string & File) {
		string Line;
		ifstream ReadedMap(File);
		//Retourne la taille de la première ligne du fichier
		getline(ReadedMap, Line);
		return Line.length();

	}//GetMaxColumn

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

	void ShowMatrice(const CMatrice & Matrice, const bool Clear = true) {

		if (Clear) ClearScreen();
		PrintLines(2);
		Couleur(KReset);

		for (unsigned i(0); i < Matrice.size(); ++i) {
			SetTextMiddle();
			for (unsigned a(0); a < Matrice[i].size(); ++a) {

				/*GESTION DES CoulEURS*/
				if (Matrice[i][a] == CaseObstacle) {
					Couleur(KBold);
					Couleur(KHNoir);
				}

				if (Matrice[i][a] == BonusX)
					Couleur(KVert, KHVert);

				if (Matrice[i][a] == BonusY)
					Couleur(KVert, KHVert);

				if (Matrice[i][a] == BonusZ)
					Couleur(KVert, KHVert);

				if (Matrice[i][a] == TokenPlayerX) {
					Couleur(KBold);
					Couleur(KRouge, KHCyan);
				}

				if (Matrice[i][a] == TokenPlayerY) {
					Couleur(KBold);
					Couleur(KBleu, KHJaune);
				}

				if (Matrice[i][a] == CaseEmpty)
					Couleur(KMagenta);

				cout << Matrice[i][a];
				Couleur(KReset);

			}

			cout << endl;
		}
	}//ShowMatrice

	CMatrice LoadMapByFile(const string & MapName) {

		const string FullName = "./map/MyMap_" + MapName + ".map";
		unsigned NbLine = GetMaxLine(FullName);
		unsigned NbColumn = GetMaxColumn(FullName);
		CMatrice LoadedMatrice = InitMatrice(NbLine, NbColumn, PlayerX, PlayerY);

		vector <string> MapLines;
		string ReadedLine;
		ifstream ifs(FullName);

		while (!ifs.eof()) {

			getline(ifs, ReadedLine);
			if (!(ReadedLine == "\n\r" || ReadedLine[0] == '\n'))
				MapLines.push_back(ReadedLine);
		}

		ifs.clear();

		for (unsigned i(0); i < LoadedMatrice.size(); ++i) {
			for (unsigned a(0); a < LoadedMatrice[i].size(); ++a) {
				LoadedMatrice[i][a] = MapLines[i][a];

			}
		}

		return LoadedMatrice;
	}

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

		SPlayer Winner = GetWinner(PlayerX, PlayerY, Tour);

		if (IsBot)
			ClearScreen();


		PrintLines(1);
		Couleur(KRouge, KHCyan);

		PrintLines(6);
		cout << endl << endl << "[!] Le joueur '" << Winner.m_token << "' a gagné avec : ";

		Couleur(KHBleu, KHGris); cout << Winner.m_score;
		Couleur(KRouge, KHCyan); cout << " points" << '!' << endl << '\r';

		if (BShowHistory) {
			PrintLines(3);
			Couleur(KRouge, KHJaune);

			cout << "Historique du gagnant : \n\r" << endl << "n° | Mouv \n\r" << "________\n\r";

			for (unsigned i(0); i < Winner.m_history.size(); ++i)
				cout << "| " << i << " | " << Winner.m_history[i] << " | " << "\n\r";

		}

		Couleur(KReset);
		endwin();
		return;
	} //DisplayWin()

	// BONUS

	SBonus InitBonus(const unsigned AxeX, const unsigned AxeY, const char Token) {

		SBonus Bonus;

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
					SLog += "\n\rVotre taille a augmenté!. \n\rVous pouvez désormé traverser les obstacles";
					Player.m_score += 25;
					SLog += "\n\r\n\rCe bonus vous a fait gagner 25 points!";

				}

				if (Matrice[i][j] == BonusY) {

					Player.m_score += 15;
					SLog += "\n\r\n\rCe bonus ne fait strictement rien\n\rExcepté vous faire gagner 15 points!";


				}

				if (Matrice[i][j] == BonusZ) {

					SLog += "\n\rDes obstacles aléatoires ont été placés!";
					Player.m_score += 10;
					SLog += "\n\r\n\rCe bonus vous a fait gagner 10 points!";
					GenerateStaticObject(Matrice, KDifficult, 2, 1);

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

		for (int i = -1; i < 1; ++i) {
			if ((Matrice[Obstacle.m_Y + i][Obstacle.m_X + i] != (int)Matrice.size())) {

				Matrice[Obstacle.m_Y][Obstacle.m_X] = Obstacle.m_token;
				VObstacle.push_back(Obstacle);
			}
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

	void GenerateStaticObject(CMatrice & Map, unsigned & Difficulty, const unsigned & NbObs = 0, const unsigned & NbBonus = 0) {

		SObstacle TmpObs;
		SBonus TmpBonus;
		int RndBX, RndBY;

		unsigned Choix;

		if (1 == Difficulty) {/*EASY*/


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
				RndBY = Rand(round(KSizeY - round(KSizeY / 3)) - Rand(1, 3), round(KSizeY - round(KSizeY / 5)) + Rand(1, 3));

				if (Choix == 1) TmpBonus = InitBonus(RndBX - 1, RndBY, BonusY);
				else if (Choix == 2) TmpBonus = InitBonus(RndBX - 1, RndBY, BonusX);
				else if (Choix == 3) TmpBonus = InitBonus(RndBX - 1, RndBY, BonusZ);

				PutBonus(Map, TmpBonus);
			}
		}

		else if (2 == Difficulty) { /*HARD*/


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
			RndBY = Rand(round(KSizeY - round(KSizeY / 3)) - Rand(1, 3), round(KSizeY - round(KSizeY / 5)));

			for (unsigned i(0); i < NbBonus; ++i) {
				Choix = Rand(1, 3);

				if (1 == Choix)  TmpBonus = InitBonus(RndBX - 1, RndBY, BonusY);
				else if (2 == Choix) TmpBonus = InitBonus(RndBX - 1, RndBY, BonusX);
				else if (3 == Choix) TmpBonus = InitBonus(RndBX - 1, RndBY, BonusZ);

				PutBonus(Map, TmpBonus);
			}
		}
		else cout << '\r' << endl << "[!] Vérifiez vos options, le difficulté doit être comprise entre 1 (facile) et 2 (difficile)" << endl;


	}//GenerateStaticObject

	// PLAYERS

	bool IsBonusTaken(const SPlayer & Player) {
		return (1 == Player.m_sizeX ? false : true);
	}//IsBonusTaken()

	SPlayer InitPlayer(const unsigned Largeur, const unsigned Hauteur, const unsigned  AxeX, const unsigned  AxeY, const char  Token) {

		SPlayer Player;

		Player.m_sizeX = Largeur;
		Player.m_sizeY = Hauteur;
		Player.m_X = AxeX;
		Player.m_Y = AxeY;
		Player.m_token = Token;
		/*Constant values below*/
		Player.m_score = 0;

		return Player;
	}//InitPlayer()

	void MovePlayer(CMatrice & Matrice, char Move, SPlayer & Player) {

		unsigned Additional(0);
		if (IsMovementForbidden(Player, Move)) return;
		Player.m_history.push_back(toupper(Move));
		if (IsBonusTaken(Player)) Additional = 1;

		if (Move == CMouvTop) {
			if (Player.m_Y + Player.m_sizeY > 2 + Additional)
			{
				--Player.m_Y;

				GetBonus(Matrice, Player);
				for (unsigned i(Player.m_X); i < Player.m_X + Player.m_sizeX; ++i) {
					Matrice[Player.m_Y + Player.m_sizeY][i] = CaseEmpty;
					Matrice[Player.m_Y][i] = Player.m_token;

				}
			}
		}


		else if (Move == CMouvBot) {

			if (Player.m_Y + Player.m_sizeY < Matrice.size() - 1) {
				++Player.m_Y;
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
				--Player.m_X;
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
				++Player.m_X;
				GetBonus(Matrice, Player);

				for (unsigned i(Player.m_Y); i < Player.m_Y + Player.m_sizeY; ++i) {
					Matrice[i][Player.m_X - 1] = CaseEmpty;
					Matrice[i][Player.m_X + Player.m_sizeX - 1] = Player.m_token;
				}
			}
		}
	}//MovePlayer()

	void KeyEvent(const int & ch, CMatrice & Map, SPlayer & Player) {

		if (ch == CMouvTop)
			MovePlayer(Map, CMouvTop, Player);
		else if (ch == CMouvBot)
			MovePlayer(Map, CMouvBot, Player);
		else if (ch == CMouvLeft)
			MovePlayer(Map, CMouvLeft, Player);
		else if (ch == CMouvRight)
			MovePlayer(Map, CMouvRight, Player);
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

	// IA (BOT)

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

	// EDITOR

	void ExportMatrice(CMatrice & Matrice, const string & DestFile) {

		string StrMatrice;
		for (unsigned i(0); i < Matrice.size(); ++i) {
			for (unsigned a(0); a < Matrice[i].size(); ++a) {

				StrMatrice += Matrice[i][a];

			}
			if (i < Matrice.size() - 1)
				StrMatrice += '\n';
		}

		ofstream ofs(DestFile);
		if (ofs.is_open()) {

			ofs << StrMatrice;
			ofs.close();
		}
		else cout << "[!] Impossible d'écrire ici...\n\r";
		cout << "[+] Fin de l'opération !";
	} //ExportMatrice()

	string AskForMap() {

		string MapName;

		cout << "\n\r[?] Quel est le nom de votre carte (Nom : mot comprit entre 'Mymap_' et '.map')? : ";
		Couleur(KReset);
		cin >> MapName;

		return MapName;
	} //AskForMap()

	bool IsPersoMapRecquiered() {

		char Choice;
		string MapName;

		ShowTitle("multi.title");
		PrintLines(1);

	recheck:

		Couleur(KRouge);
		cout << "\n\r\n\r[?] Voulez-vous charger une map prédéfinie ? [y /n /l]";
		cout << "\n\r(Vous pouvez lister vos maps avec la touche L) : ";
		cin >> Choice;

		if ('y' == Choice || 'Y' == Choice) return true;

		else if ('l' == Choice || 'L' == Choice) {

			Couleur(KReset);
			PrintLines(1);
			system("ls ./map");
			PrintLines(1);
			goto recheck;
		}

		else if ('n' == Choice || 'n' == Choice) return false;

	} //isPersoMapRecquiered

	void DetectObstacle(CMatrice & Map) {

		SObstacle TmpObstacle;

		for (unsigned i(0); i < Map.size(); ++i) {
			for (unsigned a(0); a < Map[i].size(); ++a) {
				if (Map[i][a] == CaseObstacle) {
					TmpObstacle = InitObstacle(i, a, 'e');
					VObstacle.push_back(TmpObstacle);
				}
			}
		}

	} //DetectObstacle()


	// DISPLAYS

	void DisplayMulti() {

		unsigned Nbround = GetTourMax();
		int ch;
		CMatrice Map;

		PlayerX = InitPlayer(1, 1, 1, 1, TokenPlayerX);


		if (IsPersoMapRecquiered()) {
			Map = LoadMapByFile(AskForMap());
			DetectObstacle(Map);
			PlayerY = InitPlayer(1, 1, KSizeX - 2, KSizeY - 2, TokenPlayerY);

		}

		else {
			Map = InitMatrice(KSizeX + 1, KSizeY + 1, PlayerX, PlayerY); /* +1 due à la bordure de '#' le long de la Matrice */

			if (KDifficult == 1)
				GenerateStaticObject(Map, KDifficult, 6, 5);
			else if (KDifficult == 2)
				GenerateStaticObject(Map, KDifficult, Rand(7, 9), 4);

			PlayerY = InitPlayer(1, 1, KSizeX - 1, KSizeY - 1, TokenPlayerY);

		}

		InitCurses();
		ShowMatrice(Map, false);

		for (unsigned i(0); i < Nbround * 2; ++i) {

			SPlayer &actualPlayer = (i % 2 == 0 ? PlayerX : PlayerY);

			ShowTitle("multi.title");
			ShowMatrice(Map, false);
			ListenKeyboard();

			DisplayInfos(actualPlayer);

			Couleur(KMagenta);
			cout << endl << "[?] Au Tour du joueur '"; Couleur(KBleu); cout << actualPlayer.m_token << '\'' << endl << '\r';
			Couleur(KReset);

			if (0 != i) ch = getch();

			KeyEvent(ch, Map, actualPlayer);

			if (CheckIfWin(PlayerX, PlayerY)) {
				DisplayWin(i);
				return;
			}

			refresh();
			endwin();

			Couleur(KBleu, KHJaune);
			ClearScreen();
			SetTextMiddle();
			PrintLines(16);

			cout << "[!] Egalité !" << endl;
			Couleur(KReset);

			endwin();

		}
	}//DisplayMulti()

	void DisplayLog() {
		if (SLog != "") {
			Couleur(KRouge, KHGris);
			cout << endl << "[!] Dernière information : " << SLog << "\n\r";
			SLog.clear();
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
		ShowTitle("option.title");

		cout << endl << endl << "Liste des options par defaut : " << endl << endl;
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

		unsigned Choice(0);

		ClearScreen();
		ShowTitle("menu.title");

		vector <string> menulist = { "Jouer contre l'ordinateur (IA)", "Jouer à plusieurs", "Editeur de map" ,"Options", "Quitter" };


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
			DisplayEditor();
			break;
		case 4:
			DisplayOption();
			break;
		case 5:
			exit(0);

		default:

			Couleur(KRouge, KHCyan);
			cout << endl << "[!] Choix invalide !" << endl;
			Couleur(KReset);
		}
	} //DisplayMenu()

	void DisplaySoloIA() {

		unsigned Nbround = GetTourMax();
		bool IsPublicMap = IsPersoMapRecquiered();
		int Key;
		CMatrice Map;
		unsigned TourIA, Tour(0);

		PlayerX = InitPlayer(1, 1, 1, 1, TokenPlayerX);

		if (IsPublicMap) {

			PlayerY = InitPlayer(1, 1, KSizeX - 2, KSizeY - 2, TokenPlayerY);
			Map = LoadMapByFile(AskForMap());
			DetectObstacle(Map);
		}

		else {

			PlayerY = InitPlayer(1, 1, KSizeX - 1, KSizeY - 1, TokenPlayerY);
			Map = InitMatrice(KSizeX + 1, KSizeY + 1, PlayerX, PlayerY); /* +1 due à la bordure de '#' le long de la Matrice */

			if (KDifficult == 1)
				GenerateStaticObject(Map, KDifficult, 6, 5);
			else
				GenerateStaticObject(Map, KDifficult, Rand(7, 9), 4);
		}

		InitCurses();

		SetTextMiddle();
		ShowMatrice(Map);

		for (; Tour < Nbround * 2; ++Tour) {
			/*PlayerX = User. PlayerY = IA.*/
			SPlayer &actualPlayer = (Tour % 2 == 0 ? PlayerX : PlayerY);

			PrintLines(1);
			ShowTitle("solo.title");

			ShowMatrice(Map, false);

			ListenKeyboard();

			DisplayInfos(PlayerX);

			Couleur(KMagenta);
			cout << endl << "[+] A vous de jouer '"; Couleur(KBleu); cout << PlayerX.m_token << "' !" << endl << '\r';
			Couleur(KReset);

			if (!(Tour % 2 == 0))
				Key = getch();

			if (actualPlayer.m_token == PlayerX.m_token)
				KeyEvent(Key, Map, PlayerX);

			else if (actualPlayer.m_token == PlayerY.m_token) {
				MoveBot(Key, Map, TourIA);
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
			SetTextMiddle();
			PrintLines(16);
			cout << "[!] Egalité !" << endl;
			Couleur(KReset);

			endwin();

		}
	} //DisplaySoloIA()

	void DisplayEditor() {
		int Key;
		bool IsFirstTime = true;
		unsigned NbLine, NbColumn;

		ShowTitle("editor.title");
		for (unsigned i(0); i < 3; ++i) cout << endl;

		Couleur(KCyan);
		cout << "Quelle sera la taille de la carte ? \n\rTaille en hauteur : ";
		cin >> NbLine;
		cout << "Taille en largeur : ";
		cin >> NbColumn;

		CMatrice EmptyMatrice = InitMatrice(NbLine, NbColumn, PlayerX, PlayerY);
		SPlayer EmptyPlayer = InitPlayer(1, 1, 1, 1, '-');
		SBonus EmptyBonus;
		SObstacle EmptyObstacle;

		InitCurses();

		while (true) {


			ShowTitle("editor.title");
			PrintLines(3);
			Couleur(KBleu, KHJaune);
			cout << "[+] Votre carte ressemble actuellement à cela : \n\r";

			ShowMatrice(EmptyMatrice, false);
			Couleur(KCyan);
			cout << "\n\r\r\n\rUtilisez les A,Z,S,D pour vous diriger sur la map. \n\rCliquez sur " << BonusX << ", " << BonusY << " ou " << BonusZ << " pour placer des bonus" <<
				"\n\rCliquez sur ENTREE pour placer des obstacles\n\rUne fois votre edition finie, veuillez cliquer sur la touche M (menu)\n\r\n\rPour sauvegarder, veuillez maintenir CTRL+S. \n\r";
			Couleur(KReset);
			ListenKeyboard();

			if (!IsFirstTime)
				Key = getch();

			refresh();
			endwin();

			if (Key == 'm') {
				DisplayMenu();
				endwin();
				break;
			}


			//DEPLACEMENTS
			else if (Key == 'z')
				MovePlayer(EmptyMatrice, Key, EmptyPlayer);

			else if (Key == 'q')
				MovePlayer(EmptyMatrice, Key, EmptyPlayer);

			else if (Key == 's')
				MovePlayer(EmptyMatrice, Key, EmptyPlayer);

			else if (Key == 'd')
				MovePlayer(EmptyMatrice, Key, EmptyPlayer);

			//CREATION BONUS
			else if (Key == tolower(BonusX)) {
				EmptyBonus = InitBonus(EmptyPlayer.m_X, EmptyPlayer.m_Y, BonusX);
				MovePlayer(EmptyMatrice, 'd', EmptyPlayer);
				PutBonus(EmptyMatrice, EmptyBonus);
			}

			else if (Key == tolower(BonusY)) {
				EmptyBonus = InitBonus(EmptyPlayer.m_X, EmptyPlayer.m_Y, BonusY);
				MovePlayer(EmptyMatrice, 'd', EmptyPlayer);
				PutBonus(EmptyMatrice, EmptyBonus);
			}

			else if (Key == tolower(BonusZ)) {
				EmptyBonus = InitBonus(EmptyPlayer.m_X, EmptyPlayer.m_Y, BonusZ);
				MovePlayer(EmptyMatrice, 'd', EmptyPlayer);
				PutBonus(EmptyMatrice, EmptyBonus);
			}

			//OBSTACLE

			else if ('\n' == Key /*ENTREE*/) {
				EmptyObstacle = InitObstacle(EmptyPlayer.m_X, EmptyPlayer.m_Y, CaseObstacle);
				MovePlayer(EmptyMatrice, 'd', EmptyPlayer);
				PutObstacle(EmptyMatrice, EmptyObstacle);
			}


			//SAVE

			else if ((char(19)) == Key /*CTRL+S*/) {
				EmptyMatrice[EmptyPlayer.m_Y][EmptyPlayer.m_X] = CaseEmpty;

				string Name;
				Couleur(KRouge);
				cout << "\n\r[?] Nom de la map ? ";
				cin >> Name;
				string FullName = "./map/MyMap_" + Name + ".map";
				ExportMatrice(EmptyMatrice, FullName);
				Couleur(KCyan);
				cout << "[+]Map exportée à l'emplacement : " << FullName << "\n\r";
				Couleur(KReset);

				refresh();
				endwin();
				SLog.clear();
				DisplayMenu();
			}

			ClearScreen();
			IsFirstTime = false;

			//SOLVING OBSTACLE BUG FOR MAP CREATION

			EmptyMatrice[1][1] = CaseEmpty;

		}


		ShowMatrice(EmptyMatrice);
		refresh();
		endwin();
		DisplayMenu();
	}//DisplayEditor()
}

int main() {

	InitOptions();
	DisplayMenu();
	endwin();
	Couleur(KReset);
	PrintLines(3);

	return 0;
} //main()
