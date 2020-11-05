using System.Windows;

namespace Minesweeper.WPF
{
    /// <summary>
    /// The custom game dialog, Note: Win in the custom mode
    /// won't add player to the fastest times list.
    /// </summary>
    public partial class DlgCustom : Window
    {
        public DlgCustom()
        {
            InitializeComponent();
            DataContext = this;
        }

        public int Columns { get; set; }
        public int Rows { get; set; }
        public int Mines { get; set; }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            if (Rows < 9)     Rows = 9;
            if (Rows > 24)    Rows = 24;
            if (Columns < 9)  Columns = 9;
            if (Columns > 30) Columns = 30;
            if (Mines < 10)   Mines = 10;
            if (Mines > (Columns - 1) * (Rows - 1)) Mines = (Columns - 1) * (Rows - 1);
            DialogResult = true;
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }
    }
}
