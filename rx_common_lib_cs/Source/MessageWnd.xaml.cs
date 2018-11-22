using System.Windows;

namespace RX_Common
{
    /// <summary>
    /// Interaction logic for Message.xaml
    /// </summary>
    public partial class MessageWnd : Window
    {
        public MessageWnd(string title, string message)
        {
            InitializeComponent();
            Title = title;
            Message.Text = message;
        }
    }
}
