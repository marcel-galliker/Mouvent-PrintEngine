using RX_LabelComposer.Models;
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace RX_LabelComposer.Views
{
    /// <summary>
    /// Interaction logic for BoxPropertiesView.xaml
    /// </summary>
    public partial class BoxPropertiesView : UserControl
    {
        public BoxPropertiesView()
        {
            InitializeComponent();
        }

        public LayoutDefinition _LayoutDef = LayoutDefinition.The_LayoutDef;
        public LayoutDefinition LayoutDef => _LayoutDef;

         //--- ContentBox_TextChanged ---------------------------------------------
        private void ContentBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            TextBox s = sender as TextBox;
            if (s != null && _LayoutDef != null && _LayoutDef.ActiveBox != null)
            {
                _LayoutDef.ActiveBox.Content = s.Text;
            }
            e.Handled = false;
        }

        //--- ContentBox_KeyDown ---------------------------------------------------
        private Key _Key;
        private int _SelLength;
        private void ContentBox_KeyDown(object sender, KeyEventArgs e)
        {
            _Key = e.Key;
            TextBox s = sender as TextBox;
            if (s != null) _SelLength = s.SelectionLength;
            int start = s.SelectionStart - 1;
            if (_Key == Key.Back && start>0 && s.Text[start]=='}')
            {
                s.BeginChange();
                while (start >= 0 && s.Text[start] != '{')
                {
                    start--;
                    _SelLength++;
                }
                s.Text = s.Text.Substring(0, start) + s.Text.Substring(start + _SelLength+1);
                s.SelectionStart=start;
                s.SelectionLength=0;
                e.Handled = true;
                s.EndChange(); 
            }
        }

        //--- ContentBox_SelectionChanged ----------------------------------------------------------------
        private void ContentBox_SelectionChanged(object sender, RoutedEventArgs e)
        {
            TextBox s = sender as TextBox;
            if (s!=null)
            {
                //--- mark DataField selections ----------------
                int start = s.SelectionStart;
                int len = s.SelectionLength;
                String str = s.Text;

                if (_SelLength > 0 && s.SelectionLength == 0 && _Key == Key.Left)
                {
                    e.Handled = true;
                    _Key = Key.None; // stop recursion
                    s.BeginChange();
                    s.SelectionLength = 0;
                    if (start > 0) start--;
                    if (str[start]=='\n' || str[start]=='\r') start--;
                    s.SelectionStart = start;
                    s.EndChange();                    
                    return;
                }
                                
                for (int i = start; i >= 0 && i<str.Length; i--)
                {
                    if (str[i] == '}' && i!=start) break;
                    if (str[i] == '{')
                    {
                        len += (start - i);
                        start = i;
                        break;
                    }
                }
                if (len == 0 && start>0 && start<str.Length && str[start] == '{' ) len = 1;
                for (int i = start + len; i < str.Length; i++)
                {
                    if (str[i] == '{') break;
                    if (str[i] == '}')
                    {
                        len = i - start + 1;
                        break;
                    }
                }
                if (start != s.SelectionStart || len != s.SelectionLength)
                {
                    s.BeginChange(); 
                    s.SelectionLength = len;
                    s.SelectionStart = start;
                    s.EndChange();
                }
            }
        }

        //--- ContentBox_PreviewTextInput -----------------------------------------------
        private void ContentBox_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            //--- Filter '{' and '}'
            if (e.Text.Contains("{") || e.Text.Contains("}"))
                e.Handled = true;
        }
    }
}
