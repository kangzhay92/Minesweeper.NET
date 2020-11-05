#include "pch.h"
#include "Board.h"
#include <random>

static std::mt19937_64 randGen;

namespace Minesweeper {
namespace Core {

/// <summary>
/// Start a new game based on game difficulty.
/// </summary>
/// <param name="difficulty">Game difficulty, must be Beginner, Advanced, or Expert</param>
void Board::NewGame(GameDifficulty difficulty)
{
	Debug::Assert(difficulty != GameDifficulty::Custom, "Use NewGame(int,int,int) function for custom board type");

	this->difficulty = difficulty;
	switch (difficulty) {
		case GameDifficulty::Beginner:
			rows = 9;
			cols = 9;
			mines = 10;
			break;
		case GameDifficulty::Advanced:
			rows = 16;
			cols = 16;
			mines = 40;
			break;
		case GameDifficulty::Expert:
			rows = 16;
			cols = 30;
			mines = 99;
			break;
	}
	Build();
}

/// <summary>
/// Start a new custom game.
/// </summary>
/// <param name="cols">Number of colums</param>
/// <param name="rows">Number of rows</param>
/// <param name="mines">Number of mines</param>
void Board::NewGame(int cols, int rows, int mines)
{
	this->cols = cols;
	this->rows = rows;
	this->mines = mines;
	this->difficulty = GameDifficulty::Custom;
	Build();
}

/// <summary>
/// Start a new game from the existing values.
/// </summary>
void Board::NewGame()
{
	if (difficulty != GameDifficulty::Custom) {
		NewGame(difficulty);
	} else {
		NewGame(cols, rows, mines);
	}
}

/// <summary>
/// Reset the game board to initial state.
/// </summary>
void Board::Build()
{
	status = GameStatus::Waiting;
	time = 0;
	numFlags = 0;
	boxesLeft = cols * rows - mines;

	// Draw the boxes
	Boxes->Clear();
	for (int row = 0; row < rows; row++) {
		for (int col = 0; col < cols; col++) {
			auto box = CreateBox();
			box->Column = col;
			box->Row = row;
			box->IsMine = false;
			box->MinesAround = 0;
			box->Flag = FlagType::Normal;
			Boxes->Add(box);
		}
	}
	for each (auto box in Boxes){
		box->Draw(MineImage::Box);
	}

	// Draw the timer, counter, and face button UI
	if (Timer) {
		Timer->Draw(0);
	}
	if (Counter) {
		Counter->Draw(mines);
	}
	if (FaceButton) {
		FaceButton->Draw(FaceImage::Smile);
	}
}

/// <summary>
/// Syntactic sugar for accessing specified box from
/// the boxes list.
/// </summary>
/// <param name="col">Box column on the game board</param>
/// <param name="row">Box row on the game board.</param>
/// <returns>The designed box, or null if col and row is out of the bounds.</returns>
IBox^ Board::default::get(int col, int row)
{
	if (col < 0 || row < 0 || col >= cols || row >= rows) {
		return nullptr;
	}
	return Boxes[row % rows * cols + col % cols];
}

/// <summary>
/// Place mines at random position on the board. This method won't be called
/// until the player press a box on the board while starting a new game. 
/// </summary>
void Board::PlaceMines()
{
	// Initialize random number generator, using mersenne twister algorithm for faster performance
	randGen.seed(DateTimeOffset::Now.ToUnixTimeMilliseconds());
	
	// Spread the mines using uniform distribution, temporary set the selected box
	// to mine and switch back to normal box later. So player won't hit a mine at 
	// the first time.
	int mine = 0;
	SelectedBox->IsMine = true;
	while (mine < mines) {
		auto cDist = std::uniform_int_distribution<>(0, cols - 1);
		auto rDist = std::uniform_int_distribution<>(0, rows - 1);
		int col = cDist(randGen);
		int row = rDist(randGen);
		if (!this[col, row]->IsMine) {
			mine++;
			this[col, row]->IsMine = true;
		}
	}
	SelectedBox->IsMine = false;

	// Set the number for each boxes, the number indicates total of
	// mines on the 3x3 cells around the box
	for each (auto box in Boxes) {
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				auto neighbor = this[box->Column + i, box->Row + j];
				if (!neighbor) {
					continue;
				}
				if (neighbor->IsMine) {
					box->MinesAround++;
				}
			}
		}
	}
}

/// <summary>
/// Win condition occurred when the player complete all boxes without hit a mine.
/// </summary>
void Board::CheckWin()
{
	if (boxesLeft == 0) {
		// set all mines on the board to flag image
		status = GameStatus::Won;
		if (numFlags < mines) {  
			for each (auto box in Boxes) {
				if (box->IsMine && box->Flag != FlagType::Flag) {
					box->Draw(MineImage::Flag);
				}
			}
		}
		numFlags = mines;

		// redraw the counter and stop the timer
		if (Counter) {
			Counter->Draw(0);
		}
		if (Timer) {
			Timer->Stop();
		}

		// then congratulate the player if OnWin function was set, and
		// not in custom mode.
		if (difficulty != GameDifficulty::Custom && OnWin) {
			OnWin->Invoke();
		}
	}
}

