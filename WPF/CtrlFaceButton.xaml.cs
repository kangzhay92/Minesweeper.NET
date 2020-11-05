using Minesweeper.Core;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace Minesweeper.WPF
{
    /// <summary>
    /// Face button UI
    /// </summary>
    public partial class FaceButton : FrameworkElement, IFaceButton
    {
        public FaceButton()
        {
            InitializeComponent();

            Visual = new DrawingVisual();
            AddVisualChild(Visual);

            FacesImage = Application.Current.FindResource("FacesImage") as ResourceDictionary;

            MouseLeftButtonDown += FaceButton_MouseLeftButtonDown;
            MouseLeftButtonUp += FaceButton_MouseLeftButtonUp;
            MouseLeave += FaceButton_MouseLeave;
        }

        private void FaceButton_MouseLeave(object sender, MouseEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                board.FaceButtonUpdate(false);
            }
        }

        private void FaceButton_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            board.FaceButtonUpdate(true);
        }

        private void FaceButton_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            board.FaceButtonPress();
        }

        public void Draw(FaceImage img)
        {
            var image = FacesImage[img.ToString()] as BitmapSource;
            DrawingContext ctx = Visual.RenderOpen();
            ctx.DrawImage(image, new Rect(0, 0, image.Width, image.Height));
            ctx.Close();
        }

        private DrawingVisual Visual { get; }
        private ResourceDictionary FacesImage { get; }

        protected override int VisualChildrenCount => 1;
        protected override Visual GetVisualChild(int index) => Visual;

        public CtrlGameBoard.GameBoard Board 
        { 
            get => board; 
            set
            {
                board = value;
                board.FaceButton = this;
            }
        }

        private CtrlGameBoard.GameBoard board;
    }
}
