using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace RX_LabelComposer.Common
{
    public class LabelArea : Canvas
    {
        private static LabelBox _MovingBox;
        public LabelBox MovingBox
        {
            get { return _MovingBox; }
            set { _MovingBox = value; }
        }

        private Point offset;
        private Point pos;

        protected override void OnMouseUp(MouseButtonEventArgs e)
        {
            if (_MovingBox!=null)
            {
                _MovingBox.SetScreenPos(pos.X - offset.X, pos.Y - offset.Y);
                _MovingBox = null;
            }
            base.OnMouseUp(e);
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            pos = e.GetPosition(this);

            if (_MovingBox != null)
            {
                _MovingBox.SetScreenPos(pos.X - offset.X, pos.Y - offset.Y);
                e.Handled = true;
            }
            else
            {
                //--- test selected boxes first ----
                for (int i = 0; i < Children.Count; i++)
                {
                    LabelBox box = Children[i] as LabelBox;
                    if (box != null && box.IsSelected && box.Hit(pos))
                    {
                        offset = e.GetPosition(box);
                        return;
                    }
                }
                for (int i = 0; i < Children.Count; i++)
                {
                    LabelBox box = Children[i] as LabelBox;
                    if (box != null && box.Hit(pos))
                    {
                        offset = e.GetPosition(box);
                        return;
                    }
                }
            }
        }
    }
}
