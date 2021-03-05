using System.Windows;
using System.Windows.Data;

namespace RX_DigiPrint.Helpers
{
    /// <summary>
    /// A custom <see cref="Binding"/> object that provides convenient access to culture-specific resources at run time.
    /// </summary>
    public class LocalizedBinding : Binding
    {
        #region Constructors

        /// <summary>
        /// Creates a new instance of the <see cref="LocalizedBinding"/> class.
        /// </summary>
        public LocalizedBinding()
        {
            this.Mode = BindingMode.OneWay;
            this.Source = TranslationSource.Instance;
        }

        /// <summary>
        /// Creates a new instance of the <see cref="LocalizedBinding"/> class.
        /// </summary>
        /// <param name="name">The name of the resource to retrieve.</param>
        public LocalizedBinding(string name)
            : base("[" + name + "]")
        {
            this.Mode = BindingMode.OneWay;
            this.Source = TranslationSource.Instance;
        }

        #endregion

        /// <summary>
        /// Sets the name of the resource to retrieve.
        /// </summary>
        public string Key
        {
            set
            {
                Path = new PropertyPath("[" + value + "]");
            }
        }
    }
}
