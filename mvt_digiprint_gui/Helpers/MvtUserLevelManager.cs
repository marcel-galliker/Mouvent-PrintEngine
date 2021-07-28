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
        Maintenance = 2,
        Engineer = 4
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
        /// Adds a User to the database
        /// </summary>
        /// <param name="name">Name of new User</param>
        /// <param name="validUntil">Validity of new User</param>
        /// <returns>True if User was added, false otherwise</returns>
        public bool AddUser(String name, DateTime validUntil)
        {
            if (String.IsNullOrWhiteSpace(name)) return false;

            List<User> users = this.RetrieveUsers();

            /* Random GUID is created and secret key is made to look like this : MvtAuth-<user-name>-<GUID> */
            Guid guid = Guid.NewGuid();
            String random_guid = Convert.ToString(guid).Replace("-", "").Substring(0, 5);

            /* User is added */
            User newSup = new User(name, random_guid, validUntil, machineName);
            users.Add(newSup);
            this.WriteUsers(users);

            OnNewMessage(name + " was added as maintenance user");
            return true;
        }

        /// <summary>
        /// Removes a User
        /// </summary>
        /// <param name="name">Name of the User to remove</param>
        /// <returns>True if User was removed, false otherwise</returns>
        public bool RemoveUser(String name)
        {
            if (String.IsNullOrWhiteSpace(name)) return false;

            /* Get Users list */
            List<User> users = this.RetrieveUsers();

            /* Find User and delete him if he exists */
            foreach (User s in users)
            {
                if (s.Username == name)
                {
                    users.Remove(s);
                    OnNewMessage(name + " was removed from maintenance user list");
                    return this.WriteUsers(users);
                }
            }

            return false;
        }

        /// <summary>
        /// Gets MvtTOTP object containing given User's codes
        /// </summary>
        /// <param name="name">Name of the User</param>
        /// <returns>An MvtTOTP object with the codes of the User</returns>
        public MvtTOTP GetUserCode(String name)
        {
            if (String.IsNullOrWhiteSpace(name))
            {
                Console.WriteLine("Name problem in GetUserCode()");
                return new MvtTOTP("bad", false, " ");
            }

            List<User> users = this.RetrieveUsers();

            String secretKey = "temp";
            foreach (User s in users)
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
        /// Gets all Users's names
        /// </summary>
        /// <returns>A list containing all Users names</returns>
        public List<String> GetUserList()
        {
            List<String> users = new List<string>();
            List<User> sups = this.RetrieveUsers();

            foreach (User s in sups)
            {
                users.Add(s.Username);
            }

            return users;
        }

        /// <summary>
        /// Function to get an ObservableCollection of Users
        /// </summary>
        /// <returns>An ObservableCollection<User> Object with all Users</returns>
        public ObservableCollection<User> GetAllUsers()
        {
            return new ObservableCollection<User>(this.RetrieveUsers());
        }

        /// <summary>
        /// Gives the validity of a User
        /// </summary>
        /// <param name="name">Name of the User</param>
        /// <returns>The date until which a User is valid</returns>
        public DateTime GetUserValidity(String name)
        {
            List<User> users = this.RetrieveUsers();

            if (users.Exists(x => x.Username == name))
            {
                User s = users.Find(x => x.Username == name);
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
            List<User> users = this.RetrieveUsers();

            /* For all secrets, create an object to check if it's valid */
            foreach (User s in users)
            {
                MvtTOTP temp = new MvtTOTP(s.GetSecretString(), false, machineName);
                if (temp.Verify(pin))
                {
                    /* Check for date validity */
                    if (s.Validity < DateTime.Now)
                    {
                        return false;
                    }

                    level = User_Level.Maintenance;
                    OnNewMessage(s.Username + " has logged in");
                    return true;
                }
            }

            /* Check if it's a Mouvent access */
            MvtTOTP testMvt = new MvtTOTP(CreateMouventSecret(), true, machineName);
            if (testMvt.Verify(pin))
            {
                level = User_Level.Engineer;
                OnNewMessage("Engineer has logged in");
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
        /// Changes the Validity date of a User
        /// </summary>
        /// <param name="name">Name of the User</param>
        /// <param name="d">Date to set for new validity</param>
        /// <returns>true if validity was changed, false otherwise</returns>
        public bool ChangeUserValidity(String name, DateTime d)
        {
            List<User> users = this.RetrieveUsers();

            User s = users.Find(x => x.Username == name);

            if (s == null)
            {
                return false;
            }

            s.Validity = d;

            return this.WriteUsers(users);
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
        /// Helper function to get all Users from xml file
        /// </summary>
        /// <returns>A list of all Users</returns>
        private List<User> RetrieveUsers()
        {
            List<User> sups = new List<User>();

            XmlSerializer serializer = new XmlSerializer(typeof(List<User>));
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
                    sups = (List<User>)serializer.Deserialize(fs);
                }
                catch (Exception e) /* We get here if XML file is empty */
                {
                    Console.WriteLine(e.Message);
                }
            }

            return sups;
        }

        /// <summary>
        /// Helper function to write all Users to an xml file
        /// </summary>
        /// <param name="users">List of Users to write</param>
        /// <returns>True if written without problem, false otherwise</returns>
        private bool WriteUsers(List<User> users)
        {
            using (Stream fs = new FileStream(dbPath, FileMode.Create, FileAccess.Write, FileShare.None))
            {
                XmlSerializer serializer = new XmlSerializer(typeof(List<User>));
                try
                {
                    serializer.Serialize(fs, users);
                }
                catch (Exception e)
                {
                    MessageBox.Show("Problem writing maintenance users");
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
    /// Class to represent a User
    /// </summary>
    [Serializable()]
    public class User
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

        public User() { }
        public User(String username, String secretKey, DateTime validity, String machineName)
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

