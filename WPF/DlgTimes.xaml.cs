using System.Windows;

namespace Minesweeper.WPF
{
    /// <summary>
    /// Fastest times dialog
    /// </summary>
    public partial class DlgTimes : Window
    {
        public DlgTimes()
        {
            InitializeComponent();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }
    }
}
