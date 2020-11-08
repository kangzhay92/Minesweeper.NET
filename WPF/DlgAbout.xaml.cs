using System.Diagnostics;
using System.Reflection;
using System.Windows;
using System.Windows.Navigation;

namespace Minesweeper.WPF
{
    /// <summary>
    /// About dialog, the content was automatically loaded from assembly
    /// </summary>
    public partial class DlgAbout : Window
    {
        public DlgAbout()
        {
            InitializeComponent();

            DataContext = this;
            Assembly assembly = Assembly.GetEntryAssembly();
            Version = assembly.GetName().Version.ToString();
            AppName = assembly.GetName().Name;

            var copyright = assembly.GetCustomAttribute<AssemblyCopyrightAttribute>();
            var description = assembly.GetCustomAttribute<AssemblyDescriptionAttribute>();
            var company = assembly.GetCustomAttribute<AssemblyCompanyAttribute>();

            Copyright = copyright.Copyright;
            Description = description.Description;
            Publisher = company.Company;
        }

        // This properties automatically loaded from assembly, use Assembly Information
        // on the project properties to modify
        public string AppName { get; set; }
        public string Version { get; set; }
        public string Copyright { get; set; }
        public string Description { get; set; }
        public string Publisher { get; set; }

        // This properties can be set outside the constructor
        public string Link { get; set; }
        public string LinkText { get; set; }
        public string AdditionalInfo { get; set; }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }

        private void Website_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            Process.Start(new ProcessStartInfo(e.Uri.AbsoluteUri));
            e.Handled = true;
        }

        private void Window_MouseLeftButtonDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            DragMove();
        }
    }
}
