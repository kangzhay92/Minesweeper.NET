#pragma once

using namespace System;
using namespace System::Diagnostics;
using namespace System::Collections::Generic;

namespace Minesweeper {
namespace Core {
	public enum class FaceImage
	{
		Press, Cool, Dead, Ooh, Smile
	};

	public enum class MineImage
	{
		MinePress,
		One, Two, Three, Four, Five, Six, Seven, Eight,
		Box, Flag, Question, Explode, Wrong, Mine, QuestionPress
	};

	public enum class GameStatus
	{
		Waiting, Playing, GameOver, Won
	};

	public enum class GameDifficulty
	{
		Beginner, Advanced, Expert, Custom
	};

	public enum class FlagType
	{
		Normal, Question, Flag, Complete
	};
}}
