using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Xml.Serialization;
using System.IO;
using System.Windows;
using System.Globalization;

namespace RX_DigiPrint.Helpers
{
    /// <summary>
    /// Enum mocking the existing user types values
    /// </summary>
    public enum User_Level
    {
        Operator = 1,
        Supervisor = 2,
        Mouvent = 4
    }

    public delegate void LogMessage(String message);

    /// <summary>
    /// Class used to manage user levels
    /// </summary>
    public class MvtUserLevelManager
    {
        /* Constants */
        /// <summary>
        /// Base to the secret key that will be used for Authenticator code generation
        /// </summary>
        private const String SECRET_KEY_BASE = "MvtAuth-";
        /// <summary>
        /// GUID used for Mouvent secret keys
        /// </summary>
        private const String MVT_GUID = "f2f25d1e-d765-4e25-a777-552742b4e80d";
        /// <summary>
        /// Default path for database
        /// </summary>
        private const String DB_DEFAULT_PATH = "..\\..\\Stored_Data/db.xml";

        private User_Level level;
        private readonly String dbPath;
        private readonly String version;
        private readonly String machineName;

        public event LogMessage NewMessage; // event

        /// <summary>
        /// Constructor taking a version number for the database
        /// </summary>
        /// <param name="dbPath">Path to database file</param>
        public MvtUserLevelManager(String version, String machineName)
        {

            this.level = User_Level.Operator;
            this.dbPath = Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, DB_DEFAULT_PATH));

