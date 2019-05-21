using System;
using System.Deployment.Application;
using System.Windows.Forms;

namespace RX_Common
{
    public class RxUpdate
    {
        public static void DoUpdate()
        {
            UpdateCheckInfo info = null;

            if (ApplicationDeployment.IsNetworkDeployed)
            {
                ApplicationDeployment ad = ApplicationDeployment.CurrentDeployment;

                MessageWnd msg = new MessageWnd("Updates", "Checking for updates.");
                msg.Show();                

                try
                {
                    info = ad.CheckForDetailedUpdate();
                }
                catch (DeploymentDownloadException dde)
                {
                    msg.Close();
                    System.Windows.Forms.MessageBox.Show("The new version of the application cannot be downloaded at this time. \n\nPlease check your network connection, or try again later. Error: " + dde.Message);
                    return;
                }
                catch (InvalidDeploymentException ide)
                {
                    msg.Close();
                    System.Windows.Forms.MessageBox.Show("Cannot check for a new version of the application. The ClickOnce deployment is corrupt. Please redeploy the application and try again. Error: " + ide.Message);
                    return;
                }
                catch (InvalidOperationException ioe)
                {
                    msg.Close();
                    System.Windows.Forms.MessageBox.Show("This application cannot be updated. It is likely not a ClickOnce application. Error: " + ioe.Message);
                    return;
                }

                msg.Close();
                if (info.UpdateAvailable)
                {
                    Boolean doUpdate = true;

                    if (!info.IsUpdateRequired)
                    {
                        DialogResult dr = System.Windows.Forms.MessageBox.Show("An update is available. Would you like to update the application now?", "Update Available", MessageBoxButtons.OKCancel);
                        if (!(System.Windows.Forms.DialogResult.OK == dr))
                        {
                            doUpdate = false;
                        }
                    }
                    else
                    {
                        // Display a message that the app MUST reboot. Display the minimum required version.
                        System.Windows.Forms.MessageBox.Show("This application has detected a mandatory update from your current " + 
                            "version to version " + info.MinimumRequiredVersion.ToString() + 
                            ". The application will now install the update and restart.", 
                            "Update Available", System.Windows.Forms.MessageBoxButtons.OK, 
                            System.Windows.Forms.MessageBoxIcon.Information);
                    }

                    if (doUpdate)
                    {
                        try
                        {
                            ad.Update();
                            System.Windows.Forms.MessageBox.Show("The application has been upgraded, and will now restart.");
                            System.Windows.Forms.Application.Restart();
                        }
                        catch (DeploymentDownloadException dde)
                        {
                            System.Windows.Forms.MessageBox.Show("Cannot install the latest version of the application. \n\nPlease check your network connection, or try again later. Error: " + dde);
                            return;
                        }
                    }
                }
                else System.Windows.Forms.MessageBox.Show("The application is up to date.", "Up to date");
            }
            else System.Windows.Forms.MessageBox.Show("The application is not started as ClickOnce.");
        }

    }
}
