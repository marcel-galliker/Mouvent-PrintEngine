using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;

namespace RX_DigiPrint.Views.SupervisorsView
{
    /// <summary>
    /// Class managin the user login View
    /// </summary>
    public partial class UserLogin : CustomWindow
    {
        private MvtUserLevelManager ulm;

        /// <summary>
        /// Constructor for UserLogin
        /// </summary>
        /// <param name="ulm">Level manager that will contain the level after verification of the pin</param>
        public UserLogin(MvtUserLevelManager ulm)
        {
            InitializeComponent();
            this.ulm = ulm;
            this.ulm.NewMessage += LogUserConnection;
            this.KeyDown += UserLogin_KeyDown;
        }

        private void UserLogin_KeyDown(object sender, KeyEventArgs e)
        {
            String nextDigit = null;
            if (e.Key == Key.Back)
            {
                SuppressChar_Click(sender, null);
                return;
            }
            else if (e.Key >= Key.D0 && e.Key <= Key.D9)
            {
                nextDigit = (e.Key - Key.D0).ToString();
                
            }
            else if (e.Key >= Key.NumPad0 && e.Key <= Key.NumPad9)
            {
                nextDigit = (e.Key - Key.NumPad0).ToString();
            }

            if(nextDigit != null && !CompletePinEntered())
            {
                    NextDigit().Text = nextDigit;               
            }
        }

        private void LogIn_Click(object sender, RoutedEventArgs e)
        {
            /* Verify if a complete PIN was entered */
            if (!CompletePinEntered())
            {
                AuthenticationStatus.Content = "Enter a complete PIN";
                AuthenticationStatus.Visibility = Visibility.Visible;
                BlinkErrorLabel();
                return;
            }

            /* Get the entered PIN and verify it */
            String enteredCode = UserPin0.Text + UserPin1.Text + UserPin2.Text + UserPin3.Text + UserPin4.Text + UserPin5.Text;
            if (!ulm.Verify(enteredCode))
            {
                AuthenticationStatus.Content = "Authentication failed";
                AuthenticationStatus.Visibility = Visibility.Visible;
                BlinkErrorLabel();
                return;
            }
            this.DialogResult = true;
            this.Close();
        }

        private bool CompletePinEntered()
        {
            foreach (UIElement u in TopPanel.Children)
            {
                if (u is TextBox box && String.IsNullOrWhiteSpace(box.Text)) return false;
            }
            return true;
        }

        private TextBox NextDigit()
        {
            foreach (UIElement u in TopPanel.Children)
            {
                if (u is TextBox box && String.IsNullOrWhiteSpace(box.Text)) return u as TextBox;
            }
            return null;
        }

        private void BlinkErrorLabel()
        {
            DoubleAnimation anim = new DoubleAnimation();
            anim.AutoReverse = true;
            anim.From = 1;
            anim.To = 0;
            anim.Duration = TimeSpan.FromSeconds(0.3);
            Storyboard sb = new Storyboard();
            sb.Children.Add(anim);
            Storyboard.SetTargetProperty(anim, new PropertyPath("(TextBox.Opacity)"));
            Storyboard.SetTarget(anim, AuthenticationStatus);
            sb.Begin();
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }

        private void DeletePin_Click(object sender, RoutedEventArgs e)
        {
            foreach (UIElement u in TopPanel.Children)
            {
                if (u is TextBox box) box.Text = "";
            }
        }

        private void SuppressChar_Click(object sender, RoutedEventArgs e)
        {
            for(var i = TopPanel.Children.Count - 1; i >= 0; --i)
            {
                if (TopPanel.Children[i] is TextBox box && !String.IsNullOrWhiteSpace(box.Text))
                {
                    ((TextBox)TopPanel.Children[i]).Text = "";
                    return;
                }
            }
        }

        private void Number_Click(object sender, RoutedEventArgs e)
        {
            if (!CompletePinEntered())
            {
                NextDigit().Text = (String)((sender as Button).Content);

            }
        }

        public static void LogUserConnection(String message)
        {
            RxGlobals.Events.AddItem(new LogItem(message));
        }
    }
}