            String[] versionParsed = version.Split('.');
            this.version = versionParsed[0] + "." + versionParsed[1];
            this.machineName = machineName;
        }

        /// <summary>
        /// Constructor taking a path for the database
        /// </summary>
        /// <param name="dbPath">Path to database file</param>
        public MvtUserLevelManager(String version, String machineName, String dbPath)
        {
            level = User_Level.Operator;
            /* Careful, no check as to whether a file is specified in the path or not */
            if (String.IsNullOrWhiteSpace(dbPath) || !Directory.Exists(Path.GetDirectoryName(dbPath)))
            {
                dbPath = Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, DB_DEFAULT_PATH));
                MessageBox.Show("Does the generated path directory exist ?\n" + Directory.Exists(dbPath));
                MessageBox.Show("Defaulting to default database path");
            }
            this.dbPath = dbPath;

            String[] versionParsed =version.Split('.');
            this.version = versionParsed[0] + "." + versionParsed[1];
            this.machineName = machineName;
        }

        /// <summary>
        /// Helper function used to create Mouvent codes
        /// </summary>
        /// <param name="secret">Mouvent secret key to be used</param>
        /// <returns>An MvtTOTP object containing the Mouvent user data</returns>
        public MvtTOTP GetMvtTOTP(String secret)
        {
            return new MvtTOTP(secret, true, null);
        }

        /// <summary>
        /// Creates a Mouvent secret String
        /// </summary>
        /// <returns>A Mouvent secret string</returns>
        public String CreateMouventSecret()
        {
            return CreateMouventSecret(this.version);
        }

        public static String CreateMouventSecret(String version)
        {
            /* First the week number is determined according to ISO8601, hence the additional operations */
            DateTime date = DateTime.Now;
            int day = (int)CultureInfo.CurrentCulture.Calendar.GetDayOfWeek(date);
            int week = CultureInfo.CurrentCulture.Calendar.GetWeekOfYear(date.AddDays(4 - (day == 0 ? 7 : day)), CalendarWeekRule.FirstFourDayWeek, DayOfWeek.Monday);

            /* Then the secret string is created to look like this : MvtAuth-<machine-version>-<week-number>-<GUID> */
            return SECRET_KEY_BASE + version + "-" + week.ToString() + "-" + MVT_GUID;
        }

        /// <summary>
        /// Adds a Supervisor to the database
        /// </summary>
        /// <param name="name">Name of new supervisor</param>
        /// <param name="validUntil">Validity of new supervisor</param>
        /// <returns>True if supervisor was added, false otherwise</returns>
        public bool AddSupervisor(String name, DateTime validUntil)
        {
            if (String.IsNullOrWhiteSpace(name)) return false;

            List<Supervisor> supervisors = this.RetrieveSupervisors();

            /* Random GUID is created and secret key is made to look like this : MvtAuth-<user-name>-<GUID> */
            Guid guid = Guid.NewGuid();
            String random_guid = Convert.ToString(guid).Replace("-", "").Substring(0, 5);

            /* Supervisor is added */
            Supervisor newSup = new Supervisor(name, random_guid, validUntil, machineName);
            supervisors.Add(newSup);
            this.WriteSupervisors(supervisors);

            OnNewMessage(name + " was added as supervisor");
            return true;
        }

        /// <summary>
        /// Removes a Supervisor
        /// </summary>
        /// <param name="name">Name of the Supervisor to remove</param>
        /// <returns>True if supervisor was removed, false otherwise</returns>
        public bool RemoveSupervisor(String name)
        {
            if (String.IsNullOrWhiteSpace(name)) return false;

            /* Get supervisors list */
            List<Supervisor> supervisors = this.RetrieveSupervisors();

            /* Find supervisor and delete him if he exists */
            foreach (Supervisor s in supervisors)
            {
                if (s.Username == name)
                {
                    supervisors.Remove(s);
                    OnNewMessage(name + " was removed from supervisor list");
                    return this.WriteSupervisors(supervisors);
                }
            }

            return false;
        }

        /// <summary>
        /// Gets MvtTOTP object containing given supervisor's codes
        /// </summary>
        /// <param name="name">Name of the supervisor</param>
        /// <returns>An MvtTOTP object with the codes of the supervisor</returns>
        public MvtTOTP GetSupervisorCode(String name)
        {
            if (String.IsNullOrWhiteSpace(name))
            {
                Console.WriteLine("Name problem in GetSupervisorCode()");
                return new MvtTOTP("bad", false, " ");
            }

            List<Supervisor> supervisors = this.RetrieveSupervisors();

            String secretKey = "temp";
            foreach (Supervisor s in supervisors)
            {
                if (s.Username == name)
                {
                    secretKey = s.GetSecretString();
                    break;
                }
            }

            return new MvtTOTP(secretKey, false, machineName);
        }

        /// <summary>
        /// Gets all supervisors's names
        /// </summary>
        /// <returns>A list containing all supervisors names</returns>
        public List<String> GetSupervisorList()
        {
            List<String> supervisors = new List<string>();
            List<Supervisor> sups = this.RetrieveSupervisors();

            foreach (Supervisor s in sups)
            {
                supervisors.Add(s.Username);
            }

            return supervisors;
        }

        /// <summary>
        /// Function to get an ObservableCollection of Supervisors
        /// </summary>
        /// <returns>An ObservableCollection<Supervisor> Object with all supervisors</returns>
        public ObservableCollection<Supervisor> GetAllSupervisors()
        {
            return new ObservableCollection<Supervisor>(this.RetrieveSupervisors());
        }

        /// <summary>
        /// Gives the validity of a supervisor
        /// </summary>
        /// <param name="name">Name of the supervisor</param>
        /// <returns>The date until which a supervisor is valid</returns>
        public DateTime GetSupervisorValidity(String name)
        {
            List<Supervisor> supervisors = this.RetrieveSupervisors();

            if (supervisors.Exists(x => x.Username == name))
            {
                Supervisor s = supervisors.Find(x => x.Username == name);
                return s.Validity;
            }

            return new DateTime();
        }

        /// <summary>
        /// Verifies if an entered pin has any match
        /// </summary>
        /// <param name="pin">Pin to be verified</param>
        /// <returns>True if there's a match, false otherwise</returns>
        public bool Verify(String pin)
        {
            List<Supervisor> supervisors = this.RetrieveSupervisors();

            /* For all secrets, create an object to check if it's valid */
            foreach (Supervisor s in supervisors)
            {
                MvtTOTP temp = new MvtTOTP(s.GetSecretString(), false, machineName);
                if (temp.Verify(pin))
                {
                    /* Check for date validity */
                    if (s.Validity < DateTime.Now)
                    {
                        return false;
                    }

                    level = User_Level.Supervisor;
                    OnNewMessage(s.Username + " has logged in");
                    return true;
                }
            }

            /* Check if it's a Mouvent access */
            MvtTOTP testMvt = new MvtTOTP(CreateMouventSecret(), true, machineName);
            if (testMvt.Verify(pin))
            {
                level = User_Level.Mouvent;
                OnNewMessage("Mouvent user has logged in");
                return true;
            }

            return false;
        }

        /// <summary>
        /// Gets the actual user level
        /// </summary>
        /// <returns>The actual user level</returns>
        public int GetLevel()
        {
            return (int) level;
        }

        /// <summary>
        /// Changes the Validity date of a Supervisor
        /// </summary>
        /// <param name="name">Name of the supervisor</param>
        /// <param name="d">Date to set for new validity</param>
        /// <returns>true if validity was changed, false otherwise</returns>
        public bool ChangeSupervisorValidity(String name, DateTime d)
        {
            List<Supervisor> supervisors = this.RetrieveSupervisors();

            Supervisor s = supervisors.Find(x => x.Username == name);

            if (s == null)
            {
                return false;
            }

            s.Validity = d;

            return this.WriteSupervisors(supervisors);
        }

        /// <summary>
        /// Helper function to check if actual level is sufficient
        /// </summary>
        /// <param name="dl">Level to check against</param>
        /// <returns>true if the actual level is equal or higher than demanded, false otherwise</returns>
        public bool CheckDemandedLevel(User_Level dl)
        {
            return level >= dl;
        }

        /// <summary>
        /// Helper function to get all supervisors from xml file
        /// </summary>
        /// <returns>A list of all supervisors</returns>
        private List<Supervisor> RetrieveSupervisors()
        {
            List<Supervisor> sups = new List<Supervisor>();

            XmlSerializer serializer = new XmlSerializer(typeof(List<Supervisor>));
            if (!Directory.Exists(Path.GetDirectoryName(dbPath)))
            {
                try
                {
                    Directory.CreateDirectory(Path.GetDirectoryName(dbPath));
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message);
                    MessageBox.Show("Database Path Problem, directory creation failed");
                    return sups;
                }
            }
            if (!File.Exists(dbPath))
            {
                try
                {
                    File.Create(dbPath).Dispose();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message);
                    MessageBox.Show("Database Path Problem, file creation failed");
                    return sups;
                }
            }
            using (FileStream fs = File.OpenRead(dbPath))
            {
                try
                {
                    sups = (List<Supervisor>)serializer.Deserialize(fs);
                }
                catch (Exception e) /* We get here if XML file is empty */
                {
                    Console.WriteLine(e.Message);
                }
            }

            return sups;
        }

        /// <summary>
        /// Helper function to write all supervisors to an xml file
        /// </summary>
        /// <param name="supervisors">List of supervisors to write</param>
        /// <returns>True if written without problem, false otherwise</returns>
        private bool WriteSupervisors(List<Supervisor> supervisors)
        {
            using (Stream fs = new FileStream(dbPath, FileMode.Create, FileAccess.Write, FileShare.None))
            {
                XmlSerializer serializer = new XmlSerializer(typeof(List<Supervisor>));
                try
                {
                    serializer.Serialize(fs, supervisors);
                }
                catch (Exception e)
                {
                    MessageBox.Show("Problem writing supervisors");
                    Console.WriteLine(e.Message);
                    return false;
                }
            }

            return true;
        }

        protected virtual void OnNewMessage(String message)
        {
            NewMessage?.Invoke(message);
        }
    }

    /// <summary>
    /// Class to represent a supervisor
    /// </summary>
    [Serializable()]
    public class Supervisor
    {
        /// <summary>
        /// Base to the secret key that will be used for Authenticator code generation
        /// </summary>
        private const String SECRET_KEY_BASE = "MvtAuth-";

        public String Username { get; set; }
        /* Public Secret Key, only gives the GUID of the key, used by the xml parser mainly */
        public String SecretKey
        {
            get
            {
                String[] parsedKey = secretKey.Split('-');
                if(parsedKey.Length < 3)
                {
                    return "";
                }
                return parsedKey[2];
            }

            set
            {
                secretKey = SECRET_KEY_BASE + Username + "-" + value;
            }
        }

        public DateTime Validity { get; set; }
        public String CodeQR { get; set; }
        public String CodeManual { get; set; }
        /* This contains the whole secret key */
        private String secretKey;

        public Supervisor() { }
        public Supervisor(String username, String secretKey, DateTime validity, String machineName)
        {
            Username = username;
            SecretKey = secretKey;
            Validity = validity;
            MvtTOTP codes = new MvtTOTP(secretKey, false, machineName);
            CodeQR = codes.GetQRCode();
            CodeManual = codes.GetManualCode();
        }

        public String GetSecretString()
        {
            return secretKey;
        }
    }
}

