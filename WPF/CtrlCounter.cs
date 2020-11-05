using Minesweeper.Core;

namespace Minesweeper.WPF
{
    /// <summary>
    /// Flag counter UI
    /// </summary>
    public class CtrlCounter : BaseCounter, ICounter
    {
        public void Draw(int number)
        {
            DrawLeds(number);
        }

        private CtrlGameBoard.GameBoard board;

        public CtrlGameBoard.GameBoard Board 
        {
            get => board;
            set
            {
                board = value;
                board.Counter = this;
            }
        }
    }
}
