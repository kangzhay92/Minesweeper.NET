using System.Windows;

namespace Minesweeper.WPF
{
    /// <summary>
    /// The congratulations dialog, that will be shown if the
    /// player win the game and beat the fastest times.
    /// </summary>
    public partial class DlgCongrats : Window
    {
        public DlgCongrats()
        {
            InitializeComponent();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }
    }
}
