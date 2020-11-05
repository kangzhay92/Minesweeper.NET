#pragma once

using namespace System;

namespace Minesweeper {
namespace Core {
	/// <summary>
	/// Interface for creating counter UI
	/// </summary>
	public interface class ICounter
	{
		void Draw(int number);
	};

	/// <summary>
	/// Interface for creating timer UI
	/// </summary>
	public interface class ITimer : ICounter
	{
		void Start();
		void Stop();
	};

	/// <summary>
	/// Interface for creating face button UI
	/// </summary>
	public interface class IFaceButton
	{
		void Draw(FaceImage image);
	};

	/// <summary>
	/// Interface for creating boxes UI
	/// </summary>
	public interface class IBox
	{
		property int Column;
		property int Row;
		property FlagType Flag;
		property bool IsMine;
		property int MinesAround;

		void Draw(MineImage image);
	};

	/// <summary>
	/// Class for handling the game logic.
	///
	/// This is an abstract class, so we must create derived class
	/// from this.
	/// </summary>
	public ref class Board abstract
	{
		using BoxCollection = IList<IBox^>;

	protected:
		virtual IBox^ CreateBox() = 0;

		property IBox^ SelectedBox;
		property int BoxesLeft { int get() { return boxesLeft; } }

	public:
		delegate void WinCallback();

		void NewGame();
		void NewGame(GameDifficulty difficulty);
		void NewGame(int columns, int rows, int mines);

		void FaceButtonPress();
		void FaceButtonUpdate(bool handled);

		void BoxPress(IBox^ box);
		void BoxUpdate(bool handled);
		void BoxChange(IBox^ box);
		void UpdateFlag(IBox^ box);
		void UpdateTimer();

		property int Columns   { int get() { return cols; } }
		property int Rows	   { int get() { return rows; } }
		property int NumMines  { int get() { return mines; } }
		property int Time      { int get() { return time; } }

		property GameDifficulty Difficulty { GameDifficulty get() { return difficulty; } }
		property BoxCollection^ Boxes {	virtual BoxCollection^ get() abstract; }
		property WinCallback^ OnWin;

		virtual property ICounter^ Counter;
		virtual property ITimer^ Timer;
		virtual property IFaceButton^ FaceButton;
		virtual property bool MarkQuestion;

		property IBox^ default[int, int]
		{
			IBox^ get(int col, int row);
		}

	private:
		void Build();
		void PlaceMines();
		void CompleteBox(IBox^ box);
		void CheckWin();

		int rows, cols, mines, boxesLeft, time, numFlags;
		GameDifficulty difficulty;
		GameStatus status;
	};
}}
