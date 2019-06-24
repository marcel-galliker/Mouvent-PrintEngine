// ****************************************************************************
//
//	rx_license_ctrl
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

// ****************************************************************************
//  
//  This program is installed on the Mouvent Server 
//  Server:     //194.209.162.30/
//  Path:       C:/Program Files (x86)/Mouvent
//  Task-Scheduler:
//  Folder:     Mouvent
//  Task:       rx_license_ctrl
//  Trigger:    Every Month the 26th
//
// ****************************************************************************






using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Mail;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using Microsoft.Exchange.WebServices.Data;
using RX_DigiPrint.Services;

namespace rx_license_ctrl
{
    class rx_license_ctrl
    {
        static void Main(string[] args)
        {
            bool all = args.Count()>0 && args[0].ToLower().Equals("-all") ;
            
            if (all) Console.WriteLine("send ALL licenses");
            else     Console.WriteLine("send Unread licenses");

            LicenseServer server = new LicenseServer();
            server.login();

            server.send_licenses(all);
            Console.WriteLine("done");
        }
    }

    //--- LicenseServer --------------------------------
    public class LicenseServer
    {
        ExchangeService _Service= new ExchangeService(ExchangeVersion.Exchange2013);

        readonly string   _UserName = "license@mouvent.com";
        readonly string   _Password = "Dun74739";

        readonly int  _Level_Supervisor =2;
        readonly int  _Level_Service    =3;
        readonly int  _Level_Mouvent    =4;

        public LicenseServer()
        {
        }

        //--- login --------------------------------
        public void login()
        {
            Console.WriteLine("login {0}", _UserName);
            _Service.Credentials = new WebCredentials(_UserName, _Password);

         //   _Service.TraceEnabled = true;
         //   _Service.TraceFlags = TraceFlags.All;

            _Service.AutodiscoverUrl(_UserName, RedirectionUrlValidationCallback);
        }

        //--- RedirectionUrlValidationCallback --------------------------------------
        private static bool RedirectionUrlValidationCallback(string redirectionUrl)
        {
          // The default for the validation callback is to reject the URL.
          bool result = false;

          Uri redirectionUri = new Uri(redirectionUrl);

          // Validate the contents of the redirection URL. In this simple validation
          // callback, the redirection URL is considered valid if it is using HTTPS
          // to encrypt the authentication credentials. 
          if (redirectionUri.Scheme == "https")
          {
            result = true;
          }
          return result;
        }

        //--- send_licenses --------------------
        public void send_licenses(bool all)
        {
            Console.WriteLine("send_licenses");            
            
            StringBuilder protocol = new StringBuilder(10000);

            _send_licenses("Supervisor", all, _Level_Supervisor,   protocol);
            _send_licenses("Service",    all, _Level_Service,      protocol);
            _send_licenses("Mouvent",    all, _Level_Mouvent,      protocol);

            EmailMessage email = new EmailMessage(_Service);

            email.ToRecipients.Add(_UserName);
            email.Subject = "Licenses Sent";
            email.Body    = new MessageBody(BodyType.Text, protocol.ToString());

            email.Send();
        }

        //--- create_license --------------------
        private string create_license(string request, int level)
        {
            string license;
            DateTime today  = DateTime.Today;
            DateTime next   = today.AddMonths(1);
            if (next.Day>25) next=next.AddMonths(1);
            DateTime expire = new DateTime(next.Year, next.Month, 1).AddDays(-1);
            license = request + "\n" 
                    + level.ToString() + "\n" 
                    + expire.Day.ToString()+'.'+expire.Month.ToString()+'.'+expire.Year.ToString();
            return license+RxBtDef.StrCrc(license);
        }

        //--- send_license ---------------------------------------
        private void send_license(EmailAddress to, string license)
        {
            try
            {
                string filepath = System.IO.Path.GetTempPath()+RxBtDef.LicFileName;
                System.IO.File.WriteAllText(filepath, license);

                EmailMessage email = new EmailMessage(_Service);

                email.ToRecipients.Add(to.Address);
                email.Attachments.AddFileAttachment(filepath);
                email.Subject = "Mouvent License";
                email.Body = new MessageBody("This is your new license. Save this file in your DOCUMENTS folder.");

                email.Send();
                System.IO.File.Delete(filepath);
            }

            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }

        //--- _send_licenses -------------------------------------
        private void _send_licenses(string subFolderName, bool all, int level, StringBuilder protocol)
        {
            int i, cnt=0;
            Console.WriteLine(string.Format("\n Licenses to {0} users", subFolderName));
            protocol.AppendLine(string.Format("\n Licenses to {0} users", subFolderName));
            ItemView   view = new ItemView(1000);

            Folder folder = FindFolder(WellKnownFolderName.MsgFolderRoot, "License");
            folder = FindFolder(folder.Id, subFolderName);
            if (folder == null)
            {
                protocol.AppendLine(string.Format("Folder not found"));
                return;
            }
            FindItemsResults<Item> results;
            if (all) results = _Service.FindItems(folder.Id, "Kind:email", view);
            else     results = _Service.FindItems(folder.Id, "Kind:email IsRead:false", view);
            for(i=0; i<results.Items.Count; i++)
            {
                EmailMessage email=results.Items[i] as EmailMessage;
                if (email!=null)
                {
                    email.Load(new PropertySet(EmailMessageSchema.Attachments));
                    if (email.Attachments.Count>0)
                    {
                        FileAttachment fileAttachment = email.Attachments[0] as FileAttachment;
                        if (fileAttachment!=null)
                        {
                            fileAttachment.Load();
                            string contentCode = System.Text.Encoding.Default.GetString(fileAttachment.Content);
                            string content = RxEncypt.Decrypt(contentCode, RxBtDef.InfoPwd);
                            string license = create_license(content, level);
                            string licCode = RxEncypt.Encrypt(license, RxBtDef.LicPwd);
                            email.Load(new PropertySet(EmailMessageSchema.From));
                            send_license(email.From, licCode);
                            Console.WriteLine("    "+email.From.Address);
                            protocol.AppendLine("    "+email.From.Address);
                        }
                    }
                    email.IsRead = true;
                    email.Update(ConflictResolutionMode.AlwaysOverwrite);
                    cnt++;
                }
            }
            Console.WriteLine("    total: "+cnt.ToString());
            protocol.AppendLine("    total: "+cnt.ToString());
        }

        //--- FindFolder ---------------------------------------
        Folder FindFolder(FolderId root, string name)
        {
            Folder folder = null;
            FolderView foldersview = new FolderView(1000);
            int i;

            try
            {
                SearchFilter searchFilter = new SearchFilter.ContainsSubstring(FolderSchema.DisplayName, name);

                FindFoldersResults folders = _Service.FindFolders(root, searchFilter, foldersview);
                for (i=0; i<folders.TotalCount; i++)
                {
                    folder = folders.Folders[i];
                    if (folder.DisplayName.Equals(name)) return folder;
                }
            }
            catch { };
            return null;
        }

    }
}
