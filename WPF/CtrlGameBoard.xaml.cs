using Minesweeper.Core;
using System.Collections;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace Minesweeper.WPF
{
    /// <summary>
    /// Game board UI
    /// </summary>
    public partial class CtrlGameBoard : FrameworkElement
    {
        public CtrlGameBoard()
        {
            InitializeComponent();

            Board = new GameBoard(this);
            MinesImage = Application.Current.FindResource("MinesImage") as ResourceDictionary;

            MouseDown += GameBoard_MouseDown;
            MouseUp += GameBoard_MouseUp;
            MouseLeave += CtrlGameBoard_MouseLeave;
            MouseMove += CtrlGameBoard_MouseMove;
        }

        private void CtrlGameBoard_MouseMove(object sender, MouseEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                Point pt = e.GetPosition(sender as UIElement);
                HitTestResult result = VisualTreeHelper.HitTest(this, pt);
                if (result != null)
                {
                    Board.BoxChange(result.VisualHit as Box);
                }
            }
        }

        private void GameBoard_MouseDown(object sender, MouseButtonEventArgs e)
        {
            Point pt = e.GetPosition(sender as UIElement);
            HitTestResult result = VisualTreeHelper.HitTest(this, pt);
            if (result != null)
            {
                if (e.ChangedButton == MouseButton.Right)
                {
                    Board.UpdateFlag(result.VisualHit as Box);
                }
                else if (e.ChangedButton == MouseButton.Left)
                {
                    Board.BoxPress(result.VisualHit as Box);
                }
            }
        }

        private void GameBoard_MouseUp(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                Board.BoxUpdate(true);
            }
        }

        private void CtrlGameBoard_MouseLeave(object sender, MouseEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                Board.BoxUpdate(false);
            }
        }

        protected override Visual GetVisualChild(int index) => Board.BoxContainer[index] as Box;
        protected override int VisualChildrenCount => Board.BoxContainer.Count;

        private ResourceDictionary MinesImage { get; set; }

        public GameBoard Board { get; }

        /// <summary>
        /// Main game board
        /// </summary>
        public class GameBoard : Board
        {
            internal GameBoard(CtrlGameBoard parent)
            {
                Parent = parent;
                BoxContainer = new BoxCollection(this);
                MarkQuestion = true;
            }

            protected override IBox CreateBox()
            {
                return new Box(this);
            }

            internal CtrlGameBoard Parent { get; }
            internal BoxCollection BoxContainer { get; }

            public override IList<IBox> Boxes => BoxContainer;
        }

        /// <summary>
        /// Since VisualCollection class is sealed, we can't subclass this.
        /// So we must be manually convert VisualCollection to custom list.
        /// </summary>
        internal class BoxCollection : IList<IBox>
        {
            internal BoxCollection(GameBoard board) 
            { 
                Collection = new VisualCollection(board.Parent); 
            }

            public IBox this[int index] 
            { 
                get => Collection[index] as Box;
                set { Collection[index] = value as Box; } 
            }

            public int Count => Collection.Count;
            public bool IsReadOnly => Collection.IsReadOnly;
            public void Add(IBox item) => Collection.Add(item as Box); 
            public void Clear() => Collection.Clear();
            public bool Contains(IBox item) => Collection.Contains(item as Box);
            public void CopyTo(IBox[] array, int index) => Collection.CopyTo(array, index);
            public int IndexOf(IBox item) => Collection.IndexOf(item as Box);
            public void Insert(int index, IBox item) => Collection.Insert(index, item as Box);
            public bool Remove(IBox item) 
            {
                if (!Collection.Contains(item as Box))
                {
                    return false;
                }
                Collection.Remove(item as Box);
                return true;
            }

            public void RemoveAt(int index) => Collection.RemoveAt(index);
            public IEnumerator<IBox> GetEnumerator() => new BoxCollectionEnumerator(Collection);
            IEnumerator IEnumerable.GetEnumerator() => Collection.GetEnumerator();

            private VisualCollection Collection { get; }
        }

        /// <summary>
        /// Enumerator for the BoxCollection class
        /// </summary>
        private class BoxCollectionEnumerator : IEnumerator<IBox>
        {
            internal BoxCollectionEnumerator(VisualCollection vc) 
            { 
                e = vc.GetEnumerator();
            }

            public IBox Current => e.Current as IBox;
            object IEnumerator.Current => Current;
            public void Dispose() {}
            public bool MoveNext() => e.MoveNext();
            public void Reset() => e.Reset();

            private VisualCollection.Enumerator e;
        }

        /// <summary>
        /// Class for rendering all boxes in the game board
        /// </summary>
        private class Box : DrawingVisual, IBox
        {
            internal Box(GameBoard board)
            {
                Board = board;
            }

            private GameBoard Board { get; }
            public int MinesAround { get; set; }
            public bool IsMine { get; set; }
            public FlagType Flag { get; set; }
            public int Row { get; set; }
            public int Column { get; set; }

            public void Draw(MineImage img)
            {
                var image = Board.Parent.MinesImage[img.ToString()] as BitmapSource;
                DrawingContext ctx = RenderOpen();
                ctx.DrawImage(image, new Rect(Column * image.Width, Row * image.Height, image.Width, image.Height));
                ctx.Close();
            }
        }
    }
}
