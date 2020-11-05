using Minesweeper.Core;
using Minesweeper.WPF.Properties;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;

namespace Minesweeper.WPF
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();

            Loaded += MainWindow_Loaded;
        }

        private Dictionary<GameDifficulty, string> BestNames;
        private Dictionary<GameDifficulty, int> BestTimes;

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            // Initialize user controls
            ctrlGameBoard.Board.OnWin = OnWin;
            ctrlCounter.Board = ctrlGameBoard.Board;
            ctrlTimer.Board = ctrlGameBoard.Board;
            ctrlFaceButton.Board = ctrlGameBoard.Board;

            // Load application settings from persistent data, and start new game
            LoadSettings();
            UpdateMenu();
            UpdateGameScreen();
        }

        /// <summary>
        /// Load application settings from user data
        /// </summary>
        private void LoadSettings()
        {
            // Initialize the fastest times data
            BestNames = new Dictionary<GameDifficulty, string>()
            {
                { GameDifficulty.Beginner, Settings.Default.Name0 },
                { GameDifficulty.Advanced, Settings.Default.Name1 },
                { GameDifficulty.Expert,   Settings.Default.Name2 }
            };
            BestTimes = new Dictionary<GameDifficulty, int>()
            {
                { GameDifficulty.Beginner, Settings.Default.Time0 },
                { GameDifficulty.Advanced, Settings.Default.Time1 },
                { GameDifficulty.Expert,   Settings.Default.Time2 }
            };

            // Initialize mark question menu, to enable question mark
            // while playing the game
            ctrlGameBoard.Board.MarkQuestion = Settings.Default.MarkQuestion;
            
            // Start new game based on difficulty
            if (Settings.Default.Difficulty == GameDifficulty.Custom)
            {
                ctrlGameBoard.Board.NewGame(Settings.Default.Columns, Settings.Default.Rows, Settings.Default.Mines);
            }
            else
            {
                ctrlGameBoard.Board.NewGame(Settings.Default.Difficulty);
            }
        }

        /// <summary>
        /// Store application settings to user data
        /// </summary>
        private void SaveSettings()
        {
            Settings.Default.Name0 = BestNames[GameDifficulty.Beginner];
            Settings.Default.Name1 = BestNames[GameDifficulty.Advanced];
            Settings.Default.Name2 = BestNames[GameDifficulty.Expert];
            Settings.Default.Time0 = BestTimes[GameDifficulty.Beginner];
            Settings.Default.Time1 = BestTimes[GameDifficulty.Advanced];
            Settings.Default.Time2 = BestTimes[GameDifficulty.Expert];

            Settings.Default.MarkQuestion = ctrlGameBoard.Board.MarkQuestion;
            Settings.Default.Difficulty = ctrlGameBoard.Board.Difficulty;
            
            if (ctrlGameBoard.Board.Difficulty == GameDifficulty.Custom)
            {
                Settings.Default.Rows = ctrlGameBoard.Board.Rows;
                Settings.Default.Columns = ctrlGameBoard.Board.Columns;
                Settings.Default.Mines = ctrlGameBoard.Board.NumMines;
            }

            Settings.Default.Save();
        }

        /// <summary>
        /// Reload the canvas and window size based on game board
        /// </summary>
        private void UpdateGameScreen()
        {
            ctrlGameBoard.Width = ctrlGameBoard.Board.Columns * 16;
            ctrlGameBoard.Height = ctrlGameBoard.Board.Rows * 16;

            ctrlGameScreen.Width = ctrlGameBoard.Width + 10;
            ctrlGameScreen.Height = ctrlGameBoard.Height + 38;

            Width = ctrlGameScreen.Width + 2;
            Height = ctrlGameScreen.Height + 46;

            Canvas.SetLeft(ctrlFaceButton, ctrlGameScreen.Width / 2 - ctrlFaceButton.Width / 2);
            Canvas.SetLeft(ctrlCounter, ctrlGameScreen.Width - 5 - ctrlCounter.Width);
        }

        /// <summary>
        /// Reload the main menu
        /// </summary>
        private void UpdateMenu()
        {
            mnuMarkQuestion.IsChecked = ctrlGameBoard.Board.MarkQuestion;
            mnuBeginner.IsChecked = ctrlGameBoard.Board.Difficulty == GameDifficulty.Beginner;
            mnuAdvanced.IsChecked = ctrlGameBoard.Board.Difficulty == GameDifficulty.Advanced;
            mnuExpert.IsChecked = ctrlGameBoard.Board.Difficulty == GameDifficulty.Expert;
            mnuCustom.IsChecked = ctrlGameBoard.Board.Difficulty == GameDifficulty.Custom;
        }

        /// <summary>
        /// Show fastest times dialog using data from settings
        /// </summary>
        private void ShowFastestTimesDialog()
        {
            DlgTimes dlg = new DlgTimes
            {
                Owner = this,
                lblName0 = { Content = BestNames[GameDifficulty.Beginner] },
                lblName1 = { Content = BestNames[GameDifficulty.Advanced] },
                lblName2 = { Content = BestNames[GameDifficulty.Expert] },
                lblTime0 = { Content = BestTimes[GameDifficulty.Beginner] },
                lblTime1 = { Content = BestTimes[GameDifficulty.Advanced] },
                lblTime2 = { Content = BestTimes[GameDifficulty.Expert] }
            };
            dlg.ShowDialog();
        }

        private void btnMinimize_Click(object sender, RoutedEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }

        private void btnClose_Click(object sender, RoutedEventArgs e)
        {
            SaveSettings();
            Application.Current.Shutdown();
        }

        private void mnuExit_Click(object sender, RoutedEventArgs e)
        {
            SaveSettings();
            Application.Current.Shutdown();
        }

        private void mnuNew_Click(object sender, RoutedEventArgs e)
        {
            ctrlGameBoard.Board.NewGame();
        }

        private void mnuMarkQuestion_Click(object sender, RoutedEventArgs e)
        {
            ctrlGameBoard.Board.MarkQuestion = !ctrlGameBoard.Board.MarkQuestion;
            UpdateMenu();
        }

        private void mnuBeginner_Click(object sender, RoutedEventArgs e)
        {
            ctrlGameBoard.Board.NewGame(GameDifficulty.Beginner);
            UpdateGameScreen();
            UpdateMenu();
        }

        private void mnuAdvanced_Click(object sender, RoutedEventArgs e)
        {
            ctrlGameBoard.Board.NewGame(GameDifficulty.Advanced);
            UpdateGameScreen();
            UpdateMenu();
        }

        private void mnuExpert_Click(object sender, RoutedEventArgs e)
        {
            ctrlGameBoard.Board.NewGame(GameDifficulty.Expert);
            UpdateGameScreen();
            UpdateMenu();
        }

        private void mnuCustom_Click(object sender, RoutedEventArgs e)
        {
            DlgCustom dlg = new DlgCustom
            {
                Owner = this,
                Columns = ctrlGameBoard.Board.Columns,
                Rows = ctrlGameBoard.Board.Rows,
                Mines = ctrlGameBoard.Board.NumMines
            };
            dlg.ShowDialog();
            if (dlg.DialogResult == true)
            {
                ctrlGameBoard.Board.NewGame(dlg.Columns, dlg.Rows, dlg.Mines);
                UpdateGameScreen();
            }
            UpdateMenu();
        }

        private void mnuFastestTimes_Click(object sender, RoutedEventArgs e)
        {
            ShowFastestTimesDialog();
        }

        /// <summary>
        /// This function will be called on win condition, congratulate the player here!
        /// </summary>
        private void OnWin()
        {
            if (ctrlGameBoard.Board.Time < BestTimes[ctrlGameBoard.Board.Difficulty])
            {
                DlgCongrats dlg = new DlgCongrats
                {
                    Owner = this
                };
                dlg.ShowDialog();
                if (dlg.DialogResult == true)
                {
                    BestNames[ctrlGameBoard.Board.Difficulty] = dlg.txtName.Text;
                    BestTimes[ctrlGameBoard.Board.Difficulty] = ctrlGameBoard.Board.Time;
                    SaveSettings();
                    ShowFastestTimesDialog();
                }
            }
        }
    }
}
