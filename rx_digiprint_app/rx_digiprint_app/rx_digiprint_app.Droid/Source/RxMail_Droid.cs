using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Android.App;
using Android.Content;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using System.IO;
using RX_DigiPrint.Services;

namespace DigiPrint.Models
{
    public class RxMailserver_Driod : IRxMailServer
    {
        public Action<Intent> StartActivity;

        //--- Send -------------------------------------------
        public void Send(RxMail mail)
        {                        
            var email = new Intent(Intent.ActionSend);
            email.PutExtra (Intent.ExtraEmail, new string[]{mail.To} );
            email.PutExtra (Intent.ExtraSubject, mail.Subject);

            email.PutExtra (Intent.ExtraText, mail.Text);
            {
                string dir = Android.OS.Environment.GetExternalStoragePublicDirectory(Android.OS.Environment.DirectoryDocuments).Path;
                System.IO.Directory.CreateDirectory(dir);

                string filepath = Path.Combine(Android.OS.Environment.GetExternalStoragePublicDirectory(Android.OS.Environment.DirectoryDocuments).Path, "requestinfo.txt");

                System.IO.File.WriteAllText(filepath, mail.AttachmentContent);
                Java.IO.File file = new Java.IO.File(filepath);
                file.SetReadable (true, false);
                email.PutExtra(Intent.ExtraStream, Android.Net.Uri.FromFile(file));
                email.AddFlags(ActivityFlags.GrantReadUriPermission);
            }
  
            email.SetType ("message/rfc822");
            StartActivity(email);
        }

    }
}