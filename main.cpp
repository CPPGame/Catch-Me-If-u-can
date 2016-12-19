#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <stdlib.h>   
#include <time.h>    
#include <random>


using namespace std;

const string KReset("0");
const string KNoir("30");
const string KRouge("31");
const string KVert("32");
const string KJaune("33");
const string KBleu("34");
const string KMagenta("35");
const string KCyan("36");

const char KLeft = 'q';
const char KTop = 'z';
const char KDown = 's';
const char KRight = 'd';

const char KEmpty = '.';
const char KObstacle = 'O';
const char KBorderLine = '_';
const char KBorderColumn = '|';

const unsigned KSizeY(10);
const unsigned KSizeX(10);

typedef vector <char> CVLine;
typedef vector <CVLine> CMatrix;
typedef vector <pair<unsigned, unsigned>> CForbidenPos;
namespace {
	typedef struct {
		unsigned m_X;
		unsigned m_Y;
		unsigned m_sizeX;
		unsigned m_sizeY;
		char m_token;
		string m_color;
	} SPlayer;

	typedef struct {
		unsigned m_X;
		unsigned m_Y;
		char m_token;
		string m_color;
	} SObstacle;

	typedef struct {
		unsigned m_X;
		unsigned m_Y;
		unsigned m_sizeX;
		unsigned m_sizeY;
		char m_token;
		string m_color;
	} SBonus;

	//AUTRE 
	void Couleur(const string & coul) {
		cout << "\033[" << coul << "m";

	}

	void ClearScreen() {
		cout << "\033[H\033[2J";
	}

	int IntRandom(int min, int max) { // Taken from Stackoverflow
		random_device randm;
		mt19937 rng(randm());
		uniform_int_distribution<int> uni(min, max);

		return uni(rng);
	}


	// MATRICE

	CMatrix InitMat(unsigned NbLine, unsigned NbColumn, SPlayer & FirstPlayer, SPlayer & SecondPlayer, bool ShowBorder) {
		CMatrix tmpMatrix;
		tmpMatrix.resize(NbLine);

		for (unsigned i(0); i < NbLine; ++i)
			for (unsigned j(0); j < NbColumn; ++j)
				tmpMatrix[i].push_back(KEmpty);



		for (unsigned i(FirstPlayer.m_Y); i < FirstPlayer.m_Y + FirstPlayer.m_sizeY; ++i)
			for (unsigned j(FirstPlayer.m_X); j < FirstPlayer.m_X + FirstPlayer.m_sizeX; ++j)
				tmpMatrix[i][j] = FirstPlayer.m_token;


		for (unsigned i(SecondPlayer.m_Y); i < SecondPlayer.m_Y + SecondPlayer.m_sizeY; ++i)
			for (unsigned j(SecondPlayer.m_X); j < SecondPlayer.m_X + SecondPlayer.m_sizeX; ++j)
				tmpMatrix[i][j] = SecondPlayer.m_token;

		if (ShowBorder) {
			for (unsigned i(0); i < NbLine; ++i) {

				tmpMatrix[i][0] = KBorderColumn;
				tmpMatrix[i][NbColumn - 1] = KBorderColumn;
			}

			for (unsigned i(0); i < NbColumn; ++i) {
				tmpMatrix[0][i] = KBorderLine;
				tmpMatrix[NbLine - 1][i] = KBorderLine;
			}

		}


		return tmpMatrix;
	}

	void ShowMatrix(const CMatrix & Mat, SPlayer & FirstPlayer, SPlayer & SecondPlayer) {
		ClearScreen();
		Couleur(KReset);
		for (CVLine line : Mat) {

			for (char C : line)
			{

				if (C == FirstPlayer.m_token) Couleur(FirstPlayer.m_color);
				if (C == SecondPlayer.m_token) Couleur(SecondPlayer.m_color);
				if ((C == FirstPlayer.m_token) || (C == SecondPlayer.m_token)) Couleur(KReset);
				cout << C;
			}
			cout << endl;
		}
	}

