using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace Minesweeper.WPF
{
    /// <summary>
    /// Base class for all Led Display UI
    /// </summary>
    public partial class BaseCounter : FrameworkElement
    {
        public BaseCounter()
        {
            InitializeComponent();

            Leds = new VisualCollection(this);
            Leds.Add(new LedDisplay(this));
            Leds.Add(new LedDisplay(this));
            Leds.Add(new LedDisplay(this));

            LedsImage = Application.Current.FindResource("LedsImage") as CroppedBitmap[];
        }

        private VisualCollection Leds { get; }
        private CroppedBitmap[] LedsImage { get; }

        protected override int VisualChildrenCount => Leds.Count;
        protected override Visual GetVisualChild(int index) => Leds[index];

        public void DrawLeds(int number)
        {
            int count = number;
            LedDisplay image;

            if (count < 1000)
            {
                int value;
                
                // Draw left image 
                if (count >= 0)
                {
                    // 0 to 9
                    image = Leds[0] as LedDisplay;
                    value = count / 100;
                    image.Value = value;
                    count -= value * 100;
                }
                else
                {
                    // Negative sign
                    image = Leds[0] as LedDisplay;
                    image.Value = 10;
                    count = -count;
                }

                // Draw middle image
                value = count / 10;
                image = Leds[1] as LedDisplay;
                image.Value = value;

                // Draw right image
                count -= value * 10;
                image = Leds[2] as LedDisplay;
                image.Value = count;
            }
            else
            {
                // Draw negative values to all display number
                foreach (var led in Leds)
                {
                    image = led as LedDisplay;
                    image.Value = 10;
                }
            }

            // Then force redraw
            for (int i = 0; i < Leds.Count; i++)
            {
                image = Leds[i] as LedDisplay;
                image.Draw(i);
            }
        }

        /// <summary>
        /// Image for each number in the display.
        /// </summary>
        private class LedDisplay : DrawingVisual
        {
            public LedDisplay(BaseCounter parent)
            {
                Counter = parent;
                Value = 0;
            }

            public void Draw(int index)
            {
                BitmapSource img = Counter.LedsImage[Value];
                DrawingContext ctx = RenderOpen();
                ctx.DrawImage(img, new Rect(index * img.PixelWidth, 0, img.Width, img.Height));
                ctx.Close();
            }

            private BaseCounter Counter { get; }
            public int Value { set; private get; }
        }
    }
}
