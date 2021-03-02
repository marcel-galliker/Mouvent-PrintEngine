using System;
using Google.Authenticator;

namespace RX_DigiPrint.Helpers
{
	/// <summary>
	/// Class used for TOTP authentification using Google Authenticator
	/// </summary>
	public class MvtTOTP
	{
		private readonly String secret;
		private readonly SetupCode setupInfo;
		private const String DEFAULT_NAME = "Unknown";
		/// <summary>
		/// Unique constructor for MvtTOTP objects
		/// </summary>
		/// <param name="secret"> Secret key that will be used for setup code generation and validity check </param>
		/// <param name="mouvent"> Decides if Mouvent or not (supervisor otherwise) </param>
		/// <param name="machineName"> Name of the machine, must be given for non mouvent codes </param>
		/// <exception cref="ArgumentException"> Thrown when given secret or machine name is null, empty or whitespaces </exception>
		public MvtTOTP(String secret, bool mouvent, String machineName)
		{
			if(String.IsNullOrWhiteSpace(secret))
            {
				throw new ArgumentException("valid secret must be entered");
            }
			if(String.IsNullOrWhiteSpace(machineName))
            {
				machineName = DEFAULT_NAME;
            }
			/* Secret key for this object is saved */
			this.secret = secret;

			/* Generating data to create setupInfo */
			TwoFactorAuthenticator tfa = new TwoFactorAuthenticator();

			/* Generating strings displayed on google authenticator */
			String mouventLevel = "Mouvent ";
			mouventLevel += mouvent ? "Service" : "Supervisor";
			String[] secretKeyParsed = secret.Split('-');
			String machineOrVersion = mouvent ? secretKeyParsed[1] : machineName;

			setupInfo = tfa.GenerateSetupCode(mouventLevel, machineOrVersion, secret, false);
		}

		/// <summary>
		/// Function to get the QR code
		/// </summary>
		/// <returns>The QR code base64 image as a String</returns>
		public String GetQRCode()
		{
			return setupInfo.QrCodeSetupImageUrl;
		}

		/// <summary>
		/// Function to get the manual code
		/// </summary>
		/// <returns>The manual code to enter as a String</returns>
		public String GetManualCode()
		{
			return setupInfo.ManualEntryKey;
		}

		/// <summary>
		/// Function verifying if a pin is correct
		/// </summary>
		/// <param name="pin"> The pin to be checked with the secret key </param>
		/// <returns>true if the pin is ok, false otherwise</returns>
		public bool Verify(String pin)
		{
			TwoFactorAuthenticator tfa = new TwoFactorAuthenticator();
			return tfa.ValidateTwoFactorPIN(secret, pin);
		}

		/// <summary>
		/// Helper function to get a secret's pin
		/// </summary>
		/// <returns> The pin associated to the object's secret </returns>
		public String GetPin()
		{
			TwoFactorAuthenticator tfa = new TwoFactorAuthenticator();
			return tfa.GetCurrentPIN(secret);
		}
	}
}