	// WIN CHECK/STAT

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
		AxeX, const unsigned AxeY, const char Token, const string Color) {

		SBonus tmpBonus;

		tmpBonus.m_sizeX = largeur;
		tmpBonus.m_sizeY = hauteur;
		tmpBonus.m_X = AxeX;
		tmpBonus.m_Y = AxeY;
		tmpBonus.m_token = Token;
		tmpBonus.m_color = Color;

		return tmpBonus;
	}

	void PutBonus(CMatrix & Matrice, SBonus & Bonus) {
		Matrice[Bonus.m_Y][Bonus.m_X] = Bonus.m_token;
	}

	void GetBonus(CMatrix & Mat, SPlayer & Player) {
		for (unsigned i(Player.m_Y); i < Player.m_Y + Player.m_sizeY; ++i) {
			for (unsigned j(Player.m_X); j < Player.m_X + Player.m_sizeX; ++j) {
				//Début de la détéction des bonus

				if (Mat[i][j] == 'B') {

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

	SObstacle InitObstacle(const unsigned
		AxeX, const unsigned AxeY, const char Token, const string Color, CForbidenPos & ForbidenPositions) {

		SObstacle tmpObstacle;

		tmpObstacle.m_X = AxeX;
		tmpObstacle.m_Y = AxeY;
		tmpObstacle.m_token = Token;
		tmpObstacle.m_color = Color;
		ForbidenPositions.push_back(make_pair(AxeX, AxeY));
		return tmpObstacle;
	}


	void PutObstacle(CMatrix & Matrice, const SObstacle & obstacle) {
		Matrice[obstacle.m_X][obstacle.m_Y] = obstacle.m_token;
	}

	void GenerateRandomObstacles(CMatrix & Matrice, SObstacle & Obstacle, const unsigned & SizeObs) {
		unsigned tmpX, tmpY;
		for (unsigned i(0); i < SizeObs; ++i)
		{
			tmpX = Obstacle.m_X;
			tmpY = Obstacle.m_Y;
			int intrandom = IntRandom(1, 2);


			if (intrandom == 1) {
				Obstacle.m_X = tmpX + 1;
				PutObstacle(Matrice, Obstacle);
			}
			else if (intrandom == 2) {
				Obstacle.m_Y = tmpY + 1;
				PutObstacle(Matrice, Obstacle);

			}
		}
	}

	bool CanMove(SPlayer & Player, const CForbidenPos ForbidenPositions) {

		for (unsigned i(0); i < ForbidenPositions.size(); ++i) {
			for (unsigned i(0); i <= Player.m_sizeX; ++i) {
 				if (ForbidenPositions[i].first == (Player.m_X + i + 1) && ForbidenPositions[i].second == (Player.m_Y + i - 1)) return false;
				if (ForbidenPositions[i].first == (Player.m_X + i - 1) && ForbidenPositions[i].second == (Player.m_Y + i - 1)) return false;
				if (ForbidenPositions[i].first == (Player.m_X + i - 1) && ForbidenPositions[i].second == (Player.m_Y + i + 1)) return false;

				if (ForbidenPositions[i].first == (Player.m_X + i) && ForbidenPositions[i].second == (Player.m_Y + i + 1)) return false;
				if (ForbidenPositions[i].first == (Player.m_X + i) && ForbidenPositions[i].second == (Player.m_Y + i - 1)) return false;

				if (ForbidenPositions[i].first == (Player.m_X + i + 1) && ForbidenPositions[i].second == (Player.m_Y + i)) return false;
				if (ForbidenPositions[i].first == (Player.m_X + i) && ForbidenPositions[i].second == (Player.m_Y + i + 1)) return false;

				if (ForbidenPositions[i].first == (Player.m_X + i - 1) && ForbidenPositions[i].second == (Player.m_Y + i)) return false;
				if (ForbidenPositions[i].first == (Player.m_X + i) && ForbidenPositions[i].second == (Player.m_Y + i - 1)) return false;

				if (ForbidenPositions[i].first == (Player.m_X + i) && ForbidenPositions[i].second == (Player.m_Y + i)) return false;
				if (ForbidenPositions[i].first == (Player.m_X - i + 1) && ForbidenPositions[i].second == (Player.m_Y - i + 1)) return false;
				if (ForbidenPositions[i].first == (Player.m_X - i + 1) && ForbidenPositions[i].second == (Player.m_Y - i - 1)) return false;
				if (ForbidenPositions[i].first == (Player.m_X - i - 1) && ForbidenPositions[i].second == (Player.m_Y - i - 1)) return false;
				if (ForbidenPositions[i].first == (Player.m_X - i - 1) && ForbidenPositions[i].second == (Player.m_Y - i + 1)) return false;
								if (ForbidenPositions[i].first == (Player.m_X - i) && ForbidenPositions[i].second == (Player.m_Y - i + 1)) return false;
				if (ForbidenPositions[i].first == (Player.m_X - i) && ForbidenPositions[i].second == (Player.m_Y - i - 1)) return false;

				if (ForbidenPositions[i].first == (Player.m_X - i + 1) && ForbidenPositions[i].second == (Player.m_Y - i)) return false;
				if (ForbidenPositions[i].first == (Player.m_X - i) && ForbidenPositions[i].second == (Player.m_Y - i + 1)) return false;

				if (ForbidenPositions[i].first == (Player.m_X - i - 1) && ForbidenPositions[i].second == (Player.m_Y - i)) return false;
				if (ForbidenPositions[i].first == (Player.m_X - i) && ForbidenPositions[i].second == (Player.m_Y - i - 1)) return false;

				if (ForbidenPositions[i].first == (Player.m_X - i) && ForbidenPositions[i].second == (Player.m_Y - i)) return false;

			}
		
			
			/*if (ForbidenPositions[i].first == (Player.m_X + 1) && ForbidenPositions[i].second == (Player.m_Y + 1)) return false;
			if (ForbidenPositions[i].first == (Player.m_X + 1) && ForbidenPositions[i].second == (Player.m_Y - 1)) return false;
			if (ForbidenPositions[i].first == (Player.m_X - 1) && ForbidenPositions[i].second == (Player.m_Y - 1)) return false;
			if (ForbidenPositions[i].first == (Player.m_X - 1) && ForbidenPositions[i].second == (Player.m_Y + 1)) return false;

			if (ForbidenPositions[i].first == (Player.m_X) && ForbidenPositions[i].second == (Player.m_Y + 1)) return false;
			if (ForbidenPositions[i].first == (Player.m_X) && ForbidenPositions[i].second == (Player.m_Y - 1)) return false;

			if (ForbidenPositions[i].first == (Player.m_X + 1) && ForbidenPositions[i].second == (Player.m_Y)) return false;
			if (ForbidenPositions[i].first == (Player.m_X) && ForbidenPositions[i].second == (Player.m_Y +1)) return false;

			if (ForbidenPositions[i].first == (Player.m_X - 1) && ForbidenPositions[i].second == (Player.m_Y)) return false;
			if (ForbidenPositions[i].first == (Player.m_X) && ForbidenPositions[i].second == (Player.m_Y - 1)) return false;

			if (ForbidenPositions[i].first == (Player.m_X ) && ForbidenPositions[i].second == (Player.m_Y )) return false;
		*/
		}

		return true;

	}

	// PLAYERS

	SPlayer InitPlayer(const unsigned  largeur, const unsigned  hauteur, const unsigned  AxeX, const unsigned  AxeY, const char  Token, const string  Color) {

		SPlayer tmpPlayer;

		tmpPlayer.m_sizeX = largeur;
		tmpPlayer.m_sizeY = hauteur;
		tmpPlayer.m_X = AxeX;
		tmpPlayer.m_Y = AxeY;
		tmpPlayer.m_token = Token;
		tmpPlayer.m_color = Color;

		return tmpPlayer;
	}

	void MovePlayer(CMatrix & Mat, char Move, SPlayer & player, CForbidenPos & ForbidenPos) {
		switch (Move)
		{
		case KTop:
			if (CanMove(player, ForbidenPos)) {
				if (player.m_Y > 0)
				{
					player.m_Y = player.m_Y - 1;
					GetBonus(Mat, player);
					for (unsigned i(player.m_X); i < player.m_X + player.m_sizeX; ++i)
						Mat[player.m_Y + player.m_sizeY][i] = KEmpty;
					for (unsigned i(player.m_X); i < player.m_X + player.m_sizeX; ++i)
						Mat[player.m_Y][i] = player.m_token;

				}
			}
			break;
		case KDown:
			if (CanMove(player, ForbidenPos)) {
				if (player.m_Y + player.m_sizeY < Mat.size())
				{


					player.m_Y = player.m_Y + 1;
					GetBonus(Mat, player);

					for (unsigned i(player.m_X); i < player.m_X + player.m_sizeX; ++i)
						Mat[player.m_Y - 1][i] = KEmpty;
					for (unsigned i(player.m_X); i < player.m_X + player.m_sizeX; ++i)
						Mat[player.m_Y + player.m_sizeY - 1][i] = player.m_token;
				}
			}

			break;
		case KLeft:
			if (CanMove(player, ForbidenPos)) {
				if (player.m_X > 0)
				{

					player.m_X = player.m_X - 1;
					GetBonus(Mat, player);
					for (unsigned i(player.m_Y); i < player.m_Y + player.m_sizeY; ++i)
						Mat[i][player.m_X + player.m_sizeX] = KEmpty;
					for (unsigned i(player.m_Y); i < player.m_Y + player.m_sizeY; ++i)
						Mat[i][player.m_X] = player.m_token;
				}

			}
			break;
		case KRight:
			if (CanMove(player, ForbidenPos)) {
				if (player.m_X + player.m_sizeX < Mat[0].size())
				{

					player.m_X = player.m_X + 1;
					GetBonus(Mat, player);
					for (unsigned i(player.m_Y); i < player.m_Y + player.m_sizeY; ++i)
						Mat[i][player.m_X - 1] = KEmpty;
					for (unsigned i(player.m_Y); i < player.m_Y + player.m_sizeY; ++i)
						Mat[i][player.m_X + player.m_sizeX - 1] = player.m_token;

				}
			}
			break;


		}
	}

	//AUTRE
	unsigned AskTourMax() {
		unsigned nbrnds;
		cout << "Entrez le nombre de rounds" << endl;
		cin >> nbrnds;
		return nbrnds;
	}

}

int main()
{

	unsigned NbRnds = AskTourMax();
	char EnteredKey;

	CForbidenPos ForbidenPos;
	SPlayer FirstPlayer = InitPlayer(1, 1, 1, KSizeY / 2, 'X', KRouge);
	SPlayer SecondPlayer = InitPlayer(1, 1, KSizeX - 2, KSizeY - 2, 'Y', KBleu);
	SObstacle FirstObstacle = InitObstacle((KSizeX / 2), (KSizeX / 2), KObstacle, KMagenta, ForbidenPos);
	SBonus FirstBonus = InitBonus(2, 2, 4, 4, 'B', KRouge);
	CMatrix Map = InitMat(KSizeX + 1, KSizeY + 1, FirstPlayer, SecondPlayer, true); // +1 due à la bordure de '#' le long de la matrice

	GenerateRandomObstacles(Map, FirstObstacle, 4);
	PutBonus(Map, FirstBonus);

	ShowMatrix(Map, FirstPlayer, SecondPlayer);
	for (unsigned i(0); i < NbRnds * 2; ++i)
	{

		ShowMatrix(Map, FirstPlayer, SecondPlayer);
		SPlayer & actualPlayer = (i % 2 == 0 ? FirstPlayer : SecondPlayer);

		cout << "Au tour de " << actualPlayer.m_token << endl;
		cin >> EnteredKey;

		MovePlayer(Map, EnteredKey, actualPlayer, ForbidenPos);

		if (CheckIfWin(FirstPlayer, SecondPlayer))
		{
			cout << GetWinner(FirstPlayer, SecondPlayer, i) << " a gagné !" << endl;
			return 0;
		}
	}
	cout << "Egalité !" << endl;
	return 0;
}