/// <summary>
/// This method called after the player click a box on the game board.
/// </summary>
/// <param name="box">The designed box</param>
void Board::CompleteBox(IBox^ box)
{
	if (box->Flag != FlagType::Complete && box->Flag != FlagType::Flag) {
		box->Flag = FlagType::Complete;
		if (box->IsMine) {
			// if the box is a mine, we lose
			status = GameStatus::GameOver;
		} else if (status != GameStatus::GameOver) {
			// continue the playing elsewhere
			boxesLeft--;
		}
		switch (box->MinesAround) {
			// draw the box number, empty box if no mines around
			case 0: box->Draw(MineImage::MinePress); break;
			case 1: box->Draw(MineImage::One);       break;
			case 2: box->Draw(MineImage::Two);       break;
			case 3: box->Draw(MineImage::Three);     break;
			case 4: box->Draw(MineImage::Four);      break;
			case 5: box->Draw(MineImage::Five);      break;
			case 6: box->Draw(MineImage::Six);       break;
			case 7: box->Draw(MineImage::Seven);     break;
			case 8: box->Draw(MineImage::Eight);     break;
		}
		if (box->MinesAround == 0) {
			// recursively complete the box if there is no mines
			// in the 3x3 cells around the box
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					auto neighbor = this[box->Column + i, box->Row + j];
					if (neighbor != nullptr) {
						CompleteBox(neighbor);
					}
				}
			}
		}
	}
}

/// <summary>
/// Called when user press the face button
/// </summary>
void Board::FaceButtonPress()
{
	Debug::Assert(FaceButton != nullptr);
	FaceButton->Draw(FaceImage::Press);
}

/// <summary>
/// Called when user completely click the face button.
/// </summary>
/// <param name="handled">set false if you want to redraw the face button only (not triggering new game)</param>
void Board::FaceButtonUpdate(bool handled)
{
	Debug::Assert(FaceButton != nullptr);
	switch (status) {
		case GameStatus::GameOver:
			FaceButton->Draw(FaceImage::Dead);
			break;
		case GameStatus::Won:
			FaceButton->Draw(FaceImage::Cool);
			break;
		default:
			FaceButton->Draw(FaceImage::Smile);
			break;
	}
	if (handled) {
		NewGame();
	}
}

/// <summary>
/// Called when user press a box on the game board.
/// </summary>
/// <param name="box">The selected box</param>
void Board::BoxPress(IBox^ box)
{
	if (status != GameStatus::Won && status != GameStatus::GameOver) {
		if (box->Flag != FlagType::Flag && box->Flag != FlagType::Complete) {
			SelectedBox = box;
		}
		if (SelectedBox) {
			SelectedBox->Draw(SelectedBox->Flag == FlagType::Question ? MineImage::QuestionPress : MineImage::MinePress);
		}
		if (FaceButton) {
			FaceButton->Draw(FaceImage::Ooh);
		}
	}
}

/// <summary>
/// Called when user completely click the selected box.
/// </summary>
/// <param name="handled">set false if you want to redraw the selected box only (won't open the box)</param>
void Board::BoxUpdate(bool handled)
{
	if (status != GameStatus::Won && status != GameStatus::GameOver) {
		if (handled) {
			if (SelectedBox) {
				if (status != GameStatus::Playing) {
					// Start game for the first time player press the box
					status = GameStatus::Playing;
					PlaceMines();
					if (Timer) {
						Timer->Start();
					}
				}
				CompleteBox(SelectedBox);
			}
			if (status == GameStatus::GameOver) {
				for each (auto b in Boxes) {
					if (b->IsMine) {
						// reveal all mines in the board
						if (b->Flag == FlagType::Normal) {
							b->Draw(MineImage::Mine);
						} else if (b->Flag == FlagType::Complete) {
							b->Draw(MineImage::Explode);
						}
					} else {
						// if the box is not mine and player place flag on it,
						// show the wrong image
						if (b->Flag == FlagType::Flag) {
							b->Draw(MineImage::Wrong);
						}
					}
				}
				if (Timer) {
					Timer->Stop();
				}
			} else {
				CheckWin();
			}
		} else {
			if (SelectedBox) {
				SelectedBox->Draw(SelectedBox->Flag == FlagType::Question ? MineImage::Question : MineImage::Box);
			}
		}
	}
	SelectedBox = nullptr;
	if (FaceButton) {
		// redraw the face button
		FaceButtonUpdate(false);
	}
}

/// <summary>
/// Switch to another box (on left mouse button click and move the mouse)
/// </summary>
/// <param name="box">The box that want to set</param>
void Board::BoxChange(IBox^ box)
{
	if (status != GameStatus::Won && status != GameStatus::GameOver) {
		if (SelectedBox) {
			SelectedBox->Draw(SelectedBox->Flag == FlagType::Question ? MineImage::Question : MineImage::Box);
		}
		SelectedBox = nullptr;
		BoxPress(box);
	}
}

/// <summary>
/// Set the flag of a box (on right mouse button click)
/// </summary>
/// <param name="box">The box that want to set the flag</param>
void Board::UpdateFlag(IBox^ box)
{
	switch (box->Flag) {
		case FlagType::Complete:
			return;
		case FlagType::Flag:
			box->Flag = MarkQuestion ? FlagType::Question : FlagType::Normal;
			box->Draw(box->Flag == FlagType::Question ? MineImage::Question : MineImage::Box);
			numFlags--;
			break;
		case FlagType::Question:
			box->Flag = FlagType::Normal;
			box->Draw(MineImage::Box);
			break;
		default:
			box->Flag = FlagType::Flag;
			box->Draw(MineImage::Flag);
			numFlags++;
			break;
	}
	if (Counter) {
		Counter->Draw(mines - numFlags);
	}
}

/// <summary>
/// Redraw the timer UI
/// </summary>
void Board::UpdateTimer()
{
	Debug::Assert(Timer != nullptr);
	if (status == GameStatus::Playing) {
		time++;
		Timer->Draw(time);
	}
}

}}
