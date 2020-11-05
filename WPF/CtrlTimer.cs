using Minesweeper.Core;
using System;
using System.Windows.Threading;

namespace Minesweeper.WPF
{
    /// <summary>
    /// Timer UI
    /// </summary>
    public class CtrlTimer : BaseCounter, ITimer
    {
        public CtrlTimer() : base()
        {
            Timer = new DispatcherTimer();
            Timer.Interval = new TimeSpan(0, 0, 0, 1, 120);
            Timer.Tick += Timer_Tick;
        }

        private CtrlGameBoard.GameBoard board;

        private DispatcherTimer Timer { get; }
        public CtrlGameBoard.GameBoard Board
        {
            get => board;
            set
            {
                board = value;
                board.Timer = this;
            }
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            board.UpdateTimer();
        }

        public void Draw(int number)
        {
            DrawLeds(number);
        }

        public void Start() { Timer.Start(); }
        public void Stop() { Timer.Stop(); }
    }
}
